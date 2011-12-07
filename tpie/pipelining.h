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

namespace tpie {

struct parameter_container_0 {
	template <typename R, typename dest_t>
	R construct(const dest_t & dest) const {
		return R(dest);
	}

	template <typename R>
	R construct() const {
		return R();
	}
};

template <typename T1>
struct parameter_container_1 {
	parameter_container_1(T1 t1) : t1(t1) {}

	template <typename R, typename dest_t>
	R construct(const dest_t & dest) const {
		return R(dest, t1);
	}

	template <typename R>
	R construct() const {
		return R(t1);
	}
private:
	T1 t1;
};

struct pipeline_v {
	virtual void run() = 0;
};

template <typename R, typename param_t>
struct pipeline_ : public pipeline_v {
	pipeline_(const param_t & params) : r(params) {}
	void run() {
		r.run();
	}
private:
	R r;
};

struct pipeline {
	template <typename T>
	pipeline(const T & from) {
		p = new T(from);
	}
	void operator()() {
		p->run();
	}
private:
	pipeline_v * p;
};

template <typename gen_t, typename param_t>
struct terminator {
	typedef gen_t type;
	terminator(const param_t & params) : params(params) {
	}
	param_t params;
};

template <template < class dest_t> class gen_t, typename param_t = parameter_container_0>
struct generate {
	typedef param_t parameter_type;

	generate() {
	}

	generate(const param_t & params) : params(params) {
	}

	template <template <class dest_t> class R>
	struct abe {
		template <class dest_t>
		struct box {
			typedef typename gen_t<R<dest_t> >::item_type item_type;
			gen_t<R<dest_t> > inner;

			void run() {
				inner.run();
			}

			box(const dest_t & dest, const std::pair<param_t, typename R<dest_t>::parameter_type> & params)
				: inner(params.first.template construct<gen_t<R<dest_t> >, R<dest_t> >
						(params.second.template construct<R<dest_t>, dest_t>(dest))) {
			}
		};
	};

	template <typename R, typename P>
	struct tbox {
		tbox(const P & params) : r(params.first.template construct<gen_t<R> >(params.second.template construct<R>())) {
		}
		void run() {
			r.run();
		}
	private:
		gen_t<R> r;
	};

	template <template <class dest_t> class R>
	generate<abe<R>::template box, std::pair<param_t, typename generate<R>::parameter_type> > operator|(const generate<R> & r) {
		return generate<abe<R>::template box, std::pair<param_t, typename generate<R>::parameter_type> >(std::make_pair(params, r.params));
	}

	template <typename R, typename S>
	pipeline_<tbox<R, std::pair<param_t, S> >, std::pair<param_t, S> > operator|(const terminator<R, S> term) {
		return pipeline_<tbox<R, std::pair<param_t, S> >, std::pair<param_t, S> >(std::make_pair(params, term.params));
	}

	void run() {
	}

	param_t params;
};

template <template <class dest_t> class gen_t>
generate<gen_t, parameter_container_0>
gengen() {
	return generate<gen_t, parameter_container_0>();
}

template <template <class dest_t> class gen_t, typename T1>
generate<gen_t, parameter_container_1<T1> >
gengen(T1 t1) {
	return generate<gen_t, parameter_container_1<T1> >(t1);
}

template <typename gen_t, typename T1>
terminator<gen_t, parameter_container_1<T1> > genterminator(T1 t1) {
	return terminator<gen_t, parameter_container_1<T1> >(t1);
}

template <typename dest_t>
struct input_t {
	typedef typename dest_t::item_type item_type;

	input_t(const dest_t & dest, file_stream<item_type> & fs) : dest(dest), fs(fs) {
	}

	void run() {
		while (fs.can_read()) {
			dest.push(fs.read());
		}
	}
private:
	dest_t dest;
	file_stream<item_type> & fs;
};

template<typename T>
generate<input_t, parameter_container_1<file_stream<T> &> > input(file_stream<T> & fs) {
	return gengen<input_t, file_stream<T> &>(fs);
}

template <typename dest_t>
struct identity_t {
	typedef typename dest_t::item_type item_type;

	identity_t(const dest_t & dest) : dest(dest) {
	}

	void push(const item_type & item) {
		std::cout << item << std::endl;
		dest.push(item);
	}
private:
	dest_t dest;
};

generate<identity_t> identity() {
	return gengen<identity_t>();
}

template <typename T>
struct output_t {
	typedef T item_type;

	output_t(file_stream<T> & fs) : fs(fs) {
	}

	void push(const T & item) {
		std::cout << item << std::endl;
		fs.write(item);
	}
private:
	file_stream<T> & fs;
};

template <typename T>
terminator<output_t<T>, parameter_container_1<file_stream<T> &> > output(file_stream<T> & fs) {
	return genterminator<output_t<T>, file_stream<T> &>(fs);
}

}

#endif
