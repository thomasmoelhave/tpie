// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
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

#ifndef TPIE_OPEN_TYPE_H
#define TPIE_OPEN_TYPE_H

#include <string>

namespace tpie {

struct open {
	enum type {
		/** Open a file for reading only. */
		read_only =  00000001,
		/** Open a file for writing only.
		 * Content is truncated. */
		write_only = 00000002,
		/** Neither sequential access nor random access is intended.
		 * Corresponds to POSIX_FADV_NORMAL. */
		access_normal = 00000004,
		/** Random access is intended.
		 * Corresponds to POSIX_FADV_RANDOM and FILE_FLAG_RANDOM_ACCESS (Win32). */
		access_random = 00000010,
		/** Compress some blocks
		 * according to available resources (time, memory). */
		compression_normal = 00000020,
		/** Compress all blocks according to the preferred compression scheme
		 * which can be set using
		 * tpie::the_compressor_thread().set_preferred_compression(). */
		compression_all = 00000040,
		/** Truncate the file if it already exists. */
		truncate_file = 00000100,
		/** Enable readahead for file. */
		readahead_enabled = 00000200,


		defaults = 0
	};

	static void validate_flags(open::type flags);
	static bool has_compression(open::type flags);

	friend inline open::type operator|(open::type a, open::type b)
	{ return (open::type) ((int) a | (int) b); }
	friend inline open::type operator&(open::type a, open::type b)
	{ return (open::type) ((int) a & (int) b); }
	friend inline open::type operator^(open::type a, open::type b)
	{ return (open::type) ((int) a ^ (int) b); }
	friend inline open::type operator~(open::type a)
	{ return (open::type) ~(int) a; }
};

} // namespace tpie

#endif // TPIE_OPEN_TYPE_H
