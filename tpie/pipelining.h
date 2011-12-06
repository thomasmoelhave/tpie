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

#ifndef __TPIE_PIPELINING_H__
#define __TPIE_PIPELINING_H__

#include <iostream>
#include <tpie/file_stream.h>
//#include <tpie/pipelining/source.h>

namespace tpie {

template <typename T, typename D>
struct type_or {
	typedef const T & type;
};

template <typename D>
struct type_or<void, D> {
	typedef D type;
};

template <typename src_t, typename dest_t>
struct pipe {
	virtual ~pipe() {}
	virtual typename type_or<dest_t, bool>::type operator()(typename type_or<src_t, bool>::type src) = 0;
};

template <typename src_t, typename dest_t>
struct virtualpipe : public pipe<src_t, dest_t> {
	virtual ~virtualpipe() {}
	template <typename from_t>
	virtualpipe(const from_t & from) {
		wrapee = new from_t(from);
	}
	typename type_or<dest_t, bool>::type operator()(typename type_or<src_t, bool>::type src) {
		return (*wrapee)(true);
	}
private:
	virtualpipe();
	pipe<src_t, dest_t> *wrapee;
};

template <typename left_t, typename transfer_t, typename right_t>
struct pullbox : public pipe<typename left_t::src_t, typename right_t::dest_t> {
	typedef typename left_t::src_t src_t;
	typedef typename right_t::dest_t dest_t;

	pullbox(const left_t & left, const right_t & right)
		: left(left) // copy
		, right(right) { // copy

		// do nothing
	}

	typename type_or<dest_t, bool>::type operator()(typename type_or<src_t, bool>::type src) {
		std::cout << "dyt bot" << std::endl;
		return right.use(left, src);
	}

private:
	left_t left;
	right_t right;
};

template<typename T>
struct file_stream_source_t {
	typedef void src_t;

	file_stream_source_t(file_stream<T> & fs)
		: fs(fs) {

		// do nothing
	}

	T read() {
		return fs.read();
	}

	bool can_read() {
		return fs.can_read();
	}

	template<typename TT>
	pullbox<file_stream_source_t<T>, T, TT> operator<<(TT next) {
		return pullbox<file_stream_source_t<T>, T, TT>(*this, next);
	}

private:
	file_stream<T> & fs;
};

template<typename T>
file_stream_source_t<T> source(file_stream<T> & fs) {
	return file_stream_source_t<T>(fs);
}

template<typename T>
struct file_stream_sink_t {
	typedef void dest_t;

	file_stream_sink_t(file_stream<T> & fs)
		: fs(fs) {

		// do nothing
	}

	template<typename left_t>
	void use(left_t & left) {
		while (left.can_read()) {
			write(left.read());
		}
	}

	void write(const T & item) {
		fs.write(item);
	}

private:
	file_stream<T> & fs;
};

template<typename T>
file_stream_sink_t<T> sink(file_stream<T> & fs) {
	return file_stream_sink_t<T>(fs);
}

template<typename T>
struct cout_sink_t {
	typedef void dest_t;

	template<typename left_t>
	bool use(left_t & left, bool) {
		while (left.can_read()) {
			std::cout << left.read() << std::endl;
		}
		return true;
	}
};

}

#endif
