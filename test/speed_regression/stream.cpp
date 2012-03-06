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

#include <tpie/tpie.h>
#include <tpie/stream.h>
#include <iostream>
#include "testtime.h"
#include "stat.h"
#include <boost/filesystem/operations.hpp>

using namespace tpie;
using namespace tpie::ami;
using namespace tpie::test;

const size_t default_mb=1;

typedef uint64_t count_t;

void usage() {
	std::cout << "Parameters: [times] [mb]" << std::endl;
}

void test(size_t mb, size_t times) {
    {
	stream<uint64_t> s("tmp", WRITE_STREAM);
	TPIE_OS_SIZE_T sz = 0;
	s.main_memory_usage(&sz, STREAM_USAGE_MAXIMUM);
	std::cout << "Stream memory usage: " << sz << std::endl;
    }
	std::vector<const char *> names;
	std::vector<uint64_t> ti;
	names.resize(3);
	ti.resize(3);
	names[0] = "Write";
	names[1] = "Read";
	names[2] = "Hash";
	tpie::test::stat s(names);
	count_t count=mb*1024*1024/sizeof(uint64_t);
	
	for(size_t i = 0; i < times; ++i) {
		test_realtime_t start;
		test_realtime_t end;
		
		boost::filesystem::remove("tmp");
		
		//The purpose of this test is to test the speed of the io calls, not the file system
		getTestRealtime(start);
		{
			stream<uint64_t> s("tmp", WRITE_STREAM);
			uint64_t x=42;
			for(count_t i=0; i < count; ++i) s.write_item(x);
		}
		getTestRealtime(end);
		s(testRealtimeDiff(start,end));
		
		uint64_t hash = 0;
		getTestRealtime(start);
		{
			stream<uint64_t> s("tmp", READ_STREAM);
			uint64_t * x;
			for(count_t i=0; i < count; ++i) {
				s.read_item(&x);
				hash = hash * 13 + *x;
			}
		}
		getTestRealtime(end);
		hash %= 100000000000000;
		s(testRealtimeDiff(start,end));
		s(hash);
		boost::filesystem::remove("tmp");
		s(ti);

	}
}

int main(int argc, char **argv) {
	size_t times = 10;
	size_t mb = default_mb;

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

	std::cout << "Wrating and Reading " << mb << " MB" << std::endl;
	tpie::tpie_init();
	::test(mb, times);
	tpie::tpie_finish();
	return EXIT_SUCCESS;
}
