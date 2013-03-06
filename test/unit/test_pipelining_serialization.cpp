// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2013 The TPIE development team
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
#include <tpie/serialization_stream.h>

using namespace tpie;
using namespace tpie::pipelining;

void populate_test_data(std::vector<std::string> & test) {
	char c = '!';
	size_t ante = 0;
	size_t prev = 1;
	size_t size = 0;
	for (size_t i = 0; i < 31; ++i) {
		size_t cur = ante+prev;
		ante = prev;
		prev = cur;

		test.push_back(std::string(cur, c++));

		size += cur;
	}
}

bool basic_test() {
	tpie::temp_file f_in;
	tpie::temp_file f_out;
	std::vector<std::string> testData;
	populate_test_data(testData);
	{
		serialization_writer wr;
		wr.open(f_in.path());
		for (size_t i = 0; i < testData.size(); ++i) {
			wr.serialize(testData[i]);
		}
		wr.close();
	}
	{
		serialization_reader rd;
		rd.open(f_in.path());
		serialization_writer wr;
		wr.open(f_out.path());
		pipeline p = serialization_input(rd) | serialization_output<std::string>(wr);
		p.plot(log_info());
		p();
		wr.close();
	}
	{
		serialization_reader rd;
		rd.open(f_out.path());
		for (size_t i = 0; i < testData.size(); ++i) {
			if (!rd.can_read()) {
				log_error() << "Could not read item" << std::endl;
				return false;
			}
			std::string d;
			rd.unserialize(d);
			if (d != testData[i]) {
				log_error() << "Wrong item read" << std::endl;
				return false;
			}
		}
	}
	return true;
}

bool reverse_test() {
	tpie::temp_file f_in;
	tpie::temp_file f_out;
	std::vector<std::string> testData;
	populate_test_data(testData);
	{
		serialization_writer wr;
		wr.open(f_in.path());
		for (size_t i = testData.size(); i--;) {
			wr.serialize(testData[i]);
		}
		wr.close();
	}
	{
		serialization_reader rd;
		rd.open(f_in.path());
		serialization_writer wr;
		wr.open(f_out.path());
		pipeline p = serialization_input(rd) | serialization_reverser() | serialization_output<std::string>(wr);
		p.plot(log_info());
		p();
		wr.close();
	}
	{
		serialization_reader rd;
		rd.open(f_out.path());
		for (size_t i = 0; i < testData.size(); ++i) {
			if (!rd.can_read()) {
				log_error() << "Could not read item" << std::endl;
				return false;
			}
			std::string d;
			rd.unserialize(d);
			if (d != testData[i]) {
				log_error() << "Wrong item read" << std::endl;
				return false;
			}
		}
	}
	return true;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
	.test(basic_test, "basic")
	.test(reverse_test, "reverse")
	;
}
