// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2013, The TPIE development team
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
#include <tpie/compressed_stream.h>

bool basic_test(size_t n) {
	tpie::compressed_stream<size_t> s;
	s.open();
	for (size_t i = 0; i < n; ++i) {
		s.write(i);
	}
	s.seek(0);
	for (size_t i = 0; i < n; ++i) {
		if (!s.can_read()) {
			tpie::log_error() << "!can_read @ " << i << " out of " << n << std::endl;
			return false;
		}
		if (s.read() != i)
			return false;
	}
	if (s.can_read()) {
		tpie::log_error() << "can_read @ end of stream" << std::endl;
		return false;
	}
	return true;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
		.test(basic_test, "basic", "n", static_cast<size_t>(1000))
		;
}
