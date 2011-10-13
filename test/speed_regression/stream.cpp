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
#include <tpie/stream.h>
#include <iostream>
#include "testtime.h"
#include <boost/filesystem/operations.hpp>

using namespace tpie::ami;
using namespace tpie::test;

const size_t count_default=1024*1024/sizeof(uint64_t);

void usage() {
	std::cout << "Parameters: [times] [count]" << std::endl;
}

void test(size_t count) {
	test_realtime_t start;
	test_realtime_t end;

	//The purpose of this test is to test the speed of the io calls, not the file system
	getTestRealtime(start);
	{
		stream<uint64_t> s("tmp", WRITE_STREAM);
		uint64_t x=42;
		for(size_t i=0; i < count*1024; ++i) s.write_item(x);
	}
	getTestRealtime(end);
	std::cout << testRealtimeDiff(start,end);
	std::cout.flush();
	
	getTestRealtime(start);
	{
		stream<uint64_t> s("tmp", READ_STREAM);
		uint64_t * x;
		for(size_t i=0; i < count*1024; ++i) s.read_item(&x);
	}
	getTestRealtime(end);
	std::cout << " " << testRealtimeDiff(start,end);
	std::cout.flush();

	boost::filesystem::remove("tmp");
	
	getTestRealtime(start);
	{
		uint64_t x[1024];
		std::fill(x+0, x+1024, 42);
		stream<uint64_t> s("tmp", WRITE_STREAM);
		for(size_t i=0; i < count; ++i) s.write_array(x,1024);
	}
	getTestRealtime(end);
	std::cout << " " << testRealtimeDiff(start,end);
	std::cout.flush();
	
	getTestRealtime(start);
	{
		uint64_t x[1024];
		std::fill(x+0, x+1024, 42);
		stream<uint64_t> s("tmp", READ_STREAM);
		for(size_t i=0; i < count; ++i) {TPIE_OS_OFFSET y=1024; s.read_array(x,&y);}
	}
	getTestRealtime(end);
	std::cout << " " << testRealtimeDiff(start,end) << std::endl;
}

int main(int argc, char **argv) {
	size_t times = 10;
	size_t count = count_default;

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
		std::stringstream(argv[2]) >> count;
		if (!count) {
			usage();
			return EXIT_FAILURE;
		}
	}

	std::cout << "Writing " << count << "*1024 items, reading them, writing " << count << " arrays, reading them" << std::endl;

	tpie::tpie_init();

	for (size_t i = 0; i < times || !times; ++i) {
		test(count);
	}
	
	tpie::tpie_finish();

	return EXIT_SUCCESS;
}
