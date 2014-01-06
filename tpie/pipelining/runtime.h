// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2013, 2014, The TPIE development team
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

#ifndef TPIE_PIPELINING_RUNTIME_H
#define TPIE_PIPELINING_RUNTIME_H

#include <tpie/fractional_progress.h>
#include <tpie/pipelining/tokens.h>

namespace tpie {

namespace pipelining {

namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \brief  Directed graph with nodes of type T.
///
/// The node set is implied by the endpoints of the edges.
///
/// Computes the topological order using depth first search.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class graph {
public:
	void add_node(T v) {
		m_nodes.insert(v);
		m_edgeLists[v]; // ensure v has an edge list
	}

	void add_edge(T u, T v) {
		add_node(u);
		add_node(v);
		m_edgeLists[u].push_back(v);
	}

	size_t size() const {
		return m_nodes.size();
	}

	bool has_edge(T u, T v) const {
		const std::vector<T> & edgeList = m_edgeLists.find(u)->second;
		return std::find(edgeList.begin(), edgeList.end(), v) != edgeList.end();
	}

	void topological_order(std::vector<T> & result) const {
		const size_t N = m_nodes.size();
		depth_first_search dfs(m_edgeLists);
		std::vector<std::pair<size_t, T> > nodes(N);
		for (typename std::map<T, std::vector<T> >::const_iterator i = m_edgeLists.begin();
			 i != m_edgeLists.end(); ++i)
			nodes.push_back(std::make_pair(dfs.visit(i->first), i->first));
		std::sort(nodes.begin(), nodes.end(), std::greater<std::pair<size_t, T> >());
		result.resize(N);
		for (size_t i = 0; i < N; ++i) result[i] = nodes[i].second;
	}

private:
	std::set<T> m_nodes;
	std::map<T, std::vector<T> > m_edgeLists;

	class depth_first_search {
	public:
		depth_first_search(const std::map<T, std::vector<T> > & edgeLists)
			: m_time(0)
			, m_edgeLists(edgeLists)
		{
		}

		size_t visit(T u) {
			if (m_finishTime.count(u)) return m_finishTime[u];
			m_finishTime[u] = 0;
			++m_time;
			const std::vector<T> & edgeList = get_edge_list(u);
			for (size_t i = 0; i < edgeList.size(); ++i) visit(edgeList[i]);
			return m_finishTime[u] = m_time++;
		}

	private:
		const std::vector<T> & get_edge_list(T u) {
			typename std::map<T, std::vector<T> >::const_iterator i = m_edgeLists.find(u);
			if (i == m_edgeLists.end())
				throw tpie::exception("get_edge_list: no such node");
			return i->second;
		}

		size_t m_time;
		const std::map<T, std::vector<T> > & m_edgeLists;
		std::map<T, size_t> m_finishTime;
	};
};

///////////////////////////////////////////////////////////////////////////////
/// Helper methods for memory assignment.
/// The memory assignment algorithm is in runtime::get_memory_factor.
///////////////////////////////////////////////////////////////////////////////
class memory_runtime {
public:
	memory_runtime(const std::vector<node *> & nodes);

	// Node accessors
	memory_size_type minimum_memory(size_t i) const;
	memory_size_type maximum_memory(size_t i) const;
	double fraction(size_t i) const;

	// Node accessor aggregates
	memory_size_type sum_minimum_memory() const;
	memory_size_type sum_maximum_memory() const;
	double sum_fraction() const;

	// Node mutator
	void set_memory(size_t i, memory_size_type mem);

	void assign_memory(double factor);

	// Special case of assign_memory when factor is zero.
	void assign_minimum_memory();

	memory_size_type sum_assigned_memory(double factor) const;

	memory_size_type get_assigned_memory(size_t i, double factor) const;

	static memory_size_type clamp(memory_size_type lo, memory_size_type hi,
								  double v);

private:
	const std::vector<node *> & m_nodes;
	memory_size_type m_minimumMemory;
	memory_size_type m_maximumMemory;
	double m_fraction;
};

class runtime {
	node_map & m_nodeMap;

public:
	runtime(node_map::ptr nodeMap);

	size_t get_node_count();

	void go(stream_size_type items,
			progress_indicator_base & progress,
			memory_size_type memory);

	void get_item_sources(std::vector<node *> & itemSources);

	void get_item_sinks(std::vector<node *> & itemSinks);

	void get_phase_map(std::map<node *, size_t> & phaseMap);

	void get_phase_graph(const std::map<node *, size_t> & phaseMap,
						 graph<size_t> & phaseGraph);

	// Compute the inverse of a permutation f : {0, 1, ... N-1} -> {0, 1, ... N-1}
	static std::vector<size_t> inverse_permutation(const std::vector<size_t> & f);

	void get_phases(const std::map<node *, size_t> & phaseMap,
					const graph<size_t> & phaseGraph,
					std::vector<bool> & evacuateWhenDone,
					std::vector<std::vector<node *> > & phases);

	void get_item_flow_graphs(std::vector<std::vector<node *> > & phases,
							  std::vector<graph<node *> > & itemFlow);

	void get_actor_graphs(std::vector<std::vector<node *> > & phases,
						  std::vector<graph<node *> > & actors);

	void get_graph(std::vector<node *> & phase, graph<node *> & result,
				   bool itemFlow);

	bool is_initiator(node * n);

	bool has_initiator(const std::vector<node *> & phase);

	void ensure_initiators(const std::vector<std::vector<node *> > & phases);

	void prepare_all(const std::vector<graph<node *> > & itemFlow);

	void propagate_all(const graph<node *> & itemFlow);

	void go_initators(const std::vector<node *> & phase, progress_indicator_base & pi);

	static void assign_memory(const std::vector<std::vector<node *> > & phases,
							  memory_size_type memory);

	static double get_memory_factor(const memory_runtime & rt, memory_size_type memory);

};

}

}

}

#endif // TPIE_PIPELINING_RUNTIME_H
