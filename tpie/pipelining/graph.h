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

	inline void assign_memory(memory_size_type m) const {
		if (m < minimum_memory()) {
			TP_LOG_WARNING_ID("Not enough memory for this phase. We have " << m << " but we require " << minimum_memory() << '.');
			assign_minimum_memory();
			return;
		}
		memory_size_type remaining = m;
		double fraction = memory_fraction();
		//std::cout << "Remaining " << m << " fraction " << fraction << " segments " << m_segments.size() << std::endl;
		std::vector<char> assigned(m_segments.size());
		while (true) {
			bool done = true;
			for (size_t i = 0; i < m_segments.size(); ++i) {
				if (assigned[i]) continue;
				pipe_segment * s = m_segments[i];
				memory_size_type min = s->get_minimum_memory();
				double frac = s->get_memory_fraction();
				double to_assign = frac/fraction * remaining;
				if (to_assign < min) {
					done = false;
					s->set_available_memory(min);
					assigned[i] = true;
					remaining -= min;
					fraction -= frac;
				}
			}
			if (!done) continue;
			for (size_t i = 0; i < m_segments.size(); ++i) {
				if (assigned[i]) continue;
				pipe_segment * s = m_segments[i];
				double frac = s->get_memory_fraction();
				double to_assign = frac/fraction * remaining;
				s->set_available_memory(static_cast<memory_size_type>(to_assign));
			}
			break;
		}
	}

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

	std::vector<size_t> execution_order() {
		dfs(*this);
		std::vector<std::pair<int, size_t> > nodes;
		for (nodemap_t::iterator i = finish_times.begin(); i != finish_times.end(); ++i) {
			nodes.push_back(std::make_pair(-i->second, i->first));
		}
		std::sort(nodes.begin(), nodes.end());
		std::vector<size_t> result(nodes.size());
		size_t j = 0;
		for (size_t i = nodes.size(); i--;) {
			result[j++] = nodes[i].second;
		}
		return result;
	}
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

	void go_all(stream_size_type n, Progress::base & pi) {
		Progress::fp fp(&pi);
		array<auto_ptr<Progress::sub> > subindicators(m_phases.size());
		for (size_t i = 0; i < m_phases.size(); ++i) {
			phase & curphase = m_phases[i];
			std::string name = curphase.get_name();
			if (0 == name.size()) log_error() << "Phase has no name" << std::endl;
			std::string uid = curphase.get_unique_id();
			subindicators[i].reset(tpie_new<Progress::sub>(fp, uid.c_str(), TPIE_FSI, n, name.c_str()));
		}

		fp.init();
		for (size_t i = 0; i < m_phases.size(); ++i) {
			if (m_evacuatePrevious[i]) m_phases[i-1].evacuate_all();
			m_phases[i].go(*subindicators[i]);
		}
		fp.done();
	}

private:
	const segment_map & map;
	phases_t m_phases;
	std::vector<bool> m_evacuatePrevious;

	void calc_phases() {
		map.assert_authoritative();
		typedef std::map<id_t, size_t> ids_t;
		typedef std::map<size_t, id_t> ids_inv_t;
		ids_t ids;
		ids_inv_t ids_inv;
		size_t nextid = 0;
		for (segment_map::mapit i = map.begin(); i != map.end(); ++i) {
			ids.insert(std::make_pair(i->first, nextid));
			ids_inv.insert(std::make_pair(nextid, i->first));
			++nextid;
		}
		tpie::disjoint_sets<size_t> phases(nextid);
		for (size_t i = 0; i < nextid; ++i) phases.make_set(i);

		const segment_map::relmap_t relations = map.get_relations();
		for (segment_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
			if (i->second.second != depends) phases.union_set(ids[i->first], ids[i->second.first]);
		}
		// `phases` holds a map from segment to phase number

		phasegraph g(phases, nextid);

		// establish phase relationships
		for (segment_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
			if (i->second.second == depends) g.depends(phases.find_set(ids[i->first]), phases.find_set(ids[i->second.first]));
		}

		// toposort the phase graph and find the phase numbers in the execution order
		std::vector<size_t> internalexec = g.execution_order();
		m_phases.resize(internalexec.size());
		m_evacuatePrevious.resize(internalexec.size(), false);

		std::vector<bool>::iterator j = m_evacuatePrevious.begin();
		for (size_t i = 0; i < internalexec.size(); ++i, ++j) {
			// all segments with phase number internalexec[i] should be executed in phase i

			// first, insert phase representatives
			m_phases[i].add(map.get(ids_inv[internalexec[i]]));
			*j = i > 0 && !g.is_depending(internalexec[i], internalexec[i-1]);
		}
		for (ids_inv_t::iterator i = ids_inv.begin(); i != ids_inv.end(); ++i) {
			pipe_segment * representative = map.get(ids_inv[phases.find_set(i->first)]);
			pipe_segment * current = map.get(i->second);
			if (current == representative) continue;
			for (size_t i = 0; i < m_phases.size(); ++i) {
				if (m_phases[i].count(representative)) {
					m_phases[i].add(current);
					break;
				}
			}
		}
	}

};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_GRAPH_H__
