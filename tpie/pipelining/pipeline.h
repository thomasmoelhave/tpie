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

#include <tpie/tpie_export.h>
#include <tpie/types.h>
#include <iostream>
#include <tpie/pipelining/tokens.h>
#include <tpie/progress_indicator_null.h>
#include <tpie/file_manager.h>
#include <tpie/memory.h>
#include <unordered_set>
#include <mutex>

namespace tpie::pipelining {
namespace bits {


///////////////////////////////////////////////////////////////////////////////
/// \class pipeline_base
/// Virtual superclass for pipelines and subpipelines
///////////////////////////////////////////////////////////////////////////////
class TPIE_EXPORT pipeline_base_base {
public:
	pipeline_base_base();
	pipeline_base_base(const pipeline_base_base &) = default;
	pipeline_base_base & operator=(const pipeline_base_base &) = default;
	pipeline_base_base(pipeline_base_base &&) = default;
	pipeline_base_base & operator=(pipeline_base_base &&) = default;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Generate a GraphViz plot of the pipeline
	///
	/// When rendered with dot, GraphViz will place the nodes in the
	/// topological order of the item flow graph with items flowing from the
	/// top downwards.
	///
	/// Thus, a downwards arrow in the plot is a push edge, and an upwards
	/// arrow is a pull edge (assuming no cycles in the item flow graph).
	///
	/// Compared to plot_full, sorts, buffers and reversers will be represented
	/// as one node in the graph as apposed to 3 or 2. Nodes added by
	/// virtual wrapping will not be showed at all
	///
	///////////////////////////////////////////////////////////////////////////
	void plot(std::ostream & out) {plot_impl(out, false);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Generate a GraphViz plot of the actor graph.
	///
	/// When rendered with dot, GraphViz will place the nodes in the
	/// topological order of the item flow graph with items flowing from the
	/// top downwards.
	///
	/// Thus, a downwards arrow in the plot is a push edge, and an upwards
	/// arrow is a pull edge (assuming no cycles in the item flow graph).
	///////////////////////////////////////////////////////////////////////////
	void plot_full(std::ostream & out) {plot_impl(out, true);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Virtual dtor.
	///////////////////////////////////////////////////////////////////////////
	virtual ~pipeline_base_base() {}

	void forward_any(std::string key, any_noncopyable value);
	
	bool can_fetch(std::string key);

	any_noncopyable & fetch_any(std::string key);

	node_map::ptr get_node_map() const {
		return m_nodeMap;
	}

	void output_memory(std::ostream & o) const;

	size_t uid() const {return m_uid;};
protected:
	
	node_map::ptr m_nodeMap;	
	size_t m_uid;
private:
	void plot_impl(std::ostream & out, bool full);	
};
	

///////////////////////////////////////////////////////////////////////////////
/// \class pipeline_base
/// Virtual superclass for pipelines implementing the function call operator.
///////////////////////////////////////////////////////////////////////////////
class TPIE_EXPORT pipeline_base: public pipeline_base_base {
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Invoke the pipeline.
	///////////////////////////////////////////////////////////////////////////
	void operator()(stream_size_type items, progress_indicator_base & pi,
					memory_size_type filesAvailable, memory_size_type mem,
					const char * file, const char * function);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Invoke the pipeline with amount of available files automatically
	/// configured.
	///////////////////////////////////////////////////////////////////////////
	void operator()(stream_size_type items, progress_indicator_base & pi,
					memory_size_type mem,
					const char * file, const char * function) {
		operator()(items, pi, get_file_manager().available(), mem, file, function);
	}

	double memory() const {
		return m_memory;
	}

	void order_before(pipeline_base & other);
protected:
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
	typedef typename fact_t::constructed_type gen_t;

	pipeline_impl(fact_t & factory) {
		this->m_memory = factory.memory();

		auto n = std::unique_ptr<gen_t>(new gen_t(factory.construct()));
		this->m_nodeMap = n->get_node_map()->find_authority();
		this->m_nodeMap->increment_pipeline_ref();
		this->m_nodeMap->add_owned_node(std::move(n));
	}

	pipeline_impl(const pipeline_impl & o) = delete;
	pipeline_impl & operator=(const pipeline_impl & o) = delete;

	pipeline_impl(pipeline_impl && o) = default;
	pipeline_impl & operator=(pipeline_impl && o) = default;

	~pipeline_impl() override {
		if (this->m_nodeMap) {
			this->m_nodeMap->find_authority()->decrement_pipeline_ref();
		}
	}
};

} // namespace bits


TPIE_EXPORT extern std::unordered_set<bits::pipeline_base_base *> current_pipelines;
TPIE_EXPORT extern std::mutex current_pipelines_mutex;

///////////////////////////////////////////////////////////////////////////////
/// \class pipeline
///
/// This class is used to avoid writing the template argument in the
/// pipeline_impl type.
///////////////////////////////////////////////////////////////////////////////
class TPIE_EXPORT pipeline {
public:
	pipeline() {}
	pipeline(pipeline &&) = default;
	pipeline(const pipeline &) = default;
	pipeline & operator=(pipeline &&) = default;
	pipeline & operator=(const pipeline &) = default;

	template <typename T>
	pipeline(T from) {
		*this = std::move(from);
	}

	template <typename T>
	pipeline & operator=(T from) {
		p.reset(new T(std::move(from)));
		return *this;
	}

	pipeline(const std::shared_ptr<bits::pipeline_base> & p): p(p) {}

	void operator()() {
		progress_indicator_null pi;
		(*p)(1, pi, get_file_manager().available(), get_memory_manager().available(), nullptr, nullptr);
	}

	void operator()(stream_size_type items, progress_indicator_base & pi,
					const char * file, const char * function) {
		(*p)(items, pi, get_file_manager().available(), get_memory_manager().available(), file, function);
	}

	void operator()(stream_size_type items, progress_indicator_base & pi,
					memory_size_type mem,
					const char * file, const char * function) {
		(*p)(items, pi, get_file_manager().available(), mem, file, function);
	}

	void operator()(stream_size_type items, progress_indicator_base & pi,
			memory_size_type filesAvailable, memory_size_type mem,
					const char * file, const char * function) {
		(*p)(items, pi, filesAvailable, mem, file, function);
	}
	
	void plot(std::ostream & os = std::cout) {
		p->plot(os);
	}

	void plot_full(std::ostream & os = std::cout) {
		p->plot_full(os);
	}
	
	inline double memory() const {
		return p->memory();
	}

	bits::node_map::ptr get_node_map() const {
		return p->get_node_map();
	}

	bool can_fetch(std::string key) {
		return p->can_fetch(key);
	}

	any_noncopyable & fetch_any(std::string key) {
		return p->fetch_any(key);
	}

	template <typename T>
	T & fetch(std::string key) {
		any_noncopyable &a = fetch_any(key);
		return any_cast<T>(a);
	}

	void forward_any(std::string key, any_noncopyable value) {
		p->forward_any(key, std::move(value));
	}

	template <typename T>
	void forward(std::string key, T value) {
		forward_any(key, any_noncopyable(std::move(value)));
	}

	pipeline & then(pipeline & other) {
		p->order_before(*other.p);
		return other;
	}

	void output_memory(std::ostream & o) const {p->output_memory(o);}
private:
	std::shared_ptr<bits::pipeline_base> p;
};

} // namespace tpie::pipelining

#endif // __TPIE_PIPELINING_PIPELINE_H__
