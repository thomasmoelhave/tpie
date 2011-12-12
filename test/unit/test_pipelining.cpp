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
#include <tpie/pipelining.h>
#include <tpie/file_stream.h>

using namespace tpie;

typedef uint64_t test_t;

template <typename dest_t>
struct multiply_t {
	typedef test_t item_type;

	multiply_t(const dest_t & dest, uint64_t factor)
		: dest(dest), factor(factor) {
	}

	void push(const test_t & item) {
		dest.push(factor*item);
	}

	dest_t dest;
	uint64_t factor;
};

generate<factory_1<multiply_t, uint64_t> > multiply(uint64_t factor) {
	return factory_1<multiply_t, uint64_t>(factor);
}

bool pipelining_test() {
	{
		file_stream<test_t> input;
		input.open("input");
		input.write(1);
		input.write(2);
		input.write(3);
	}
	{
		file_stream<test_t> in;
		in.open("input");
		file_stream<test_t> out;
		out.open("output");
		input_t<multiply_t<multiply_t<output_t<test_t> > > > p = (input(in) | multiply(3) | multiply(2) | output(out));
		p();
	}
	return true;
}

int main() {
	tpie_initer _(32);
	if (!pipelining_test()) return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
