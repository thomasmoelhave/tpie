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
#include <set>

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

	void print_memory(double c, std::ostream & os);

private:
	const std::vector<node *> & m_nodes;
	memory_size_type m_minimumMemory;
	memory_size_type m_maximumMemory;
	double m_fraction;
};

///////////////////////////////////////////////////////////////////////////////
/// Helper methods for memory assignment.
/// The memory assignment algorithm is in runtime::get_memory_factor.
///////////////////////////////////////////////////////////////////////////////
class datastructure_runtime {
public:
	datastructure_runtime(const std::vector<std::vector<node *> > & phases, node_map & nodeMap);

	memory_size_type sum_minimum_memory(size_t phase) const; // sum the minimum memory for datastructures used in the phase
	double sum_fraction(size_t i) const; // sum the fractions for datastructures used in phase i
	memory_size_type sum_assigned_memory(double factor, size_t phase) const; // sum the assigned memory for datastructures used in the phase
	void minimize_factor(double factor, size_t phase); // the factor for the datastructure in the phase is set to be no higher than the given factor
	memory_size_type sum_assigned_memory(size_t phase) const; // sum the assigned memory for datastructures used in the phase using the factors given to the minimize_factor method
	void assign_memory();

	//void print_memory(double c, std::ostream & os);
private:
	static memory_size_type clamp(memory_size_type lo, memory_size_type hi, double v);

	struct datastructure_info_t {
		memory_size_type min;
		memory_size_type max;
		double priority;
		memory_size_type right_most_phase;
		memory_size_type left_most_phase;
		double factor;

		datastructure_info_t()
		: min(0)
		, max(std::numeric_limits<memory_size_type>::max())
		, priority(1)
		, right_most_phase(0)
		, left_most_phase(std::numeric_limits<memory_size_type>::max())
		, factor(std::numeric_limits<double>::max())
		{}
	};

	std::map<std::string, datastructure_info_t> m_datastructures;
	node_map & m_nodeMap;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  Execute the pipeline contained in a node_map.
///////////////////////////////////////////////////////////////////////////////
class runtime {
	node_map & m_nodeMap;

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Construct a runtime object.
	///
	/// Does nothing other than copy the smart pointer given.
	///////////////////////////////////////////////////////////////////////////
	runtime(node_map::ptr nodeMap);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Number of nodes contained in node map.
	///
	/// Returns m_nodeMap.size().
	///////////////////////////////////////////////////////////////////////////
	size_t get_node_count();

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Execute the pipeline.
	///
	/// This is the main entry point. The method go() sets up all nodes for
	/// execution and executes all initiators in turn:
	///
	/// Call node::prepare in item source to item sink order for each phase.
	///
	/// Assign memory according to memory constraints and memory priorities.
	///
	/// For each phase, call propagate, begin, go and end on nodes as
	/// appropriate. We call propagate in item source to item sink order;
	/// we call begin in leaf to root actor order; we call end in root to leaf
	/// actor order.
	///////////////////////////////////////////////////////////////////////////
	void go(stream_size_type items,
			progress_indicator_base & progress,
			memory_size_type memory);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Get all sources of the item flow graph.
	///
	/// An item source node has no ingoing edges in the item flow graph
	/// of its phase, and its phase does not depend on any phases.
	///
	/// This is the set of nodes used when forwarding out-of-band data into
	/// the pipeline.
	///////////////////////////////////////////////////////////////////////////
	void get_item_sources(std::vector<node *> & itemSources);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Get all sinks of the item flow graph.
	///
	/// An item sink node has no outgoing edges in the item flow graph
	/// of its phase, and no phase depends on its phase.
	///
	/// This is the set of nodes used when fetching out-of-band data from
	/// the pipeline.
	///////////////////////////////////////////////////////////////////////////
	void get_item_sinks(std::vector<node *> & itemSinks);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Partition nodes into phases (using union-find).
	///////////////////////////////////////////////////////////////////////////
	void get_phase_map(std::map<node *, size_t> & phaseMap);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Set up phase graph so we can find a topological order.
	///////////////////////////////////////////////////////////////////////////
	void get_phase_graph(const std::map<node *, size_t> & phaseMap,
						 graph<size_t> & phaseGraph);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compute the inverse of a permutation.
	///
	/// A permutation of N elements is given as a std::vector of size N,
	/// in which each entry maps to a distinct integer in [0, N).
	/// The inverse permutation of f is g if and only if
	/// f[g[i]] == g[f[i]] == i  for all i in [0,N).
	///////////////////////////////////////////////////////////////////////////
	static std::vector<size_t> inverse_permutation(const std::vector<size_t> & f);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compute topological phase order.
	///
	/// The vector phases[i] will contain the nodes in the ith phase to run.
	/// If no node in phases[i] has a dependency to a node in phases[i-1],
	/// evacuateWhenDone[i] is set to true.
	///////////////////////////////////////////////////////////////////////////
	void get_phases(const std::map<node *, size_t> & phaseMap,
					const graph<size_t> & phaseGraph,
					std::vector<bool> & evacuateWhenDone,
					std::vector<std::vector<node *> > & phases);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Internal method used by go().
	///////////////////////////////////////////////////////////////////////////
	void get_item_flow_graphs(std::vector<std::vector<node *> > & phases,
							  std::vector<graph<node *> > & itemFlow);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Internal method used by go().
	///////////////////////////////////////////////////////////////////////////
	void get_actor_graphs(std::vector<std::vector<node *> > & phases,
						  std::vector<graph<node *> > & actors);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Internal method used by get_{actor,item_flow}_graphs().
	///////////////////////////////////////////////////////////////////////////
	void get_graph(std::vector<node *> & phase, graph<node *> & result,
				   bool itemFlow);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Check if the node is a phase initiator.
	///
	/// A node is a phase initiator if it has no ingoing edges in the actor
	/// graph, or in other words if no node pushes to it or pulls from it.
	///////////////////////////////////////////////////////////////////////////
	bool is_initiator(node * n);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Equivalent to any_of(begin(phase), end(phase), is_initiator).
	///////////////////////////////////////////////////////////////////////////
	bool has_initiator(const std::vector<node *> & phase);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Ensure that all phases have at least one initiator.
	///
	/// If a phase has no initiators, throw no_initiator_node().
	///////////////////////////////////////////////////////////////////////////
	void ensure_initiators(const std::vector<std::vector<node *> > & phases);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Call prepare on all nodes in item source to sink order.
	///////////////////////////////////////////////////////////////////////////
	void prepare_all(const std::vector<graph<node *> > & itemFlow);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Call evacuate on all nodes for which can_evacuate() is true.
	///////////////////////////////////////////////////////////////////////////
	void evacuate_all(const std::vector<node *> & phase);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Call propagate on all nodes in item source to sink order.
	///////////////////////////////////////////////////////////////////////////
	void propagate_all(const graph<node *> & itemFlow);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Call set_progress_indicator on all nodes in the phase.
	///////////////////////////////////////////////////////////////////////////
	void set_progress_indicators(const std::vector<node *> & phase,
								 progress_indicator_base & pi);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Call go() on all initiators after setting the given progress
	/// indicator.
	///////////////////////////////////////////////////////////////////////////
	void go_initiators(const std::vector<node *> & phase);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Internal method used by go().
	///////////////////////////////////////////////////////////////////////////
	static void assign_memory(const std::vector<std::vector<node *> > & phases,
							  memory_size_type memory, node_map & nodeMap);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Internal method used by assign_memory().
	///////////////////////////////////////////////////////////////////////////
	static double get_memory_factor(memory_size_type memory,
									memory_size_type phase,
									const memory_runtime & mrt,
									const datastructure_runtime & drt,
									bool datastructures_locked);

};

}

}

}

#endif // TPIE_PIPELINING_RUNTIME_H
