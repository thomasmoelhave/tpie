// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
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

#include <boost/progress.hpp>
#include <sstream>
#include <tpie/array.h>
#include <tpie/tpie.h>

typedef size_t test_t;

using namespace tpie;

int main(int argc, char ** argv) {
	tpie_init();
	size_t mb = 1 << 7;
	if (argc > 1) std::stringstream(argv[1]) >> mb;
	size_t repeats = 64;
	if (argc > 2) std::stringstream(argv[2]) >> repeats;
	const size_t sz = mb/sizeof(test_t)*(1<<20);
	std::cout << mb << " MB, " << repeats << " repeats" << std::endl;
	{
		boost::progress_timer _;
		test_t res = 0;
		for (size_t j = 0; j < repeats; ++j) {
			tpie::array<test_t> a(sz);
			for (test_t i = 0; i < sz; i += 4096/sizeof(test_t)) {
				if (i) res ^= a[i-4096/sizeof(test_t)];
				a[i] = i+1;
			}
		}
		std::cout << res << std::endl;
	}
	{
		boost::progress_timer _;
		test_t res = 0;
		for (size_t j = 0; j < repeats; ++j) {
			test_t * a = tpie_new_array<test_t>(sz);
			for (test_t i = 0; i < sz; i += 4096/sizeof(test_t)) {
				if (i) res ^= a[i-4096/sizeof(test_t)];
				a[i] = i+1;
			}
			tpie_delete_array(a, sz);
		}
		std::cout << res << std::endl;
	}
	return 0;
}
