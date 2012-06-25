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
#include <tpie/portability.h>
#include <tpie/tpie.h>
#include <tpie/util.h>
#include <tpie/sort.h>
#include <tpie/stream.h>
#include <tpie/prime.h>
#include "common.h"
#include <iterator>
#include <tpie/progress_indicator_null.h>
#include <tpie/progress_indicator_arrow.h>
#include <vector>

class primeit {
public:
	typedef size_t prime_t;
	typedef off_t sprime_t;

private:
	primeit(prime_t s, prime_t y, prime_t i = 0) : s(s), i(i), y(y) {}

public:
	prime_t s;
	prime_t i;
	prime_t y;
	static primeit begin(prime_t size) {
		prime_t s = tpie::next_prime(size);
		prime_t y = size-16;
		prime_t i = 0;
		return primeit(s,y,i);
	}
	static primeit end(prime_t size) {
		prime_t s = tpie::next_prime(size);
		prime_t y = size-16;
		prime_t i = s;
		return primeit(s,y,i);
	}
	prime_t operator*() const {
		return static_cast<prime_t>((static_cast<tpie::uint64_t>(i) * static_cast<tpie::uint64_t>(y)) % s);
	}
	// pre-increment
	primeit& operator++() {
		++i;
		return *this;
	}
	sprime_t operator-(const primeit & other) const {
		return i-other.i;
	}
	bool operator==(const primeit & other) const {
		return s == other.s && i == other.i && y == other.y;
	}
	bool operator!=(const primeit & other) const {
		return !(*this == other);
	}
	bool operator<(const primeit & other) const {
		return i < other.i;
	}
	primeit operator+(prime_t j) const {
		return primeit(s, y, i+j);
	}
	primeit operator+(sprime_t j) const {
		return primeit(s, y, static_cast<prime_t>(i+j));
	}
};

namespace std {
template <> struct iterator_traits<primeit> {
	typedef random_access_iterator_tag iterator_category;
	typedef primeit::prime_t value_type;
	typedef primeit::sprime_t difference_type;
	typedef primeit::prime_t* pointer;
	typedef primeit::prime_t reference;
};
}

using namespace tpie;

bool ami_sort_test(size_t size) {
	ami::stream<size_t> mystream;
	primeit begin = primeit::begin(size);
	primeit end = primeit::end(size);
	size_t s = static_cast<size_t>(end-begin);

	while (begin != end) {
		size_t x= *begin;
		++begin;
		mystream.write_item(x);
	}
	ami::sort(&mystream);

	mystream.seek(0);

	size_t * x = 0;
	for(size_t i=0; i < s; ++i) {
		mystream.read_item( &x );
		if (*x != i) return false;
	}
	return true;
}

void tiny_test(teststream & ts, size_t max) {
	for (size_t size = 0; size <= max; ++size) {
		ts << size;
		progress_indicator_null pi(1);
		temp_file tmp;
		file_stream<size_t> mystream;
		mystream.open(tmp.path());

		std::vector<size_t> write(size);
		for (size_t i = 0; i < size; ++i) {
			write[i] = size-i;
		}

		mystream.write(write.begin(), write.end());
		sort(mystream, pi);

		mystream.seek(0);
		bool bad = false;
		for(size_t i=0; i < size; ++i) {
			size_t x = mystream.read();
			if (x != i+1) {
				ts << "Read failed" << failure();
				bad = true;
				break;
			}
		}
		if (bad) continue;
		ts << result(!mystream.can_read());
	}
}

template<typename Progress>
bool sort_test(size_t size, Progress & pi) {
	temp_file tmp;
	file_stream<size_t> mystream;
	mystream.open(tmp.path());

	primeit begin = primeit::begin(size);
	primeit end = primeit::end(size);
	size_t s = static_cast<size_t>(end-begin);
	tpie::log_info() << "Writing " << s << " elements" << std::endl;
	std::vector<char> seen(s);
	for (primeit i = begin; i != end; ++i) {
		seen[*i]++;
	}
	for (size_t i = 0; i < s; ++i) {
		if (seen[i] != 1) tpie::log_info() << i << " = " << (int) seen[i] << std::endl;
	}

	mystream.write(begin, end);
	sort(mystream, pi);

	mystream.seek(0);
	tpie::log_info() << "Verifying " << s << " elements" << std::endl;
	for(size_t i=0; i < s; ++i) {
		size_t x = mystream.read();
		if (x != i) {
			tpie::log_info() << "Element " << i << " is wrong; expected " << i << " but got " << x << std::endl;
			return false;
		}
	}
	if (mystream.can_read()) {
		tpie::log_info() << "Too many elements in stream" << std::endl;
		return false;
	}
	return true;
}

bool small_test(size_t n) {
	progress_indicator_null pi(1);
	return sort_test(n, pi);
}

bool large_test(size_t n) {
	progress_indicator_arrow pi("Sort", n, tpie::log_info());
	return sort_test(n, pi);
}

bool tall_test(size_t n) {
	const size_t mem = 22*1024*1024;
	tpie::get_memory_manager().set_limit(mem);
	progress_indicator_arrow pi("Sort", n, tpie::log_info());
	return sort_test(n, pi);
}

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.multi_test(tiny_test, "tiny", "n", 5)
		.test(small_test, "small", "n", 8*1024*1024)
		.test(tall_test, "tall", "n", 22*1024*1024)
		.test(large_test, "large", "n", 128*1024*1024)
		.test(ami_sort_test, "amismall", "n", 8*1024*1024)
		.test(ami_sort_test, "amilarge", "n", 128*1024*1024);
}
