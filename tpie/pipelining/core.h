// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, The TPIE development team
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

#ifndef __TPIE_PIPELINING_CORE_H__
#define __TPIE_PIPELINING_CORE_H__

#include <tpie/tpie_assert.h>
#include <tpie/types.h>
#include <iostream>
#include <deque>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <tpie/pipelining/pipe_segment.h>
#include <tpie/pipelining/graph.h>
#include <tpie/progress_indicator_null.h>

namespace tpie {

namespace pipelining {

///////////////////////////////////////////////////////////////////////////////
/// \class pipeline_base
/// Virtual superclass for pipelines implementing the function call operator.
///////////////////////////////////////////////////////////////////////////////
class pipeline_base {
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Invoke the pipeline.
	///////////////////////////////////////////////////////////////////////////
	void operator()(stream_size_type items, progress_indicator_base & pi, memory_size_type mem);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Generate a GraphViz graph documenting the pipeline flow.
	///////////////////////////////////////////////////////////////////////////
	void plot(std::ostream & out);

	double memory() const {
		return m_memory;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Virtual dtor.
	///////////////////////////////////////////////////////////////////////////
	virtual ~pipeline_base() {}

	segment_map::ptr get_segment_map() const {
		return m_segmap;
	}

protected:
	segment_map::ptr m_segmap;
	double m_memory;
};

///////////////////////////////////////////////////////////////////////////////
/// \class pipeline_impl
/// \tparam fact_t Factory type
/// Templated subclass of pipeline_base for push pipelines.
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
struct pipeline_impl : public pipeline_base {
	typedef typename fact_t::generated_type gen_t;

	inline pipeline_impl(const fact_t & factory)
		: r(factory.construct())
	{
		this->m_memory = factory.memory();
		this->m_segmap = r.get_segment_map();
	}

	inline operator gen_t() {
		return r;
	}

private:
	gen_t r;
};

///////////////////////////////////////////////////////////////////////////////
/// \class pipeline
///
/// This class is used to avoid writing the template argument in the
/// pipeline_impl type.
///////////////////////////////////////////////////////////////////////////////
struct pipeline {
	template <typename T>
	inline pipeline(const T & from) {
		p = new T(from);
	}
	inline ~pipeline() {
		delete p;
	}
	inline void operator()() {
		progress_indicator_null pi;
		(*p)(1, pi, get_memory_manager().available());
	}
	inline void operator()(stream_size_type items, progress_indicator_base & pi) {
		(*p)(items, pi, get_memory_manager().available());
	}
	inline void operator()(stream_size_type items, progress_indicator_base & pi, memory_size_type mem) {
		(*p)(items, pi, mem);
	}
	inline void plot(std::ostream & os = std::cout) {
		p->plot(os);
	}
	inline double memory() const {
		return p->memory();
	}
	inline segment_map::ptr get_segment_map() const {
		return p->get_segment_map();
	}
private:
	pipeline_base * p;
};

namespace bits {

template <typename child_t>
struct pair_factory_base {
	inline double memory() const {
		return self().fact1.memory() + self().fact2.memory();
	}

	inline void name(const std::string & n, priority_type) {
		push_breadcrumb(n);
	}

	void push_breadcrumb(const std::string & n) {
		self().fact1.push_breadcrumb(n);
		self().fact2.push_breadcrumb(n);
	}

private:
	inline child_t & self() {return *static_cast<child_t*>(this);}
	inline const child_t & self() const {return *static_cast<const child_t*>(this);}
};

template <class fact1_t, class fact2_t>
struct pair_factory : public pair_factory_base<pair_factory<fact1_t, fact2_t> > {
	template <typename dest_t>
	struct generated {
		typedef typename fact1_t::template generated<typename fact2_t::template generated<dest_t>::type>::type type;
	};

	inline pair_factory(const fact1_t & fact1, const fact2_t & fact2)
		: fact1(fact1), fact2(fact2) {
	}

	template <typename dest_t>
	inline typename generated<dest_t>::type
	construct(const dest_t & dest) const {
		return fact1.construct(fact2.construct(dest));
	}

	fact1_t fact1;
	fact2_t fact2;
};

template <class fact1_t, class termfact2_t>
struct termpair_factory : public pair_factory_base<termpair_factory<fact1_t, termfact2_t> > {
	typedef typename fact1_t::template generated<typename termfact2_t::generated_type>::type generated_type;

	inline termpair_factory(const fact1_t & fact1, const termfact2_t & fact2)
		: fact1(fact1), fact2(fact2) {
		}

	fact1_t fact1;
	termfact2_t fact2;

	inline generated_type
	construct() const {
		return fact1.construct(fact2.construct());
	}
};

} // namespace bits

template <typename child_t>
struct pipe_base {
	inline child_t & memory(double amount) {
		self().factory.memory(amount);
		return self();
	}

	inline double memory() const {
		return self().factory.memory();
	}

	inline child_t & name(const std::string & n, priority_type p = PRIORITY_USER) {
		self().factory.name(n, p);
		return self();
	}

	inline child_t & breadcrumb(const std::string & n) {
		self().factory.push_breadcrumb(n);
		return self();
	}

private:
	inline child_t & self() {return *static_cast<child_t*>(this);}
	inline const child_t & self() const {return *static_cast<const child_t*>(this);}
};

template <typename fact_t>
struct pipe_end : pipe_base<pipe_end<fact_t> > {
	typedef fact_t factory_type;

	inline pipe_end() {
	}

	inline pipe_end(const fact_t & factory) : factory(factory) {
	}

	fact_t factory;
};

///////////////////////////////////////////////////////////////////////////////
/// \class pipe_middle
///
/// A pipe_middle class pushes input down the pipeline.
///
/// \tparam fact_t A factory with a construct() method like the factory_0,
///                factory_1, etc. helpers.
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
struct pipe_middle : pipe_base<pipe_middle<fact_t> > {
	typedef fact_t factory_type;

	inline pipe_middle() {
	}

	inline pipe_middle(const fact_t & factory) : factory(factory) {
	}

	///////////////////////////////////////////////////////////////////////////
	/// The pipe operator combines this generator/filter with another filter.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact2_t>
	inline pipe_middle<bits::pair_factory<fact_t, fact2_t> >
	operator|(const pipe_middle<fact2_t> & r) {
		return bits::pair_factory<fact_t, fact2_t>(factory, r.factory);
	}

	///////////////////////////////////////////////////////////////////////////
	/// This pipe operator combines this generator/filter with a terminator to
	/// make a pipeline.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact2_t>
	inline pipe_end<bits::termpair_factory<fact_t, fact2_t> >
	operator|(const pipe_end<fact2_t> & r) {
		return bits::termpair_factory<fact_t, fact2_t>(factory, r.factory);
	}

	fact_t factory;
};

template <typename fact_t>
struct pipe_begin : pipe_base<pipe_begin<fact_t> > {
	typedef fact_t factory_type;

	inline pipe_begin() {
	}

	inline pipe_begin(const fact_t & factory) : factory(factory) {
	}

	template <typename fact2_t>
	inline pipe_begin<bits::pair_factory<fact_t, fact2_t> >
	operator|(const pipe_middle<fact2_t> & r) {
		return bits::pair_factory<fact_t, fact2_t>(factory, r.factory);
	}

	template <typename fact2_t>
	inline pipeline_impl<bits::termpair_factory<fact_t, fact2_t> >
	operator|(const pipe_end<fact2_t> & r) {
		return bits::termpair_factory<fact_t, fact2_t>(factory, r.factory);
	}

	fact_t factory;
};

template <typename fact_t>
struct pullpipe_end : pipe_base<pullpipe_end<fact_t> > {
	typedef fact_t factory_type;

	inline pullpipe_end() {
	}

	inline pullpipe_end(const fact_t & factory) : factory(factory) {
	}

	fact_t factory;
};

template <typename fact_t>
struct pullpipe_middle : pipe_base<pullpipe_middle<fact_t> > {
	typedef fact_t factory_type;

	inline pullpipe_middle() {
	}

	inline pullpipe_middle(const fact_t & factory) : factory(factory) {
	}

	template <typename fact2_t>
	inline pullpipe_middle<bits::pair_factory<fact2_t, fact_t> >
	operator|(const pipe_middle<fact2_t> & r) {
		return bits::pair_factory<fact2_t, fact_t>(r.factory, factory);
	}

	template <typename fact2_t>
	inline pullpipe_end<bits::termpair_factory<fact2_t, fact_t> >
	operator|(const pipe_end<fact2_t> & r) {
		return bits::termpair_factory<fact2_t, fact_t>(r.factory, factory);
	}

	fact_t factory;
};

template <typename fact_t>
struct pullpipe_begin : pipe_base<pullpipe_begin<fact_t> > {
	typedef fact_t factory_type;

	inline pullpipe_begin() {
	}

	inline pullpipe_begin(const fact_t & factory) : factory(factory) {
	}

	template <typename fact2_t>
	inline pullpipe_begin<bits::termpair_factory<fact2_t, fact_t> >
	operator|(const pullpipe_middle<fact2_t> & r) {
		return bits::termpair_factory<fact2_t, fact_t>(r.factory, factory);
	}

	template <typename fact2_t>
	inline pipeline_impl<bits::termpair_factory<fact2_t, fact_t> >
	operator|(const pullpipe_end<fact2_t> & r) {
		return bits::termpair_factory<fact2_t, fact_t>(r.factory, factory);
	}

	fact_t factory;
};

}

}

#endif
