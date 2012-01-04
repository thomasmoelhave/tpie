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

namespace pipelining {

/**
 * \class input_t
 *
 * file_stream input generator.
 */
template <typename dest_t>
struct input_t {
	typedef typename dest_t::item_type item_type;

	inline input_t(const dest_t & dest, file_stream<item_type> & fs) : dest(dest), fs(fs) {
	}

	inline void operator()() {
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
inline pipe_middle<factory_1<input_t, file_stream<T> &> > input(file_stream<T> & fs) {
	return factory_1<input_t, file_stream<T> &>(fs);
}

/**
 * \class pull_input_t
 *
 * file_stream pull input generator.
 */
template <typename T>
struct pull_input_t {
	typedef T item_type;

	inline pull_input_t(file_stream<T> & fs) : fs(fs) {
	}

	inline void begin() {
	}

	inline T pull() {
		return fs.read();
	}

	inline bool can_pull() {
		return fs.can_read();
	}

	inline void end() {
	}

	file_stream<T> & fs;
};

template<typename T>
inline datasource<pull_input_t<T> > pull_input(file_stream<T> & fs) {
	return pull_input_t<T>(fs);
}

/**
 * \class output_t
 *
 * file_stream output terminator.
 */
template <typename T>
struct output_t {
	typedef T item_type;

	inline output_t(file_stream<T> & fs) : fs(fs) {
	}

	inline void begin() {
	}

	inline void push(const T & item) {
		fs.write(item);
	}

	inline void end() {
	}
private:
	file_stream<T> & fs;
};

template <typename T>
inline termfactory_1<output_t<T>, file_stream<T> &> output(file_stream<T> & fs) {
	return fs;
}

/**
 * \class pull_output_t
 *
 * file_stream output pull data source.
 */
template <typename source_t>
struct pull_output_t {
	typedef typename source_t::item_type item_type;

	inline pull_output_t(const source_t & source, file_stream<item_type> & fs) : source(source), fs(fs) {
	}

	inline void operator()() {
		source.begin();
		while (source.can_pull()) {
			fs.write(source.pull());
		}
		source.end();
	}

	source_t source;
	file_stream<item_type> & fs;
};

template<typename T>
inline pull_factory_1<pull_output_t, file_stream<T> &> pull_output(file_stream<T> & fs) {
	return fs;
}

}

}

#endif
