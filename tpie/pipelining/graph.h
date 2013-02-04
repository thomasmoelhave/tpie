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

#include <tpie/disjoint_sets.h>
#include <tpie/dummy_progress.h>
#include <tpie/tpie_assert.h>
#include <vector>
#include <stack>
#include <tpie/pipelining/predeclare.h>

namespace tpie {

namespace pipelining {

namespace bits {

class phase {
public:
	class node_graph;

	phase();
	phase(const phase &);
	phase & operator=(const phase &);
	~phase();

	inline void set_initiator(node * s) {
		tp_assert(m_initiator == 0, "Initiator set twice");
		m_initiator = s;
	}

	bool is_initiator(node * s);

	void add(node * s);

	void add_successor(node * from, node * to, bool push);

	inline size_t count(node * s) {
		for (size_t i = 0; i < m_nodes.size(); ++i) {
			if (m_nodes[i] == s) return 1;
		}
		return 0;
	}

	void go(progress_indicator_base & pi);

	void evacuate_all() const;

	void assign_memory(memory_size_type m) const;

	const std::string & get_name() const;

	std::string get_unique_id() const;

private:
	/** Graph of nodes in this phase. Initialised in constructor. Populated
	 * by graph_traits::calc_phases using add and add_successor. */
	std::auto_ptr<node_graph> itemFlowGraph;

	/** Graph of nodes in this phase. Initialised in constructor. Populated
	 * by graph_traits::calc_phases using add and add_successor. */
	std::auto_ptr<node_graph> actorGraph;

	/** a pointer is a weak reference to something that isn't reference counted. */
	std::vector<node *> m_nodes;

	node * m_initiator;

	void assign_minimum_memory() const;
};

class graph_traits {
public:
	typedef std::vector<phase> phases_t;
	typedef phases_t::iterator phaseit;
	typedef progress_types<true> Progress;

	static memory_size_type memory_usage(size_t phases);

	graph_traits(const node_map & map);

	const phases_t & phases() {
		return m_phases;
	}

	void go_all(stream_size_type n, Progress::base & pi);

private:
	const node_map & map;
	phases_t m_phases;
	std::vector<bool> m_evacuatePrevious;

	void calc_phases();

};

} // namespace bits

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_GRAPH_H__
