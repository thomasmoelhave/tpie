// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, The TPIE development team
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
#include "../app_config.h"

#include <tpie/tpie.h>
#include <tpie/priority_queue.h>
#include <iostream>
#include "testtime.h"

using namespace tpie::test;

static const TPIE_OS_SIZE_T mm_avail = 32*1024*1024;

//const size_t size=1024*1024/sizeof(uint64_t);

void pqtest_elements(size_t elems, TPIE_OS_SIZE_T mm_avail, double blockFactor = 1.0) {
	test_realtime_t start;
	test_realtime_t begin; // after ctor
	test_realtime_t push; // after pushing
	test_realtime_t pop; // after popping
	test_realtime_t end; // after dtor

	std::cout << blockFactor << " " << elems << " ";
	std::cout.flush();
	getTestRealtime(start);
	{
		tpie::ami::priority_queue<uint64_t> pq(mm_avail, blockFactor);
		getTestRealtime(begin);
		for (size_t el = 0; el < elems; ++el) {
			pq.push(4373 + 7879*el);
		}
		getTestRealtime(push);
		std::cout << testRealtimeDiff(begin, push) << " ";
		std::cout.flush();
		for (size_t el = 0; el < elems; ++el) {
			pq.pop();
		}
		getTestRealtime(pop);
	}
	getTestRealtime(end);
	std::cout << testRealtimeDiff(push, pop) << " " << testRealtimeDiff(start, end) << std::endl;
	std::cout.flush();
}

void usage() {
	std::cout << "Parameters: <times> <elements> [<memory>]" << std::endl;
}

int main(int argc, char **argv) {
	tpie::tpie_init();
	TPIE_OS_SIZE_T memlimit = 50*1024*1024;
	tpie::get_memory_manager().set_limit(memlimit);

	if (argc == 2) {
		usage();
		return EXIT_FAILURE;
	}

	if (argc < 2) {
		TPIE_OS_SIZE_T memory = mm_avail;
		std::cout << "Memory: " << memory << " available, " << memlimit << " limit" << std::endl;
		std::cout << "Blockfact Elems Push Pop Total" << std::endl;

		size_t base = 64*1024;
		const size_t times = 4;
		while (true) {
			const size_t end = base*2;
			for (size_t elements = base; elements < end; elements += base/times) {
				pqtest_elements(elements, memory);
			}
			base *= 2;
		}
	} else { // argc > 2
		bool blockFactorTest = false;
		double blockFactor = 1.0;
		size_t times, elements;
		TPIE_OS_SIZE_T memory = mm_avail;
		if (std::string(argv[1]) == "-b") {
			++argv; --argc;
			std::stringstream(argv[1]) >> blockFactor;
			++argv; --argc;
		} else if (std::string(argv[1]) == "-B") {
			++argv; --argc;
			blockFactorTest = true;
		}
		std::stringstream(argv[1]) >> times;
		std::stringstream(argv[2]) >> elements;
		if (argc > 3) {
			std::stringstream(argv[3]) >> memory;
			if (!memory) {
				usage();
				return EXIT_FAILURE;
			}
		}
		if (!elements) {
			usage();
			return EXIT_FAILURE;
		}
		std::cout << "Memory: " << memory << " available, " << memlimit << " limit" << std::endl;
		std::cout << times << " times, " << elements << " elements" << std::endl;
		if (blockFactorTest)
			std::cout << "Block factor test." << std::endl;
		std::cout << "Blockfact Elems Push Pop Total" << std::endl;

		if (times == 0) {
			blockFactorTest = false;
		}

		for (size_t i = 0; i < times || times == 0; ++i) {
			pqtest_elements(elements, memory, blockFactorTest ? (1.0+i)/times : blockFactor);
		}
	}

	tpie::tpie_finish();

	return EXIT_SUCCESS;
}
