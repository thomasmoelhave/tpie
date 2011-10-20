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
#include <tpie/progress_indicator_arrow.h>
#include <tpie/memory.h>
#include "../speed_regression/testtime.h"

//#define TPIE_TEST_PARALLEL_SORT
#ifdef TPIE_TEST_PARALLEL_SORT
	#include <parallel/algorithm>
#endif

static bool progress;
static bool stdsort;

using namespace tpie;

template<size_t min_size>
bool basic1(const size_t elements = 1024*1024) {
	const size_t stepevery = elements / 16;
	boost::rand48 prng(42);
	std::vector<int> v1(elements);
	std::vector<int> v2(elements);

	auto_ptr<progress_indicator_arrow> pi;
	auto_ptr<fractional_progress> fp;
	auto_ptr<fractional_subindicator> gen_p;
	auto_ptr<fractional_subindicator> std_p;
	auto_ptr<fractional_subindicator> par_p;
	if (progress) {
		pi.reset(tpie_new<progress_indicator_arrow>("Parallel sort", elements));
		fp.reset(tpie_new<fractional_progress>(pi.get()));

		gen_p.reset(tpie_new<fractional_subindicator>(*fp.get(), "Generate", TPIE_FSI, elements, "Generate"));
		if (stdsort) std_p.reset(tpie_new<fractional_subindicator>(*fp.get(), "std::sort", TPIE_FSI, elements, "std::sort"));
		par_p.reset(tpie_new<fractional_subindicator>(*fp.get(), "parallel_sort", TPIE_FSI, elements, "parallel_sort"));
		fp->init();
	}

	if (progress) gen_p->init(elements/stepevery);
	size_t nextstep = stepevery;
	for (size_t i = 0; i < elements; ++i) {
		if (progress && i == nextstep) {
			gen_p->step();
			nextstep += stepevery;
		}
		if (stdsort)
			v1[i] = v2[i] = prng();
		else
			v1[i] = prng();
	}
	if (progress) gen_p->done();

	//boost::posix_time::time_duration t1;
	//boost::posix_time::time_duration t2;
	{
		boost::posix_time::ptime start=boost::posix_time::microsec_clock::local_time();
		parallel_sort_impl<std::vector<int>::iterator, std::less<int>, min_size > s(progress ? par_p.get() : 0);
		s(v2.begin(), v2.end());
		boost::posix_time::ptime end=boost::posix_time::microsec_clock::local_time();
		std::cout << end-start << " " << std::endl;
	}

	if (stdsort) {
		boost::posix_time::ptime start=boost::posix_time::microsec_clock::local_time();
		if (progress) std_p->init(1);
		#ifdef TPIE_TEST_PARALLEL_SORT
		__gnu_parallel::sort(v1.begin(), v1.end());
		#else
		std::sort(v1.begin(), v1.end());
		#endif
		if (progress) std_p->done();
		boost::posix_time::ptime end=boost::posix_time::microsec_clock::local_time();
		std::cout << end-start << " " << std::endl;
	}

	if (progress) fp->done();

	if (stdsort && v1 != v2) {
		std::cerr << "std::sort and parallel_sort disagree" << std::endl;
		return false;
	}
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
	parallel_sort_impl<std::vector<int>::iterator, std::less<int> > s(0);
	s(v2.begin(), v2.end());
	boost::posix_time::ptime t3=boost::posix_time::microsec_clock::local_time();
	if(v1 != v2) {std::cerr << "Failed" << std::endl; return false;}
	std::cout << "std: " << (t2-t1) << " ours: " << t3-t2 << std::endl;
	if( (t2-t1)*3 < (t3-t2) ) {std::cerr << "Too slow" << std::endl; return false;}
	return true;
}

bool bad_case() {
	const size_t n = 1024*1024;
	std::vector<int> v1;
	std::vector<int> v2;
	for (size_t i=0; i < 8*n; ++i) {
		const int el = (i % n && i != (8*n-1)) ? 42 : 36;
		v1.push_back(el);
		v2.push_back(el);
	}

	boost::posix_time::ptime t1=boost::posix_time::microsec_clock::local_time();
	std::sort(v1.begin(), v1.end());
	boost::posix_time::ptime t2=boost::posix_time::microsec_clock::local_time();
	parallel_sort_impl<std::vector<int>::iterator, std::less<int>, 42> s(0);
	s(v2.begin(), v2.end());
	boost::posix_time::ptime t3=boost::posix_time::microsec_clock::local_time();
	if(v1 != v2) {std::cerr << "Failed" << std::endl; return false;}
	std::cout << "std: " << (t2-t1) << " ours: " << t3-t2 << std::endl;
	if( (t2-t1)*3 < (t3-t2) ) {std::cerr << "Too slow" << std::endl; return false;}
	return true;
}

void stress_test() {
	boost::rand48 prng(42);
	for (size_t size_base = 1024;; size_base *= 2) {
		for (size_t size = size_base; size < size_base * 2; size += size_base / 4) {
			std::vector<size_t> v1(size);
			std::vector<size_t> v2(size);
			for (size_t i=0; i < size; ++i) {
				v1[i] = v2[i] = prng();
			}
			std::cout << size << " " << std::flush;

			boost::posix_time::time_duration t1;
			boost::posix_time::time_duration t2;
			{
				boost::posix_time::ptime start=boost::posix_time::microsec_clock::local_time();
				std::sort(v1.begin(), v1.end());
				boost::posix_time::ptime end=boost::posix_time::microsec_clock::local_time();
				std::cout << "std: " << (t1 = end-start) << std::flush;
			}
			{
				boost::posix_time::ptime start=boost::posix_time::microsec_clock::local_time();
				parallel_sort_impl<std::vector<size_t>::iterator, std::less<size_t>, 524288/8 > s(0);
				s(v2.begin(), v2.end());
				boost::posix_time::ptime end=boost::posix_time::microsec_clock::local_time();
				std::cout << " ours: " << (t2 = end-start) << std::endl;
			}
			if( t1*3 < t2  ) {std::cerr << "Too slow" << std::endl; return;}
		}
	}
}


int main(int argc, char **argv) {
	tpie_initer _;
	progress = stdsort = true;
	--argc; ++argv;
	std::string test;
	while (argc) {
		std::string arg = argv[0];
		if (arg == "--no-progress") {
			progress = false;
		} else if (arg == "--no-stdsort") {
			stdsort = false;
		} else {
			test = arg;
			break;
		}
		--argc; ++argv;
	}
#define USAGE ("Usage: [--no-progress] [--no-stdsort] <basic1|basic2|medium|large|verylarge|equal_elements|bad_case|stress_test>")
	if (!argc) {
		std::cerr << USAGE << std::endl;
		return EXIT_FAILURE;
	}
	if (test == "basic1") {
		return basic1<2>(1024*1024) ? EXIT_SUCCESS : EXIT_FAILURE;
	} else if (test == "basic2") {
		return basic1<8>(8*8) ? EXIT_SUCCESS : EXIT_FAILURE;
	} else if (test == "medium") {
		return basic1<1024*1024>(1024*1024*24) ? EXIT_SUCCESS : EXIT_FAILURE;
	} else if (test == "large") {
		return basic1<1024*1024>(1024*1024*256) ? EXIT_SUCCESS : EXIT_FAILURE;
	} else if (test == "verylarge") {
		return basic1<1024*1024>(1024*1024*768) ? EXIT_SUCCESS : EXIT_FAILURE;
	} else if (test == "equal_elements") {
		exit(equal_elements()?EXIT_SUCCESS:EXIT_FAILURE);
	} else if (test == "bad_case") {
		return bad_case() ? EXIT_SUCCESS : EXIT_FAILURE;
	} else if (test == "stress_test") {
		stress_test();
		return EXIT_FAILURE;
	}

	std::cerr << USAGE << std::endl;
	return EXIT_FAILURE;
}
