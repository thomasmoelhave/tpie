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
#include <tpie/disjoint_sets.h>
#include <iostream>

using namespace tpie;
using namespace std;

#define DIE(msg) {std::cerr << msg << std::endl; return false;}

bool basic_test() {
	disjoint_sets<int> s1(307);
	for (int i=0; i < 307; ++i) {
		if (s1.is_set(i)) DIE("is_set failed");
		s1.make_set(i);
		if (!s1.is_set(i)) DIE("is_set failed");
		if (s1.count_sets() != (size_t)i+1) DIE("count_sets faild");
	}

	for (int i=1; i < 307; ++i) {
		s1.union_set(i-1, i);
		if (s1.find_set(i-1) != s1.find_set(i)) DIE("find_set failed");
		if (s1.count_sets() != 307-i) DIE("count_sets failed");
 	}
	return true;
}

class disjointsets_memory_test: public memory_test {
public:
	disjoint_sets<int> * a;
	virtual void alloc() {a = new disjoint_sets<int>(123456);}
	virtual void free() {delete a;}
	virtual size_type claimed_size() {return disjoint_sets<int>::memory_usage(123456);}
};

int main(int argc, char **argv) {
	if(argc != 2) return 1;
	std::string test(argv[1]);
	if (test == "basic")
		 return basic_test()?EXIT_SUCCESS:EXIT_FAILURE;
	else if (test == "memory") 
		return disjointsets_memory_test()()?EXIT_SUCCESS:EXIT_FAILURE;
	return EXIT_FAILURE;
}
