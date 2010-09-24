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

#include <tpie/internal_vector.h>

using namespace tpie;

bool basic_test() {
	internal_vector<int> s(52);
	for(size_t i=0; i < 52; ++i)
		s.push_back((i * 104729) % 2251);
	for(int i=51; i >= 0; --i) {
		if (s.size() != (size_t)i+1) return false;
		if (s.back() != ((int)i * 104729) % 2251) return false;
		s.pop_back();
	}
	if (!s.empty()) return false;
	return true;
}

class vector_memory_test: public memory_test {
public:
	internal_vector<int> * a;
	virtual void alloc() {a = new internal_vector<int>(123456);}
	virtual void free() {delete a;}
	virtual size_type claimed_size() {return internal_vector<int>::memory_usage(123456);}
};

int main(int argc, char **argv) {
	if(argc != 2) return 1;
	std::string test(argv[1]);
	if (test == "basic")
		return basic_test()?EXIT_SUCCESS:EXIT_FAILURE;
	else if (test == "memory") 
		return vector_memory_test()()?EXIT_SUCCESS:EXIT_FAILURE;
	return EXIT_FAILURE;
}


