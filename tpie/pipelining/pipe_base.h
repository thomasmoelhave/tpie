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

#ifndef __TPIE_PIPELINING_PIPE_BASE_H__
#define __TPIE_PIPELINING_PIPE_BASE_H__

#include <tpie/types.h>
#include <tpie/pipelining/priority_type.h>
#include <tpie/pipelining/graph.h>
#include <tpie/pipelining/pair_factory.h>
#include <tpie/pipelining/pipeline.h>

namespace tpie {

namespace pipelining {

namespace bits {

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

} // namespace bits

template <typename fact_t>
struct pipe_end : bits::pipe_base<pipe_end<fact_t> > {
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
struct pipe_middle : bits::pipe_base<pipe_middle<fact_t> > {
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
struct pipe_begin : bits::pipe_base<pipe_begin<fact_t> > {
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
	inline bits::pipeline_impl<bits::termpair_factory<fact_t, fact2_t> >
	operator|(const pipe_end<fact2_t> & r) {
		return bits::termpair_factory<fact_t, fact2_t>(factory, r.factory).final();
	}

	fact_t factory;
};

template <typename fact_t>
struct pullpipe_end : bits::pipe_base<pullpipe_end<fact_t> > {
	typedef fact_t factory_type;

	inline pullpipe_end() {
	}

	inline pullpipe_end(const fact_t & factory) : factory(factory) {
	}

	fact_t factory;
};

template <typename fact_t>
struct pullpipe_middle : bits::pipe_base<pullpipe_middle<fact_t> > {
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
struct pullpipe_begin : bits::pipe_base<pullpipe_begin<fact_t> > {
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
	inline bits::pipeline_impl<bits::termpair_factory<fact2_t, fact_t> >
	operator|(const pullpipe_end<fact2_t> & r) {
		return bits::termpair_factory<fact2_t, fact_t>(r.factory, factory).final();
	}

	fact_t factory;
};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_PIPE_BASE_H__
