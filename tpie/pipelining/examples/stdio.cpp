// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
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


/* TPIE pipelining example.
 * Compile with:
 *   g++ -I. -O3 tpie/pipelining/examples/stdio.cpp -o stdio
 */


#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <tpie/pipelining/stdio.h>
#include <tpie/pipelining/numeric.h>

using namespace tpie;

std::string progname;

// This is the basic usage:
void do_pipeline(int factor, int term) {
	pipeline p = scanf_ints() | linear(factor, term) | printf_ints();
	p();
}

inline void usage() {
	std::cout << "Usage: " << progname << " a b\n"
		<< "Reads integers n on stdin and outputs integers a*n+b on stdout." << std::endl;
}

#define USAGE do { usage(); return 0; } while (0)

#define GETINT(var) do { \
	std::string a(argv[0]); \
	if (a == "0") var = 0; \
	else {\
		std::stringstream(a) >> var; \
		if (!var) USAGE;\
	}\
	--argc, ++argv;\
} while (0)

int main(int argc, char ** argv) {
	progname = argv[0];
	if (argc < 3) USAGE;
	--argc, ++argv;
	int factor, term;
	GETINT(factor);
	GETINT(term);

	do_pipeline(factor, term);
	return 0;
}
