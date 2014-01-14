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

#include "blocksize_2MB.h"

#include <boost/filesystem/operations.hpp>
#include <tpie/tpie.h>
#include <iostream>
#include "testtime.h"
#include "stat.h"
#include "testinfo.h"

#include <tpie/file_stream.h>
#include <tpie/sort.h>  
#include <tpie/pipelining.h>
#include <tpie/tpie_assert.h>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/mersenne_twister.hpp>

using namespace tpie;
using namespace tpie::test;
using namespace tpie::pipelining;

const size_t mb_default=1;

void usage() {
	std::cout << "Parameters: [times] [mb] [memory]" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// Node that pushes n pseudorandom natural numbers
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
class generator_t : public node {
	dest_t dest;
	memory_size_type n;
public:
	generator_t(const dest_t & dest, const memory_size_type & n) : dest(dest), n(n)
	{
		add_push_destination(dest);
		set_name("Integer generator");
	}

	virtual void go() override {
		boost::mt19937 rng(42);
		boost::uniform_int<memory_size_type> dist(0, std::numeric_limits<memory_size_type>::max());
		boost::variate_generator<boost::mt19937&, boost::uniform_int<memory_size_type> > generator(rng, dist);

		for(memory_size_type i = 0; i < n; ++i) dest.push(generator() % n);
	}
};

typedef pipe_begin<factory_1<generator_t, memory_size_type> > generator;

///////////////////////////////////////////////////////////////////////////////
/// Node that checks the order of the pushed items
///////////////////////////////////////////////////////////////////////////////
class check_t : public node {
	memory_size_type last;
public:
	typedef memory_size_type item_type;

	check_t() : last(0)
	{
		set_name("Non-descending order check");
	}

	void push(const item_type & item) {
		//log_info() << item << std::endl;
		if(last > item) {
			std::cout << "Not sorted!" << std::endl;
		}
		last = item;
	}
};

typedef pipe_end<termfactory_0<check_t> > check;

void test(size_t mb, size_t times) {
	std::vector<const char *> names;
	names.resize(4);
	names[0] = "Parameters";
	names[1] = "Phase 1";
	names[2] = "Phase 2";
	names[3] = "Phase 3";
	tpie::test::stat s(names);
	memory_size_type count=static_cast<memory_size_type>(mb)*1024*1024/sizeof(memory_size_type);

	for (memory_size_type i=0; i < times; ++i) {
		test_realtime_t start;
		test_realtime_t end;

		merge_sorter<memory_size_type, false> merge_sorter;

		// Set parameters
		getTestRealtime(start);
		{
			//merge_sorter.set_available_memory(get_memory_manager().available());
			memory_size_type mem = 1024*1024*1000;
			merge_sorter.set_parameters(get_block_size() / sizeof(memory_size_type), 32);
		}
		getTestRealtime(end);
		s(testRealtimeDiff(start, end));

		// Phase 1 - Push elements
		boost::mt19937 rng(42);
		boost::uniform_int<memory_size_type> dist(0, std::numeric_limits<memory_size_type>::max());
		boost::variate_generator<boost::mt19937&, boost::uniform_int<memory_size_type> > generator(rng, dist);

		getTestRealtime(start);
		{
			merge_sorter.begin();
			for(memory_size_type i = 0; i < count; ++i) {
				merge_sorter.push(generator());
			}
			merge_sorter.end();
		}
		getTestRealtime(end);
		s(testRealtimeDiff(start, end));

		// Phase 2 - Perform merges
		dummy_progress_indicator pi;
		getTestRealtime(start);
		{
			merge_sorter.calc(pi);
		}
		getTestRealtime(end);
		s(testRealtimeDiff(start, end));

		// Phase 3 - Pull
		memory_size_type l = 0;
		getTestRealtime(start);
		{
			for(memory_size_type i = 0; i < count; ++i) {
				memory_size_type e = merge_sorter.pull();
				tp_assert(e >= l, "Elements were not sorted");
				l = e;
			}
		}
		getTestRealtime(end);
		s(testRealtimeDiff(start, end));		

	}
}

int main(int argc, char **argv) {
	size_t times = 10;
	size_t mb = mb_default;
	size_t memory = 1024;
			
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
	if (argc > 3) {
		std::stringstream(argv[3]) >> memory;
		if (!memory) {
			usage();
			return EXIT_FAILURE;
		}
	}

	testinfo t("Sort speed test", memory, mb, times);
	::test(mb, times);
	return EXIT_SUCCESS;
}
