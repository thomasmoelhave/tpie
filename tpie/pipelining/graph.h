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

///////////////////////////////////////////////////////////////////////////////
/// \file graph.h  Traverse pipeline graphs
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_PIPELINING_GRAPH_H__
#define __TPIE_PIPELINING_GRAPH_H__

#include <tpie/pipelining/core.h>
#include <tpie/pipelining/tokens.h>
#include <tpie/disjoint_sets.h>
#include <tpie/dummy_progress.h>
#include <vector>
#include <stack>

namespace tpie {

namespace pipelining {

template <typename Graph>
size_t dfs_from(Graph & g, size_t start, size_t time) {
	g.finish_times[start] = time++; // discover time
	std::vector<size_t> & neighbours = g.edges[start];
	for (size_t i = 0; i < neighbours.size(); ++i) {
		if (g.finish_times[neighbours[i]] != 0) continue;
		time = dfs_from(g, neighbours[i], time);
	}
	g.finish_times[start] = time++; // finish time;
	return time;
}

template <typename Graph>
void dfs(Graph & g) {
	// initialize finish times
	for (typename Graph::nodemap_t::iterator i = g.finish_times.begin(); i != g.finish_times.end(); ++i) {
		i->second = 0;
	}
	// dfs from all nodes
	size_t time = 1;
	for (typename Graph::nodemap_t::iterator i = g.finish_times.begin(); i != g.finish_times.end(); ++i) {
		if (i->second != 0) continue;
		time = dfs_from(g, i->first, time);
	}
}

struct phase {
	inline phase()
		: m_memoryFraction(0.0)
		, m_minimumMemory(0)
		, m_initiator(0)
	{
	}

	inline void set_initiator(pipe_segment * s) {
		m_initiator = s;
	}

	inline bool is_initiator(pipe_segment * s) {
		segment_map::ptr m = s->get_segment_map();
		segment_map::id_t id = s->get_id();
		return m->in_degree(id, pushes) == 0 && m->in_degree(id, pulls) == 0;
	}

	inline void add(pipe_segment * s) {
		if (count(s)) return;
		if (is_initiator(s)) set_initiator(s);
		m_segments.push_back(s);
		m_memoryFraction += s->get_memory_fraction();
		m_minimumMemory += s->get_minimum_memory();
	}

	inline size_t count(pipe_segment * s) {
		for (size_t i = 0; i < m_segments.size(); ++i) {
			if (m_segments[i] == s) return 1;
		}
		return 0;
	}

	inline void go(progress_indicator_base & pi) const {
		m_initiator->go(pi);
	}

	inline void evacuate_all() const {
		for (size_t i = 0; i < m_segments.size(); ++i) {
			if (m_segments[i]->can_evacuate())
				m_segments[i]->evacuate();
		}
	}

	inline double memory_fraction() const {
		return m_memoryFraction;
	}

	inline memory_size_type minimum_memory() const {
		return m_minimumMemory;
	}

	void assign_memory(memory_size_type m) const;

	inline const std::string & get_name() const {
		priority_type highest = std::numeric_limits<priority_type>::min();
		size_t highest_segment = 0;
		for (size_t i = 0; i < m_segments.size(); ++i) {
			if (m_segments[i]->get_name_priority() > highest) {
				highest_segment = i;
				highest = m_segments[i]->get_name_priority();
			}
		}
		return m_segments[highest_segment]->get_name();
	}

	inline std::string get_unique_id() const {
		std::stringstream uid;
		for (size_t i = 0; i < m_segments.size(); ++i) {
			uid << typeid(*m_segments[i]).name() << ':';
		}
		return uid.str();
	}

private:
	std::vector<pipe_segment *> m_segments;
	double m_memoryFraction;
	memory_size_type m_minimumMemory;
	pipe_segment * m_initiator;

	inline void assign_minimum_memory() const {
		for (size_t i = 0; i < m_segments.size(); ++i) {
			m_segments[i]->set_available_memory(m_segments[i]->get_minimum_memory());
		}
	}
};

struct phasegraph {
	typedef std::map<size_t, int> nodemap_t;
	typedef std::map<size_t, std::vector<size_t> > edgemap_t;
	nodemap_t finish_times;
	edgemap_t edges;

	phasegraph(tpie::disjoint_sets<size_t> & phases, size_t ids) {
		for (size_t i = 0; i < ids; ++i) {
			if (!phases.is_set(i)) continue;
			size_t rep = phases.find_set(i);
			if (edges.count(rep)) continue;
			edges.insert(make_pair(rep, std::vector<size_t>()));
			finish_times.insert(std::make_pair(rep, 0));
		}
	}

	inline void depends(size_t depender, size_t dependee) {
		edges[depender].push_back(dependee);
	}

	inline bool is_depending(size_t depender, size_t dependee) {
		for (size_t i = 0; i < edges[depender].size(); ++i) {
			if (edges[depender][i] == dependee) return true;
		}
		return false;
	}

	std::vector<size_t> execution_order();
};

struct graph_traits {
	typedef segment_map::id_t id_t;
	typedef std::vector<phase> phases_t;
	typedef phases_t::iterator phaseit;
	typedef progress_types<true> Progress;

	graph_traits(const segment_map & map)
		: map(map)
	{
		map.assert_authoritative();
		calc_phases();
	}

	double sum_memory() {
		double sum = 0.0;
		for (size_t i = 0; i < m_phases.size(); ++i) {
			sum += m_phases[i].memory_fraction();
		}
		return sum;
	}

	memory_size_type sum_minimum_memory() {
		memory_size_type sum = 0;
		for (size_t i = 0; i < m_phases.size(); ++i) {
			sum += m_phases[i].minimum_memory();
		}
		return sum;
	}

	const phases_t & phases() {
		return m_phases;
	}

	void go_all(stream_size_type n, Progress::base & pi);

private:
	const segment_map & map;
	phases_t m_phases;
	std::vector<bool> m_evacuatePrevious;

	void calc_phases();

};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_GRAPH_H__
