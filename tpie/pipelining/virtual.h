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
/// \file virtual.h  Virtual wrappers for nodes
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_PIPELINING_VIRTUAL_H__
#define __TPIE_PIPELINING_VIRTUAL_H__

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/pipeline.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/helpers.h>

namespace tpie::pipelining {

///////////////////////////////////////////////////////////////////////////////
/// \brief  Virtual base class for extra data to go with virtual chunks.
/// When given to a virtual_chunk constructor, it will be automatically
/// std::deleted the virtual chunk is garbage collected.
///////////////////////////////////////////////////////////////////////////////
class virtual_container {
public:
	virtual ~virtual_container() {}
};


// Predeclare
template <typename Input>
class virtual_chunk_end;

// Predeclare
template <typename Input, typename Output>
class virtual_chunk;

// Predeclare
template <typename Output>
class virtual_chunk_begin;

// Predeclare
template <typename Input>
class virtual_chunk_pull_end;

// Predeclare
template <typename Input, typename Output>
class virtual_chunk_pull;

// Predeclare
template <typename Output>
class virtual_chunk_pull_begin;
	
namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \brief The maybe_add_const_ref helper struct adds const & to a type unless
/// the type is already const, reference or pointer type.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
struct maybe_add_const_ref {
	typedef const T & type;
};

template <typename T>
struct maybe_add_const_ref<const T &> {
	typedef const T & type;
};

template <typename T>
struct maybe_add_const_ref<const T *> {
	typedef const T * type;
};

template <typename T>
struct maybe_add_const_ref<T &> {
	typedef T & type;
};

template <typename T>
struct maybe_add_const_ref<T *> {
	typedef T * type;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Virtual base node that is injected into the beginning of a
/// virtual chunk. For efficiency, the push method accepts a const reference
/// type unless the item type is already const/ref/pointer.
///////////////////////////////////////////////////////////////////////////////
template <typename Input>
class virtsrc : public node {
	typedef typename maybe_add_const_ref<Input>::type input_type;

public:
	virtual const node_token & get_token() = 0; //TODO this need not be virtual
	virtual void push(input_type v) = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Concrete implementation of virtsrc.
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t, typename T>
class virtsrc_impl : public virtsrc<T> {
public:
	typedef T item_type;

private:
	typedef typename maybe_add_const_ref<item_type>::type input_type;
	dest_t dest;

public:
	virtsrc_impl(dest_t dest)
		: dest(std::move(dest))
	{
		node::add_push_destination(this->dest);
		this->set_name("Virtual source", PRIORITY_INSIGNIFICANT);
		this->set_plot_options(node::PLOT_BUFFERED | node::PLOT_SIMPLIFIED_HIDE);
	}

	const node_token & get_token() final {
		return node::get_token();
	}

	void push(input_type v) final {
		dest.push(v);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Virtual node that is injected into the end of a virtual
/// chunk. May be dynamically connected to a virtsrc using the set_destination
/// method.
///////////////////////////////////////////////////////////////////////////////
template <typename Output>
class virtrecv : public node {
	virtrecv *& m_self;
	virtsrc<Output> * m_virtdest;

public:
	typedef Output item_type;

	virtrecv(virtrecv *& self)
		: m_self(self)
		, m_virtdest(nullptr)
	{
		m_self = this;
		this->set_name("Virtual destination", PRIORITY_INSIGNIFICANT);
		this->set_plot_options(node::PLOT_BUFFERED | node::PLOT_SIMPLIFIED_HIDE);
	}

	virtrecv(virtrecv && o)
		: node(std::move(o))
		, m_self(o.m_self)
		, m_virtdest(std::move(o.m_virtdest)) {
		m_self = this;
	}

	void begin() final {
		if (m_virtdest == nullptr) {
			throw tpie::exception("No virtual destination");
		}
	}

	void push(typename maybe_add_const_ref<Output>::type v) {
		m_virtdest->push(v);
	}

	void set_destination(virtsrc<Output> * dest) {
		if (m_virtdest != nullptr) {
			throw tpie::exception("Virtual destination set twice");
		}

		m_virtdest = dest;
		add_push_destination(dest->get_token());
	}

	void change_destination(virtsrc<Output> * dest) {
		m_virtdest = dest;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Virtual base node that is injected into the beginning of a
/// virtual chunk.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class virtpullsrc : public node {
	using item_type = T;
public:
	virtual const node_token & get_token() = 0;
	virtual T pull() = 0;
	virtual bool can_pull() = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Concrete implementation of virtsrc.
///////////////////////////////////////////////////////////////////////////////
template <typename src_t, typename T>
class virtpullsrc_impl : public virtpullsrc<T> {
private:
	src_t src;
public:
	virtpullsrc_impl(src_t src) : src(std::move(src)) {
		node::add_pull_source(this->src);
		this->set_name("Virtual pull source", PRIORITY_INSIGNIFICANT);
		this->set_plot_options(node::PLOT_BUFFERED | node::PLOT_SIMPLIFIED_HIDE);
	}

	const node_token & get_token() {
		return node::get_token();
	}

	T pull() final {return src.pull();}
	bool can_pull() final {return src.can_pull();}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Virtual node that is injected into the end of a virtual
/// chunk. May be dynamically connected to a virtsrc using the set_destination
/// method.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class virtpullrecv : public node {
	virtpullrecv *& m_self;
	virtpullsrc<T> * m_virtsrc;
public:
	typedef T item_type;

	virtpullrecv(virtpullrecv *& self)
		: m_self(self)
		, m_virtsrc(nullptr)
	{
		m_self = this;
		this->set_name("Virtual pull destitation", PRIORITY_INSIGNIFICANT);
		this->set_plot_options(node::PLOT_BUFFERED | node::PLOT_SIMPLIFIED_HIDE);
	}

	virtpullrecv(virtpullrecv && o)
		: node(std::move(o))
		, m_self(o.m_self)
		, m_virtsrc(std::move(o.m_virtsrc)) {
		m_self = this;
	}

	void begin() final {
		if (m_virtsrc == nullptr) {
			throw tpie::exception("No virtual pull source");
		}
	}

	T pull() {return m_virtsrc->pull();}
	bool can_pull() {return m_virtsrc->can_pull();}

	void set_source(virtpullsrc<T> * src) {
		if (m_virtsrc != nullptr) {
			throw tpie::exception("Virtual source pull set twice");
		}

		m_virtsrc = src;
		add_pull_source(src->get_token());
	}

	void change_source(virtpullsrc<T> * src) {
		m_virtsrc = src;
	}
};


///////////////////////////////////////////////////////////////////////////////
/// \brief Ownership of nodes. This class can only be instantiated
/// through static methods that return a virt_node::ptr, providing reference
/// counting so that nodes are only instantiated once each and are
/// destroyed when the pipeline object goes out of scope.
///
/// This class either owns a node or two virt_nodes. The first case is
/// used in the virtual_chunk constructors that accept a single pipe_base. The
/// second case is used in the virtual_chunk pipe operators.
///////////////////////////////////////////////////////////////////////////////
class virt_node {
public:
	typedef std::shared_ptr<virt_node> ptr;

private:
	std::unique_ptr<node> m_node;
	std::unique_ptr<virtual_container> m_container;
	ptr m_left;
	ptr m_right;

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Take std::new-ownership of given node.
	///////////////////////////////////////////////////////////////////////////
	static ptr take_own(node * pipe) {
		virt_node * n = new virt_node();
		n->m_node.reset(pipe);
		ptr res(n);
		return res;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Aggregate ownership of virt_nodes.
	///////////////////////////////////////////////////////////////////////////
	static ptr combine(ptr left, ptr right) {
		virt_node * n = new virt_node();
		n->m_left = left;
		n->m_right = right;
		ptr res(n);
		return res;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Set and/or reset the virtual_container assigned to this virtual
	/// node.
	///////////////////////////////////////////////////////////////////////////
	void set_container(virtual_container * ctr) {
		m_container.reset(ctr);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Helper class that throws an exception on behalf of virtual_chunks
/// that have not been assigned a pipe_middle. When the input and output are
/// different, a virtual_chunk_missing_middle is thrown.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename U, typename Result>
struct assert_types_equal_and_return {
	static Result go(...) {
		throw virtual_chunk_missing_middle();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Template partial specialization that just returns the parameter
/// given when the input and output types of a virtual chunk are the same
/// (implicit identity function).
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename Result>
struct assert_types_equal_and_return<T, T, Result> {
	static Result go(Result r) {
		return r;
	}
};


class access {
public:
	template <typename Input>
	static virtsrc<Input> * get_input(const virtual_chunk_end<Input> &);
	template <typename Input, typename Output>
	static virtsrc<Input> * get_input(const virtual_chunk<Input, Output> &);
	template <typename Input, typename Output>
	static virtrecv<Output> * get_output(const virtual_chunk<Input, Output> &);
	template <typename Output>
	static virtrecv<Output> * get_output(const virtual_chunk_begin<Output> &);

	template <typename Input>
	static virtpullrecv<Input> * get_input(const virtual_chunk_pull_end<Input> &);
	template <typename Input, typename Output>
	static virtpullrecv<Input> * get_input(const virtual_chunk_pull<Input, Output> &);
	template <typename Input, typename Output>
	static virtpullsrc<Output> * get_output(const virtual_chunk_pull<Input, Output> &);
	template <typename Output>
	static virtpullsrc<Output> * get_output(const virtual_chunk_pull_begin<Output> &);

	template <typename T, typename ...TT>
	static T construct(TT && ... vv) {return T(std::forward<TT>(vv)...);}
};


///////////////////////////////////////////////////////////////////////////////
/// \brief Base class of virtual chunks. Owns a virt_node.
///////////////////////////////////////////////////////////////////////////////
class virtual_chunk_base : public pipeline_base {
	// pipeline_base has virtual dtor and shared_ptr to m_nodeMap
	friend class access;
protected:
	virt_node::ptr m_node;

	virtual_chunk_base(node_map::ptr nodeMap, virt_node::ptr ptr)
		: m_node(ptr)
	{
		this->m_nodeMap = nodeMap;
	}

	virtual_chunk_base(node_map::ptr nodeMap) {
		this->m_nodeMap = nodeMap;
	}
public:
	virtual_chunk_base() {}

	virt_node::ptr get_node() const { return m_node; }

	void set_container(virtual_container * ctr) {
		m_node->set_container(ctr);
	}

	bool empty() const { return m_node.get() == nullptr; }
};

} //namespace bits
	

///////////////////////////////////////////////////////////////////////////////
/// \brief Virtual chunk that has no output (that is, virtual consumer).
///////////////////////////////////////////////////////////////////////////////
template <typename Input>
class virtual_chunk_end : public bits::virtual_chunk_base {
	friend class bits::access;
	typedef bits::access acc;
	typedef bits::virtsrc<Input> input_type;
	input_type * m_input;

	input_type * get_input() const { return m_input; }

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that combines two virtual chunks. Assumes that the
	/// virtual nodes are already connected. You should not use this
	/// constructor directly; instead, use the pipe operator.
	///////////////////////////////////////////////////////////////////////////
	template <typename Mid>
	virtual_chunk_end(const virtual_chunk<Input, Mid> & left,
					  const virtual_chunk_end<Mid> & right);
	
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that leaves the virtual chunk unassigned.
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk_end()
		: m_input(nullptr)
	{}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that recursively constructs a node and takes
	/// ownership of it.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk_end(pipe_end<fact_t> && pipe, virtual_container * ctr = nullptr) {
		*this = std::forward<pipe_end<fact_t>>(pipe);
		set_container(ctr);
	}


	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct a node and assign it to this virtual chunk.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk_end & operator=(pipe_end<fact_t> && pipe) {
		if (this->m_node) {
			log_error() << "Virtual chunk assigned twice" << std::endl;
			throw tpie::exception("Virtual chunk assigned twice");
		}

		typedef typename fact_t::constructed_type constructed_type;
		m_input = new bits::virtsrc_impl<constructed_type, Input>(pipe.factory.construct());
		this->m_node = bits::virt_node::take_own(m_input);
		this->m_nodeMap = m_input->get_node_map();

		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Test if it is not an empty chunk
	///////////////////////////////////////////////////////////////////////////
	explicit operator bool() const noexcept {
		return m_input != nullptr;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Virtual chunk that has input and output.
///////////////////////////////////////////////////////////////////////////////
template <typename Input, typename Output=Input>
class virtual_chunk : public bits::virtual_chunk_base {
	friend class bits::access;
	typedef bits::access acc;
	typedef bits::virtsrc<Input> input_type;
	typedef bits::virtrecv<Output> output_type;
	input_type * m_input;
	output_type * m_output;
	input_type * get_input() const { return m_input; }
	output_type * get_output() const { return m_output; }

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that combines two virtual chunks. Assumes that the
	/// virtual nodes are already connected. You should not use this
	/// constructor directly; instead, use the pipe operator.
	///////////////////////////////////////////////////////////////////////////
	template <typename Mid>
	virtual_chunk(const virtual_chunk<Input, Mid> & left,
				  const virtual_chunk<Mid, Output> & right)
		: virtual_chunk_base(left.get_node_map(), bits::virt_node::combine(left.get_node(), right.get_node()))
	{
		m_input = acc::get_input(left);
		m_output = acc::get_output(right);
	}
	
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that leaves the virtual chunk unassigned.
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk()
		: m_input(nullptr)
		, m_output(nullptr)
	{}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that recursively constructs a node and takes
	/// ownership of it.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk(pipe_middle<fact_t> && pipe, virtual_container * ctr = nullptr) {
		*this = std::forward<pipe_middle<fact_t>>(pipe);
		set_container(ctr);
	}


	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct a virtual chunk from an end and a begin chunk
	///
	/// Items pushed into the virtual chunk are pushed into the left end chunk
	/// Items push out of the right begin chenk are pushed out of the virtual chunk
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk(const virtual_chunk_end<Input> & left,
				  const virtual_chunk_begin<Output> & right)
		: virtual_chunk_base(left.get_node_map(), bits::virt_node::combine(left.get_node(), right.get_node()))
	{
		m_input = acc::get_input(left);
		m_output = acc::get_output(right);
	}

	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct a node and assign it to this virtual chunk.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk & operator=(pipe_middle<fact_t> && pipe) {
		if (this->m_node) {
			log_error() << "Virtual chunk assigned twice" << std::endl;
			throw tpie::exception("Virtual chunk assigned twice");
		}
		typedef typename fact_t::template constructed_type<output_type> constructed_type;
		output_type temp(m_output);
		this->m_nodeMap = temp.get_node_map();
		fact_t f = std::move(pipe.factory);
		f.set_destination_kind_push();
		m_input = new bits::virtsrc_impl<constructed_type, Input>(f.construct(std::move(temp)));
		this->m_node = bits::virt_node::take_own(m_input);

		return *this;
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Connect this virtual chunk to another chunk.
	///////////////////////////////////////////////////////////////////////////
	template <typename NextOutput>
	virtual_chunk<Input, NextOutput> operator|(virtual_chunk<Output, NextOutput> dest) {
		if (empty()) {
			return *bits::assert_types_equal_and_return<Input, Output, virtual_chunk<Input, NextOutput> *>
				::go(&dest);
		}
		bits::virtsrc<Output> * dst=acc::get_input(dest);
		if (dest.empty() || !dst) {
			return *bits::assert_types_equal_and_return<Output, NextOutput, virtual_chunk<Input, NextOutput> *>
				::go(this);
		}
		m_output->set_destination(dst);
		return acc::construct<virtual_chunk<Input, NextOutput>>(*this, dest);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Connect this virtual chunk to another chunk.
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk_end<Input> operator|(virtual_chunk_end<Output> dest) {
		if (empty()) {
			return *bits::assert_types_equal_and_return<Input, Output, virtual_chunk_end<Input> *>
				::go(&dest);
		}
		m_output->set_destination(acc::get_input(dest));
		return acc::construct<virtual_chunk_end<Input>>(*this, dest);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Test if it is not an empty chunk
	///////////////////////////////////////////////////////////////////////////
	explicit operator bool() const noexcept {
		return m_input != nullptr;
	}
};

template <typename Input>
template <typename Mid>
virtual_chunk_end<Input>::virtual_chunk_end(const virtual_chunk<Input, Mid> & left,
											const virtual_chunk_end<Mid> & right)
	: virtual_chunk_base(left.get_node_map(),
						 bits::virt_node::combine(left.get_node(), right.get_node()))
{
	m_input = acc::get_input(left);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Virtual chunk that has no input (that is, virtual producer).
///////////////////////////////////////////////////////////////////////////////
template <typename Output>
class virtual_chunk_begin : public bits::virtual_chunk_base {
	friend class bits::access;
	typedef bits::access acc;
	typedef bits::virtrecv<Output> output_type;
	output_type * m_output;
	output_type * get_output() const { return m_output; }

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that combines two virtual chunks. Assumes that the
	/// virtual nodes are already connected. You should not use this
	/// constructor directly; instead, use the pipe operator.
	///////////////////////////////////////////////////////////////////////////
	template <typename Mid>
	virtual_chunk_begin(const virtual_chunk_begin<Mid> & left,
						const virtual_chunk<Mid, Output> & right)
		: virtual_chunk_base(left.get_node_map(),
							 bits::virt_node::combine(left.get_node(), right.get_node()))
	{
		m_output = acc::get_output(right);
	}
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that leaves the virtual chunk unassigned.
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk_begin()
		: m_output(nullptr)
	{}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that recursively constructs a node and takes
	/// ownership of it.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk_begin(pipe_begin<fact_t> && pipe, virtual_container * ctr = nullptr) {
		*this = std::forward<pipe_begin<fact_t>>(pipe);
		set_container(ctr);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct a node and assign it to this virtual chunk.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk_begin & operator=(pipe_begin<fact_t> && pipe) {
		if (this->m_node) {
			log_error() << "Virtual chunk assigned twice" << std::endl;
			throw tpie::exception("Virtual chunk assigned twice");
		}
		typedef typename fact_t::template constructed_type<output_type> constructed_type;
		output_type temp(m_output);
		this->m_nodeMap = m_output->get_node_map();
		fact_t f = std::move(pipe.factory);
		f.set_destination_kind_push();
		this->m_node = bits::virt_node::take_own(new constructed_type(f.construct(std::move(temp))));
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Connect this virtual chunk to another chunk.
	///////////////////////////////////////////////////////////////////////////
	template <typename NextOutput>
	virtual_chunk_begin<NextOutput> operator|(virtual_chunk<Output, NextOutput> dest) {
		if (empty()) throw virtual_chunk_missing_begin();
		if (dest.empty()) {
			return *bits::assert_types_equal_and_return<Output, NextOutput, virtual_chunk_begin<NextOutput> *>
				::go(this);
		}
		m_output->set_destination(acc::get_input(dest));
		return acc::construct<virtual_chunk_begin<NextOutput>>(*this, dest);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Connect this virtual chunk to another chunk.
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk_base operator|(virtual_chunk_end<Output> dest) {
		if (empty()) throw virtual_chunk_missing_begin();
		if (dest.empty()) throw virtual_chunk_missing_end();
		m_output->set_destination(acc::get_input(dest));
		return acc::construct<virtual_chunk_base>(
			this->m_nodeMap,
			bits::virt_node::combine(get_node(), dest.get_node()));
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Test if it is not an empty chunk
	///////////////////////////////////////////////////////////////////////////
	explicit operator bool() const noexcept {
		return m_output != nullptr;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Virtual chunk that has no input (that is, virtual producer).
///////////////////////////////////////////////////////////////////////////////
template <typename Input>
class virtual_chunk_pull_end : public bits::virtual_chunk_base {
	friend class bits::access;
	typedef bits::access acc;
	typedef bits::virtpullrecv<Input> input_type;
	input_type * m_input;
	input_type * get_input() const { return m_input; }

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that combines two virtual chunks. Assumes that the
	/// virtual nodes are already connected. You should not use this
	/// constructor directly; instead, use the pipe operator.
	///////////////////////////////////////////////////////////////////////////
	template <typename Mid>
	virtual_chunk_pull_end(const virtual_chunk_pull<Input, Mid> & left,
						   const virtual_chunk_pull_end<Mid> & right);

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that leaves the virtual chunk unassigned.
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk_pull_end()
		: m_input(nullptr)
	{}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that recursively constructs a node and takes
	/// ownership of it.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk_pull_end(pullpipe_end<fact_t> && pipe, virtual_container * ctr = nullptr) {
		*this = std::forward<pullpipe_end<fact_t>>(pipe);
		set_container(ctr);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct a node and assign it to this virtual chunk.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk_pull_end & operator=(pullpipe_end<fact_t> && pipe) {
		if (this->m_node) {
			log_error() << "Virtual chunk assigned twice" << std::endl;
			throw tpie::exception("Virtual chunk assigned twice");
		}
		typedef typename fact_t::template constructed_type<input_type> constructed_type;
		input_type temp(m_input);
		this->m_nodeMap = m_input->get_node_map();
		fact_t f = std::move(pipe.factory);
		f.set_destination_kind_pull();
		this->m_node = bits::virt_node::take_own(new constructed_type(f.construct(std::move(temp))));
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Test if it is not an empty chunk
	///////////////////////////////////////////////////////////////////////////
	explicit operator bool() const noexcept {
		return m_input != nullptr;
	}
};


///////////////////////////////////////////////////////////////////////////////
/// \brief Virtual chunk that has input and output.
///////////////////////////////////////////////////////////////////////////////
template <typename Input, typename Output=Input>
class virtual_chunk_pull : public bits::virtual_chunk_base {
	friend class bits::access;
	typedef bits::access acc;
	typedef bits::virtpullsrc<Output> output_type;
	typedef bits::virtpullrecv<Input> input_type;
	input_type * m_input;
	output_type * m_output;
	input_type * get_input() const { return m_input; }
	output_type * get_output() const { return m_output; }

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that combines two virtual chunks. Assumes that the
	/// virtual nodes are already connected. You should not use this
	/// constructor directly; instead, use the pipe operator.
	///////////////////////////////////////////////////////////////////////////
	template <typename Mid>
	virtual_chunk_pull(const virtual_chunk_pull<Input, Mid> & left,
					   const virtual_chunk_pull<Mid, Output> & right)
		: virtual_chunk_base(left.get_node_map(), bits::virt_node::combine(left.get_node(), right.get_node()))
	{
		m_input = acc::get_input(left);
		m_output = acc::get_output(right);
	}
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that leaves the virtual chunk unassigned.
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk_pull()
		: m_input(nullptr)
		, m_output(nullptr)
	{}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that recursively constructs a node and takes
	/// ownership of it.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk_pull(pullpipe_middle<fact_t> && pipe, virtual_container * ctr = nullptr) {
		*this = std::forward<pullpipe_middle<fact_t>>(pipe);
		set_container(ctr);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct a virtual chunk pull from an end and a begin chunk
	///
	/// Items pushed into the virtual chunk are pushed into the left end chunk
	/// Items push out of the right begin chenk are pushed out of the virtual chunk
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk_pull(const virtual_chunk_pull_end<Input> & left,
					   const virtual_chunk_pull_begin<Output> & right)
		: virtual_chunk_base(left.get_node_map(), bits::virt_node::combine(left.get_node(), right.get_node())) {
		m_input = acc::get_input(left);
		m_output = acc::get_output(right);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct a node and assign it to this virtual chunk.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk_pull & operator=(pullpipe_middle<fact_t> && pipe) {
		if (this->m_node) {
			log_error() << "Virtual chunk assigned twice" << std::endl;
			throw tpie::exception("Virtual chunk assigned twice");
		}
		typedef typename fact_t::template constructed_type<input_type> constructed_type;
		input_type temp(m_input);
		this->m_nodeMap = temp.get_node_map();
		fact_t f = std::move(pipe.factory);
		f.set_destination_kind_push();
		m_output = new bits::virtpullsrc_impl<constructed_type, Output>(f.construct(std::move(temp)));
		this->m_node = bits::virt_node::take_own(m_output);
		return *this;
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Connect this virtual chunk to another chunk.
	///////////////////////////////////////////////////////////////////////////
	template <typename NextOutput>
	virtual_chunk_pull<Input, NextOutput> operator|(virtual_chunk_pull<Output, NextOutput> dest) {
		if (empty()) {
			return *bits::assert_types_equal_and_return<Input, Output, virtual_chunk_pull<Input, NextOutput> *>
				::go(&dest);
		}
		if (dest.empty()) {
			return *bits::assert_types_equal_and_return<Output, NextOutput, virtual_chunk_pull<Input, NextOutput> *>
				::go(this);
		}
		acc::get_input(dest)->set_source(get_output());
		return acc::construct<virtual_chunk_pull<Input, NextOutput>>(*this, dest);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Connect this virtual chunk to another chunk.
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk_pull_end<Input> operator|(virtual_chunk_pull_end<Output> dest) {
		if (empty()) {
			return *bits::assert_types_equal_and_return<Input, Output, virtual_chunk_pull_end<Input> *>
				::go(&dest);
		}
		acc::get_input(dest)->set_source(get_output());
		return acc::construct<virtual_chunk_pull_end<Input>>(*this, dest);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Test if it is not an empty chunk
	///////////////////////////////////////////////////////////////////////////
	explicit operator bool() const noexcept {
		return m_input != nullptr;
	}
};


///////////////////////////////////////////////////////////////////////////////
/// \brief Virtual chunk that has no output (that is, virtual consumer).
///////////////////////////////////////////////////////////////////////////////
template <typename Output>
class virtual_chunk_pull_begin : public bits::virtual_chunk_base {
	friend class bits::access;
	typedef bits::access acc;
	typedef bits::virtpullsrc<Output> output_type;
	output_type * m_output;

	output_type * get_output() const { return m_output; }

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that combines two virtual chunks. Assumes that the
	/// virtual nodes are already connected. You should not use this
	/// constructor directly; instead, use the pipe operator.
	///////////////////////////////////////////////////////////////////////////
	template <typename Mid>
	virtual_chunk_pull_begin(const virtual_chunk_pull_begin<Mid> & left,
							 const virtual_chunk_pull<Mid, Output> & right)
		: virtual_chunk_base(left.get_node_map(),
							 bits::virt_node::combine(left.get_node(), right.get_node())) {
		m_output = acc::get_output(right);
	}
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that leaves the virtual chunk unassigned.
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk_pull_begin()
		: m_output(nullptr)
	{}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor that recursively constructs a node and takes
	/// ownership of it.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk_pull_begin(pullpipe_begin<fact_t> && pipe, virtual_container * ctr = nullptr) {
		*this = std::forward<pullpipe_begin<fact_t>>(pipe);
		set_container(ctr);
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct a node and assign it to this virtual chunk.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact_t>
	virtual_chunk_pull_begin & operator=(pullpipe_begin<fact_t> && pipe) {
		if (this->m_node) {
			log_error() << "Virtual chunk assigned twice" << std::endl;
			throw tpie::exception("Virtual chunk assigned twice");
		}

		typedef typename fact_t::constructed_type constructed_type;
		m_output = new bits::virtpullsrc_impl<constructed_type, Output>(pipe.factory.construct());
		this->m_node = bits::virt_node::take_own(m_output);
		this->m_nodeMap = m_output->get_node_map();

		return *this;
	}


	///////////////////////////////////////////////////////////////////////////
	/// \brief Connect this virtual chunk to another chunk.
	///////////////////////////////////////////////////////////////////////////
	virtual_chunk_base operator|(virtual_chunk_pull_end<Output> dest) {
		if (empty()) throw virtual_chunk_missing_begin();
		if (dest.empty()) throw virtual_chunk_missing_end();
		acc::get_input(dest)->set_source(get_output());
		return acc::construct<virtual_chunk_base>(
			this->m_nodeMap,
			bits::virt_node::combine(get_node(), dest.get_node()));
	}



	///////////////////////////////////////////////////////////////////////////
	/// \brief Connect this virtual chunk to another chunk.
	///////////////////////////////////////////////////////////////////////////
	template <typename NextOutput>
	virtual_chunk_pull_begin<NextOutput> operator|(virtual_chunk_pull<Output, NextOutput> dest) {
		if (empty()) throw virtual_chunk_missing_begin();
		if (dest.empty()) {
			return *bits::assert_types_equal_and_return<Output, NextOutput, virtual_chunk_pull_begin<NextOutput> *>
				::go(this);
		}
		acc::get_input(dest)->set_source(get_output());
		return acc::construct<virtual_chunk_pull_begin<NextOutput>>(*this, dest);
	}


	///////////////////////////////////////////////////////////////////////////
	/// \brief Test if it is not an empty chunk
	///////////////////////////////////////////////////////////////////////////
	explicit operator bool() const noexcept {
		return m_output != nullptr;
	}
};


template <typename Input>
template <typename Mid>
virtual_chunk_pull_end<Input>::virtual_chunk_pull_end(const virtual_chunk_pull<Input, Mid> & left,
													  const virtual_chunk_pull_end<Mid> & right)
	: virtual_chunk_base(left.get_node_map(), bits::virt_node::combine(left.get_node(), right.get_node())) {
	m_input = acc::get_input(left);
}

	
namespace bits {

template <typename Input>
virtsrc<Input> * access::get_input(const virtual_chunk_end<Input> & chunk) {
	return chunk.get_input();
}

template <typename Input, typename Output>
virtsrc<Input> * access::get_input(const virtual_chunk<Input, Output> & chunk) {
	return chunk.get_input();
}

template <typename Input, typename Output>
virtrecv<Output> * access::get_output(const virtual_chunk<Input, Output> & chunk) {
	return chunk.get_output();
}

template <typename Output>
virtrecv<Output> * access::get_output(const virtual_chunk_begin<Output> & chunk) {
	return chunk.get_output();
}

	
template <typename Input>
virtpullrecv<Input> * access::get_input(const virtual_chunk_pull_end<Input> & chunk) {
	return chunk.get_input();
}

template <typename Input, typename Output>
virtpullrecv<Input> * access::get_input(const virtual_chunk_pull<Input, Output> & chunk) {
	return chunk.get_input();
}

template <typename Input, typename Output>
virtpullsrc<Output> * access::get_output(const virtual_chunk_pull<Input, Output> & chunk) {
	return chunk.get_output();
}

template <typename Output>
virtpullsrc<Output> * access::get_output(const virtual_chunk_pull_begin<Output> & chunk) {
	return chunk.get_output();
}

template <typename dest_t, typename T>
class vfork_node : public node {
public:
	typedef T item_type;

	vfork_node(dest_t && dest, virtual_chunk_end<T> out)
		: vnode(out.get_node())
		, dest2(bits::access::get_input(out))
		, dest(std::move(dest))	{
		add_push_destination(this->dest);
		if (dest2) add_push_destination(*dest2);
	}
	
	void push(T v) {
		dest.push(v);
		if (dest2) dest2->push(v);
	}
	
private:
	// This counted reference ensures dest2 is not deleted prematurely.
	virt_node::ptr vnode;
	
	virtsrc<T> * dest2;
	
	dest_t dest;
};

template <typename T>
class devirtualize_end_node : public node {
public:
	typedef T item_type;

	template <typename TT>
	devirtualize_end_node(TT out, std::unique_ptr<node> tail=std::unique_ptr<node>())
		: vnode(out.get_node())
		, tail(std::move(tail))
		, dest(bits::access::get_input(out))
	{
		if (dest) add_push_destination(*dest);
	}

	void push(T v) {
		if (dest) dest->push(v);
	}

private:
	// This counted reference ensures dest is not deleted prematurely.
	virt_node::ptr vnode;
	std::unique_ptr<node> tail;
	virtsrc<T> * dest;
};


template <typename dest_t, typename T>
class devirtualize_begin_node: public virtsrc<T> {
public:
	using input_type = typename maybe_add_const_ref<T>::type;

	template <typename TT>
	devirtualize_begin_node(dest_t dest, TT input)
		: vnode(input.get_node())
		, dest(std::move(dest)) {
		node::add_push_destination(this->dest);
		this->set_name("Virtual source", PRIORITY_INSIGNIFICANT);
		this->set_plot_options(node::PLOT_BUFFERED | node::PLOT_SIMPLIFIED_HIDE);
		recv = bits::access::get_output(input);
		recv->set_destination(this);
	}

	devirtualize_begin_node(devirtualize_begin_node && o)
		: virtsrc<T>(std::move(o))
		, vnode(std::move(o.vnode))
		, recv(o.recv)
		, dest(std::move(o.dest)) {
		o.recv = nullptr;
		if (recv) {
			recv->change_destination(this);
		}
	}

	devirtualize_begin_node & operator=(devirtualize_begin_node && o) {
		assert(recv == nullptr);
		virtsrc<T>::operator=(std::move(o));
		vnode = std::move(o.vnode);
		recv = o.recv;
		o.recv = nullptr;
		if (recv) {
			recv->change_destination(this);
		}
	}
	
	const node_token & get_token() final {
		return node::get_token();
	}

	void push(input_type v) final {
		dest.push(v);
	}
	
	virt_node::ptr vnode;
	virtrecv<T> * recv = nullptr;
	dest_t dest;
};

template <typename Input, typename Output>
class devirtualization_factory: public factory_base {
public:
	template <typename dest_t>
	using constructed_type = bits::devirtualize_end_node<Input>;

	devirtualization_factory(const devirtualization_factory &) = delete;
	devirtualization_factory(devirtualization_factory &&) = default;
	devirtualization_factory & operator=(const devirtualization_factory &) = delete;
	devirtualization_factory & operator=(devirtualization_factory &&) = default;

	explicit devirtualization_factory(virtual_chunk<Input, Output> chunk) : chunk(chunk) {}

	template <typename dest_t>
	bits::devirtualize_end_node<Input> construct(dest_t && dest) {
		auto destnode = std::make_unique<bits::devirtualize_begin_node<dest_t, Output>>(std::move(dest), chunk);
		this->init_sub_node(*destnode);
		bits::devirtualize_end_node<Input> r(chunk, std::move(destnode));
		this->init_sub_node(r);
		return r;
	}
	
	virtual_chunk<Input, Output> chunk;
};


template <typename T>
class devirtualize_pull_begin_node : public node {
public:
	typedef T item_type;

	template <typename TT>
	devirtualize_pull_begin_node(TT out, std::unique_ptr<node> tail=std::unique_ptr<node>())
		: vnode(out.get_node())
		, tail(std::move(tail))
		, src(bits::access::get_output(out))
	{
		if (src) add_pull_source(*src);
	}

	bool can_pull() {return src->can_pull();}
	item_type pull() {return src->pull();}
private:
	// This counted reference ensures src is not deleted prematurely.
	virt_node::ptr vnode;
	std::unique_ptr<node> tail;
	virtpullsrc<T> * src;
};

template <typename src_t, typename T>
class devirtualize_pull_end_node: public virtpullsrc<T> {
public:
	template <typename TT>
	devirtualize_pull_end_node(src_t src, TT output)
		: vnode(output.get_node())
		, src(std::move(src)) {
		node::add_pull_source(this->src),
		this->set_name("Virtual source", PRIORITY_INSIGNIFICANT);
		this->set_plot_options(node::PLOT_BUFFERED | node::PLOT_SIMPLIFIED_HIDE);
		input = bits::access::get_input(output);
		input->set_source(this);
	}

	devirtualize_pull_end_node(devirtualize_pull_end_node && o)
		: virtpullsrc<T>(std::move(o))
		, vnode(std::move(o.vnode))
		, input(o.input)
		, src(std::move(o.src)) {
		o.input = nullptr;
		if (input) {
			input->change_source(this);
		}
	}

	devirtualize_pull_end_node & operator=(devirtualize_pull_end_node && o) {
		assert(input == nullptr);
		virtpullsrc<T>::operator=(std::move(o));
		vnode = std::move(o.vnode);
		input = o.input;
		o.input = nullptr;
		if (input) {
			input->change_source(this);
		}
	}
	
	const node_token & get_token() final {
		return node::get_token();
	}

	T pull() final {return src.pull();}
	bool can_pull() final {return src.can_pull();}
	
	virt_node::ptr vnode;
	virtpullrecv<T> * input = nullptr;
	src_t src;
};

template <typename Input, typename Output>
class devirtualization_pull_factory: public factory_base {
public:
	template <typename src_t>
	using constructed_type = bits::devirtualize_pull_begin_node<Output>;
	
	devirtualization_pull_factory(const devirtualization_pull_factory &) = delete;
	devirtualization_pull_factory(devirtualization_pull_factory &&) = default;
	devirtualization_pull_factory & operator=(const devirtualization_pull_factory &) = delete;
	devirtualization_pull_factory & operator=(devirtualization_pull_factory &&) = default;

	explicit devirtualization_pull_factory(virtual_chunk_pull<Input, Output> chunk) : chunk(chunk) {}

	template <typename src_t>
	bits::devirtualize_pull_begin_node<Input> construct(src_t && src) {
		auto srcnode = std::make_unique<bits::devirtualize_pull_end_node<src_t, Input>>(std::move(src), chunk);
		this->init_sub_node(*srcnode);
		bits::devirtualize_pull_begin_node<Output> r(chunk, std::move(srcnode));
		this->init_sub_node(r);
		return r;
	}
	
	virtual_chunk_pull<Input, Output> chunk;
};

	
	
} // namespace bits

/**
 * \brief Convert a virtual_chunk_end for use at the end of a normal none virtual pipeline
 */
template <typename Input>
pipe_end<termfactory<bits::devirtualize_end_node<Input>, virtual_chunk_end<Input>>> devirtualize(const virtual_chunk_end<Input> & out) {
	return {out};
}

/**
 * \brief Convert a virtual_chunk_begin for use at the beginning of a normal none virtual pipeline
 */
template <typename Output>
pipe_begin<tfactory<bits::devirtualize_begin_node, Args<Output>, virtual_chunk_begin<Output>>> devirtualize(const virtual_chunk_begin<Output> & in) {
	return {in};
}

/**
 * \brief Convert a virtual_chunk for use in the middle of a normal none virtual pipeline
 */
template <typename Input, typename Output>
pipe_middle<bits::devirtualization_factory<Input, Output>> devirtualize(const virtual_chunk<Input, Output> & mid) {
	return {mid};
}

/**
 * \brief Convert a virtual_chunk_pull_begin for use at the beginning of a normal none virtual pull pipeline
 */
template <typename Input>
pullpipe_begin<termfactory<bits::devirtualize_pull_begin_node<Input>, virtual_chunk_pull_begin<Input>>> devirtualize(const virtual_chunk_pull_begin<Input> & out) {
	return {out};
}

/*
 * \brief Convert a virtual_chunk_pull_end for use at the end of a normal none virtual pull pipeline
 */
template <typename Input>
pullpipe_end<tfactory<bits::devirtualize_pull_end_node, Args<Input>, virtual_chunk_pull_end<Input>>> devirtualize(const virtual_chunk_pull_end<Input> & in) {
	return {in};
}

/*
 * \brief Convert a virtual_chunk_pull for use in the middle of a normal none virtual pull pipeline
 */
template <typename Input, typename Output>
pullpipe_middle<bits::devirtualization_pull_factory<Input, Output>> devirtualize(const virtual_chunk_pull<Input, Output> & mid) {
	return {mid};
}
	
template <typename T>
pipe_middle<tfactory<bits::vfork_node, Args<T>, virtual_chunk_end<T> > > fork_to_virtual(const virtual_chunk_end<T> & out) {
	return {out};
}
	
template <typename T>
virtual_chunk<T> vfork(const virtual_chunk_end<T> & out) {
	if (out.empty()) return virtual_chunk<T>();
	return fork_to_virtual(out);
}

template <typename T>
inline virtual_chunk<T> chunk_if(bool b, virtual_chunk<T> t) {
	if (b)
		return t;
	else
		return virtual_chunk<T>();
}

template <typename T>
virtual_chunk_end<T> chunk_end_if(bool b, virtual_chunk_end<T> t) {
	if (b)
		return t;
	else 
		return virtual_chunk_end<T>(null_sink<T>());
}

} // namespace tpie::pipelining

#endif // __TPIE_PIPELINING_VIRTUAL_H__
