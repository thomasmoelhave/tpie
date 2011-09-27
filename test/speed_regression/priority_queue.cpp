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
#include <tpie/progress_indicator_arrow.h>

using namespace tpie::test;

//const size_t size=1024*1024/sizeof(uint64_t);

const TPIE_OS_SIZE_T defmemory = 750*1024*1024;

void pqtest_elements(size_t elems, double blockFactor = 1.0, tpie::progress_indicator_arrow *progress = 0) {
	test_realtime_t start;
	test_realtime_t begin; // after ctor
	test_realtime_t push; // after pushing
	test_realtime_t pop; // after popping
	test_realtime_t end; // after dtor

	std::cout << blockFactor << " " << elems << " ";
	std::cout.flush();
	getTestRealtime(start);
	{
		tpie::ami::priority_queue<uint64_t> pq(1.0, blockFactor);
		getTestRealtime(begin);
		for (size_t el = 0; el < elems; ++el) {
			if (progress != 0)
				progress->step();

			pq.push(4373 + 7879*el);
		}
		getTestRealtime(push);
		std::cout << testRealtimeDiff(begin, push) << " ";
		std::cout.flush();
		for (size_t el = 0; el < elems; ++el) {
			if (progress != 0)
				progress->step();

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

	if (argc == 2) {
		usage();
		return EXIT_FAILURE;
	}

	if (argc < 2) {
		TPIE_OS_SIZE_T memory = defmemory;
		tpie::get_memory_manager().set_limit(memory);
		std::cout << "Memory limit: " << memory << std::endl;
		std::cout << "Blockfact Elems Push Pop Total" << std::endl;

		size_t base = 64*1024;
		const size_t times = 4;
		while (true) {
			const size_t end = base*2;
			for (size_t elements = base; elements < end; elements += base/times) {
				pqtest_elements(elements);
			}
			base *= 2;
		}
	} else { // argc > 2
		bool blockFactorTest = false;
		double blockFactor = 1.0;
		size_t times, elements;
		TPIE_OS_SIZE_T memory = defmemory;
		if (std::string(argv[1]) == "-b") {
			++argv; --argc;
			std::stringstream(argv[1]) >> blockFactor;
			++argv; --argc;
		} else if (std::string(argv[1]) == "-B") {
			++argv; --argc;
			blockFactorTest = true;
		}
		bool use_progress = false;
		if (std::string(argv[1]) == "--progress") {
			use_progress = true;
			++argv; --argc;
		}
		tpie::auto_ptr<tpie::progress_indicator_arrow> progress;
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
		tpie::get_memory_manager().set_limit(memory);
		std::cout << "Memory limit: " << memory << std::endl;
		std::cout << times << " times, " << elements << " elements" << std::endl;
		if (blockFactorTest)
			std::cout << "Block factor test." << std::endl;
		if (!use_progress)
			std::cout << "Blockfact Elems Push Pop Total" << std::endl;

		if (times == 0) {
			blockFactorTest = false;
		}

		for (size_t i = 0; i < times || times == 0; ++i) {
			if (use_progress)
				progress.reset(tpie::tpie_new<tpie::progress_indicator_arrow>("Priority queue speed test", elements*2));
			pqtest_elements(elements, blockFactorTest ? (1.0+i)/times : blockFactor, use_progress ? progress.get() : 0);
			if (use_progress) {
				progress.reset(0);
				std::cout << std::endl;
			}
		}
	}

	tpie::tpie_finish();

	return EXIT_SUCCESS;
}
