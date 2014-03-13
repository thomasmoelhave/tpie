// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014, The TPIE development team
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
#include <tpie/tpie.h>
#include <tpie/btree/internal_store.h>
#include <tpie/btree/tree.h>
#include <algorithm>
#include <set>
using namespace tpie;
using namespace std;

template <typename node_t, typename iter_t> 
bool compare(node_t & n, iter_t & i, iter_t end) {
	if (n.leaf()) {
		for (size_t j=0; j < n.count(); ++j) {
			if (i == end) return false;
			if (n.value(j) != *i) return false;
			++i;
		}
	} else {
		for (size_t j=0; j < n.count(); ++j) {
			n.child(j);
			if (!compare(n, i, end)) return false;
			n.parent();
		}
	}
	return true;
}

template <typename tree_t, typename set_t>
bool compare(tree_t & t, set_t & s) {
	if (t.empty()) return s.empty();
	typename tree_t::node_type node=t.root();
	typename set_t::iterator iter=s.begin();
	return compare(node, iter, s.end()) && iter == s.end();
}


bool basic_test() {
	typedef btree_internal_store<int> store;
    
	btree<store> tree;
	set<int> tree2;
	
	std::vector<int> x;
    for (int i=0; i < 1234; ++i) {
        x.push_back(i);
	}
	std::random_shuffle(x.begin(), x.end());
	
	for (size_t i=0; i < x.size(); ++i) {
		tree.insert(x[i]);
		tree2.insert(x[i]);
		if (!compare(tree, tree2)) return false;
		if (!tree.size() == tree2.size()) return false;
	}

	std::random_shuffle(x.begin(), x.end());
	
	for (size_t i=0; i < x.size(); ++i) {
		tree.remove(x[i]);
		tree2.erase(x[i]);
		if (!compare(tree, tree2)) return false;
		if (!tree.size() == tree2.size()) return false;
	}

	
	return true;
}

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(basic_test, "basic");
}

