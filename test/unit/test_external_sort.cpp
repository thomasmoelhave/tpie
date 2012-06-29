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
#include <tpie/fractional_progress.h>
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


bool test2(uint64_t size) {
	//next prime( sqrt(2)**i ) for i in 1..128
	uint64_t primes[] = {2ull, 3ull, 3ull, 5ull, 7ull, 11ull, 13ull, 17ull, 23ull, 37ull, 47ull, 67ull, 97ull, 131ull, 191ull, 257ull, 367ull, 521ull, 727ull, 1031ull, 1451ull, 2053ull, 2897ull, 4099ull, 5801ull, 8209ull, 11587ull, 16411ull, 23173ull, 32771ull, 46349ull, 65537ull, 92683ull, 131101ull, 185369ull, 262147ull, 370759ull, 524309ull, 741457ull, 1048583ull, 1482919ull, 2097169ull, 2965847ull, 4194319ull, 5931649ull, 8388617ull, 11863289ull, 16777259ull, 23726569ull, 33554467ull, 47453149ull, 67108879ull, 94906297ull, 134217757ull, 189812533ull, 268435459ull, 379625083ull, 536870923ull, 759250133ull, 1073741827ull, 1518500279ull, 2147483659ull, 3037000507ull, 4294967311ull, 6074001001ull, 8589934609ull, 12148002047ull, 17179869209ull, 24296004011ull, 34359738421ull, 48592008053ull, 68719476767ull, 97184016049ull, 137438953481ull, 194368032011ull, 274877906951ull, 388736063999ull, 549755813911ull, 777472128049ull, 1099511627791ull, 1554944255989ull, 2199023255579ull, 3109888512037ull, 4398046511119ull, 6219777023959ull, 8796093022237ull, 12439554047911ull, 17592186044423ull, 24879108095833ull, 35184372088891ull, 49758216191633ull, 70368744177679ull, 99516432383281ull, 140737488355333ull, 199032864766447ull, 281474976710677ull, 398065729532981ull, 562949953421381ull, 796131459065743ull, 1125899906842679ull, 1592262918131449ull, 2251799813685269ull, 3184525836262943ull, 4503599627370517ull, 6369051672525833ull, 9007199254740997ull, 12738103345051607ull, 18014398509482143ull, 25476206690103097ull, 36028797018963971ull, 50952413380206277ull, 72057594037928017ull, 101904826760412407ull, 144115188075855881ull, 203809653520824899ull, 288230376151711813ull, 407619307041649457ull, 576460752303423619ull, 815238614083298939ull, 1152921504606847009ull, 1630477228166597791ull, 2305843009213693967ull, 3260954456333195593ull, 4611686018427388039ull, 6521908912666391129ull, 9223372036854775837ull, 13043817825332782231ull};
	for (size_t i=0; true; ++i) {
		if (primes[i] < size) continue;
		size=primes[i];
		break;
	}
	
	tpie::progress_indicator_arrow pi("", size);
	tpie::fractional_progress fp(&pi);
	tpie::fractional_subindicator write_p(fp, "Generate", TPIE_FSI, size, "Generate");
	tpie::fractional_subindicator sort_p(fp, "Sort", TPIE_FSI, size, "Sort");
	tpie::fractional_subindicator read_p(fp, "Check", TPIE_FSI, size, "Check");
	fp.init();
	write_p.init(size);
	tpie::file_stream<uint64_t> fs;
	fs.open();
	for(uint64_t i=0; i < size; ++i) {
		uint64_t a=((i + 17397146566654936721ull) * 2213) % size;
		uint64_t b=((i + 1011213664434068507ull) * 4259) % size;
		uint64_t c=((i + 13509602713777558990ull) * 5297) % size;
		fs.write(a);
		fs.write(b);
		fs.write(c);
		write_p.step();
	}
	write_p.done();
	sort(fs, sort_p);
	fs.seek(0);
	read_p.init(size);
	for(uint64_t i=0; i < size; ++i) {
		for (int j=0; j < 3; ++j) {
			uint64_t x=fs.read();
			if (x != i) {
				std::cerr << "Expected " << i << " got " << x << std::endl;
				return false;
			}
		}
		read_p.step();
	}
	read_p.done();
	fp.done();
	return true;
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
	} else if (test == "very_large") {
		return test2(1024*1024*1024);
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
