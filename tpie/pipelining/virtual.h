// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2012, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

///////////////////////////////////////////////////////////////////////////////
/// \file virtual.h  Virtual wrappers for pipe_segments
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_PIPELINING_VIRTUAL_H__
#define __TPIE_PIPELINING_VIRTUAL_H__

namespace tpie {

namespace pipelining {

namespace bits {

template <typename Input>
class virtsrc : public pipe_segment {
public:
	virtual const segment_token & get_token() = 0;
	virtual void push(Input v) = 0;
};

template <typename dest_t>
class virtsrc_impl : public virtsrc<typename dest_t::item_type> {
public:
	typedef typename dest_t::item_type item_type;

	dest_t dest;
	virtsrc_impl(const dest_t & dest)
		: dest(dest)
	{
		pipe_segment::add_push_destination(dest);
		this->set_name("Virtual source", PRIORITY_INSIGNIFICANT);
	}

	const segment_token & get_token() {
		return pipe_segment::get_token();
	}

	void push(item_type v) {
		dest.push(v);
	}
};

template <typename Output>
class virtrecv : public pipe_segment {
	virtrecv *& m_self;
	virtsrc<Output> * m_virtdest;

public:
	typedef Output item_type;

	virtrecv(virtrecv *& self)
		: m_self(self)
		, m_virtdest(0)
	{
		m_self = this;
		set_name("Virtual destination", PRIORITY_INSIGNIFICANT);
	}

	virtrecv(const virtrecv & other)
		: pipe_segment(other)
		, m_self(other.m_self)
		, m_virtdest(other.m_virtdest)
	{
		m_self = this;
	}

	void begin() {
		tpie::log_info() << this << " begin " << m_virtdest << std::endl;
		pipe_segment::begin();
		if (m_virtdest == 0) {
			throw false;
		}
	}

	void push(Output v) {
		m_virtdest->push(v);
	}

	void set_destination(virtsrc<Output> * dest) {
		tp_assert(m_virtdest == 0, "Destination set twice");

		m_virtdest = dest;
		add_push_destination(dest->get_token());
	}
};

class virt_node {
public:
	typedef boost::shared_ptr<virt_node> ptr;

private:
	std::auto_ptr<pipe_segment> m_pipeSegment;
	ptr m_left;
	ptr m_right;

public:
	// Take std::new-ownership of given pipe_segment
	static ptr take_own(pipe_segment * pipe) {
		virt_node * n = new virt_node();
		n->m_pipeSegment.reset(pipe);
		ptr res(n);
		return res;
	}

	static ptr combine(ptr left, ptr right) {
		virt_node * n = new virt_node();
		n->m_left = left;
		n->m_right = right;
		ptr res(n);
		return res;
	}
};

template <typename T, typename U, typename Result>
struct assert_types_equal_and_return {
	static Result go(...) {
		throw virtual_chunk_missing_middle();
	}
};

template <typename T, typename Result>
struct assert_types_equal_and_return<T, T, Result> {
	static Result go(Result r) {
		return r;
	}
};

class virtual_chunk_base : public pipeline_base {
	// pipeline_base has virtual dtor and shared_ptr to m_segmap
	template <typename Any>
	void operator|(Any);
protected:
	virt_node::ptr m_node;
public:
	virtual_chunk_base() {}

	virt_node::ptr get_node() const { return m_node; }
	virtual_chunk_base(segment_map::ptr segmap, virt_node::ptr ptr)
		: m_node(ptr)
	{
		this->m_segmap = segmap;
	}

	virtual_chunk_base(segment_map::ptr segmap) {
		this->m_segmap = segmap;
	}

	operator bool() { return m_node; }
};

} // namespace bits

template <typename Input, typename Output>
class virtual_chunk;

template <typename Input>
class virtual_chunk_end : public bits::virtual_chunk_base {
	typedef bits::virtsrc<Input> src_type;
	src_type * m_src;

public:
	src_type * get_source() const { return m_src; }

	virtual_chunk_end()
		: m_src(0)
	{}

	template <typename fact_t>
	virtual_chunk_end(const pipe_end<fact_t> & pipe) {
		*this = pipe;
	}

	template <typename Mid>
	virtual_chunk_end(const virtual_chunk<Input, Mid> & left,
					  const virtual_chunk_end<Mid> & right);

	template <typename fact_t>
	virtual_chunk_end & operator=(const pipe_end<fact_t> & pipe) {
		if (this->m_node) {
			log_error() << "Virtual chunk assigned twice" << std::endl;
			throw std::runtime_error("Virtual chunk assigned twice");
		}

		typedef typename fact_t::generated_type generated_type;
		m_src = new bits::virtsrc_impl<generated_type>(pipe.factory.construct());
		this->m_node = bits::virt_node::take_own(m_src);
		this->m_segmap = m_src->get_segment_map();

		return *this;
	}
};

template <typename Input, typename Output>
class virtual_chunk : public bits::virtual_chunk_base {
	typedef bits::virtsrc<Input> src_type;
	typedef bits::virtrecv<Output> recv_type;
	src_type * m_src;
	recv_type * m_recv;
public:
	src_type * get_source() const { return m_src; }
	recv_type * get_destination() const { return m_recv; }

	virtual_chunk()
		: m_src(0)
		, m_recv(0)
	{}

	template <typename fact_t>
	virtual_chunk(const pipe_middle<fact_t> & pipe) {
		*this = pipe;
	}

	template <typename Mid>
	virtual_chunk(const virtual_chunk<Input, Mid> & left,
				  const virtual_chunk<Mid, Output> & right)
		: virtual_chunk_base(bits::virt_node::combine(left.get_node(), right.get_node()))
	{
		m_src = left.get_source();
		m_recv = right.get_destination();
	}

	template <typename fact_t>
	virtual_chunk & operator=(const pipe_middle<fact_t> & pipe) {
		if (this->m_node) {
			log_error() << "Virtual chunk assigned twice" << std::endl;
			throw std::runtime_error("Virtual chunk assigned twice");
		}
		typedef typename fact_t::template generated<recv_type>::type generated_type;
		recv_type temp(m_recv);
		m_src = new bits::virtsrc_impl<generated_type>(pipe.factory.construct(temp));
		this->m_node = bits::virt_node::take_own(m_src);
		this->m_segmap = temp.get_segment_map();

		return *this;
	}

	template <typename NextOutput>
	virtual_chunk<Input, NextOutput> operator|(virtual_chunk<Output, NextOutput> dest) {
		if (!*this) {
			return *bits::assert_types_equal_and_return<Input, Output, virtual_chunk<Input, NextOutput> *>
				::go(&dest);
		}
		m_recv->set_destination(dest.get_source());
		return virtual_chunk<Input, NextOutput>(*this, dest);
	}

	virtual_chunk_end<Input> operator|(virtual_chunk_end<Output> dest) {
		if (!*this) {
			return *bits::assert_types_equal_and_return<Input, Output, virtual_chunk_end<Input> *>
				::go(&dest);
		}
		m_recv->set_destination(dest.get_source());
		return virtual_chunk_end<Input>(*this, dest);
	}
};

template <typename Input>
template <typename Mid>
virtual_chunk_end<Input>::virtual_chunk_end(const virtual_chunk<Input, Mid> & left,
											const virtual_chunk_end<Mid> & right)
	: virtual_chunk_base(left.get_segment_map(),
						 bits::virt_node::combine(left.get_node(), right.get_node()))
{
	m_src = left.get_source();
}

template <typename Output>
class virtual_chunk_begin : public bits::virtual_chunk_base {
	typedef bits::virtrecv<Output> recv_type;
	recv_type * m_recv;
public:
	recv_type * get_destination() const { return m_recv; }

	virtual_chunk_begin()
		: m_recv(0)
	{}

	template <typename fact_t>
	virtual_chunk_begin(const pipe_begin<fact_t> & pipe) {
		*this = pipe;
	}

	template <typename Mid>
	virtual_chunk_begin(const virtual_chunk_begin<Mid> & left,
						const virtual_chunk<Mid, Output> & right)
		: virtual_chunk_base(left.get_segment_map(),
							 bits::virt_node::combine(left.get_node(), right.get_node()))
	{
		m_recv = right.get_destination();
	}

	template <typename fact_t>
	virtual_chunk_begin & operator=(const pipe_begin<fact_t> & pipe) {
		if (this->m_node) {
			log_error() << "Virtual chunk assigned twice" << std::endl;
			throw std::runtime_error("Virtual chunk assigned twice");
		}
		typedef typename fact_t::template generated<recv_type>::type generated_type;
		recv_type temp(m_recv);
		this->m_node = bits::virt_node::take_own(new generated_type(pipe.factory.construct(temp)));
		this->m_segmap = m_recv->get_segment_map();
		return *this;
	}

	template <typename NextOutput>
	virtual_chunk_begin<NextOutput> operator|(virtual_chunk<Output, NextOutput> dest) {
		if (!*this) throw virtual_chunk_missing_begin();
		if (!dest) {
			return *bits::assert_types_equal_and_return<Output, NextOutput, virtual_chunk_begin<NextOutput> *>
				::go(this);
		}
		m_recv->set_destination(dest.get_source());
		return virtual_chunk_begin<NextOutput>(*this, dest);
	}

	virtual_chunk_base operator|(virtual_chunk_end<Output> dest) {
		if (!*this) throw virtual_chunk_missing_begin();
		if (!dest) throw virtual_chunk_missing_end();
		m_recv->set_destination(dest.get_source());
		return virtual_chunk_base(this->m_segmap,
								  bits::virt_node::combine(get_node(), dest.get_node()));
	}
};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_VIRTUAL_H__
