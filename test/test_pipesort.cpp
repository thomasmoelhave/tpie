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

#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include <tpie/sort.h>  
#include <tpie/pipelining.h>
#include <tpie/tpie_assert.h>
#include <tpie/progress_indicator_arrow.h>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/mersenne_twister.hpp>

using namespace tpie;
using namespace tpie::pipelining;


///////////////////////////////////////////////////////////////////////////////
/// Node that pushes n pseudorandom natural numbers
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
class start_t : public node {
	dest_t dest;
	memory_size_type n;
public:
	start_t(const dest_t & dest, const memory_size_type & n) : dest(dest), n(n)
	{
		add_push_destination(dest);
		set_name("Integer generator");
	}

	virtual void go() override {
		boost::mt19937 rng(42);
		boost::uniform_int<memory_size_type> dist(0, std::numeric_limits<memory_size_type>::max());
		boost::variate_generator<boost::mt19937&, boost::uniform_int<memory_size_type> > generator(rng, dist);

		for(memory_size_type i = 0; i < n; ++i) dest.push(generator());
	}
};

typedef pipe_begin<factory_1<start_t, memory_size_type> > start;

class end_t : public node {
	memory_size_type last;
public:
	typedef memory_size_type item_type;

	end_t() : last(0)
	{
		set_name("Non-descending order check");
	}

	void push(const item_type & item) {
		tp_assert(last <= item, "The numbers were not returned in non-descending order.");
		last = item;
	}
};

typedef pipe_end<termfactory_0<end_t> > end;

int main() {
	tpie::tpie_init();

	// Calling tpie_finish() before the pipeline is destructed would result in a segmentation fault. A new scope is created to avoid this.
	{
	get_memory_manager().set_limit(50*1024*1024);

	pipeline p = start(6284630 * 10 + 6548) | sort() | end();
	//p.plot();
	p();
	}

	tpie_finish();

	return 0;
}