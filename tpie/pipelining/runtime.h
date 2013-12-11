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

namespace tpie {

namespace pipelining {

namespace bits {

template <typename T>
class graph {
public:
	void add_edge(T u, T v) {
		m_edgeLists[u].push_back(v);
	}

	bool has_edge(T u, T v) {
		const std::vector<T> & edgeList = m_edgeLists[u];
		return std::find(edgeList.begin(), edgeList.end(), v) != edgeList.end();
	}

	void topological_order(std::vector<T> & result) {
		depth_first_search dfs(m_edgeLists);
		std::vector<std::pair<size_t, T> > nodes(nodeCount);
		for (std::map<T, std::vector<T> >::iterator i = m_edgeLists.begin();
			 i != m_edgeLists.end(); ++i)
			nodes.push_back(std::make_pair(dfs.visit(i->first), i->first));
		std::sort(nodes.begin(), nodes.end(), std::greater<std::pair<size_t, T> >());
		result.resize(nodes.size());
		for (size_t i = 0; i < result.size(); ++i) result[i] = nodes[i].second;
	}

private:
	std::map<T, std::vector<T> > m_edgeLists;

	class depth_first_search {
	public:
		depth_first_search(const std::map<T, std::vector<T> > & edgeLists)
			: m_time(0)
			, m_edgeLists(edgeLists)
		{
		}

		size_t visit(T u) {
			if (m_finishTime.count(u)) return;
			m_finishTime[u] = 0;
			++m_time;
			const std::vector<T> & edgeList = m_edgeLists[u];
			for (size_t i = 0; i < edgeList.size(); ++i) visit(edgeList[i]);
			m_finishTime[u] = m_time++;
		}

	private:
		size_t m_time;
		const std::map<T, std::vector<T> > & m_edgeLists;
		std::map<T, size_t> m_finishTime;
	};
};

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

	void init(stream_size_type n, progress_indicator_base * pi, const std::vector<std::vector<node *> > & phases) {
		fp = new fractional_progress(pi);
		const size_t N = phases.size();
		m_progressIndicators.resize(N);
		for (size_t i = 0; i < N; ++i) {
			std::string uid = get_phase_uid(phases[i]);
			std::string name = get_phase_name(phases[i]);
			m_progressIndicators[i] = new progress_indicator_subindicator(
				fp, uid.c_str(), TPIE_FSI, n, name.c_str());
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
	std::vector<progress_indicator_subindicator *> m_progressIndicators;
};

class phase_progress_indicator {
public:
	phase_progress_indicator(progress_indicators & pi, size_t phaseNumber,
							 const std::vector<node *> & nodes)
		: m_pi(pi.m_progressIndicators[phaseNumber])
	{
		for (size_t j = 0; j < phase.size(); ++j) {
			steps += phase[j]->get_steps();
		}
		m_pi->init(steps);
	}

	~phase_progress_indicator() {
		m_pi->done();
	}

	progress_indicator_subindicator * get() {
		return m_pi;
	}

private:
	progress_indicator_subindicator * m_pi;
};

class begin_end {
public:
	begin_end(graph<node> & actorGraph) {
		actorGraph.topological_order(m_topologicalOrder);
		for (size_t i = 0; i < m_topologicalOrder.size(); ++i)
			m_topologicalOrder[i]->begin();
	}

	~begin_end() {
		for (size_t i = m_topologicalOrder.size(); i--;)
			m_topologicalOrder[i]->end();
	}

private:
	std::vector<node *> m_topologicalOrder;
};

class runtime {
	node_map::ptr m_nodeMap;

public:
	runtime(node_map::ptr nodeMap)
		: m_nodeMap(nodeMap)
	{
	}

	void go(stream_size_type items,
			progress_indicator_base * progress,
			memory_size_type memory) {
		// Partition nodes into phases (using union-find)
		std::map<node *, size_t> phaseMap;
		get_phase_map(*nodeMap, phaseMap);

		// Build phase graph
		graph<size_t> phaseGraph;
		get_phase_graph(*nodeMap, phaseMap, phaseGraph);

		std::vector<std::vector<node *> > phases;
		std::vector<bool> evacuateWhenDone;
		get_phases(phaseMap, phaseGraph, evacuateWhenDone, phases);

		// Build item flow graph and actor graph for each phase
		std::vector<graph<node *> > itemFlow;
		get_item_flow_graphs(phases, itemFlow);
		std::vector<graph<node *> > actor;
		get_actor_graphs(phases, actor);

		// Check that each phase has at least one initiator
		ensure_initiators(*nodeMap, phases);

		// Toposort item flow graph for each phase and call node::prepare in item source to item sink order
		prepare_all(itemFlow);

		// Gather node memory requirements and assign memory to each phase
		assign_memory(phases, memory);

		// Construct fractional progress indicators (getting the name of each phase)
		progress_indicators pi(progress);
		pi.init(phases);

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
			// call end in root to leaf actor order in begin_end dtor
		}
	}

private:
	static
	get_phase_map(node_map & nodeMap, std::map<node *, size_t> & phaseMap) {
		std::map<node *, size_t> numbering;
		std::vector<node *> nodeOrder;
		for (node_map::map_it i = nodeMap.begin(); i != nodeMap.end(); ++i) {
			numbering[i->second] = nodeOrder.size();
			nodeOrder.push_back(i->second);
		}
		const size_t N = nodeOrder.size();

		tpie::disjoint_sets<size_t> unionFind(N);
		for (size_t i = 0; i < N; ++i) unionFind.make_set(i);

		const node_map::relmap_t & relations = map.get_relations();
		for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
			if (i->second.second != depends)
				unionFind.union_set(ids[i->first], ids[i->second.first]);
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

	static
	get_phase_graph(const node_map & nodeMap,
					const std::map<node *, size_t> & phaseMap,
					graph<size_t> & phaseGraph)
	{
		const node_map::relmap_t & relations = nodeMap.get_relations();
		for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
			if (i->second.second == depends)
				phaseGraph.add_edge(phaseMap[i->second.first],
									phaseMap[i->first]);
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

	static get_phases(const std::map<node *, size_t> & phaseMap,
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
			phases[topoOrderMap[i->second]].push_back(i->first);
		}

		evacuateWhenDone.resize(phases.size(), false);
		for (size_t i = 0; i + 1 < phases.size(); ++i) {
			if (!phaseGraph.has_edge(topoOrderMap[i], topoOrderMap[i+1]))
				evacuateWhenDone[i] = true;
		}
	}

	static get_item_flow_graphs(const node_map & nodeMap,
								std::vector<std::vector<node *> > & phases,
								std::vector<graph<node *> > & itemFlow)
	{
		itemFlow.resize(phases.size());
		for (size_t i = 0; i < phases.size(); ++i)
			get_graph(nodeMap, phases[i], itemFlow[i], true);
	}

	static get_actor_graphs(const node_map & nodeMap,
							std::vector<std::vector<node *> > & phases,
							std::vector<graph<node *> > & actors) {
		actors.resize(phases.size());
		for (size_t i = 0; i < phases.size(); ++i)
			get_graph(nodeMap, phases[i], actors[i], false);
	}

	static get_graph(const node_map & nodeMap, std::vector<node *> & phase,
					 graph<node *> & result, bool itemFlow)
	{
		const node_map::relmap_t & relations = nodeMap.get_relations();
		typedef node_map::relmapit relmapit;
		for (size_t i = 0; i < phase.size(); ++i) {
			std::pair<relmapit, relmapit> edges = relations.equal_range(phase[i]->get_id());
			for (relmapit j = edges.first; j != edges.second; ++j) {
				node * u = nodeMap.find(j->first);
				node * v = nodeMap.find(j->second.first);
				if (j->second.second == depends) continue;
				if (itemFlow && j->second.second == pulls) std::swap(u, v);
				result.add_edge(u, v);
			}
		}
	}

	static bool is_initiator(node_map & m, node * n) {
		node_map::id_t id = n->get_id();
		return m.in_degree(id, pushes) == 0 && m.in_degree(id, pulls) == 0;
	}

	static bool has_initiator(node_map & m, const std::vector<node *> & phase) {
		for (size_t i = 0; i < phase.size(); ++i) {
			if (is_initiator(m, phase[i])) return true;
		}
		return false;
	}

	static void ensure_initiators(const std::vector<std::vector<node *> > & phases) {
		for (size_t i = 0; i < phases.size(); ++i) {
			if (!has_initiator(phases[i]))
				throw no_initiator_node();
		}
	}

	static void prepare_all(const std::vector<graph<node *> > & itemFlow) {
		for (size_t i = 0; i < itemFlow.size(); ++i) {
			const graph<node *> & g = itemFlow[i];
			std::vector<node *> topoOrder;
			g.topological_order(topoOrder);
			for (size_t i = 0; i < topoOrder.size(); ++i) {
				topoOrder[i]->prepare();
			}
		}
	}

	static void assign_memory(const std::vector<std::vector<node *> > & phases) {
		for (size_t i = 0; i < phases.size(); ++i)
			assign_phase_memory(phases[i]);
	}

	static void assign_phase_memory(const std::vector<node *> & phase) {
		// TODO: Insert code from graph.cpp phase::assign_memory
	}

	static void propagate_all(const graph<node *> itemFlow) {
		std::vector<node *> topoOrder;
		itemFlow.topological_order(topoOrder);
		for (size_t i = 0; i < topoOrder.size(); ++i) {
			topoOrder[i]->propagate();
		}
	}

	static void go_initators(const std::vector<node *> phase, progress_indicator_base * pi) {
		std::vector<node *> initiators;
		for (size_t i = 0; i < phase.size(); ++i) {
			if (is_initiator(m, phase[i])) initiators.push_back(phase[i]);
		}
		for (size_t i = 0; i < initiators.size(); ++i) {
			initiators[i]->go(pi);
		}
	}


};

}

}

}

#endif // TPIE_PIPELINING_RUNTIME_H
