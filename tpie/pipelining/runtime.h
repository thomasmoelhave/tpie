// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2013, The TPIE development team
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
#include <tpie/disjoint_sets.h>

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
/// \brief  Helper class for RAII-style progress indicators.
///
/// init calls fractional_progress::init,
/// and the destructor calls fractional_progress::done.
///
/// Instantiate phase_progress_indicators
/// to call init and done on subindicators.
///////////////////////////////////////////////////////////////////////////////
class progress_indicators {
public:
	progress_indicators() {
	}

	~progress_indicators() {
		if (fp) fp->done();
		for (size_t i = 0; i < m_progressIndicators.size(); ++i) {
			delete m_progressIndicators[i];
		}
		m_progressIndicators.resize(0);
		delete fp;
		fp = NULL;
	}

	void init(stream_size_type n, progress_indicator_base & pi, const std::vector<std::vector<node *> > & phases) {
		fp = new fractional_progress(&pi);
		const size_t N = phases.size();
		m_progressIndicators.resize(N);
		for (size_t i = 0; i < N; ++i) {
			std::string uid = get_phase_uid(phases[i]);
			std::string name = get_phase_name(phases[i]);
			m_progressIndicators[i] = new fractional_subindicator(
				*fp, uid.c_str(), TPIE_FSI, n, name.c_str());
		}
		fp->init();
	}

private:
	std::string get_phase_uid(const std::vector<node *> & phase) {
		std::stringstream uid;
		for (size_t i = 0; i < phase.size(); ++i) {
			uid << typeid(*phase[i]).name() << ':';
		}
		return uid.str();
	}

	std::string get_phase_name(const std::vector<node *> & phase) {
		priority_type highest = std::numeric_limits<priority_type>::min();
		size_t highest_node = 0;
		for (size_t i = 0; i < phase.size(); ++i) {
			if (phase[i]->get_name_priority() > highest) {
				highest_node = i;
				highest = phase[i]->get_name_priority();
			}
		}
		return phase[highest_node]->get_name();
	}

	friend class phase_progress_indicator;

	fractional_progress * fp;
	std::vector<fractional_subindicator *> m_progressIndicators;
};

///////////////////////////////////////////////////////////////////////////////
/// RAII-style progress indicator for a single phase.
/// Constructor computes number of steps and calls init; destructor calls done.
///////////////////////////////////////////////////////////////////////////////
class phase_progress_indicator {
public:
	phase_progress_indicator(progress_indicators & pi, size_t phaseNumber,
							 const std::vector<node *> & nodes)
		: m_pi(pi.m_progressIndicators[phaseNumber])
	{
		stream_size_type steps = 0;
		for (size_t j = 0; j < nodes.size(); ++j) {
			steps += nodes[j]->get_steps();
		}
		m_pi->init(steps);
	}

	~phase_progress_indicator() {
		m_pi->done();
	}

	fractional_subindicator & get() {
		return *m_pi;
	}

private:
	fractional_subindicator * m_pi;
};

///////////////////////////////////////////////////////////////////////////////
/// RAII-style begin/end handling on nodes.
/// The constructor calls begin on nodes in the phase in actor graph,
/// and the destructor calls end in the reverse order.
///////////////////////////////////////////////////////////////////////////////
class begin_end {
public:
	begin_end(graph<node *> & actorGraph) {
		actorGraph.topological_order(m_topologicalOrder);
		for (size_t i = m_topologicalOrder.size(); i--;) {
			m_topologicalOrder[i]->set_state(node::STATE_IN_BEGIN);
			m_topologicalOrder[i]->begin();
			m_topologicalOrder[i]->set_state(node::STATE_AFTER_BEGIN);
		}
	}

	~begin_end() {
		for (size_t i = 0; i < m_topologicalOrder.size(); ++i) {
			m_topologicalOrder[i]->set_state(node::STATE_IN_END);
			m_topologicalOrder[i]->end();
			m_topologicalOrder[i]->set_state(node::STATE_AFTER_END);
		}
	}

private:
	std::vector<node *> m_topologicalOrder;
};

///////////////////////////////////////////////////////////////////////////////
/// Helper methods for memory assignment.
/// The memory assignment algorithm is in runtime::assign_phase_memory.
///////////////////////////////////////////////////////////////////////////////
class memory_runtime {
public:
	memory_runtime(const std::vector<node *> & nodes)
		: m_nodes(nodes)
		, m_minimumMemory(0)
		, m_maximumMemory(0)
		, m_fraction(0.0)
	{
		const size_t N = m_nodes.size();
		for (size_t i = 0; i < N; ++i) {
			m_minimumMemory += minimum_memory(i);
			m_maximumMemory += maximum_memory(i);
			m_fraction += fraction(i);
		}
	}

	// Node accessors
	memory_size_type minimum_memory(size_t i) { return m_nodes[i]->get_minimum_memory(); }
	memory_size_type maximum_memory(size_t i) { return m_nodes[i]->get_maximum_memory(); }
	double fraction(size_t i) { return m_nodes[i]->get_memory_fraction(); }

	// Node accessor aggregates
	memory_size_type sum_minimum_memory() { return m_minimumMemory; }
	memory_size_type sum_maximum_memory() { return m_maximumMemory; }
	double sum_fraction() { return m_fraction; }

	// Node mutator
	void set_memory(size_t i, memory_size_type mem) { m_nodes[i]->set_available_memory(mem); }

	void assign_memory(double factor) {
		for (size_t i = 0; i < m_nodes.size(); ++i)
			set_memory(i, get_assigned_memory(i, factor));
	}

	// Special case of assign_memory when factor is zero.
	void assign_minimum_memory() {
		for (size_t i = 0; i < m_nodes.size(); ++i)
			set_memory(i, minimum_memory(i));
	}

	memory_size_type sum_assigned_memory(double factor) {
		memory_size_type memoryAssigned = 0;
		for (size_t i = 0; i < m_nodes.size(); ++i)
			memoryAssigned += get_assigned_memory(i, factor);
		return memoryAssigned;
	}

	memory_size_type get_assigned_memory(size_t i, double factor) {
		return clamp(minimum_memory(i), maximum_memory(i),
					 factor * fraction(i));
	}

	static memory_size_type clamp(memory_size_type lo, memory_size_type hi,
								  double v)
	{
		if (v < lo) return lo;
		if (v > hi) return hi;
		return static_cast<memory_size_type>(v);
	}

private:
	const std::vector<node *> & m_nodes;
	memory_size_type m_minimumMemory;
	memory_size_type m_maximumMemory;
	double m_fraction;
};

class runtime {
	node_map & m_nodeMap;

public:
	runtime(node_map::ptr nodeMap)
		: m_nodeMap(*nodeMap)
	{
	}

	size_t get_node_count() {
		return m_nodeMap.size();
	}

	void go(stream_size_type items,
			progress_indicator_base & progress,
			memory_size_type memory)
	{
		if (get_node_count() == 0)
			throw tpie::exception("no nodes in pipelining graph");

		// Partition nodes into phases (using union-find)
		std::map<node *, size_t> phaseMap;
		get_phase_map(phaseMap);
		if (phaseMap.size() != get_node_count())
			throw tpie::exception("get_phase_map did not return correct number of nodes");

		// Build phase graph
		graph<size_t> phaseGraph;
		get_phase_graph(phaseMap, phaseGraph);

		std::vector<std::vector<node *> > phases;
		std::vector<bool> evacuateWhenDone;
		get_phases(phaseMap, phaseGraph, evacuateWhenDone, phases);

		// Build item flow graph and actor graph for each phase
		std::vector<graph<node *> > itemFlow;
		get_item_flow_graphs(phases, itemFlow);
		std::vector<graph<node *> > actor;
		get_actor_graphs(phases, actor);

		// Check that each phase has at least one initiator
		ensure_initiators(phases);

		// Toposort item flow graph for each phase
		// and call node::prepare in item source to item sink order
		prepare_all(itemFlow);

		// Gather node memory requirements and assign memory to each phase
		assign_memory(phases, memory);

		// Construct fractional progress indicators
		// (getting the name of each phase)
		progress_indicators pi;
		pi.init(items, progress, phases);

		for (size_t i = 0; i < phases.size(); ++i) {
			// Run each phase:
			// call propagate in item source to item sink order
			propagate_all(itemFlow[i]);
			// sum number of steps and call pi.init()
			phase_progress_indicator phaseProgress(pi, i, phases[i]);
			// call begin in leaf to root actor order
			begin_end beginEnd(actor[i]);
			// call go on initiators
			go_initators(phases[i], phaseProgress.get());
			// call end in root to leaf actor order in ~begin_end
			// call pi.done in ~phase_progress_indicator
		}
		// call fp->done in ~progress_indicators
	}

	void get_item_sources(std::vector<node *> & itemSources) {
		typedef node_map::id_t id_t;
		std::set<id_t> possibleSources;
		for (node_map::mapit i = m_nodeMap.begin(); i != m_nodeMap.end(); ++i) {
			possibleSources.insert(i->first);
		}
		const node_map::relmap_t & relations = m_nodeMap.get_relations();
		for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
			if (i->second.second == depends) continue;

			id_t from = i->first;
			id_t to = i->second.first;

			if (i->second.second == pulls) std::swap(from, to);

			possibleSources.erase(to);
		}
		for (std::set<id_t>::iterator i = possibleSources.begin();
			 i != possibleSources.end(); ++i) {
			itemSources.push_back(m_nodeMap.get(*i));
		}
	}

	void get_item_sinks(std::vector<node *> & itemSinks) {
		typedef node_map::id_t id_t;
		std::set<id_t> possibleSinks;
		for (node_map::mapit i = m_nodeMap.begin(); i != m_nodeMap.end(); ++i) {
			possibleSinks.insert(i->first);
		}
		const node_map::relmap_t & relations = m_nodeMap.get_relations();
		for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
			if (i->second.second == depends) continue;

			id_t from = i->first;
			id_t to = i->second.first;

			if (i->second.second == pulls) std::swap(from, to);

			possibleSinks.erase(from);
		}
		for (std::set<id_t>::iterator i = possibleSinks.begin();
			 i != possibleSinks.end(); ++i) {
			itemSinks.push_back(m_nodeMap.get(*i));
		}
	}

	void get_phase_map(std::map<node *, size_t> & phaseMap) {
		std::map<size_t, size_t> numbering;
		std::vector<node *> nodeOrder;
		for (node_map::mapit i = m_nodeMap.begin(); i != m_nodeMap.end(); ++i) {
			numbering[i->second->get_id()] = nodeOrder.size();
			nodeOrder.push_back(i->second);
		}
		const size_t N = nodeOrder.size();

		tpie::disjoint_sets<size_t> unionFind(N);
		for (size_t i = 0; i < N; ++i)
			unionFind.make_set(i);

		const node_map::relmap_t & relations = m_nodeMap.get_relations();
		for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
			if (i->second.second != depends)
				unionFind.union_set(numbering[i->first], numbering[i->second.first]);
		}

		const size_t NIL = N;
		std::vector<size_t> phaseNumber(N, NIL);
		size_t nextPhase = 0;
		for (size_t i = 0; i < N; ++i) {
			size_t & phase = phaseNumber[unionFind.find_set(i)];
			if (phase == NIL) {
				phase = nextPhase++;
			}
			phaseMap[nodeOrder[i]] = phase;
		}
	}

	void get_phase_graph(const std::map<node *, size_t> & phaseMap,
						 graph<size_t> & phaseGraph)
	{
		for (std::map<node *, size_t>::const_iterator i = phaseMap.begin();
			 i != phaseMap.end(); ++i) {
			phaseGraph.add_node(i->second);
		}

		const node_map::relmap_t & relations = m_nodeMap.get_relations();
		for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
			if (i->second.second == depends)
				phaseGraph.add_edge(phaseMap.find(m_nodeMap.get(i->second.first))->second,
									phaseMap.find(m_nodeMap.get(i->first))->second);
		}
	}

	// Compute the inverse of a permutation f : {0, 1, ... N-1} -> {0, 1, ... N-1}
	static std::vector<size_t> inverse_permutation(const std::vector<size_t> & f) {
		std::vector<size_t> result(f.size(), f.size());
		for (size_t i = 0; i < f.size(); ++i) {
			if (f[i] >= f.size())
				throw tpie::exception("inverse_permutation: f has bad range");
			if (result[f[i]] != f.size())
				throw tpie::exception("inverse_permutation: f is not injective");
			result[f[i]] = i;
		}
		for (size_t i = 0; i < result.size(); ++i) {
			if (result[i] == f.size())
				throw tpie::exception("inverse_permutation: f is not surjective");
		}
		return result;
	}

	void get_phases(const std::map<node *, size_t> & phaseMap,
					const graph<size_t> & phaseGraph,
					std::vector<bool> & evacuateWhenDone,
					std::vector<std::vector<node *> > & phases)
	{
		std::vector<size_t> topologicalOrder;
		phaseGraph.topological_order(topologicalOrder);
		// topologicalOrder[0] is the first phase to run,
		// topologicalOrder[1] the next, and so on.

		// Compute inverse permutation such that
		// topoOrderMap[i] is the time at which we run phase i.
		std::vector<size_t> topoOrderMap = inverse_permutation(topologicalOrder);

		// Distribute nodes according to the topological order
		phases.resize(topologicalOrder.size());
		for (std::map<node *, size_t>::const_iterator i = phaseMap.begin();
			 i != phaseMap.end(); ++i)
		{
			log_debug() << "Node " << i->first << " goes to phase " << i->second
				<< std::endl;
			phases[topoOrderMap[i->second]].push_back(i->first);
		}

		evacuateWhenDone.resize(phases.size(), false);
		for (size_t i = 0; i + 1 < phases.size(); ++i) {
			if (!phaseGraph.has_edge(topoOrderMap[i], topoOrderMap[i+1]))
				evacuateWhenDone[i] = true;
		}
	}

	void get_item_flow_graphs(std::vector<std::vector<node *> > & phases,
							  std::vector<graph<node *> > & itemFlow)
	{
		itemFlow.resize(phases.size());
		for (size_t i = 0; i < phases.size(); ++i)
			get_graph(phases[i], itemFlow[i], true);
	}

	void get_actor_graphs(std::vector<std::vector<node *> > & phases,
						  std::vector<graph<node *> > & actors)
	{
		actors.resize(phases.size());
		for (size_t i = 0; i < phases.size(); ++i)
			get_graph(phases[i], actors[i], false);
	}

	void get_graph(std::vector<node *> & phase, graph<node *> & result,
				   bool itemFlow)
	{
		const node_map::relmap_t & relations = m_nodeMap.get_relations();
		typedef node_map::relmapit relmapit;
		for (size_t i = 0; i < phase.size(); ++i) {
			std::pair<relmapit, relmapit> edges = relations.equal_range(phase[i]->get_id());
			for (relmapit j = edges.first; j != edges.second; ++j) {
				node * u = m_nodeMap.get(j->first);
				node * v = m_nodeMap.get(j->second.first);
				if (j->second.second == depends) continue;
				if (itemFlow && j->second.second == pulls) std::swap(u, v);
				result.add_edge(u, v);
			}
		}
	}

	bool is_initiator(node * n) {
		node_map::id_t id = n->get_id();
		return m_nodeMap.in_degree(id, pushes) == 0 && m_nodeMap.in_degree(id, pulls) == 0;
	}

	bool has_initiator(const std::vector<node *> & phase) {
		for (size_t i = 0; i < phase.size(); ++i)
			if (is_initiator(phase[i])) return true;
		return false;
	}

	void ensure_initiators(const std::vector<std::vector<node *> > & phases) {
		for (size_t i = 0; i < phases.size(); ++i)
			if (!has_initiator(phases[i])) throw no_initiator_node();
	}

	void prepare_all(const std::vector<graph<node *> > & itemFlow) {
		for (size_t i = 0; i < itemFlow.size(); ++i) {
			const graph<node *> & g = itemFlow[i];
			std::vector<node *> topoOrder;
			g.topological_order(topoOrder);
			for (size_t i = 0; i < topoOrder.size(); ++i) {
				topoOrder[i]->set_state(node::STATE_IN_PREPARE);
				topoOrder[i]->prepare();
				topoOrder[i]->set_state(node::STATE_AFTER_PREPARE);
			}
		}
	}

	void propagate_all(const graph<node *> & itemFlow) {
		std::vector<node *> topoOrder;
		itemFlow.topological_order(topoOrder);
		for (size_t i = 0; i < topoOrder.size(); ++i) {
			topoOrder[i]->set_state(node::STATE_IN_PROPAGATE);
			topoOrder[i]->propagate();
			topoOrder[i]->set_state(node::STATE_AFTER_PROPAGATE);
		}
	}

	void go_initators(const std::vector<node *> & phase, progress_indicator_base & pi) {
		std::vector<node *> initiators;
		for (size_t i = 0; i < phase.size(); ++i) {
			phase[i]->set_progress_indicator(&pi);
			if (is_initiator(phase[i])) initiators.push_back(phase[i]);
		}
		for (size_t i = 0; i < initiators.size(); ++i)
			initiators[i]->go();
	}

	static void assign_memory(const std::vector<std::vector<node *> > & phases, memory_size_type memory) {
		for (size_t i = 0; i < phases.size(); ++i)
			assign_phase_memory(phases[i], memory);
	}

	static void assign_phase_memory(const std::vector<node *> & phase, memory_size_type memory) {
		memory_runtime rt(phase);

		if (rt.sum_minimum_memory() > memory) {
			log_warning() << "Not enough memory for pipelining phase ("
						  << rt.sum_minimum_memory() << " > " << memory << ")"
						  << std::endl;
			rt.assign_minimum_memory();
			return;
		}

		// This case is handled specially to avoid dividing by zero later on.
		if (rt.sum_fraction() < 1e-9) {
			rt.assign_minimum_memory();
			return;
		}

		double c_lo = 0.0;
		double c_hi = 1.0;
		// Exponential search
		memory_size_type oldMemoryAssigned = 0;
		while (true) {
			double factor = memory * c_hi / rt.sum_fraction();
			memory_size_type memoryAssigned = rt.sum_assigned_memory(factor);
			if (memoryAssigned < memory && memoryAssigned != oldMemoryAssigned)
				c_hi *= 2;
			else
				break;
			oldMemoryAssigned = memoryAssigned;
		}

		// Binary search
		while (c_hi - c_lo > 1e-6) {
			double c = c_lo + (c_hi-c_lo)/2;
			double factor = memory * c / rt.sum_fraction();
			memory_size_type memoryAssigned = rt.sum_assigned_memory(factor);

			if (memoryAssigned > memory) {
				c_hi = c;
			} else {
				c_lo = c;
			}
		}

		rt.assign_memory(c_lo);
	}

};

}

}

}

#endif // TPIE_PIPELINING_RUNTIME_H
