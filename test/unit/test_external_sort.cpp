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
private:
	primeit(int64_t s, int64_t y, int64_t i = 0) : s(s), i(i), y(y) {}
public:
	int64_t s;
	int64_t i;
	int64_t y;
	static primeit begin(int64_t size) {
		int64_t s = tpie::next_prime(size);
		int64_t y = size-16;
		int64_t i = 0;
		return primeit(s,y,i);
	}
	static primeit end(int64_t size) {
		int64_t s = tpie::next_prime(size);
		int64_t y = size-16;
		int64_t i = s;
		return primeit(s,y,i);
	}
	size_t operator*() const {
		return (i * y) % s;
	}
	// pre-increment
	primeit& operator++() {
		++i;
		return *this;
	}
	int operator-(const primeit & other) const {
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
	primeit operator+(int64_t j) const {
		return primeit(s, y, i+j);
	}
	primeit operator+(uint64_t j) const {
		return primeit(s, y, i+j);
	}
};

namespace std {
template <> struct iterator_traits<primeit> {
	typedef random_access_iterator_tag iterator_category;
	typedef int64_t value_type;
	typedef int64_t difference_type;
	typedef int64_t* pointer;
	typedef int64_t reference;
};
}

using namespace tpie;

bool ami_sort_test(size_t size) {
	ami::stream<int64_t> mystream;
	primeit begin = primeit::begin(size);
	primeit end = primeit::end(size);
	int64_t s = end-begin;

	while (begin != end) {
		size_t x= *begin;
		++begin;
		mystream.write_item(x);
	}
	ami::sort(&mystream);

	mystream.seek(0);

	int64_t * x = 0;
	for(int64_t i=0; i < s; ++i) {
		mystream.read_item( &x );
		if (*x != i) return false;
	}
	return true;
}

template <typename Progress>
bool small_sort_test(int64_t size, Progress & pi) {
	temp_file tmp;
	file_stream<int64_t> mystream;
	mystream.open(tmp.path());

	std::vector<int64_t> write(size);
	for (int64_t i = 0; i < size; ++i) {
		write[i] = size-i;
	}

	mystream.write(write.begin(), write.end());
	sort(mystream, pi);

	mystream.seek(0);
	for(int64_t i=0; i < size; ++i) {
		int64_t x = mystream.read();
		if (x != i+1) return false;
	}
	return !mystream.can_read();
}

template<typename Progress>
bool sort_test(size_t size, Progress & pi) {
	temp_file tmp;
	file_stream<int64_t> mystream;
	mystream.open(tmp.path());

	primeit begin = primeit::begin(size);
	primeit end = primeit::end(size);
	int64_t s = end-begin;

	mystream.write(begin, end);
	sort(mystream, pi);

	mystream.seek(0);
	for(int64_t i=0; i < s; ++i) {
		int64_t x = mystream.read();
		if (x != i) return false;
	}
	return !mystream.can_read();
}


bool perform_test(const std::string & test) {
	if (test == "small") {
		progress_indicator_null pi(1);
		return sort_test(1024 * 1024 * 8, pi);
	} else if (test == "tall") {
		//const int mem = (17*1024+52)*1024+760;
		const size_t size = 22*1024*1024;
		tpie::get_memory_manager().set_limit(size);
		progress_indicator_arrow pi("Sort", size);
		return sort_test(size, pi);
	} else if (test == "large") {
		progress_indicator_arrow pi("Sort", 1);
		return sort_test(1024*1024*128, pi);
	} else if (test == "amismall") {
		return ami_sort_test(1024 * 1024 * 8);
	} else if (test == "amilarge") {
		return ami_sort_test(1024*1024*128);
	}
	std::stringstream ss(test);
	size_t size;
	ss >> size;
	if (test == "0" || size > 0) {
		progress_indicator_arrow pi("Sort", size);
		if (size < 16) {
			return small_sort_test(size, pi);
		} else {
			return sort_test(size, pi);
		}
	}
	return false;
}


int main(int argc, char **argv) {
	tpie_initer _;
	if (argc != 2) return 1;
	bool ok=perform_test(std::string(argv[1]));
	return ok?EXIT_SUCCESS:EXIT_FAILURE;
}
