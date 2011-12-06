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

template <typename src_t, typename dest_t>
struct pipe {
};

template <>
struct pipe<void, void> {
	virtual ~pipe() {}
	virtual void operator()() {
		dytbot();
	}
	virtual void dytbot() = 0;
};

template<typename T>
struct is_not_void : public boost::true_type {
};

template<>
struct is_not_void<void> : public boost::false_type {
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

	void dytbot() {
		std::cout << "dyt bot" << std::endl;
		right.use(left);
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
	pipe<void, typename TT::dest_t> operator<<(TT next) {
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
	void use(left_t & left) {
		while (left.can_read()) {
			std::cout << left.read() << std::endl;
		}
	}
};

}

#endif
