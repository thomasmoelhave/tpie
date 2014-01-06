// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2014 The TPIE development team
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

#include "common.h"
#include <tpie/pipelining/runtime.h>
#include <tpie/pipelining/node.h>

using namespace tpie;
using namespace tpie::pipelining;
using namespace tpie::pipelining::bits;

class evac_node : public node {
public:
	virtual bool can_evacuate() const {
		return true;
	}
};

bool evacuate_test() {
	const size_t N = 7;
	evac_node nodes[N];

	node_map::ptr nodeMap = nodes[0].get_node_map();
	for (size_t i = 1; i < N; ++i) nodes[i].get_node_map()->union_set(nodeMap);
	nodeMap = nodeMap->find_authority();

	// In our model, each node is its own phase.
	std::map<node *, size_t> phaseMap;
	for (size_t i = 0; i < N; ++i) phaseMap[&nodes[i]] = i;

	graph<size_t> phaseGraph;
	for (size_t i = 0; i < N; ++i) phaseGraph.add_node(i);

	phaseGraph.add_edge(0, 1);
	phaseGraph.add_edge(0, 2);
	phaseGraph.add_edge(1, 3);
	phaseGraph.add_edge(2, 3);
	phaseGraph.add_edge(3, 4);
	phaseGraph.add_edge(3, 5);
	phaseGraph.add_edge(4, 6);
	phaseGraph.add_edge(5, 6);

	// 0 -- 1 ---- 3 -- 4 ---- 6
	//  \         / \         /
	//   `---- 2 ´   `---- 5 ´
	//
	// Since the result of 1 and 4 are not needed in 2 and 5 resp.,
	// (that is, there is no edge 1-2 or 4-5,)
	// 1 and 4 should be evacuated when they are done.

	std::vector<bool> expect(7);
	expect[1] = expect[4] = true;

	std::vector<bool> evacuateWhenDone;
	std::vector<std::vector<node *> > phases;

	{
		runtime rt(nodeMap);
		rt.get_phases(phaseMap, phaseGraph, evacuateWhenDone, phases);
	}

	bool bad = false;
	for (size_t i = 0; i < N; ++i) {
		if (evacuateWhenDone[i] == expect[i]) {
			log_debug() << "Node " << i << ": "
				<< (expect[i] ? "should evacuate" : "don't evacuate")
				<< std::endl;
		} else {
			log_error() << "Node " << i << ": Expected "
				<< (expect[i] ? "should evacuate" : "don't evacuate")
				<< ", got "
				<< (evacuateWhenDone[i] ? "should evacuate" : "don't evacuate")
				<< std::endl;
			bad = true;
		}
	}
	return !bad;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
	.test(evacuate_test, "evacuate")
	;
}
