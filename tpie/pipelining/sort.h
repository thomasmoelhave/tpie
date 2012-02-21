// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, The TPIE development team
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

#ifndef __TPIE_PIPELINING_SORT_H__
#define __TPIE_PIPELINING_SORT_H__

#include <tpie/pipelining/core.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/sort.h>
#include <tpie/file_stream.h>
#include <tpie/tempname.h>
#include <tpie/memory.h>

namespace tpie {

namespace pipelining {

template <typename dest_t>
struct sort_t : public pipe_segment {
	typedef typename dest_t::item_type item_type;

	inline sort_t(const dest_t & dest) : dest(dest) {
	}

	inline sort_t(const sort_t<dest_t> & other) : dest(other.dest) {
		// don't copy tmpfile or tmpstream
	}

	inline void begin() {
		tmpfile.reset(tpie::tpie_new<tpie::temp_file>());
		tmpstream.open(tmpfile->path());
	}

	inline void push(const item_type & item) {
		tmpstream.write(item);
	}

	inline void end() {
		tpie::sort(tmpstream, tmpstream);
		dest.begin();
		tmpstream.seek(0);
		while (tmpstream.can_read()) {
			dest.push(tmpstream.read());
		}
		tmpstream.close();
		tmpfile->free();
		tmpfile.reset(0);

		dest.end();
	}

	const pipe_segment * get_next() const {
		return &dest;
	}
private:
	dest_t dest;
	tpie::auto_ptr<tpie::temp_file> tmpfile;
	tpie::file_stream<item_type> tmpstream;
};

inline pipe_middle<factory_0<sort_t> >
pipesort() {
	return factory_0<sort_t>();
}

}

}

#endif
