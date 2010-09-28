// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2010, The TPIE development team
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
#include "priority_queue.h"
using namespace tpie;
using namespace std;

bool basic_test() {
	//Lets hope the external pq has a small block factor!
	MM_manager.set_memory_limit(15000000);
	ami::priority_queue<boost::uint64_t, bit_pertume_compare< std::greater<boost::uint64_t> > > pq(0.1);
	return basic_pq_test(pq, 350003);
}

bool medium_instance() {
	TPIE_OS_OFFSET iterations = 10000;
    MM_manager.set_memory_limit(16*1024*1024);
	progress_indicator_arrow progress("Running test","Running test:",1100,iterations,1);
    for(TPIE_OS_OFFSET it = 1100; it < iterations; it++)  {
		progress.step();
		ami::priority_queue<int, std::greater<int> > pq(0.75);
		std::priority_queue<int, vector<int>,std::less<int> > pq2;

		TPIE_OS_OFFSET elements = 71;
		TPIE_OS_SRANDOM(static_cast<unsigned int>(it));
		for(TPIE_OS_OFFSET i=0;i<elements;i++) {
			int src_int = TPIE_OS_RANDOM()%220;
			pq.push(src_int);
			pq2.push(src_int);
		}
		pq.pop();
		pq2.pop();
		pq.pop();
		pq2.pop();
		pq.pop();
		pq2.pop();
		TPIE_OS_OFFSET pop = 61; 
		for(TPIE_OS_OFFSET i=0;i<pop;i++) {
			if(!pq.empty()) {
				if(pq.top() != pq2.top()) {
					std::cerr << "Pop " << i << " got: " << pq.top() << " expected " << pq2.top() << std::endl;
					return false;
				}
				pq.pop();
				pq2.pop();
			}
		}
		for(TPIE_OS_OFFSET i=0;i<elements;i++) {
			int src_int = TPIE_OS_RANDOM()%220;
			pq.push(src_int);
			pq2.push(src_int);
		}
		while(!pq.empty()) {
			if(pq.top() != pq2.top()) {
				std::cerr << "Pop2, got: " << pq.top() << " expected " << pq2.top() << std::endl;
				return false;
			}
			pq.pop();
			pq2.pop();
		}
    }
	progress.done("Done");
	return true;
}

bool large_cycle(){
	size_t x = 524*1024*1024;
	MM_manager.set_memory_limit(x);
	ami::priority_queue<boost::uint64_t, bit_pertume_compare< std::greater<boost::uint64_t> > > pq(0.01);
	return cyclic_pq_test(pq, x / 10, 20000000);
}

template <bool crash_test>
bool large_instance(){
	const double PI = acos(-1.0);
	MM_manager.set_memory_limit(500*1024*1024);
	double mem_frac = crash_test ? 1.0 : 0.04;

	ami::priority_queue<boost::uint64_t, std::greater<boost::uint64_t> > pq(mem_frac);
	std::priority_queue<boost::uint64_t, vector<boost::uint64_t>, std::less<boost::uint64_t> > pq2;

	double cycle = crash_test ? 20000000000.0 : 50000000.0;
	const TPIE_OS_OFFSET iterations=500000000;
	progress_indicator_arrow progress("Running test","Running test:",0,iterations,1);
	for (TPIE_OS_OFFSET j=0; j<iterations; j++) {
		progress.step();
		double i = static_cast<double>(j);
		double th = (cos(i*2.0*PI/cycle)+1.0)*(RAND_MAX/2);
		if (!crash_test && (boost::uint64_t)pq.size() != (boost::uint64_t)pq2.size()) return false;
		if (!crash_test && pq.empty() != pq2.empty()) return false;

		if (!crash_test && !pq.empty() && pq.top()!=pq2.top()) {
			std::cerr << j << " Priority queues differ, got " << pq.top() << " but expected " 
					  << pq2.top() << std::endl;
			return false;
		}
		if (rand()<th) {
			boost::uint64_t r = rand();
			pq.push(r);
			if (!crash_test) pq2.push(r);
		} else {
			if (pq.empty()) continue;
			pq.pop();
			if (!crash_test) pq2.pop();
		}
		// if(j%5000000==0) {
		// 	std::cout << "\rElements in pq: " << pq.size() << "                             " <<  std::endl;
		// 	std::cout << "Memory available: " << MM_manager.memory_available()/1024/1024 << " MB" << std::endl;
		// }
	}
	progress.done("Done");
	return true;
}


int main(int argc, char **argv) {
	if(argc != 2) return 1;
	std::string test(argv[1]);
	if (test == "basic")
		return basic_test()?EXIT_SUCCESS:EXIT_FAILURE;
	else if (test == "medium")
		return medium_instance()?EXIT_SUCCESS:EXIT_FAILURE;
	else if (test == "large")
		return large_instance<false>()?EXIT_SUCCESS:EXIT_FAILURE;
	else if (test == "large_cycle")
		return large_cycle()?EXIT_SUCCESS:EXIT_FAILURE;
	return EXIT_FAILURE;
}
