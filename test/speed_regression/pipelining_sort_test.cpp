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

#include "blocksize_2MB.h"

#include <tpie/tpie.h>
#include <tpie/pipelining/sort.h>
#include <tpie/prime.h>
#include <vector>
#include <err.h>
#include "stat.h"
#include "testinfo.h"
#include "testtime.h"

using namespace tpie;
using namespace tpie::test;
using namespace tpie::pipelining;

typedef int64_t test_t;

void parse_args(int argc, char ** argv, stream_size_type & elements, size_t & times, stream_size_type & runLength, size_t & fanout) {
	int i = 1;
	while (i < argc) {
		std::string arg(argv[i]);
		if (arg == "-r") {
			std::stringstream ss(argv[i+1]);
			ss >> runLength;
			i += 2;
		} else if (arg == "-f") {
			std::stringstream ss(argv[i+1]);
			ss >> fanout;
			i += 2;
		} else if (arg == "-h" || arg == "--help") {
			std::cout << "Usage: " << argv[0] << " [-r runLength] [-f fanout] <elements> <times>" << std::endl;
			exit(1);
		} else break;
	}
	if (i < argc) {
		std::stringstream ss(argv[i]);
		ss >> elements;
		++i;
	}
	if (i < argc) {
		std::stringstream ss(argv[i]);
		ss >> times;
		++i;
	}
}

void test(test_t elements, size_t times, size_t runLength = 0, size_t fanout = 0) {
	std::vector<const char *> names(3);
	names[0] = "Sort";
	names[1] = "Verify";
	names[2] = "Correct";
	tpie::test::stat stats(names);
	for (size_t i = 0; i < times; ++i) {
		test_t s = tpie::next_prime(elements);
		test_t y = elements-16;

		test_realtime_t start;
		test_realtime_t end;

		getTestRealtime(start);

		merge_sorter<test_t> m;
		if (runLength) {
			m.set_parameters(runLength, fanout);
		}
		m.begin();
		size_t pushed = 0;
		for (test_t j = 0; j < s; ++j) {
			test_t el = (j * y) % s;
			if (el < elements) {
				m.push(el);
				++pushed;
			}
		}
		m.end();
		if (pushed != elements) errx(101, "Wut");

		getTestRealtime(end);
		stats(testRealtimeDiff(start, end));

		bool good = true;
		getTestRealtime(start);
		for (test_t j = 0; j < elements; ++j) {
			tp_assert(m.can_pull(), "Can pull");
			test_t el = m.pull();
			if (el != j) good = false;
		}
		getTestRealtime(end);
		stats(testRealtimeDiff(start, end));
		stats(good ? 1 : 0);
	}
}

int main(int argc, char ** argv) {
	stream_size_type elements = 10;
	size_t times = 1;
	size_t runLength = 0;
	size_t fanout = 0;
	parse_args(argc, argv, elements, times, runLength, fanout);
	testinfo t("Pipelining sort speed test", 1024, 0, times);
	sysinfo s;
	s.printinfo("Elements", elements);
	if (runLength) {
		s.printinfo("Run length", runLength);
		s.printinfo("Fanout", fanout);
	} else {
		s.printinfo("Run length", "(default)");
		s.printinfo("Fanout", "(default)");
	}
	::test(elements, times, runLength, fanout);
	return 0;
}
