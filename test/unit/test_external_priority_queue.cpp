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
#include <tpie/priority_queue.h>
#include <vector>
#include <stdint.h>
using namespace tpie;

bool basic_test() {
	uint64_t size=350003; //Must be a prime
	//Lets hope the external pq has a small block factor!
	MM_manager.set_memory_limit(1500000);
	ami::priority_queue<uint64_t> pq(1.0);
	for(uint64_t i=0; i < size; ++i)
		pq.push( (i*40849+37159)%size );
	for(uint64_t i=0; i < 2473; ++i) {
		if (pq.empty()) return false;
		if (pq.top() != i) return false;
		pq.pop();
	}
	for(uint64_t i=0; i < 2473; ++i)
		pq.push((i*40849+37159)%2473);
	for(uint64_t i=0; i < size; ++i) {
		if (pq.empty()) return false;
		if (pq.top() != i) return false;
		pq.pop();
	}
	if (!pq.empty()) return false; 
	return true;
}

int main(int argc, char **argv) {
	if(argc != 2) return 1;
	std::string test(argv[1]);
	if (test == "basic")
		return basic_test()?EXIT_SUCCESS:EXIT_FAILURE;
	return EXIT_FAILURE;
}
