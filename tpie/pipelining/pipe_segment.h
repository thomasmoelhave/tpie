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

namespace tpie {

namespace pipelining {

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

	inline memory_size_type get_minimum_memory() {
		return m_minimumMemory;
	}

	inline memory_size_type get_available_memory() {
		return m_availableMemory;
	}

	inline void set_memory_fraction(double f) {
		m_memoryFraction = f;
	}

	inline double get_memory_fraction() {
		return m_memoryFraction;
	}

	inline segment_map::ptr get_segment_map() const {
		return token.get_map();
	}

	virtual void go() {
		TP_LOG_WARNING("pipe_segment subclass " << typeid(*this).name() << " is not an initiator segment" << std::endl);
		throw not_initiator_segment();
	}

protected:
	inline pipe_segment()
		: token(this)
		, m_minimumMemory(0)
		, m_availableMemory(0)
		, m_memoryFraction(0.0)
	{
	}

	inline pipe_segment(const pipe_segment & other)
		: token(other.token, this)
		, m_minimumMemory(other.m_minimumMemory)
		, m_availableMemory(other.m_availableMemory)
		, m_memoryFraction(other.m_memoryFraction)
	{
	}

	inline pipe_segment(const segment_token & token)
		: token(token, this, true)
		, m_minimumMemory(0)
		, m_availableMemory(0)
		, m_memoryFraction(0.0)
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

	inline void set_available_memory(memory_size_type availableMemory) {
		m_availableMemory = availableMemory;
	}

private:
	memory_size_type memory;
	segment_token token;

	memory_size_type m_minimumMemory;
	memory_size_type m_availableMemory;
	double m_memoryFraction;
};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_PIPE_SEGMENT_H__
