// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, 2010, The TPIE development team
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
#ifndef _TPIE_FILE_ACCESSOR_FILE_ACCESSOR_CRTP_H
#define _TPIE_FILE_ACCESSOR_FILE_ACCESSOR_CRTP_H

#include <tpie/file_accessor/file_accessor.h>

namespace tpie {
namespace file_accessor {

template <typename child_t, bool minimizeSeeks=true>
class file_accessor_crtp: public file_accessor {
private:
	inline void read_i(void *, memory_size_type size);
	inline void write_i(const void *, memory_size_type size);
	inline void seek_i(stream_size_type size);
	stream_size_type location;
protected:
	inline void invalidateLocation();
	void throw_errno();
	void read_header();
	void write_header(bool clean);
public:
	virtual memory_size_type read(void * data, stream_size_type offset, memory_size_type size);
	virtual void write(const void * data, stream_size_type offset, memory_size_type size);
	virtual void read_user_data(void * data);
	virtual void write_user_data(const void * data);
	static inline memory_size_type memory_usage() {return sizeof(child_t);}
};
	
}
}
#endif //_TPIE_FILE_ACCESSOR_FILE_ACCESSOR_CRTP_H
