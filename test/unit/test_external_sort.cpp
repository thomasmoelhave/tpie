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

using namespace tpie;

bool sort_test(size_t size) {
	ami::stream<size_t> mystream;
	size_t s=next_prime(size);
	size_t y=size-16;
	for(size_t i=0; i < s; ++i) {
		size_t x= (uint64_t(i) * uint64_t(y)) % uint64_t(s);
		mystream.write_item(x);
	}
	ami::sort(&mystream);

	mystream.seek(0);

	size_t * x;
	for(size_t i=0; i < s; ++i) {
		mystream.read_item( &x );
		if (*x != i) return false;
	}
	return true;
}


bool perform_test(const std::string & test) {
	if (test == "small")
		return sort_test(1024 * 1024 * 8);
	else if (test == "large")
		return sort_test(1024*1024*1024);
	return false;
}


int main(int argc, char **argv) {
	if (argc != 2) return 1;
	tpie_init();
	bool ok=perform_test(std::string(argv[1]));
	tpie_finish();
	return ok?EXIT_SUCCESS:EXIT_FAILURE;
}

		
	
