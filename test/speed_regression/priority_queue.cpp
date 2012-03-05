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
/*
#include "../app_config.h"

#include <tpie/tpie.h>
#include <tpie/priority_queue.h>
#include <iostream>
#include "testtime.h"
#include <tpie/progress_indicator_arrow.h>
#include <boost/random.hpp>

using namespace tpie::test;

//const size_t size=1024*1024/sizeof(uint64_t);

const TPIE_OS_SIZE_T defmemory = 750*1024*1024;

struct segment_t {
	float x1, y1, z1, x2, y2, z2;
	segment_t(float x1, float y1, float z1, float x2, float y2, float z2)
		: x1(x1), y1(y1), z1(z1), x2(x2), y2(y2), z2(z2) {
	}
	segment_t() {
	}
	inline bool operator<(const segment_t & other) const {
		if (x1 != other.x1) return x1 < other.x1;
		if (y1 != other.y1) return y1 < other.y1;
		if (x2 != other.x2) return x2 < other.x2;
		return y2 < other.y2;
	}
};

static const float min = -1000.0f;
static const float max = 1000.0f;
boost::mt19937 rng;
boost::uniform_real<float> rng_float(min, max);
boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > gen(rng, rng_float);

template <typename test_t>
inline test_t generator(size_t);

template <>
inline segment_t generator<segment_t>(size_t) {
	return segment_t(gen(), gen(), gen(), gen(), gen(), gen());
}

template <>
inline uint64_t generator<uint64_t>(size_t el) {
	return 4373 + 7879*el;
}

template<typename test_t>
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
		tpie::ami::priority_queue<test_t> pq(1.0, blockFactor);
		getTestRealtime(begin);
		for (size_t el = 0; el < elems; ++el) {
			if (progress != 0)
				progress->step();

			pq.push(generator<test_t>(el));
		}
		getTestRealtime(push);
		std::cout << testRealtimeDiff(begin, push) << " " << std::flush;
		for (size_t el = 0; el < elems; ++el) {
			if (progress != 0)
				progress->step();

			pq.pop();
		}
		getTestRealtime(pop);
	}
	getTestRealtime(end);
	std::cout << testRealtimeDiff(push, pop) << " " << testRealtimeDiff(start, end) << std::endl;
}

void usage() {
	std::cout << "Parameters: [-s] [-b blockfact|-B] [--progress] <times> {<elements>|<blocks>b} [<memory>]\n"
		<< "-s: Use line segments (float^6) rather than uint64_t\n"
		<< "-b: Use specified block factor rather than default\n"
		<< "-B: Conduct block factor test\n"
		<< "--progress: Provide progress information\n"
		<< "<times>: Perform this number of tests\n"
		<< "<elements>: Use this number of elements in each test\n"
		<< "<blocks>b: Use this number of blocks in each test\n"
		<< "<memory>: Use at most this amount of memory (in bytes)" << std::endl;
}

template <typename test_t>
int process_args(int argc, char **argv) {
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
	if ('b' == *std::string(argv[2]).rbegin()) {
		elements = elements * 2 * 1024*1024 * blockFactor / sizeof(test_t);
	}

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
		pqtest_elements<test_t>(elements, blockFactorTest ? (1.0+i)/times : blockFactor, use_progress ? progress.get() : 0);
		if (use_progress) {
			progress.reset(0);
			std::cout << std::endl;
		}
	}

	tpie::tpie_finish();

	return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
	tpie::tpie_init();
	tpie::get_memory_manager().set_enforcement(tpie::memory_manager::ENFORCE_THROW);

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
				pqtest_elements<uint64_t>(elements);
			}
			base *= 2;
		}

		tpie::tpie_finish();

		return EXIT_SUCCESS;
	} else { // argc > 2
		if (std::string(argv[1]) == "-s") {
			return process_args<segment_t>(argc-1, argv+1);
		} else {
			return process_args<uint64_t>(argc, argv);
		}
	}
}
*/
#include "../app_config.h"

#undef STREAM_UFS_BLOCK_FACTOR
#ifdef WIN32
#define STREAM_UFS_BLOCK_FACTOR 32
#else
#define STREAM_UFS_BLOCK_FACTOR 512
#endif

#include <tpie/tpie.h>
#include <iostream>
#include "testtime.h"
#include "stat.h"
#include <boost/filesystem/operations.hpp>
#include <tpie/priority_queue.h>

using namespace tpie;
using namespace tpie::ami;
using namespace tpie::test;

const size_t mb_default=1;

void usage() {
	std::cout << "Parameters: [times] [mb]" << std::endl;
}

void test(size_t mb, size_t times) {
	std::vector<const char *> names;
	names.resize(2);
	uint64_t a=0;
	names[0] = "Push";
	names[1] = "Pop";
	tpie::test::stat s(names);
	TPIE_OS_OFFSET count=TPIE_OS_OFFSET(mb)*1024*1024/sizeof(uint64_t);
	for (size_t i=0; i < times; ++i) {
		
		test_realtime_t start;
		test_realtime_t end;
		getTestRealtime(start);
		{
			tpie::ami::priority_queue<uint64_t> pq(0.95, 0.125);
		
			for(TPIE_OS_OFFSET i=0; i < count; ++i) {
				uint64_t x= (i+ 91493)*104729;
				pq.push(x);
			}
			getTestRealtime(end);
			s(testRealtimeDiff(start,end));

			getTestRealtime(start);
			for(TPIE_OS_OFFSET i=0; i < count; ++i) {
				uint64_t x=pq.top();
				pq.pop();
				a ^= x;
			}
			getTestRealtime(end);
			s(testRealtimeDiff(start,end));
		}
	}
	if (a == 42) std::cout << "oh rly" << std::endl;
}

int main(int argc, char **argv) {
	size_t times = 10;
	size_t mb = mb_default;
			
	if (argc > 1) {
		if (std::string(argv[1]) == "0") {
			times = 0;
		} else {
			std::stringstream(argv[1]) >> times;
			if (!times) {
				usage();
				return EXIT_FAILURE;
			}
		}
	}
	if (argc > 2) {
		std::stringstream(argv[2]) >> mb;
		if (!mb) {
			usage();
			return EXIT_FAILURE;
		}
	}

	std::cout << "Push and pop " << mb << "mb" << std::endl;
	tpie::tpie_init();
	tpie::get_memory_manager().set_limit(1024*1024*1024);
	::test(mb, times);
	tpie::tpie_finish();
	return EXIT_SUCCESS;
}
