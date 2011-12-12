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
#include <tpie/pipelining/std_glue.h>
#include <tpie/file_stream.h>

using namespace tpie;

typedef uint64_t test_t;

template <typename dest_t>
struct multiply_t {
	typedef test_t item_type;

	multiply_t(const dest_t & dest, uint64_t factor)
		: dest(dest), factor(factor) {
	}

	void begin() { dest.begin(); }
	void end() { dest.end(); }

	void push(const test_t & item) {
		dest.push(factor*item);
	}

	dest_t dest;
	uint64_t factor;
};

generate<factory_1<multiply_t, uint64_t> > multiply(uint64_t factor) {
	return factory_1<multiply_t, uint64_t>(factor);
}

bool vector_multiply_test() {
	std::vector<test_t> input(20);
	std::vector<test_t> expect(20);
	std::vector<test_t> output;
	for (size_t i = 0; i < 20; ++i) {
		input[i] = i;
		expect[i] = i*6;
	}
	pipeline p = input_vector(input) | multiply(3) | multiply(2) | output_vector(output);
	p();
	if (output != expect) {
		std::cout << "Output vector does not match expect vector" << std::endl;
		std::vector<test_t>::iterator expectit = expect.begin();
		while (expectit != expect.end()) {
			std::cout << *expectit << ' ';
			++expectit;
		}
		std::cout << std::endl;
		std::vector<test_t>::iterator outputit = output.begin();
		while (outputit != output.end()) {
			std::cout << *outputit << ' ';
			++outputit;
		}
		std::cout << std::endl;
		return false;
	}
	return true;
}

bool file_stream_test() {
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
		// p is actually an input_t<multiply_t<multiply_t<output_t<test_t> > > >
		pipeline p = (input(in) | multiply(3) | multiply(2) | output(out));
		p();
	}
	{
		file_stream<test_t> output;
		output.open("output");
		if (6 != output.read()) return false;
		if (12 != output.read()) return false;
		if (18 != output.read()) return false;
	}
	return true;
}

#define TEST(fun) do {if (!fun()) result = false;} while (0)

int main() {
	tpie_initer _(32);
	bool result = true;
	TEST(vector_multiply_test);
	TEST(file_stream_test);
	if (result) {
		std::cout << "pipelining: All tests pass" << std::endl;
		return EXIT_SUCCESS;
	} else {
		std::cout << "pipelining: Some tests failed" << std::endl;
		return EXIT_FAILURE;
	}
}
