// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2011, The TPIE development team
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
#include "common.h"
#include <queue>
#include <boost/filesystem.hpp>
#include <tpie/queue.h>

using namespace tpie;

inline uint64_t element(size_t i) {
	return 179424673 * i + 15485863;
}

#if 0
std::ostream & debug = std::cout;
#else
std::ostream debug(0); // bit bucket
#endif

bool queue_test(const size_t elements = 2*1024*1024/sizeof(uint64_t)) {
	const size_t maxpush = 64;
	queue<uint64_t> q1;
	std::queue<uint64_t> q2;

	size_t i = 0;

	// number of elements currently in the queue
	uint64_t l = 0;

	while (i < elements) {
		// First, push a random number of elements to our queue and std::queue.
		uint64_t push = 1 + (element(i) % (maxpush - 1));
		for (uint64_t j = 0; i < elements && j < push; ++i, ++j) {
			debug << "Push " << i << " " << element(i) << " " << std::endl;
			q1.push(element(i));
			q2.push(element(i));
		}

		l += push;

		// Next, pop a random number of elements.
		uint64_t pop = 1 + (element(i) % (l - 1));
		for (uint64_t j = 0; i < elements && j < pop; ++i, ++j) {
			debug << "Pop " << i << std::endl;

			uint64_t el = q2.front();
			q2.pop();

			uint64_t got = q1.front();

			// our queue implementation also returns an element in pop()
			if (got != q1.pop()) {
				tpie::log_info() << "pop() doesn't agree with front() on element " << i << std::endl;
				return false;
			}

			if (el != got) {
				tpie::log_info() << "front() returned incorrect element " << i << std::endl;
				return false;
			}

			debug << "Got " << el << std::endl;
		}

		l -= pop;
	}

	return true;
}

bool basic_test() {
	return queue_test();
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv, 32)
		.test(basic_test, "basic")
		.test(queue_test, "sized", "n", static_cast<size_t>(32*1024*1024/sizeof(uint64_t)))
		;
}
