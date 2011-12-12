// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
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

#ifndef __TPIE_PIPELINING_FILE_STREAM_H__
#define __TPIE_PIPELINING_FILE_STREAM_H__

#include <tpie/file_stream.h>

#include <tpie/pipelining/core.h>
#include <tpie/pipelining/factory_helpers.h>

namespace tpie {

template <typename dest_t>
struct input_t {
	typedef typename dest_t::item_type item_type;

	input_t(const dest_t & dest, file_stream<item_type> & fs) : dest(dest), fs(fs) {
	}

	void operator()() {
		dest.begin();
		while (fs.can_read()) {
			dest.push(fs.read());
		}
		dest.end();
	}
private:
	dest_t dest;
	file_stream<item_type> & fs;
};

template<typename T>
generate<factory_1<input_t, file_stream<T> &> > input(file_stream<T> & fs) {
	return factory_1<input_t, file_stream<T> &>(fs);
}

template <typename dest_t>
struct identity_t {
	typedef typename dest_t::item_type item_type;

	identity_t(const dest_t & dest) : dest(dest) {
	}

	void begin() {
		dest.begin();
	}

	void push(const item_type & item) {
		dest.push(item);
	}

	void end() {
		dest.end();
	}
private:
	dest_t dest;
};

generate<factory_0<identity_t> > identity() {
	return factory_0<identity_t>();
}

template <typename T>
struct output_t {
	typedef T item_type;

	output_t(file_stream<T> & fs) : fs(fs) {
	}

	void begin() {
	}

	void push(const T & item) {
		fs.write(item);
	}

	void end() {
	}
private:
	file_stream<T> & fs;
};

template <typename T>
terminator<termfactory_1<output_t<T>, file_stream<T> &> > output(file_stream<T> & fs) {
	return termfactory_1<output_t<T>, file_stream<T> &>(fs);
}

}

#endif
