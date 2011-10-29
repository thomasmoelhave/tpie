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
#include <tpie/serialization.h>
#include <map>
#include <boost/random/linear_congruential.hpp>
#include <boost/unordered_map.hpp>
#include <boost/filesystem.hpp>

using namespace tpie;
using namespace std;

bool testSer(bool safe) {
	boost::filesystem::remove("temp.ser");
	std::stringstream ss;
	std::vector<int> v;
	v.push_back(88);
	v.push_back(74);
	{
		tpie::serializer ser(ss, safe);
		ser << (size_t)454 << (uint8_t)42 << "Hello world" << std::string("monster") << make_pair(std::string("hello"), (float)3.3) << v;
	}

	{	
		ss.seekg(0);
		tpie::unserializer ser(ss);

		size_t a;
		uint8_t b;
		std::string c;
		std::string d;
		std::pair<std::string, float> e;
		std::vector<int> f;
		try {
			ser >> a >> b >> c >> d >> e >> f;
		} catch(serialization_error e) {
			std::cout << e.what() << std::endl;
			return false;
		}
		if (a != 454 ||
			b != 42 ||
			c != "Hello world" ||
			d != "monster" ||
			e.first != "hello" ||
			(e.second - 3.3) > 1e-9 ||
			f != v) {
				std::cout << "Unserzation failed" << std::endl;
				return false;		
		}
	}
	return true;
}

int main(/*int argc, char **argv*/) {

	//if(argc != 2) return 1;
	//std::string test(argv[1]);
	//if (test == "safe")
		return testSer(true)?EXIT_SUCCESS:EXIT_FAILURE;
	//else if (test == "unsafe")
	//	return testSer(false)?EXIT_SUCCESS:EXIT_FAILURE;
	std::cerr << "No such test" << std::endl;
	return EXIT_FAILURE;
}
