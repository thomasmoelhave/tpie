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

#ifndef MM_IMP_REGISTER
#define MM_IMP_REGISTER
#endif
#include "../app_config.h"
#include <tpie/mm.h>
#include <tpie/array.h>
#include <iostream>
using namespace tpie;

bool basic_test() {
	array<int> hat;
  
	//Resize
	hat.resize(52, 42);
	if (hat.size() != 52) return false;
	for (size_type i=0; i < 52; ++i)
		if (hat[i] != 42) return false;
  
	//Get and set
	for (size_type i=0; i < 52; ++i)
		hat[i] = (i * 104729) % 2251;
  
	const tpie::array<int> & hat2(hat);
	for (size_type i=0; i < 52; ++i)
		if (hat2[i] != (int)((i * 104729) % 2251)) return false;
  
	if (hat.empty()) return false;
	hat.resize(0);
	if (!hat.empty()) return false;
	return true;
}

bool iterator_test() {
	array<int> hat;
	hat.resize(52);

	for (size_type i=0; i < 52; ++i)
		hat[i] = (i * 104729) % 2251;
  
	{
		array<int>::const_iterator i=hat.begin();
		for (int j=0; j < 52; ++j) {
			if (i == hat.end()) return false;
			if (*i != ((j * 104729) % 2251)) return false;
			++i;
		}
		if (i != hat.end()) return false;
	}
	{
		array<int>::reverse_iterator i=hat.rbegin();
		for (int j=51; j >= 0; --j) {
			if (i == hat.rend()) return false;
			if (*i != ((j * 104729) % 2251)) return false;
			++i;
		}
		if (i != hat.rend()) return false;
	}
  
	return true;
}

struct memory_monitor {
	size_type base;
	size_type used;
	inline void begin() {
		base = MM_manager.memory_used();
		used = base;
	}
	inline void sample() {
		used = std::max(used, MM_manager.memory_used());
	}
	inline void clear() {
		used = MM_manager.memory_used();
	}
	inline void empty() {
		used = base;
	}
	inline size_type usage(int allocations) {
		return used-base - allocations*MM_manager.space_overhead();
	}
};

bool memory_test() {
	MM_manager.set_memory_limit(128*1024*1024);
	size_type g = array<int>::memory_required(123456);
	array<int> * a;
	memory_monitor mm;
	mm.begin();
	a = new array<int>(123456, 42);
	mm.sample();
	if (mm.usage(1) > g) {
		std::cerr << "Claimed to use " << g << " but used " << mm.usage(1) << std::endl;
		return false;
	}
	delete a;
	mm.empty();
	mm.sample();
	if (mm.usage(0) > 0) {
		std::cerr << "Leaked memory " << mm.usage(0) << std::endl;
		return false;
	}
	return true;
}

int main(int argc, char **argv) {
  
	if(argc != 2) return 1;
	std::string test(argv[1]);
	if (test == "basic")
		return basic_test()?EXIT_SUCCESS:EXIT_FAILURE;
	else if (test == "iterators") 
		return iterator_test()?EXIT_SUCCESS:EXIT_FAILURE;
	else if (test == "memory") 
		return memory_test()?EXIT_SUCCESS:EXIT_FAILURE;
	return EXIT_FAILURE;
}
