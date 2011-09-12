// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
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

// This program tests sequential reads and writes of 8 MB of 64-bit int items,
// sequential read and write of 8 MB of 64-bit int arrays,
// random seeking in 8 MB followed by either a read or a write.

#include "common.h"

#include <iostream>

#include <tpie/tpie.h>

#include <tpie/array.h>
#include <tpie/file_stream.h>
#include <tpie/util.h>

static const std::string TEMPFILE = "tmp";
inline uint64_t ITEM(size_t i) {return i*98927 % 104639;}
static const size_t SIZE = 8*1024*1024;
static const size_t ITEMS = SIZE/sizeof(uint64_t);
static const size_t ARRAYSIZE = 512;
static const size_t ARRAYS = SIZE/(ARRAYSIZE*sizeof(uint64_t));

int main(int argc, char **argv) {
	tpie_initer _;

	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " basic" << std::endl;
		return EXIT_FAILURE;
	}

	std::string testtype(argv[1]);

	// We only have one test
	if (testtype != "basic") {
		std::cout << "Unknown test" << std::endl;
		return EXIT_FAILURE;
	}

	// Write ITEMS items sequentially to TEMPFILE
	{
		tpie::file_stream<uint64_t> s;
		s.open(TEMPFILE);
		for(size_t i=0; i < ITEMS; ++i) s.write(ITEM(i));
	}

	// Sequential verify
	{
		tpie::file_stream<uint64_t> s;
		s.open(TEMPFILE);
		for(size_t i=0; i < ITEMS; ++i) {
			uint64_t x = s.read();
			if (x != ITEM(i)) {
				std::cout << "Expected element " << i << " = " << ITEM(i) << ", got " << x << std::endl;
				return EXIT_FAILURE;
			}
		}
	}

	// Write an ARRAYSIZE array ARRAYS times sequentially to TEMPFILE
	{
		tpie::file_stream<uint64_t> s;
		s.open(TEMPFILE);
		uint64_t x[ARRAYSIZE];
		for(size_t i=0; i < ARRAYSIZE; ++i) {
			x[i] = ITEM(i);
		}
		for(size_t i=0; i < ARRAYS; ++i) s.write(x + 0, x + ARRAYSIZE);
	}

	// Sequentially verify the arrays
	{
		tpie::file_stream<uint64_t> s;
		s.open(TEMPFILE);
		uint64_t x[ARRAYSIZE];
		for(size_t i=0; i < ARRAYS; ++i) {
			TPIE_OS_SIZE_T len = ARRAYSIZE;
			try {
				s.read(x + 0, x + len);
			} catch (tpie::end_of_stream_exception & e) {
				std::cout << "read array threw unexpected end_of_stream_exception" << std::endl;
				return EXIT_FAILURE;
			}
			for (size_t i=0; i < ARRAYSIZE; ++i) {
				if (x[i] != ITEM(i)) {
					std::cout << "Expected element " << i << " = " << ITEM(i) << ", got " << x[i] << std::endl;
					return EXIT_FAILURE;
				}
			}
		}
	}

	// Random read/write of items
	{
		tpie::file_stream<uint64_t> s;
		s.open(TEMPFILE);
		tpie::array<uint64_t> data(ITEMS);
		for (size_t i=0; i < ITEMS; ++i) {
			data[i] = ITEM(i);
			s.write(data[i]);
		}
		for (size_t i=0; i < 10; ++i) {
			// Seek to random index
			tpie::stream_offset_type idx = ITEM(i) % ITEMS;
			s.seek(idx);

			if (i%2 == 0) {
				uint64_t read = s.read();
				if (read != data[idx]) {
					std::cout << "Expected element " << idx << " to be " << data[idx] << ", got " << read << std::endl;
					return EXIT_FAILURE;
				}
			} else {
				uint64_t write = ITEM(ITEMS+i);
				data[idx] = write;
				s.write(write);
			}

			tpie::stream_offset_type newoff = s.offset();
			if (newoff != idx+1) {
				std::cout << "Offset advanced to " << newoff << ", expected " << (idx+1) << std::endl;
				return EXIT_FAILURE;
			}
		}
	}

	return EXIT_SUCCESS;
}
