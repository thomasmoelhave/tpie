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
#include <boost/date_time/posix_time/posix_time.hpp>

bool basic1() {
	boost::rand48 prng(42);
	std::vector<int> v1;
	std::vector<int> v2;
	for (size_t i=0; i < 1234567; ++i) {
		int x = prng();
		v1.push_back(x);
		v2.push_back(x);
	}
	std::sort(v1.begin(), v1.end());
	tpie::parallel_sort_impl<std::vector<int>::iterator, std::less<int>, 42> s(0);
	s(v2.begin(), v2.end());
	if(v1 != v2) {std::cerr << "Failed" << std::endl; return false;}
	return true;
}

bool equal_elements() {
	std::vector<int> v1;
	std::vector<int> v2;
	for (size_t i=0; i < 1234567; ++i) {
		v1.push_back(42);
		v2.push_back(42);
	}
	v1.push_back(1);
	v2.push_back(1);
	v1.push_back(64);
	v2.push_back(64);

	boost::posix_time::ptime t1=boost::posix_time::microsec_clock::local_time();
	std::sort(v1.begin(), v1.end());
	boost::posix_time::ptime t2=boost::posix_time::microsec_clock::local_time();
	tpie::parallel_sort_impl<std::vector<int>::iterator, std::less<int>, 42> s(0);
	s(v2.begin(), v2.end());
	boost::posix_time::ptime t3=boost::posix_time::microsec_clock::local_time();
	if(v1 != v2) {std::cerr << "Failed" << std::endl; return false;}
	std::cout << "std: " << (t2-t1) << " ours: " << t3-t2 << std::endl;
	if( (t2-t1)*3 < (t3-t2) ) {std::cerr << "Too slow" << std::endl; return false;}
	return true;
}

void stress_test() {
	boost::posix_time::time_duration d;
	{
		std::vector<int> v;
		for(int i = 0; i < 4000000; ++i)
			v.push_back(i);
		boost::posix_time::ptime t1=boost::posix_time::microsec_clock::local_time();
		tpie::parallel_sort_impl<std::vector<int>::iterator, std::less<int>, 42> s(0);
		s(v.begin(), v.end());
		boost::posix_time::ptime t2=boost::posix_time::microsec_clock::local_time();
		d=t2-t1;
	}

	boost::rand48 prng(42);
	while(true) {
		size_t size = (1 << (prng()%18)) + prng()%100;
		std::vector<int> v1;
		std::vector<int> v2;
		for (size_t i=0; i < size; ++i) {
			v1.push_back(prng());
			v2.push_back(v1.back());
		}
		std::cout << size << " ";

		boost::posix_time::ptime t1=boost::posix_time::microsec_clock::local_time();
		std::sort(v1.begin(), v1.end());
		boost::posix_time::ptime t2=boost::posix_time::microsec_clock::local_time();
		tpie::parallel_sort_impl<std::vector<int>::iterator, std::less<int>, 42> s(0);
		s(v2.begin(), v2.end());
		boost::posix_time::ptime t3=boost::posix_time::microsec_clock::local_time();
		if(v1 != v2) {std::cerr << "Failed" << std::endl; return;}
		std::cout << "std: " << (t2-t1) << " ours: " << t3-t2 << std::endl;
		if( std::max(d,(t2-t1)*3) < (t3-t2)  ) {std::cerr << "Too slow" << std::endl; return;}
	}
}



int main(int argc, char **argv) {
	if(argc != 2) return 1;
	std::string test(argv[1]);
	if (test == "basic1") {
		exit(basic1()?EXIT_SUCCESS:EXIT_FAILURE);
	} else if (test == "equal_elements") {
		exit(equal_elements()?EXIT_SUCCESS:EXIT_FAILURE);
	} else if (test == "stress_test") {
		stress_test();
		return EXIT_FAILURE;
	}

	std::cerr << "No such test" << std::endl;
	return EXIT_FAILURE;
	
}
		
