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

#include <tpie/pipelining/tokens.h>
#include <tpie/disjoint_sets.h>
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
	typedef size_t phase_t;

	graph_traits(const segment_map & map)
		: map(map)
	{
		map.assert_authoritative();
		calc_phases();
	}

	const std::vector<id_t> & execution_order() {
		return exec;
	}

private:
	const segment_map & map;
	std::vector<id_t> exec;
	std::map<id_t, phase_t> m_phases;

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

		for (size_t i = 0; i < internalexec.size(); ++i) {
			// all segments with phase number internalexec[i] should be executed in phase i

			// first, insert phase representatives
			m_phases.insert(std::make_pair(ids_inv[internalexec[i]], i));
		}
		exec.resize(internalexec.size());
		for (ids_inv_t::iterator i = ids_inv.begin(); i != ids_inv.end(); ++i) {
			phase_t thisphase = m_phases[ids_inv[phases.find_set(i->first)]];
			// map this node to the same execution phase as the phase representative
			m_phases.insert(std::make_pair(i->second, thisphase));

			// is this the phase initiator?
			if (map.in_degree(i->second, pushes) + map.in_degree(i->second, pulls) == 0) {
				// nobody pushes to or pulls from this
				exec[thisphase] = i->second;
			}
		}
	}

};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_GRAPH_H__
