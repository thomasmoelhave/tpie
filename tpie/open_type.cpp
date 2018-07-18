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

#include <tpie/open_type.h>
#include <tpie/exception.h>

namespace tpie {

namespace {
void not_both(open::type flags, open::type flag1, open::type flag2, const std::string &exception_message) {
	if ((flags & flag1) && (flags & flag2))
		throw stream_exception(exception_message);
}
}

void open::validate_flags(tpie::open::type flags) {
	not_both(flags, read_only, write_only, "Can't have both read and write only flags");

	not_both(flags, access_normal, access_random, "Can't have both 'normal' and 'random' access flags");

	not_both(flags, compression_normal, compression_all, "Can't have both 'normal' and 'all' compression flags");
}

bool open::has_compression(open::type flags) {
	return bool(flags & (open::compression_normal | open::compression_all));
}

cache_hint open::translate_cache(open::type openFlags) {
	const open::type cacheFlags =
			openFlags & (open::access_normal | open::access_random);

	switch (cacheFlags) {
		case open::access_normal: return tpie::access_normal;
		case open::access_random: return tpie::access_random;
		case 0:                   return tpie::access_sequential;
		default: throw exception("Invalid cache flags");
	}
}

compression_flags open::translate_compression(open::type openFlags) {
	const open::type compressionFlags =
			openFlags & (open::compression_normal | open::compression_all);

	switch (compressionFlags) {
		case open::compression_normal: return tpie::compression_normal;
		case open::compression_all:    return tpie::compression_all;
		case 0:                        return tpie::compression_none;
		default: throw exception("Invalid compression flags");
	}
}


open::type open::translate(access_type accessType, cache_hint cacheHint, compression_flags compressionFlags) {
	return ((
				(accessType == access_read) ? open::read_only :
				(accessType == access_write) ? open::write_only :
				open::defaults) | (

				(cacheHint == tpie::access_normal) ? open::access_normal :
				(cacheHint == tpie::access_random) ? open::access_random :
				open::defaults) | (

				(compressionFlags == tpie::compression_normal) ? open::compression_normal :
				(compressionFlags == tpie::compression_all) ? open::compression_all :
				open::defaults));
}

} // namespace tpie
