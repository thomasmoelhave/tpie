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

/**
 * Given a list of integers a0, a1, a2, ... an on standard input,
 * sort the numbers and add the ith and the (n-i)th number in the sorted order
 * and print the sums to standard out.
 */

#include <tpie/tpie.h>
#include <tpie/pipelining.h>
#include <boost/random.hpp>
#include <tpie/file_stream.h>
#include <iostream>
#include <sstream>
#include <iterator>
#include <tpie/progress_indicator_arrow.h>

using namespace tpie;
using namespace tpie::pipelining;
using namespace std;

//generates 'n' integers in [0,100]
template <typename dest_t>
struct Generator : public node {
	typedef int item_type;
	dest_t dest;
	int n;
	Generator(const dest_t& dest, int n) : dest(dest), n(n) {
		add_push_destination(dest);
		set_name("Generator");
	}
	virtual void go() override {
		for (int i = 0; i < n; ++i) {
			dest.push(rand()%100);
		}
	}
};

inline pipe_begin<factory_1<Generator, int> > generator(int n) {
	return factory_1<Generator,int>(n);
}

//increments each incoming value by 1
template <typename dest_t>
struct AddOne: public node {
	typedef int item_type;
	dest_t dest;
	AddOne(const dest_t& dest) : dest(dest) {
		add_push_destination(dest);
		set_name("AddOne");
	}
	void push(item_type a) {
		dest.push(a+1);
	}
};
inline pipe_middle<factory_0<AddOne> > addOne() {
	return factory_0<AddOne>();
}

template <typename source_t>
struct AddPairwise  {
	template <typename dest_t>
	struct type: public node {
		typedef int item_type;
		dest_t dest;
		typedef typename source_t::constructed_type puller_t;
		puller_t puller;
		type(const dest_t& dest, const source_t& src) 
			: dest(dest), puller(src.construct()) {
				add_push_destination(dest);
				add_pull_source(puller);
				set_name("AddPairwise");
			}

		virtual void go() override {
			while (puller.can_pull()) {
				//pull two numbers a,b and push a+b to dest
				int a = puller.pull();
				if (!puller.can_pull())
					throw std::logic_error("Not an even number of items in the stream.");
				int b = puller.pull();
				dest.push(a+b);
			}
		}
	};
};

template <typename source_t>
inline pipe_begin<tempfactory_1<AddPairwise<source_t>,source_t > > addPairwise(const source_t& source) {
	return tempfactory_1<AddPairwise<source_t>, source_t >(source);
}

void go() {
	int n = 100;

	//generate pipeline that 
	// 1) generates "n" numbers
	// 2) adds one to each number
	// 3) sorts the numbers
	// 4) computes the pairwise sum of the numbers 
	// 5) prints the numbers to stdout 
	//
	// we use a passive_sorter b/c we take more than one element of the
	// sorter out at a time
	passive_sorter<int> sorter;
	pipeline p1 = generator(n) | addOne() | sorter.input();
	pipeline p2 = addPairwise(sorter.output()) | printf_ints();

	//it doesn't matter if we start p1 or p2 here
	p1.plot();
	p1();
}

int main() {
	tpie_init();
	get_memory_manager().set_limit(50*1024*1024);
	go();
	tpie_finish();
	return 0;
}
