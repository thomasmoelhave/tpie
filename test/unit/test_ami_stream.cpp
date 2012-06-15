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
#include <boost/filesystem/operations.hpp>

#include <tpie/tpie.h>

#include <tpie/array.h>
#include <tpie/stream.h>
#include <tpie/util.h>
#include <tpie/types.h>

using tpie::uint64_t;

static const std::string TEMPFILE = "tmp";
inline uint64_t ITEM(size_t i) {return i*98927 % 104639;}
static const size_t TESTSIZE = 8*1024*1024;
static const size_t ITEMS = TESTSIZE/sizeof(uint64_t);
static const size_t ARRAYSIZE = 512;
static const size_t ARRAYS = TESTSIZE/(ARRAYSIZE*sizeof(uint64_t));

bool basic() {
	boost::filesystem::remove(TEMPFILE);

	// Write ITEMS items sequentially to TEMPFILE
	{
		tpie::ami::stream<uint64_t> s(TEMPFILE, tpie::ami::WRITE_STREAM);
		for(size_t i=0; i < ITEMS; ++i) s.write_item(ITEM(i));
	}

	// Sequential verify
	{
		tpie::ami::stream<uint64_t> s(TEMPFILE, tpie::ami::READ_STREAM);
		uint64_t *x = 0;
		for(size_t i=0; i < ITEMS; ++i) {
			s.read_item(&x);
			if (*x != ITEM(i)) {
				tpie::log_error() << "Expected element " << i << " = " << ITEM(i) << ", got " << *x << std::endl;
				return false;
			}
		}
	}

	// Write an ARRAYSIZE array ARRAYS times sequentially to TEMPFILE
	{
		tpie::ami::stream<uint64_t> s(TEMPFILE, tpie::ami::WRITE_STREAM);
		uint64_t x[ARRAYSIZE];
		for(size_t i=0; i < ARRAYSIZE; ++i) {
			x[i] = ITEM(i);
		}
		for(size_t i=0; i < ARRAYS; ++i) s.write_array(x, ARRAYSIZE);
	}

	// Sequentially verify the arrays
	{
		tpie::ami::stream<uint64_t> s(TEMPFILE, tpie::ami::READ_STREAM);
		uint64_t x[ARRAYSIZE];
		for(size_t i=0; i < ARRAYS; ++i) {
			TPIE_OS_SIZE_T len = ARRAYSIZE;
			s.read_array(x, len);
			if (len != ARRAYSIZE) {
				tpie::log_error() <<  "read_array only read " << len << " elements, expected " << ARRAYSIZE << std::endl;
				return false;
			}
			for (size_t i=0; i < ARRAYSIZE; ++i) {
				if (x[i] != ITEM(i)) {
					tpie::log_error() << "Expected element " << i << " = " << ITEM(i) << ", got " << x[i] << std::endl;
					return false;
				}
			}
		}
	}

	// Random read/write of items
	{
		tpie::ami::stream<uint64_t> s(TEMPFILE, tpie::ami::WRITE_STREAM);
		tpie::array<uint64_t> data(ITEMS);
		for (size_t i=0; i < ITEMS; ++i) {
			data[i] = ITEM(i);
			s.write_item(data[i]);
		}
		for (size_t i=0; i < 10; ++i) {
			// Seek to random index
			size_t idx = ITEM(i) % ITEMS;
			s.seek(idx);

			if (i%2 == 0) {
				uint64_t *read;
				s.read_item(&read);
				if (*read != data[idx]) {
					tpie::log_error() << "Expected element " << idx << " to be " << data[idx] << ", got " << *read << std::endl;
					return false;
				}
			} else {
				uint64_t write = ITEM(ITEMS+i);
				data[idx] = write;
				s.write_item(write);
			}

			tpie::stream_offset_type newoff = s.tell();
			if (static_cast<size_t>(newoff) != idx+1) {
				tpie::log_error() << "Offset advanced to " << newoff << ", expected " << (idx+1) << std::endl;
				return false;
			}
		}
	}
	return true;
}

int main(int argc, char **argv) {
	return tpie::tests(argc, argv)
		.test(basic, "basic");
}
