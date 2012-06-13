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
#include <vector>
#include <boost/filesystem/operations.hpp>
#include <boost/array.hpp>

#include <tpie/tpie.h>

#include <tpie/array.h>
#include <tpie/file_stream.h>
#include <tpie/util.h>

using tpie::uint64_t;

static const std::string TEMPFILE = "tmp";
inline uint64_t ITEM(size_t i) {return i*98927 % 104639;}
static const size_t TESTSIZE = 8*1024*1024;
static const size_t ITEMS = TESTSIZE/sizeof(uint64_t);
static const size_t ARRAYSIZE = 512;
static const size_t ARRAYS = TESTSIZE/(ARRAYSIZE*sizeof(uint64_t));

struct movable_file_stream {
	tpie::auto_ptr<tpie::file_stream<uint64_t> > fs;
	movable_file_stream() {fs.reset(tpie::tpie_new<tpie::file_stream<uint64_t> >());}
	movable_file_stream(tpie::file_stream<uint64_t> & with) {
		fs.reset(tpie::tpie_new<tpie::file_stream<uint64_t> >());
		fs->swap(with);
	}
	movable_file_stream(const movable_file_stream & other) {
		fs.reset(tpie::tpie_new<tpie::file_stream<uint64_t> >());
		fs->swap(*other.fs);
	}
	movable_file_stream & operator=(const movable_file_stream & other) {
		fs->swap(*other.fs);
		return *this;
	}
};

movable_file_stream openstream() {
	tpie::file_stream<uint64_t> fs;
	fs.open(TEMPFILE);
	return fs;
}

bool basic_test();

bool array_test() {
	try {
		tpie::file_stream<uint64_t> fs;
		fs.open(TEMPFILE);
		tpie::memory_size_type items = tpie::file<uint64_t>::block_size(1.0)/sizeof(uint64_t) + 10;
		std::vector<uint64_t> data(items, 1);
		fs.write(data.begin(), data.end());
		fs.seek(0);
		fs.read(data.begin(), data.end());
	} catch (std::exception & e) {
		std::cout << "Caught exception " << typeid(e).name() << "\ne.what(): " << e.what() << std::endl;
		return false;
	} catch (...) {
		std::cout << "Caught something other than an exception" << std::endl;
		return false;
	}
	return true;
}

bool odd_block_test() {
	typedef boost::array<char, 17> test_t;
	const size_t items = 500000;
	test_t initial_item;

	{
		tpie::file_stream<test_t> fs;
		fs.open(TEMPFILE);

		test_t item = initial_item;
		for (size_t i = 0; i < items; ++i) {
			fs.write(item);
			item[0]++;
		}
	}

	{
		tpie::file_stream<test_t> fs;
		fs.open(TEMPFILE);

		test_t item = initial_item;
		for (size_t i = 0; i < items; ++i) {
			test_t got = fs.read();
			if (got != item) {
				std::cout << "Item " << i << " is wrong" << std::endl;
				return false;
			}
			item[0]++;
		}
	}

	return true;
}

bool truncate_test() {
	typedef int test_t;
	tpie::file_stream<test_t> fs;
	fs.open("tmp");
	for (size_t i = 0; i < 1000000; ++i)
		fs.write(42);
	bool res = true;
	try {
		fs.seek(0);
	} catch (tpie::io_exception) {
		std::cout << "We should be able to seek!" << std::endl;
		res = false;
	}

	fs.truncate(42);
	for (size_t i = 0; i < 42; ++i) {
		if (!fs.can_read()) {
			std::cout << "Cannot read item " << i << std::endl;
			return false;
		}
		if (42 != fs.read()) {
			std::cout << "Item " << i << " is wrong" << std::endl;
			return false;
		}
	}
	if (fs.can_read()) {
		std::cout << "We should not be able to read after truncate!" << std::endl;
		res = false;
	}
	try {
		test_t item = fs.read();
		std::cout << "We should not be able to read after truncate!" << std::endl;
		return false;
	} catch (tpie::stream_exception) {
	}
	return res;
}

int main(int argc, char **argv) {
	tpie_initer _;

	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " [basic|array|odd]" << std::endl;
		return EXIT_FAILURE;
	}

	boost::filesystem::remove(TEMPFILE);

	std::string testtype(argv[1]);

	bool result;

	if (testtype == "array")
		result = array_test();
	else if (testtype == "basic")
		result = basic_test();
	else if (testtype == "odd")
		result = odd_block_test();
	else if (testtype == "truncate")
		result = truncate_test();
	else {
		std::cout << "Unknown test" << std::endl;
		result = false;
	}

	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}

bool basic_test() {
	// Write ITEMS items sequentially to TEMPFILE
	{
		movable_file_stream fs;
		fs = openstream();
		tpie::file_stream<uint64_t> s;
		s.swap(*fs.fs);
		for(size_t i=0; i < ITEMS; ++i) s.write(ITEM(i));
	}

	// Sequential verify
	{
		movable_file_stream fs;
		fs = openstream();
		tpie::file_stream<uint64_t> s;
		s.swap(*fs.fs);
		tpie::file_stream<uint64_t> t;
		for(size_t i=0; i < ITEMS; ++i) {
			uint64_t x = (i % 2) ? t.read() : s.read();
			if (x != ITEM(i)) {
				std::cout << "Expected element " << i << " = " << ITEM(i) << ", got " << x << std::endl;
				return false;
			}
			if (i % 3) s.swap(t);
			else t.swap(s);
		}
		s.swap(t);
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
			} catch (tpie::end_of_stream_exception &) {
				std::cout << "read array threw unexpected end_of_stream_exception" << std::endl;
				return false;
			}
			for (size_t i=0; i < ARRAYSIZE; ++i) {
				if (x[i] != ITEM(i)) {
					std::cout << "Expected element " << i << " = " << ITEM(i) << ", got " << x[i] << std::endl;
					return false;
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
			size_t idx = ITEM(i) % ITEMS;
			s.seek(idx);

			if (i%2 == 0) {
				uint64_t read = s.read();
				if (read != data[idx]) {
					std::cout << "Expected element " << idx << " to be " << data[idx] << ", got " << read << std::endl;
					return false;
				}
			} else {
				uint64_t write = ITEM(ITEMS+i);
				data[idx] = write;
				s.write(write);
			}

			tpie::stream_offset_type newoff = s.offset();
			if (static_cast<size_t>(newoff) != idx+1) {
				std::cout << "Offset advanced to " << newoff << ", expected " << (idx+1) << std::endl;
				return false;
			}
		}
	}

	return true;
}
