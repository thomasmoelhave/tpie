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

#ifndef TPIE_COMPRESSED_SCHEME_H
#define TPIE_COMPRESSED_SCHEME_H

namespace tpie {

class compression_scheme {
public:
	enum type {
		none = 0,
		snappy = 1
	};

	virtual size_t max_compressed_length(size_t srcSize) const = 0;
	virtual void compress(char * dest, const char * src, size_t srcSize, size_t * destSize) const = 0;
	virtual size_t uncompressed_length(const char * src, size_t srcSize) const = 0;
	virtual void uncompress(char * dest, const char * src, size_t srcSize) const = 0;

protected:
	~compression_scheme() {}
};

const compression_scheme & get_compression_scheme_none();
const compression_scheme & get_compression_scheme_snappy();

inline const compression_scheme & get_compression_scheme(compression_scheme::type t) {
	switch (t) {
		case compression_scheme::none:
			return get_compression_scheme_none();
		case compression_scheme::snappy:
			return get_compression_scheme_snappy();
	}
	return get_compression_scheme_none();
}

}

#endif // TPIE_COMPRESSED_SCHEME_H
