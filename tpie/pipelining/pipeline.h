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

#ifndef __TPIE_PIPELINING_PIPELINE_H__
#define __TPIE_PIPELINING_PIPELINE_H__

#include <tpie/types.h>
#include <iostream>
#include <tpie/pipelining/tokens.h>
#include <tpie/progress_indicator_null.h>

namespace tpie {

namespace pipelining {

namespace bits {

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

	node_map::ptr get_node_map() const {
		return m_segmap;
	}

protected:
	node_map::ptr m_segmap;
	double m_memory;
};

///////////////////////////////////////////////////////////////////////////////
/// \class pipeline_impl
/// \tparam fact_t Factory type
/// Templated subclass of pipeline_base for push pipelines.
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
class pipeline_impl : public pipeline_base {
public:
	typedef typename fact_t::generated_type gen_t;

	inline pipeline_impl(const fact_t & factory)
		: r(factory.construct())
	{
		this->m_memory = factory.memory();
		this->m_segmap = r.get_node_map();
	}

	inline operator gen_t() {
		return r;
	}

private:
	gen_t r;
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \class pipeline
///
/// This class is used to avoid writing the template argument in the
/// pipeline_impl type.
///////////////////////////////////////////////////////////////////////////////
class pipeline {
public:
	pipeline()
		: p(0)
	{
	}

	template <typename T>
	inline pipeline(const T & from)
		: p(0)
	{
		*this = from;
	}
	template <typename T>
	pipeline & operator=(const T & from) {
		if (p) delete p;
		p = new T(from);
		return *this;
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
	inline bits::node_map::ptr get_node_map() const {
		return p->get_node_map();
	}

	void output_memory(std::ostream & o) const;
private:
	bits::pipeline_base * p;
};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_PIPELINE_H__
