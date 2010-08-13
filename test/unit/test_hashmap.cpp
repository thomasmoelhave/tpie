// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
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
#include <tpie/hashmap.h>
#include <map>
#include <stdlib.h>
#include <tpie/dprint.h>

using namespace tpie;
using namespace std;

bool basic_test() {
	hash_map<int, char> q1(200);
	map<int, char> q2;
	srandom(42);
	for(int i=0; i < 100; ++i) {
		int k = (random()*2) % 250;
		char v = random() % 265;
		q1[k] = v;
		q2[k] = v;
	}
	while (!q2.empty()) {
		if (q1.size() != q2.size()) {
			std::cerr << "Size differs " << q1.size() << " " << q2.size() << std::endl;
			return false;
		}
		for (map<int, char>::iterator i=q2.begin(); i != q2.end(); ++i) {
			if (q1.find((*i).first) == q1.end()) return false;
			if (q1[(*i).first] != (*i).second) return false;
			if (q1.find((*i).first+1) != q1.end()) return false;
		}
		int x=(*q2.begin()).first;
		q1.erase(x);
		q2.erase(x);
	}
	return true;
}


int main(int argc, char **argv) {

	if(argc != 2) return 1;
	std::string test(argv[1]);
	if (test == "basic")
		 return basic_test()?EXIT_SUCCESS:EXIT_FAILURE;
	//else if (test == "iterators") 
	//	return iterator_test()?EXIT_SUCCESS:EXIT_FAILURE;
	//else if (test == "memory") 
//		return array_memory_test()()?EXIT_SUCCESS:EXIT_FAILURE;
	return EXIT_FAILURE;
}
