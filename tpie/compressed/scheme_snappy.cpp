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
#include <tpie/config.h>
#include <tpie/exception.h>
#include <tpie/compressed/scheme.h>

namespace {

class compression_scheme_impl : public tpie::compression_scheme {
public:

virtual size_t max_compressed_length(size_t srcSize) const override {
	return snappy::MaxCompressedLength(srcSize);
}

virtual void compress(char * dest, const char * src, size_t srcSize, size_t * destSize) const override {
	snappy::RawCompress(src, srcSize, dest, destSize);
}

virtual size_t uncompressed_length(const char * src, size_t srcSize) const override {
	size_t destSize;
	if (!snappy::GetUncompressedLength(src,
									   srcSize,
									   &destSize))
		throw tpie::stream_exception("Internal error; snappy::GetUncompressedLength failed");
	return destSize;
}

virtual void uncompress(char * dest, const char * src, size_t srcSize) const override {
	snappy::RawUncompress(src, srcSize, dest);
}

};

compression_scheme_impl the_compression_scheme;

} // unnamed namespace

namespace tpie {

const compression_scheme & get_compression_scheme_snappy() {
	return the_compression_scheme;
}

} // namespace tpie
