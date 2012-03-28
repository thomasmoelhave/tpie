// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
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

#include "blocksize_128KB.h"

#include <tpie/tpie.h>
#include <iostream>
#include "testtime.h"
#include "stat.h"
#include <boost/filesystem/operations.hpp>
#include <tpie/priority_queue.h>
#include "testinfo.h"

using namespace tpie;
using namespace tpie::ami;
using namespace tpie::test;

const size_t mb_default=1;

void usage() {
	std::cout << "Parameters: [times] [mb]" << std::endl;
}

void test(size_t mb, size_t times, double blockFactor = 0.125) {
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
			tpie::ami::priority_queue<uint64_t> pq(0.95, blockFactor);
		
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
	double blockFactor = 0.125;

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
		if (!(std::stringstream(argv[3]) >> blockFactor) || blockFactor*2*1024*1024 < 8) {
			usage();
			return EXIT_FAILURE;
		}
	}

	testinfo t("Priority queue speed test", 1024, mb, times);
	sysinfo().printinfo("Block factor", blockFactor);
	::test(mb, times, blockFactor);
	return EXIT_SUCCESS;
}
