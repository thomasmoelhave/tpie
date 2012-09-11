// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
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

#ifndef __TPIE_PIPELINING_PIPE_SEGMENT_H__
#define __TPIE_PIPELINING_PIPE_SEGMENT_H__

#include <tpie/pipelining/exception.h>
#include <tpie/pipelining/tokens.h>
#include <tpie/progress_indicator_base.h>
#include <tpie/progress_indicator_null.h>
#include <boost/any.hpp>

namespace tpie {

namespace pipelining {

// Name priorities
typedef int priority_type;
const priority_type PRIORITY_NO_NAME = 0;
const priority_type PRIORITY_INSIGNIFICANT = 5;
const priority_type PRIORITY_SIGNIFICANT = 10;
const priority_type PRIORITY_USER = 20;

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

	inline memory_size_type get_minimum_memory() const {
		return m_minimumMemory;
	}

	inline memory_size_type get_available_memory() const {
		return m_availableMemory;
	}

	inline void set_memory_fraction(double f) {
		m_memoryFraction = f;
	}

	inline double get_memory_fraction() const {
		return m_memoryFraction;
	}

	inline segment_map::ptr get_segment_map() const {
		return token.get_map();
	}

	inline segment_token::id_t get_id() const {
		return token.id();
	}

	virtual void begin() {
		forward_all();
	}

	virtual void go() {
		progress_indicator_null pi;
		go(pi);
		// if go didn't throw, it was overridden - but it shouldn't be
		log_warning() << "pipe_segment subclass " << typeid(*this).name() << " uses old go() interface" << std::endl;
	}

	// Overriding this method is deprecated
	virtual void go(progress_indicator_base &) {
		log_warning() << "pipe_segment subclass " << typeid(*this).name() << " is not an initiator segment" << std::endl;
		throw not_initiator_segment();
	}

	virtual void end() {
	}

	virtual bool can_evacuate() {
		return false;
	}

	virtual void evacuate() {
	}

	inline priority_type get_name_priority() {
		return m_namePriority;
	}

	inline const std::string & get_name() {
		return m_name;
	}

	inline void set_name(const std::string & name, priority_type priority = PRIORITY_USER) {
		m_name = name;
		m_namePriority = priority;
	}

	inline void set_breadcrumb(const std::string & breadcrumb) {
		m_name = m_name.empty() ? breadcrumb : (breadcrumb + " | " + m_name);
	}

	// Called by segment_map
	inline void add_successor(pipe_segment * succ) {
		m_successors.push_back(succ);
	}

	inline stream_size_type get_steps() {
		return m_stepsTotal;
	}

	inline void set_progress_indicator(progress_indicator_base * pi) {
		m_pi = pi;
	}

protected:
	inline pipe_segment()
		: token(this)
		, m_minimumMemory(0)
		, m_availableMemory(0)
		, m_memoryFraction(1.0)
		, m_namePriority(PRIORITY_NO_NAME)
		, m_stepsTotal(0)
		, m_stepsLeft(0)
		, m_pi(0)
	{
	}

	inline pipe_segment(const pipe_segment & other)
		: token(other.token, this)
		, m_minimumMemory(other.m_minimumMemory)
		, m_availableMemory(other.m_availableMemory)
		, m_memoryFraction(other.m_memoryFraction)
		, m_name(other.m_name)
		, m_namePriority(other.m_namePriority)
		, m_stepsTotal(other.m_stepsTotal)
		, m_stepsLeft(other.m_stepsLeft)
		, m_pi(other.m_pi)
	{
	}

	inline pipe_segment(const segment_token & token)
		: token(token, this, true)
		, m_minimumMemory(0)
		, m_availableMemory(0)
		, m_memoryFraction(1.0)
		, m_namePriority(PRIORITY_NO_NAME)
		, m_stepsTotal(0)
		, m_stepsLeft(0)
		, m_pi(0)
	{
	}

	inline void add_push_destination(const segment_token & dest) {
		segment_map::ptr m = token.map_union(dest);
		m->add_relation(token.id(), dest.id(), pushes);
	}

	inline void add_push_destination(const pipe_segment & dest) {
		add_push_destination(dest.token);
	}

	inline void add_pull_destination(const segment_token & dest) {
		segment_map::ptr m = token.map_union(dest);
		m->add_relation(token.id(), dest.id(), pulls);
	}

	inline void add_pull_destination(const pipe_segment & dest) {
		add_pull_destination(dest.token);
	}

	inline void add_dependency(const segment_token & dest) {
		segment_map::ptr m = token.map_union(dest);
		m->add_relation(token.id(), dest.id(), depends);
	}

	inline void add_dependency(const pipe_segment & dest) {
		add_dependency(dest.token);
	}

	inline void set_minimum_memory(memory_size_type minimumMemory) {
		m_minimumMemory = minimumMemory;
	}

	virtual void set_available_memory(memory_size_type availableMemory) {
		m_availableMemory = availableMemory;
	}

	template <typename T>
	inline void forward(std::string key, T value) {
		for (size_t i = 0; i < m_successors.size(); ++i) {
			m_successors[i]->m_values[key] = value;
		}
	}

	inline void forward_all() {
		for (valuemap::iterator i = m_values.begin(); i != m_values.end(); ++i) {
			forward(i->first, i->second);
		}
	}

	inline bool can_fetch(std::string key) {
		return m_values.count(key) != 0;
	}

	inline boost::any fetch_any(std::string key) {
		return m_values[key];
	}

	template <typename T>
	inline T fetch(std::string key) {
		return boost::any_cast<T>(m_values[key]);
	}

	const segment_token & get_token() {
		return token;
	}

	void set_steps(stream_size_type steps) {
		m_stepsTotal = m_stepsLeft = steps;
	}

	void step() {
		if (m_stepsLeft == 0) {
			log_warning() << typeid(*this).name() << " ==== Too many steps!" << std::endl;
		}
		--m_stepsLeft;
		m_pi->step();
	}

	friend class phase;

private:
	segment_token token;

	memory_size_type m_minimumMemory;
	memory_size_type m_availableMemory;
	double m_memoryFraction;

	std::string m_name;
	priority_type m_namePriority;

	std::vector<pipe_segment *> m_successors;
	typedef std::map<std::string, boost::any> valuemap;
	valuemap m_values;

	stream_size_type m_stepsTotal;
	stream_size_type m_stepsLeft;
	progress_indicator_base * m_pi;
};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_PIPE_SEGMENT_H__
