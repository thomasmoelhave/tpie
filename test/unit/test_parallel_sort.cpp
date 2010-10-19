// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
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
#include <tpie/parallel_sort.h>
#include <boost/random/linear_congruential.hpp>

int main(int argc, char **argv) {
	if(argc != 2) return 1;
	std::string test(argv[1]);
	boost::rand48 prng(42);
	if (test == "basic") {
		std::vector<int> v1;
		std::vector<int> v2;
		for (size_t i=0; i < 12345; ++i) {
			int x = prng();
			v1.push_back(x);
			v2.push_back(x);
		}
		std::sort(v1.begin(), v1.end());
		tpie::parallel_sort_impl<std::vector<int>::iterator, std::less<int>, 42> s(0);
		s(v2.begin(), v2.end());
		if(v1 != v2) {std::cerr << "Failed" << std::endl; exit(EXIT_FAILURE);}
		exit(EXIT_SUCCESS);
	}
	std::cerr << "No such test" << std::endl;
	return EXIT_FAILURE;
	
}
		
