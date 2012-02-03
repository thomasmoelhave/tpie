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

#undef STREAM_UFS_BLOCK_FACTOR
#ifdef WIN32
#define STREAM_UFS_BLOCK_FACTOR 32
#else
#define STREAM_UFS_BLOCK_FACTOR 512
#endif

#include <tpie/tpie.h>
#include <tpie/stream.h>
#include <tpie/sort.h>
#include <iostream>
#include "testtime.h"
#include "stat.h"
#include <boost/filesystem/operations.hpp>

using namespace tpie;
using namespace tpie::ami;
using namespace tpie::test;

const size_t mb_default=1;

void usage() {
	std::cout << "Parameters: [times] [mb]" << std::endl;
}

void test(size_t mb, size_t times) {
	std::vector<const char *> names;
	std::vector<double> ti;
	names.resize(2);
	ti.resize(2);
	names[0] = "Write";
	names[1] = "Sort";
	tpie::test::stat s(names);
	TPIE_OS_OFFSET count=TPIE_OS_OFFSET(mb)*1024*1024/sizeof(uint64_t);
	for (size_t i=0; i < times; ++i) {
		
		test_realtime_t start;
		test_realtime_t end;
		
		boost::filesystem::remove("tmp");
		
		//The purpose of this test is to test the speed of the io calls, not the file system
		getTestRealtime(start);
		{
			stream<uint64_t> s("tmp", WRITE_STREAM);
			for(TPIE_OS_OFFSET i=0; i < count; ++i) {
				uint64_t x= (i+ 91493)*104729;
				s.write_item(x);
		}
		}
		getTestRealtime(end);
		ti[0] = testRealtimeDiff(start,end);
		
		getTestRealtime(start);
		{
			stream<uint64_t> s("tmp");
			tpie::ami::sort(&s);
		}
		getTestRealtime(end);
		ti[1] = testRealtimeDiff(start,end);
		boost::filesystem::remove("tmp");
		
		s(ti);
	}
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

	std::cout << "Writin and sorting " << mb << "mb" << std::endl;
	tpie::tpie_init();
	tpie::get_memory_manager().set_limit(1024*1024*1024);
	::test(mb, times);
	tpie::tpie_finish();
	return EXIT_SUCCESS;
}
