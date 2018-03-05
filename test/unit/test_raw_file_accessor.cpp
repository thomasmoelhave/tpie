// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2018, The TPIE development team
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
#include <tpie/file_accessor/file_accessor.h>
#include <tpie/tempname.h>

using namespace tpie;

bool open_rw_new_test() {
	std::string test_str = "foobar";

	temp_file tmp;

	tpie::default_raw_file_accessor fa;

	fa.open_rw_new(tmp.path());
	fa.write_i(test_str.c_str(), test_str.size());
	fa.close_i();

	std::vector<char> result(test_str.size());

	fa.open_rw_new(tmp.path());
	fa.read_i(result.data(), result.size());

	return std::equal(test_str.begin(), test_str.end(), result.begin(), result.end());
}

bool try_open_rw_test() {
	temp_file tmp;

	tpie::default_raw_file_accessor fa;
	if (fa.try_open_rw(tmp.path())) return false;

	fa.open_wo(tmp.path());
	fa.close_i();

	if (!fa.try_open_rw(tmp.path())) return false;

	return true;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
		.test(open_rw_new_test, "open_rw_new")
		.test(try_open_rw_test, "try_open_rw")
		;
}
