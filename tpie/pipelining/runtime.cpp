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

#include <tpie/fractional_progress.h>
#include <tpie/disjoint_sets.h>
#include <tpie/pipelining/tokens.h>
#include <tpie/pipelining/node.h>
#include <tpie/pipelining/runtime.h>

namespace tpie {

namespace pipelining {

namespace bits {

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

	void init(stream_size_type n,
			  progress_indicator_base & pi,
			  const std::vector<std::vector<node *> > & phases) {
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
/// begin/end handling on nodes.
///////////////////////////////////////////////////////////////////////////////
class begin_end {
public:
	begin_end(graph<node *> & actorGraph) {
		actorGraph.topological_order(m_topologicalOrder);
	}

	void begin() {
		for (size_t i = m_topologicalOrder.size(); i--;) {
			m_topologicalOrder[i]->set_state(node::STATE_IN_BEGIN);
			m_topologicalOrder[i]->begin();
			m_topologicalOrder[i]->set_state(node::STATE_AFTER_BEGIN);
		}
	}

	void end() {
		for (size_t i = 0; i < m_topologicalOrder.size(); ++i) {
			m_topologicalOrder[i]->set_state(node::STATE_IN_END);
			m_topologicalOrder[i]->end();
			m_topologicalOrder[i]->set_state(node::STATE_AFTER_END);
		}
	}

private:
	std::vector<node *> m_topologicalOrder;
};

memory_runtime::memory_runtime(const std::vector<node *> & nodes)
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
memory_size_type memory_runtime::minimum_memory(size_t i) const {
	return m_nodes[i]->get_minimum_memory();
}

memory_size_type memory_runtime::maximum_memory(size_t i) const {
	return m_nodes[i]->get_maximum_memory();
}

double memory_runtime::fraction(size_t i) const {
	return m_nodes[i]->get_memory_fraction();
}

// Node accessor aggregates
memory_size_type memory_runtime::sum_minimum_memory() const {
	return m_minimumMemory;
}

memory_size_type memory_runtime::sum_maximum_memory() const {
	return m_maximumMemory;
}

double memory_runtime::sum_fraction() const {
	return m_fraction;
}

// Node mutator
void memory_runtime::set_memory(size_t i, memory_size_type mem) {
	m_nodes[i]->set_available_memory(mem);
}

void memory_runtime::assign_memory(double factor) {
	for (size_t i = 0; i < m_nodes.size(); ++i)
		set_memory(i, get_assigned_memory(i, factor));
}

// Special case of assign_memory when factor is zero.
void memory_runtime::assign_minimum_memory() {
	for (size_t i = 0; i < m_nodes.size(); ++i)
		set_memory(i, minimum_memory(i));
}

memory_size_type memory_runtime::sum_assigned_memory(double factor) const {
	memory_size_type memoryAssigned = 0;
	for (size_t i = 0; i < m_nodes.size(); ++i)
		memoryAssigned += get_assigned_memory(i, factor);
	return memoryAssigned;
}

memory_size_type memory_runtime::get_assigned_memory(size_t i,
													 double factor) const {
	return clamp(minimum_memory(i), maximum_memory(i),
				 factor * fraction(i));
}

/*static*/
memory_size_type memory_runtime::clamp(memory_size_type lo,
									   memory_size_type hi,
									   double v)
{
	if (v < lo) return lo;
	if (v > hi) return hi;
	return static_cast<memory_size_type>(v);
}

void memory_runtime::print_memory(double c, std::ostream & os) {
	size_t cw = 12;
	size_t prec_frac = 2;
	std::string sep(2, ' ');

	os	<< "\nPipelining phase memory assigned\n"
		<< std::setw(cw) << "Minimum"
		<< std::setw(cw) << "Maximum"
		<< std::setw(cw) << "Fraction"
		<< std::setw(cw) << "Assigned"
		<< sep << "Name\n";

	for (size_t i = 0; i < m_nodes.size(); ++i) {
		std::string frac;
		{
			std::stringstream ss;
			ss << std::fixed << std::setprecision(prec_frac)
				<< fraction(i);
			frac = ss.str();
		}

		stream_size_type lo = minimum_memory(i);
		stream_size_type hi = maximum_memory(i);
		stream_size_type assigned = get_assigned_memory(i, c);

		os	<< std::setw(cw) << lo;
		if (hi == std::numeric_limits<stream_size_type>::max()) {
			os << std::setw(cw) << "inf";
		} else {
			os << std::setw(cw) << hi;
		}
		os	<< std::setw(cw) << frac
			<< std::setw(cw) << assigned
			<< sep
			<< m_nodes[i]->get_name().substr(0, 50) << '\n';
	}
	os << std::endl;
}

runtime::runtime(node_map::ptr nodeMap)
	: m_nodeMap(*nodeMap)
{
}

size_t runtime::get_node_count() {
	return m_nodeMap.size();
}

void runtime::go(stream_size_type items,
				 progress_indicator_base & progress,
				 memory_size_type memory)
{
	if (get_node_count() == 0)
		throw tpie::exception("no nodes in pipelining graph");

	// Partition nodes into phases (using union-find)
	std::map<node *, size_t> phaseMap;
	get_phase_map(phaseMap);
	if (phaseMap.size() != get_node_count())
		throw tpie::exception("get_phase_map did not return "
							  "correct number of nodes");

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

	// Exception guarantees are the following:
	//   Progress indicators:
	//     We use RAII to match init() calls with done() calls.
	//     This means that we call done() on a progress indicator
	//     during stack unwinding if an exception is thrown.
	//   begin() and end():
	//     If an exception is thrown by an initiator,
	//     we do not call end() even though we called begin().
	//     This is to signal to the nodes that processing was aborted.
	//     A node may do finalization cleanup in its destructor
	//     rather than in end() to handle exceptions robustly.

	// Construct fractional progress indicators:
	// Get the name of each phase and call init() on the given indicator.
	progress_indicators pi;
	pi.init(items, progress, phases);

	for (size_t i = 0; i < phases.size(); ++i) {
		// Run each phase:
		// Evacuate previous if necessary
		if (i > 0 && evacuateWhenDone[i-1]) evacuate_all(phases[i-1]);
		// call propagate in item source to item sink order
		propagate_all(itemFlow[i]);
		// sum number of steps and call pi.init()
		phase_progress_indicator phaseProgress(pi, i, phases[i]);
		// set progress indicators on each node
		set_progress_indicators(phases[i], phaseProgress.get());
		// call begin in leaf to root actor order
		begin_end beginEnd(actor[i]);
		beginEnd.begin();
		// call go on initiators
		go_initiators(phases[i]);
		// call end in root to leaf actor order
		beginEnd.end();
		// call pi.done in ~phase_progress_indicator
	}
	// call fp->done in ~progress_indicators
}

void runtime::get_item_sources(std::vector<node *> & itemSources) {
	typedef node_map::id_t id_t;
	std::set<id_t> possibleSources;
	for (node_map::mapit i = m_nodeMap.begin(); i != m_nodeMap.end(); ++i) {
		possibleSources.insert(i->first);
	}
	const node_map::relmap_t & relations = m_nodeMap.get_relations();
	for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
		id_t from = i->first;
		id_t to = i->second.first;
		bits::node_relation rel = i->second.second;

		switch (rel) {
			case pushes:
				possibleSources.erase(to);
				break;
			case pulls:
			case depends:
				possibleSources.erase(from);
				break;
		}
	}
	for (std::set<id_t>::iterator i = possibleSources.begin();
		 i != possibleSources.end(); ++i) {
		itemSources.push_back(m_nodeMap.get(*i));
	}
}

void runtime::get_item_sinks(std::vector<node *> & itemSinks) {
	typedef node_map::id_t id_t;
	std::set<id_t> possibleSinks;
	for (node_map::mapit i = m_nodeMap.begin(); i != m_nodeMap.end(); ++i) {
		possibleSinks.insert(i->first);
	}
	const node_map::relmap_t & relations = m_nodeMap.get_relations();
	for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
		id_t from = i->first;
		id_t to = i->second.first;
		bits::node_relation rel = i->second.second;

		switch (rel) {
			case pushes:
				possibleSinks.erase(from);
				break;
			case pulls:
			case depends:
				possibleSinks.erase(to);
				break;
		}
	}
	for (std::set<id_t>::iterator i = possibleSinks.begin();
		 i != possibleSinks.end(); ++i) {
		itemSinks.push_back(m_nodeMap.get(*i));
	}
}

void runtime::get_phase_map(std::map<node *, size_t> & phaseMap) {
	typedef node_map::id_t id_t;
	std::map<id_t, size_t> numbering;
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

void runtime::get_phase_graph(const std::map<node *, size_t> & phaseMap,
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
/*static*/
std::vector<size_t> runtime::inverse_permutation(const std::vector<size_t> & f) {
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

void runtime::get_phases(const std::map<node *, size_t> & phaseMap,
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
		if (!phaseGraph.has_edge(topologicalOrder[i], topologicalOrder[i+1]))
			evacuateWhenDone[i] = true;
	}
}

void runtime::get_item_flow_graphs(std::vector<std::vector<node *> > & phases,
								   std::vector<graph<node *> > & itemFlow)
{
	itemFlow.resize(phases.size());
	for (size_t i = 0; i < phases.size(); ++i)
		get_graph(phases[i], itemFlow[i], true);
}

void runtime::get_actor_graphs(std::vector<std::vector<node *> > & phases,
							   std::vector<graph<node *> > & actors)
{
	actors.resize(phases.size());
	for (size_t i = 0; i < phases.size(); ++i)
		get_graph(phases[i], actors[i], false);
}

void runtime::get_graph(std::vector<node *> & phase, graph<node *> & result,
						bool itemFlow)
{
	const node_map::relmap_t & relations = m_nodeMap.get_relations();
	typedef node_map::relmapit relmapit;
	for (size_t i = 0; i < phase.size(); ++i) {
		std::pair<relmapit, relmapit> edges =
			relations.equal_range(phase[i]->get_id());
		for (relmapit j = edges.first; j != edges.second; ++j) {
			node * u = m_nodeMap.get(j->first);
			node * v = m_nodeMap.get(j->second.first);
			if (j->second.second == depends) continue;
			if (itemFlow && j->second.second == pulls) std::swap(u, v);
			result.add_edge(u, v);
		}
	}
}

bool runtime::is_initiator(node * n) {
	node_map::id_t id = n->get_id();
	return m_nodeMap.in_degree(id, pushes) == 0
		&& m_nodeMap.in_degree(id, pulls) == 0;
}

bool runtime::has_initiator(const std::vector<node *> & phase) {
	for (size_t i = 0; i < phase.size(); ++i)
		if (is_initiator(phase[i])) return true;
	return false;
}

void runtime::ensure_initiators(const std::vector<std::vector<node *> > & phases) {
	for (size_t i = 0; i < phases.size(); ++i)
		if (!has_initiator(phases[i])) throw no_initiator_node();
}

void runtime::prepare_all(const std::vector<graph<node *> > & itemFlow) {
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

void runtime::evacuate_all(const std::vector<node *> & phase) {
	for (size_t i = 0; i < phase.size(); ++i) {
		if (phase[i]->can_evacuate()) {
			phase[i]->evacuate();
			tpie::log_debug() << "Evacuated node " << phase[i]->get_id() << std::endl;
		}
	}
}

void runtime::propagate_all(const graph<node *> & itemFlow) {
	std::vector<node *> topoOrder;
	itemFlow.topological_order(topoOrder);
	for (size_t i = 0; i < topoOrder.size(); ++i) {
		topoOrder[i]->set_state(node::STATE_IN_PROPAGATE);
		topoOrder[i]->propagate();
		topoOrder[i]->set_state(node::STATE_AFTER_PROPAGATE);
	}
}

void runtime::set_progress_indicators(const std::vector<node *> & phase,
									  progress_indicator_base & pi) {
	for (size_t i = 0; i < phase.size(); ++i)
		phase[i]->set_progress_indicator(&pi);
}

void runtime::go_initiators(const std::vector<node *> & phase) {
	std::vector<node *> initiators;
	for (size_t i = 0; i < phase.size(); ++i)
		if (is_initiator(phase[i])) initiators.push_back(phase[i]);
	for (size_t i = 0; i < initiators.size(); ++i)
		initiators[i]->go();
}

/*static*/
void runtime::assign_memory(const std::vector<std::vector<node *> > & phases,
							memory_size_type memory) {
	for (size_t i = 0; i < phases.size(); ++i) {
		memory_runtime rt(phases[i]);
		double c = get_memory_factor(rt, memory);
#ifndef TPIE_NDEBUG
		rt.print_memory(c, log_debug());
#endif // TPIE_NDEBUG
		rt.assign_memory(c);
	}
}

/*static*/
double runtime::get_memory_factor(const memory_runtime & rt,
								  memory_size_type memory) {
	if (rt.sum_minimum_memory() > memory) {
		log_warning() << "Not enough memory for pipelining phase ("
					  << rt.sum_minimum_memory() << " > " << memory << ")"
					  << std::endl;
		return 0.0;
	}

	// This case is handled specially to avoid dividing by zero later on.
	if (rt.sum_fraction() < 1e-9) {
		return 0.0;
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

	return memory * c_lo / rt.sum_fraction();
}

}

}

}
