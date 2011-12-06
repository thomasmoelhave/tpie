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

bool pipelining_test() {
	{
		file_stream<test_t> input;
		input.open("input");
		input.write(1);
		input.write(2);
		input.write(3);
	}
	file_stream<test_t> input;
	input.open("input");
	//file_stream<test_t> output;
	//output.open("output");
	// file_stream_source_t<test_t> source(input);
	// file_stream_sink_t<test_t> sink(output);
	tpie::pipe<void,void> pipeline = source(input) << cout_sink_t<test_t>();
	pipeline();
	return true;
}

int main(int argc, char ** argv) {
	tpie_initer _(32);
	if (!pipelining_test()) return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
