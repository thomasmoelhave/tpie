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

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
		.test(basic_test, "basic", "n", static_cast<size_t>(1000))
		.test(read_seek_test, "read_seek", "m", static_cast<size_t>(1 << 10), "n", static_cast<size_t>(1 << 15))
		.test(position_test, "position", "n", static_cast<size_t>(1 << 19))
		;
}
