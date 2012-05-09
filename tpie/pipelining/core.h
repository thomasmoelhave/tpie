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

namespace tpie {

namespace pipelining {

struct pipe_segment;

struct segment_token {
	// Use for the simple case in which a pipe_segment owns its own token
	inline segment_token(const pipe_segment * owner)
		: m_id(tokens.size())
		, m_free(false)
	{
		tokens.push_back(owner);
	}

	// This copy constructor has two uses:
	// 1. Simple case when a pipe_segment is copied (freshToken = false)
	// 2. Advanced case when a pipe_segment is being constructed with a specific token (freshToken = true)
	inline segment_token(const segment_token & other, const pipe_segment * newOwner, bool freshToken = false)
		: m_id(other.id())
		, m_free(false)
	{
		if (freshToken) {
			tp_assert(other.m_free, "Trying to take ownership of a non-free token");
			tp_assert(tokens.at(m_id) == 0, "A token already has an owner, but m_free is true - contradiction");
		} else {
			tp_assert(!other.m_free, "Trying to copy a free token");
		}
		tokens.at(m_id) = newOwner;
	}

	// Use for the advanced case when a segment_token is allocated before the pipe_segment
	inline segment_token()
		: m_id(tokens.size())
		, m_free(true)
	{
		tokens.push_back(0);
	}

	inline size_t id() const { return m_id; }

private:
	// This should not be a global structure. Suggest mergable maps for each pipeline.
	static std::deque<const pipe_segment *> tokens;
	size_t m_id;
	bool m_free;
};

std::deque<const pipe_segment *> segment_token::tokens;

///////////////////////////////////////////////////////////////////////////////
/// Base class of all segments. A segment should inherit from pipe_segment,
/// have a single template parameter dest_t if it is not a terminus segment,
/// and implement methods begin(), push() and end(), if it is not a source
/// segment.
///////////////////////////////////////////////////////////////////////////////
struct pipe_segment {
	///////////////////////////////////////////////////////////////////////////
	/// \brief Virtual dtor.
	///////////////////////////////////////////////////////////////////////////
	virtual ~pipe_segment() {}

	virtual memory_size_type get_minimum_memory() {
		return 0;
	}

	inline memory_size_type get_available_memory() {
		return memory;
	}

protected:
	inline pipe_segment()
		: token(this)
	{
	}

	inline pipe_segment(const pipe_segment & other)
		: token(other.token, this)
	{
	}

	inline pipe_segment(const segment_token & token)
		: token(token, this, true)
	{
	}

	inline void add_push_destination(const pipe_segment & /*dest*/) {
	}

	inline void add_push_destination(const segment_token & /*dest*/) {
	}

	inline void add_pull_destination(const pipe_segment & /*dest*/) {
	}

	inline void add_pull_destination(const segment_token & /*dest*/) {
	}

	inline void add_dependency(const pipe_segment & /*dest*/) {
	}

	inline void add_dependency(const segment_token & /*dest*/) {
	}

private:
	inline void set_available_memory(memory_size_type mem) {
		memory = mem;
	}

	memory_size_type memory;
	segment_token token;

	// TODO who's our friend?
};

///////////////////////////////////////////////////////////////////////////////
/// \class pipeline_virtual
/// Virtual superclass for pipelines implementing the function call operator.
///////////////////////////////////////////////////////////////////////////////
struct pipeline_virtual {
	///////////////////////////////////////////////////////////////////////////
	/// \brief Invoke the pipeline.
	///////////////////////////////////////////////////////////////////////////
	virtual void operator()() = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Generate a GraphViz graph documenting the pipeline flow.
	///////////////////////////////////////////////////////////////////////////
	virtual void plot(std::ostream & out) = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Generate a GraphViz graph documenting the pipeline flow.
	///////////////////////////////////////////////////////////////////////////
	virtual void plot_phases(std::ostream & out) = 0;

	virtual double memory() const = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Virtual dtor.
	///////////////////////////////////////////////////////////////////////////
	virtual ~pipeline_virtual() {}
};

///////////////////////////////////////////////////////////////////////////////
/// \class pipeline_impl
/// \tparam fact_t Factory type
/// Templated subclass of pipeline_virtual for push pipelines.
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
struct pipeline_impl : public pipeline_virtual {
	typedef typename fact_t::generated_type gen_t;

	inline pipeline_impl(const fact_t & factory) : r(factory.construct()), _memory(factory.memory()) {}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Virtual dtor.
	///////////////////////////////////////////////////////////////////////////
	~pipeline_impl() {}

	void operator()() {
		r();
	}
	inline operator gen_t() {
		return r;
	}
	void plot(std::ostream & out) {
		if (&pipeline_impl<fact_t>::actual_plot) {
			return actual_plot(out);
		} else {
			out << "pipeline_impl::plot not linked" << std::endl;
		}
	}

	void plot_phases(std::ostream & out) {
		if (&pipeline_impl<fact_t>::actual_plot_phases) {
			return actual_plot_phases(out);
		} else {
			out << "pipeline_impl::plot not linked" << std::endl;
		}
	}

	double memory() const {
		return _memory;
	}

private:

// Weak linkage with GCC. This way, if actual_plot (declared below) is never
// defined (i.e. pipelining/plotter.h is not included), actual_plot will get
// the link-time address 0x0, which we handle in the plot() definition above.
//
// This way, we don't require boost/unordered_map.hpp or tpie/disjoint_sets.h
// in the pipelining framework unless the user wants Graphviz output.
// Consequently, the user won't have to link with Boost or libtpie.
//
// This way, tpie/pipelining/examples/ can contain standalone examples of the
// pipelining framework.

#ifdef __GNUC__
#	define ATTRWEAK __attribute__ ((weak))
#else
#	define ATTRWEAK
#endif

	void actual_plot(std::ostream & out) ATTRWEAK;

	void actual_plot_phases(std::ostream & out) ATTRWEAK;

	gen_t r;
	double _memory;
};

///////////////////////////////////////////////////////////////////////////////
/// \class datasource_wrapper
/// \tparam gen_t Generator type
/// Templated subclass of pipeline_virtual for pull pipelines.
///////////////////////////////////////////////////////////////////////////////
template <typename gen_t>
struct datasource_wrapper : public pipeline_virtual {
	gen_t generator;
	inline datasource_wrapper(const gen_t & generator) : generator(generator) {
	}
	///////////////////////////////////////////////////////////////////////////
	/// \brief Virtual dtor.
	///////////////////////////////////////////////////////////////////////////
	~datasource_wrapper() {}
	inline void operator()() {
		generator();
	}
	void plot(std::ostream & out) {
		out << "datasource" << std::endl;
	}
	void plot_phases(std::ostream & out) {
		out << "datasource" << std::endl;
	}
	double memory() const {
		return -1;
	}
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
		(*p)();
	}
	inline void plot() {
		p->plot(std::cout);
	}
	inline void plot_phases() {
		p->plot_phases(std::cout);
	}
	inline double memory() const {
		return p->memory();
	}
private:
	pipeline_virtual * p;
};

namespace bits {

template <typename child_t>
struct pair_factory_base {
	inline double memory() const {
		return self().fact1.memory() + self().fact2.memory();
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

///////////////////////////////////////////////////////////////////////////////
/// \class datasource
///
/// A data source pulls data up the pipeline.
///
/// \tparam gen_t Generator type
///////////////////////////////////////////////////////////////////////////////
template <typename gen_t>
struct datasource {
	inline datasource(const gen_t & gen) : generator(gen) {
	}

	template <typename fact_t>
	inline datasource<typename fact_t::template generated<gen_t>::type>
	operator|(const fact_t & fact) const {
		return fact.construct(generator);
	}

	inline operator gen_t() const {
		return generator;
	}

	inline operator datasource_wrapper<gen_t>() const {
		return generator;
	}

	inline operator pipeline() const {
		return datasource_wrapper<gen_t>(generator);
	}

	gen_t generator;
};

}

}

#endif
