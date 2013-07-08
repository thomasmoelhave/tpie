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
#include <tpie/compressed/stream.h>

bool basic_test(size_t n) {
	tpie::compressed_stream<size_t> s;
	s.open();
	for (size_t i = 0; i < n; ++i) {
		if (s.size() != i) {
			tpie::log_error() << "size() == " << s.size()
				<< ", expected " << i << std::endl;
			return false;
		}
		s.write(i);
	}
	s.seek(0);
	for (size_t i = 0; i < n; ++i) {
		if (!s.can_read()) {
			tpie::log_error() << "!can_read @ " << i << " out of " << n << std::endl;
			return false;
		}
		if (s.size() != n) {
			tpie::log_error() << "size() == " << s.size()
				<< " at position " << i << ", expected " << n << std::endl;
			return false;
		}
		size_t r = s.read();
		if (r != i) {
			tpie::log_error() << "Read " << r << " at " << i << std::endl;
			return false;
		}
	}
	if (s.can_read()) {
		tpie::log_error() << "can_read @ end of stream" << std::endl;
		return false;
	}
	return true;
}

bool read_seek_test(size_t seekPosition, size_t items) {
	if (seekPosition > items) {
		tpie::log_error() << "Invalid test parameters: " << seekPosition << " > " << items << std::endl;
		return false;
	}
	tpie::temp_file tf;

	tpie::compressed_stream<size_t> s;
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Open file" << std::endl;
	s.open(tf);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Write some items" << std::endl;
	for (size_t i = 0; i < seekPosition; ++i) s.write(i);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Get position" << std::endl;
	tpie::stream_position pos1 = s.get_position();
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Write more items" << std::endl;
	for (size_t i = seekPosition; i < items; ++i) s.write(i);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Seek to 0" << std::endl;
	s.seek(0);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Read some items" << std::endl;
	for (size_t i = 0; i < seekPosition; ++i) s.read();
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Get position" << std::endl;
	tpie::stream_position pos2 = s.get_position();
	tpie::log_debug() << s.describe() << std::endl;

	if (pos1 != pos2) {
		tpie::log_error() << "Positions differ" << std::endl;
		return false;
	}

	tpie::log_debug() << "Read more items" << std::endl;
	for (size_t i = seekPosition; i < items; ++i) s.read();
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Set position" << std::endl;
	s.set_position(pos1);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Read single item" << std::endl;
	size_t d = s.read();
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Got " << d << ", expected "
		<< seekPosition << std::endl;
	s.seek(0);
	tpie::log_debug() << s.describe() << std::endl;
	if (d != seekPosition) return false;
	return true;
}

bool position_test(size_t n) {
	tpie::temp_file tf;

	tpie::compressed_stream<size_t> s;
	s.open(tf);
	tpie::log_debug() << "Recording array of positions" << std::endl;
	tpie::array<tpie::stream_position> positions(n);
	for (size_t i = 0; i < n; ++i) {
		positions[i] = s.get_position();
		s.write(i);
	}

	tpie::log_debug() << "Verifying array of positions" << std::endl;
	s.seek(0);
	for (size_t i = 0; i < n; ++i) {
		tpie::stream_position p = s.get_position();
		if (positions[i] != p) {
			tpie::log_error() << "Disagreement in position " << i << std::endl;
			return false;
		}
		s.read();
	}

	tpie::log_debug() << "Verifying items at fib(n) positions" << std::endl;
	{
		size_t i = 0, j = 1;
		while (i + j < n) {
			size_t k = i + j;
			s.set_position(positions[k]);
			if (s.read() != k) {
				tpie::log_error() << "Bad read in position " << k << std::endl;
				return false;
			}
			i = j;
			j = k;
		}
	}

	tpie::log_debug() << "Verifying items at 2^n and 2^n-1 positions" << std::endl;
	for (size_t i = 1; i < n; i = i+i) {
		s.set_position(positions[i]);
		if (s.read() != i) {
			tpie::log_error() << "Bad read in position " << i << std::endl;
			return false;
		}
		s.set_position(positions[i-1]);
		if (s.read() != i-1) {
			tpie::log_error() << "Bad read in position " << i-1 << std::endl;
			return false;
		}
	}

	return true;
}

bool position_seek_test() {
	tpie::temp_file tf;
	tpie::compressed_stream<size_t> s;
	s.open(tf);
	for (size_t i = 0; i < 5; ++i) s.write(i);
	s.seek(0, tpie::file_stream_base::end);
	tpie::stream_position p = s.get_position();
	for (size_t i = 5; i < 10; ++i) s.write(i);
	s.seek(0);
	for (size_t i = 0; i < 10; ++i) {
		if (s.read() != i) {
			tpie::log_error() << "Bad read in position " << i << std::endl;
			return false;
		}
	}
	s.set_position(p);
	size_t x = s.read();
	if (x != 5) {
		tpie::log_error() << "Bad read after set_position; got " << x << ", expected 5" << std::endl;
		return false;
	}
	return true;
}

#define TEST_ASSERT(cond) \
	do { \
		if (!(cond)) { \
			tpie::log_error() << "Test failed on line " << __LINE__ << ": " #cond << std::endl; \
			return false; \
		} \
	} while (0)

bool position_test_1() {
	tpie::temp_file tf;
	tpie::compressed_stream<size_t> s;
	s.open(tf);

	size_t blockSize = 2*1024*1024 / sizeof(size_t);
	for (size_t i = 0; i < 2*blockSize; ++i) s.write(i);
	TEST_ASSERT(!s.can_read());
	TEST_ASSERT(s.size() == 2*blockSize);
	TEST_ASSERT(s.offset() == 2*blockSize);
	tpie::log_debug() << s.describe() << std::endl;
	s.set_position(s.get_position());
	tpie::log_debug() << s.describe() << std::endl;
	TEST_ASSERT(!s.can_read());
	TEST_ASSERT(s.size() == 2*blockSize);
	TEST_ASSERT(s.offset() == 2*blockSize);
	s.write(2*blockSize);
	TEST_ASSERT(!s.can_read());
	TEST_ASSERT(s.size() == 2*blockSize+1);
	TEST_ASSERT(s.offset() == 2*blockSize+1);
	return true;
}

bool position_test_2() {
	tpie::temp_file tf;
	tpie::compressed_stream<size_t> s;
	s.open(tf);
	size_t blockSize = 2*1024*1024 / sizeof(size_t);

	tpie::log_debug() << "Write blockSize + 5 items" << std::endl;
	for (size_t i = 0; i < blockSize + 5; ++i) s.write(i);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Get position and write 2 items" << std::endl;
	tpie::stream_position pos1 = s.get_position();
	TEST_ASSERT(!s.can_read());
	for (size_t i = blockSize + 5; i < blockSize + 7; ++i) s.write(i);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Get position, check can_read and set position" << std::endl;
	tpie::stream_position pos2 = s.get_position();
	TEST_ASSERT(!s.can_read());
	s.set_position(pos1);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Check can_read" << std::endl;
	TEST_ASSERT(s.can_read());
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Check read" << std::endl;
	TEST_ASSERT(s.read() == blockSize + 5);
	tpie::log_debug() << s.describe() << std::endl;
	TEST_ASSERT(s.can_read());

	tpie::log_debug() << "Check read" << std::endl;
	TEST_ASSERT(s.read() == blockSize + 6);
	TEST_ASSERT(!s.can_read());
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Check get_position" << std::endl;
	TEST_ASSERT(s.get_position() == pos2);
	tpie::log_debug() << s.describe() << std::endl;

	tpie::log_debug() << "Check write" << std::endl;
	s.write(blockSize + 7);
	tpie::log_debug() << s.describe() << std::endl;

	return true;
}

bool position_test_3(size_t n) {
	tpie::temp_file tf;
	tpie::compressed_stream<size_t> s;
	s.open(tf);

	size_t i = 0;
	size_t j = 1;
	size_t k = 1;
	s.write(0);
	while (k < n) {
		TEST_ASSERT(s.offset() == k);
		s.seek(0);
		TEST_ASSERT(s.offset() == 0);
		s.read();
		TEST_ASSERT(s.offset() == 1);
		if (k % 2 == 0) {
			tpie::log_debug() << "Seek to end" << std::endl;
			s.seek(0, tpie::file_stream_base::end);
		} else {
			tpie::log_debug() << "Scan to end" << std::endl;
			for (size_t n = 1; n < k; ++n) {
				TEST_ASSERT(s.read() == n);
			}
		}
		TEST_ASSERT(s.offset() == k);
		size_t target = i+j;
		i = j;
		j = target;
		tpie::log_debug() << "[" << k << ", " << target << ")" << std::endl;
		while (k < target) s.write(k++);
	}
	return true;
}

bool reopen_test(size_t n) {
	tpie::temp_file tf;

	size_t i = 0;
	size_t j = 1;
	size_t k = 0;

	while (k < n) {
		tpie::compressed_stream<size_t> s;
		s.open(tf);

		TEST_ASSERT(s.offset() == 0);
		TEST_ASSERT(s.size() == k);

		if (k % 2 == 0) {
			tpie::log_debug() << "Seek to end" << std::endl;
			s.seek(0, tpie::file_stream_base::end);
		} else {
			tpie::log_debug() << "Scan to end" << std::endl;
			for (size_t n = 0; n < k; ++n) {
				TEST_ASSERT(s.read() == n);
			}
		}

		TEST_ASSERT(s.offset() == k);
		TEST_ASSERT(!s.can_read());

		size_t target = i+j;
		i = j;
		j = target;
		tpie::log_debug() << "[" << k << ", " << target << ")" << std::endl;
		while (k < target) s.write(k++);
	}
	return true;
}

bool reopen_test_2() {
	tpie::temp_file tf;
	size_t blockSize = 2*1024*1024 / sizeof(size_t);

	tpie::stream_position pos1a;
	{
		tpie::compressed_stream<size_t> s;
		s.open(tf);
		for (size_t i = 0; i < blockSize; ++i) s.write(i);
		TEST_ASSERT(s.size() == blockSize);
		pos1a = s.get_position();
	}
	tpie::stream_position pos1b;
	tpie::stream_position pos2b;
	{
		tpie::compressed_stream<size_t> s;
		s.open(tf);
		TEST_ASSERT(s.size() == blockSize);
		TEST_ASSERT(s.offset() == 0);
		for (size_t i = 0; i < blockSize; ++i) {
			TEST_ASSERT(i == s.read());
		}
		TEST_ASSERT(s.offset() == blockSize);
		pos1b = s.get_position();
		for (size_t i = blockSize; i < 2*blockSize; ++i) s.write(i);
		pos2b = s.get_position();
		TEST_ASSERT(s.offset() == 2*blockSize);
		TEST_ASSERT(s.size() == 2*blockSize);
	}
	TEST_ASSERT(pos1a == pos1b);
	tpie::stream_position pos2c;
	{
		tpie::compressed_stream<size_t> s;
		s.open(tf);
		TEST_ASSERT(s.size() == 2*blockSize);
		s.set_position(pos1b);
		TEST_ASSERT(s.offset() == blockSize);
		TEST_ASSERT(s.size() == 2*blockSize);
		for (size_t i = blockSize; i < 2*blockSize; ++i) {
			TEST_ASSERT(i == s.read());
		}
		pos2c = s.get_position();
	}
	TEST_ASSERT(pos2b == pos2c);
	return true;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
		.test(basic_test, "basic", "n", static_cast<size_t>(1000))
		.test(read_seek_test, "read_seek", "m", static_cast<size_t>(1 << 10), "n", static_cast<size_t>(1 << 15))
		.test(position_test, "position", "n", static_cast<size_t>(1 << 19))
		.test(position_seek_test, "position_seek")
		.test(position_test_1, "position_1")
		.test(position_test_2, "position_2")
		.test(position_test_3, "position_3", "n", static_cast<size_t>(1 << 21))
		.test(reopen_test, "reopen", "n", static_cast<size_t>(1 << 21))
		.test(reopen_test_2, "reopen_2")
		;
}
