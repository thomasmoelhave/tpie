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
#include <algorithm>
#include <tpie/array.h>
#include <tpie/tpie.h>
#include <tpie/blocks/btree.h>
#include <tpie/blocks/external_store.h>
#include <tpie/tempname.h>
#include "testtime.h"
#include "testinfo.h"
#include "stat.h"

using namespace tpie;
using namespace tpie::test;

void usage() {
	std::cout << "Parameters: [repetitions] [size]" << std::endl;
}

void test(size_t times, size_t size) {
	// display code
	std::vector<const char *> names;
	names.resize(2);
	names[0] = "Insertion";
	names[1] = "Deletion";
	tpie::test::stat s(names);

	// test code
	size_t count = size * 1024 * 1024 / sizeof(int);

	test_realtime_t start;
	test_realtime_t end;

	for (size_t i = 0; i < times; ++i) {
		temp_file tmp;
		btree_external_store<int> store(tmp.path());
		btree<btree_external_store<int> > tree(store);

		// pre-protocol
		int x[count];
		for(size_t i = 0; i < count; ++i)
			x[i] = i;
		std::random_shuffle(x, x + count);

		// insertion
		getTestRealtime(start);
		for(size_t i = 0; i < count; ++i) {
			tree.insert(x[i]);
		}
		getTestRealtime(end);
		s(testRealtimeDiff(start,end));

		// deletion
		std::random_shuffle(x, x + count);

		getTestRealtime(start);
		for(size_t i = 0; i < count; ++i) {
			tree.remove(x[i]);
		}
		getTestRealtime(end);
		s(testRealtimeDiff(start,end));
	}
}

int main(int argc, char **argv) {
	size_t times = 1;
	size_t size = 5;

	if(argc > 1) {
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

	if(argc > 2) {
		if (std::string(argv[1]) == "0") {
			size = 0;
		} else {
			std::stringstream(argv[1]) >> size;
			if (!size) {
				usage();
				return EXIT_FAILURE;
			}
		}
	}

	testinfo t("External B-tree speed test", times, size);
	::test(times, size);
	return EXIT_SUCCESS;
}
