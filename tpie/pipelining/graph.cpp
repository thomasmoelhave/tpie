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

#include <tpie/pipelining/graph.h>
#include <tpie/pipelining/tokens.h>
#include <tpie/pipelining/node.h>

namespace {

template <typename Graph>
class dfs_traversal {
	typedef typename Graph::node_t node_t;
	typedef typename Graph::neighbours_t neighbours_t; // vector of node_t
	typedef typename Graph::edgemap_t edgemap_t; // maps node_t to neighbours_t
	typedef typename Graph::time_type time_type; // signed integral type
	typedef typename Graph::nodemap_t nodemap_t; // maps node_t to time_type

	Graph & g;

public:
	dfs_traversal(Graph & g) : g(g) {}

	void dfs() {
		// initialize finish times
		for (typename nodemap_t::iterator i = g.finish_times.begin(); i != g.finish_times.end(); ++i) {
			i->second = 0;
		}
		// dfs from all nodes
		time_type time = 1;
		for (typename nodemap_t::reverse_iterator i = g.finish_times.rbegin(); i != g.finish_times.rend(); ++i) {
			if (i->second != 0) continue;
			time = dfs_from(i->first, time);
		}
	}

	std::vector<node_t> toposort() {
		std::vector<std::pair<time_type, node_t> > nodes;
		nodes.reserve(g.finish_times.size());
		for (typename nodemap_t::iterator i = g.finish_times.begin(); i != g.finish_times.end(); ++i) {
			nodes.push_back(std::make_pair(-i->second, i->first));
		}
		std::sort(nodes.begin(), nodes.end());
		std::vector<node_t> result(nodes.size());
		for (size_t i = 0; i < nodes.size(); ++i) {
			result[i] = nodes[i].second;
		}
		return result;
	}

private:

	time_type dfs_from(node_t start, time_type time) {
		g.finish_times[start] = time++; // discover time
		neighbours_t & neighbours = g.edges[start];
		for (typename neighbours_t::iterator i = neighbours.begin(); i != neighbours.end(); ++i) {
			if (g.finish_times[*i] != 0) continue;
			time = dfs_from(*i, time);
		}
		g.finish_times[start] = time++; // finish time;
		return time;
	}

};

class phasegraph {
public:
	typedef size_t node_t;
	typedef int time_type;
	typedef std::map<node_t, time_type> nodemap_t;
	typedef std::vector<node_t> neighbours_t;
	typedef std::map<node_t, neighbours_t> edgemap_t;
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
		edges[dependee].push_back(depender);
	}

	inline bool is_depending(size_t depender, size_t dependee) {
		for (size_t i = 0; i < edges[dependee].size(); ++i) {
			if (edges[dependee][i] == depender) return true;
		}
		return false;
	}

	std::vector<size_t> execution_order() {
		dfs_traversal<phasegraph> dfs(*this);
		dfs.dfs();
		std::vector<size_t> result = dfs.toposort();
		return result;
	}
};

tpie::memory_size_type clamp(tpie::memory_size_type lo, tpie::memory_size_type hi, double v) {
	if (v < lo) return lo;
	if (v > hi) return hi;
	return static_cast<tpie::memory_size_type>(v);
}

} // default namespace

namespace tpie {

namespace pipelining {

namespace bits {

class phase::node_graph {
public:
	typedef node * node_t;
	typedef std::vector<node_t> neighbours_t;
	typedef std::map<node_t, neighbours_t> edgemap_t;
	typedef int time_type;
	typedef std::map<node_t, time_type> nodemap_t;

	nodemap_t finish_times;
	edgemap_t edges;
};

phase::phase()
	: itemFlowGraph(new node_graph)
	, actorGraph(new node_graph)
{
}

phase::~phase() {}

phase::phase(const phase & other)
	: itemFlowGraph(new node_graph(*other.itemFlowGraph))
	, actorGraph(new node_graph(*other.actorGraph))
	, m_nodes(other.m_nodes)
{
}

phase & phase::operator=(const phase & other) {
	itemFlowGraph.reset(new node_graph(*other.itemFlowGraph));
	actorGraph.reset(new node_graph(*other.actorGraph));
	m_nodes = other.m_nodes;
	return *this;
}

bool phase::is_initiator(node * s) {
	node_map::ptr m = s->get_node_map()->find_authority();
	node_map::id_t id = s->get_id();
	return m->in_degree(id, pushes) == 0 && m->in_degree(id, pulls) == 0;
}

void phase::add(node * s) {
	if (count(s)) return;
	m_nodes.push_back(s);
	itemFlowGraph->finish_times[s] = 0;
	actorGraph->finish_times[s] = 0;
}

void phase::add_successor(node * from, node * to, bool push) {
	itemFlowGraph->edges[from].push_back(to);
	if (push)
		actorGraph->edges[from].push_back(to);
	else
		actorGraph->edges[to].push_back(from);
}

void phase::evacuate_all() const {
	for (size_t i = 0; i < m_nodes.size(); ++i) {
		if (m_nodes[i]->can_evacuate())
			m_nodes[i]->evacuate();
	}
}

const std::string & phase::get_name() const {
	priority_type highest = std::numeric_limits<priority_type>::min();
	size_t highest_node = 0;
	for (size_t i = 0; i < m_nodes.size(); ++i) {
		if (m_nodes[i]->get_name_priority() > highest) {
			highest_node = i;
			highest = m_nodes[i]->get_name_priority();
		}
	}
	return m_nodes[highest_node]->get_name();
}

std::string phase::get_unique_id() const {
	std::stringstream uid;
	for (size_t i = 0; i < m_nodes.size(); ++i) {
		uid << typeid(*m_nodes[i]).name() << ':';
	}
	return uid.str();
}

void phase::assign_minimum_memory() const {
	for (size_t i = 0; i < m_nodes.size(); ++i) {
		m_nodes[i]->set_available_memory(m_nodes[i]->get_minimum_memory());
	}
}

memory_size_type phase::sum_assigned_memory(double factor) const {
	memory_size_type memoryAssigned = 0;
	for (size_t i = 0; i < m_nodes.size(); ++i) {
		memoryAssigned +=
			clamp(m_nodes[i]->get_minimum_memory(),
				  m_nodes[i]->get_maximum_memory(),
				  factor * m_nodes[i]->get_memory_fraction());
	}
	return memoryAssigned;
}

void phase::assign_memory(memory_size_type m) const {
	{
		dfs_traversal<phase::node_graph> dfs(*itemFlowGraph);
		dfs.dfs();
		std::vector<node *> order = dfs.toposort();
		for (size_t i = 0; i < order.size(); ++i) {
			if (order[i]->get_state() != node::STATE_FRESH) {
				throw call_order_exception(
					"Tried to call prepare on an none fresh node");
			}
			order[i]->set_state(node::STATE_IN_PREPARE);
			order[i]->prepare();
			order[i]->set_state(node::STATE_AFTER_PREPARE);
		}
	}

	double fraction = 0.0;
	memory_size_type minimumMemory = 0;
	for (size_t i = 0; i < m_nodes.size(); ++i) {
		fraction += m_nodes[i]->get_memory_fraction();
		minimumMemory += m_nodes[i]->get_minimum_memory();
	}

	if (m < minimumMemory) {
		TP_LOG_WARNING_ID("Not enough memory for this phase. We have " << m << " but we require " << minimumMemory << '.');
		assign_minimum_memory();
		return;
	}

	// This case is handled specially to avoid dividing by zero later on.
	if (fraction < 1e-9) {
		assign_minimum_memory();
		return;
	}

	std::vector<double> prio(m_nodes.size());
	for (size_t i = 0; i < m_nodes.size(); ++i) {
		prio[i] = m_nodes[i]->get_memory_fraction() / fraction;
	}

	double c_lo = 0.0;
	double c_hi = 1.0;
	// Exponential search
	memory_size_type oldMemoryAssigned = 0;
	while (true) {
		double factor = m * c_hi / fraction;
		memory_size_type memoryAssigned = sum_assigned_memory(factor);
		if (memoryAssigned < m && memoryAssigned != oldMemoryAssigned)
			c_hi *= 2;
		else
			break;
		oldMemoryAssigned = memoryAssigned;
	}

	// Binary search
	while (c_hi - c_lo > 1e-6) {
		double c = c_lo + (c_hi-c_lo)/2;
		double factor = m * c / fraction;
		memory_size_type memoryAssigned = sum_assigned_memory(factor);

		if (memoryAssigned > m) {
			c_hi = c;
		} else {
			c_lo = c;
		}
	}

	memory_size_type memoryAssigned = 0;
	double factor = m * c_lo / fraction;

	for (size_t i = 0; i < m_nodes.size(); ++i) {
		memory_size_type assign =
			clamp(m_nodes[i]->get_minimum_memory(),
				  m_nodes[i]->get_maximum_memory(),
				  factor * m_nodes[i]->get_memory_fraction());
		m_nodes[i]->set_available_memory(assign);
		memoryAssigned += assign;

		if (memoryAssigned < assign) { // overflow
			log_error() << "Assigned " << assign << " bytes of memory ("
				<< (memoryAssigned - assign) << " bytes already assigned)"
				<< std::endl;
			throw tpie::exception
				("Overflow when summing memory assigned in pipelining");
		}
	}

	if (memoryAssigned > m) {
		log_warning() << "Too much memory assigned in graph.cpp: Got " << m
			<< ", but assigned " << memoryAssigned
			<< " (" << (memoryAssigned-m) << " b too much)" << std::endl;
	}
}

void phase::print_memory(std::ostream & os) const {
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
		std::string fraction;
		{
			std::stringstream ss;
			ss << std::fixed << std::setprecision(prec_frac)
				<< m_nodes[i]->get_memory_fraction();
			fraction = ss.str();
		}

		stream_size_type lo = m_nodes[i]->get_minimum_memory();
		stream_size_type hi = m_nodes[i]->get_maximum_memory();
		stream_size_type assigned = m_nodes[i]->get_available_memory();

		os	<< std::setw(cw) << lo;
		if (hi == std::numeric_limits<stream_size_type>::max()) {
			os << std::setw(cw) << "inf";
		} else {
			os << std::setw(cw) << hi;
		}
		os	<< std::setw(cw) << fraction
			<< std::setw(cw) << assigned
			<< sep
			<< m_nodes[i]->get_name().substr(0, 50) << '\n';
	}
	os << std::endl;
}

graph_traits::graph_traits(const node_map & map)
	: map(map)
{
	map.assert_authoritative();
	calc_phases();
	map.send_successors();
}

memory_size_type graph_traits::memory_usage(size_t phases) {
	return phases * (sizeof(auto_ptr<Progress::sub>) + sizeof(Progress::sub));
}

void graph_traits::go_all(stream_size_type n, Progress::base & pi) {
	map.assert_authoritative();
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

void graph_traits::calc_phases() {
	map.assert_authoritative();
	typedef std::map<node_map::id_t, size_t> ids_t;
	typedef std::map<size_t, node_map::id_t> ids_inv_t;
	ids_t ids;
	ids_inv_t ids_inv;
	size_t nextid = 0;
	for (node_map::mapit i = map.begin(); i != map.end(); ++i) {
		ids.insert(std::make_pair(i->first, nextid));
		ids_inv.insert(std::make_pair(nextid, i->first));
		++nextid;
	}
	tpie::disjoint_sets<size_t> phases(nextid);
	for (size_t i = 0; i < nextid; ++i) phases.make_set(i);

	const node_map::relmap_t relations = map.get_relations();
	for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
		if (i->second.second != depends) phases.union_set(ids[i->first], ids[i->second.first]);
	}
	// `phases` holds a map from node to phase number

	phasegraph g(phases, nextid);

	{
		// establish phase relationships
		for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
			if (i->second.second == depends) {
				size_t depender = phases.find_set(ids[i->first]);
				size_t dependee = phases.find_set(ids[i->second.first]);
				g.depends(depender, dependee);
			}
		}

		// toposort the phase graph and find the phase numbers in the execution order
		std::vector<size_t> internalexec = g.execution_order();
		m_phases.resize(internalexec.size());
		m_evacuatePrevious.resize(internalexec.size(), false);

		std::vector<bool>::iterator j = m_evacuatePrevious.begin();
		for (size_t i = 0; i < internalexec.size(); ++i, ++j) {
			// all nodes with phase number internalexec[i] should be executed in phase i

			// first, insert phase representatives
			m_phases[i].add(map.get(ids_inv[internalexec[i]]));
			*j = i > 0 && !g.is_depending(internalexec[i], internalexec[i-1]);
		}
	}

	for (ids_inv_t::iterator i = ids_inv.begin(); i != ids_inv.end(); ++i) {
		node * representative = map.get(ids_inv[phases.find_set(i->first)]);
		node * current = map.get(i->second);
		if (current == representative) continue;
		for (size_t i = 0; i < m_phases.size(); ++i) {
			if (m_phases[i].count(representative)) {
				m_phases[i].add(current);
				break;
			}
		}
	}

	typedef std::map<node *, size_t> degree_map;
	degree_map inDegree;
	degree_map outDegree;
	for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
		if (i->second.second == depends) continue;

		node * from = map.get(i->first);
		node * to = map.get(i->second.first);

		if (i->second.second == pulls) std::swap(from, to);

		outDegree[from]++;
		inDegree[to]++;

		node * representative = map.get(ids_inv[phases.find_set(ids[i->first])]);
		for (size_t j = 0; j < m_phases.size(); ++j) {
			if (m_phases[j].count(representative)) {
				m_phases[j].add_successor(from, to, i->second.second == pushes);
				break;
			}
		}
	}

	for (node_map::mapit i = map.begin(); i != map.end(); ++i) {
		if (!inDegree.count(i->second)) {
			m_itemSources.push_back(i->second);
		}
		if (!outDegree.count(i->second)) {
			m_itemSinks.push_back(i->second);
		}
	}
}

void phase::go(progress_indicator_base & pi) {
	std::vector<node *> beginOrder;
	std::vector<node *> endOrder;
	{
		dfs_traversal<phase::node_graph> dfs(*itemFlowGraph);
		dfs.dfs();
		beginOrder = dfs.toposort();
	}
	{
		dfs_traversal<phase::node_graph> dfs(*actorGraph);
		dfs.dfs();
		endOrder = dfs.toposort();
	}
	stream_size_type totalSteps = 0;
	for (size_t i = 0; i < beginOrder.size(); ++i) {
		if (beginOrder[i]->get_state() != node::STATE_AFTER_PREPARE) {
			throw call_order_exception("Invalid state for begin");
		}
		beginOrder[i]->set_state(node::STATE_IN_BEGIN);
		beginOrder[i]->begin();
		if (beginOrder[i]->get_progress_indicator() == 0)
			beginOrder[i]->set_progress_indicator(&pi);
		totalSteps += beginOrder[i]->get_steps();
		beginOrder[i]->set_state(node::STATE_AFTER_BEGIN);
	}

	pi.init(totalSteps);
	size_t initiators = 0;
	for (size_t i = 0; i < beginOrder.size(); ++i) {
		if (!is_initiator(beginOrder[i])) continue;
		log_debug() << "Execute initiator " << beginOrder[i]->get_name()
			<< " (" << beginOrder[i]->get_id() << ")" << std::endl;
		beginOrder[i]->go();
		initiators++;
	}

	for (size_t i = 0; i < endOrder.size(); ++i) {
		if (endOrder[i]->get_state() != node::STATE_AFTER_BEGIN) {
			throw call_order_exception("Invalid state for end");
		}
		endOrder[i]->set_state(node::STATE_IN_END);
		endOrder[i]->end();
		endOrder[i]->set_state(node::STATE_AFTER_END);
	}
	pi.done();

	if (initiators == 0)
		throw no_initiator_node();

}

} // namespace bits

} // namespace pipelining

} // namespace tpie
