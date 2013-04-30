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

#include <snappy.h>

#include <tpie/file_accessor/compressed_stream_accessor.h>

namespace tpie {
namespace file_accessor {
namespace bits {

bool RawUncompress(const char* compressed, size_t compressed_length, char* uncompressed) {
	return snappy::RawUncompress(compressed, compressed_length, uncompressed);
}

bool GetUncompressedLength(const char* compressed, size_t compressed_length, size_t* result) {
	return snappy::GetUncompressedLength(compressed, compressed_length, result);
}

size_t Compress(const char* input, size_t input_length, std::string* output) {
	return snappy::Compress(input, input_length, output);
}

} // namespace tpie
} // namespace file_accessor
} // namespace bits
