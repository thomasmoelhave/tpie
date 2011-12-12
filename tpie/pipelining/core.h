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

#ifndef __TPIE_PIPELINING_CORE_H__
#define __TPIE_PIPELINING_CORE_H__

#include <iostream>
#include <tpie/file_stream.h>

namespace tpie {

/* The factory classes are factories that take the destination
 * class as a template parameter and constructs the needed user-specified
 * filter. */

template <template <typename dest_t> class R>
struct factory_0 {
	template<typename dest_t>
	struct generated {
		typedef R<dest_t> type;
	};

	template <typename dest_t>
	R<dest_t> construct(const dest_t & dest) const {
		return R<dest_t>(dest);
	}
};

template <template <typename dest_t> class R, typename T1>
struct factory_1 {
	template<typename dest_t>
	struct generated {
		typedef R<dest_t> type;
	};

	factory_1(T1 t1) : t1(t1) {}

	template <typename dest_t>
	R<dest_t> construct(const dest_t & dest) const {
		return R<dest_t>(dest, t1);
	}
private:
	T1 t1;
};

template <typename R, typename T1>
struct termfactory_1 {
	typedef R generated_type;
	termfactory_1(T1 t1) : t1(t1) {}
	R construct() const {
		return R(t1);
	}
private:
	T1 t1;
};

/* The only virtual method call in this sublibrary! */
struct pipeline_v {
	virtual void operator()() = 0;
};

template <typename fact_t>
struct pipeline_ : public pipeline_v {
	typedef typename fact_t::generated_type gen_t;
	pipeline_(const fact_t & factory) : r(factory.construct()) {}
	void operator()() {
		r();
	}
	operator gen_t() {
		return r;
	}
private:
	gen_t r;
};

struct pipeline {
	template <typename T>
	pipeline(const T & from) {
		p = new T(from);
	}
	void operator()() {
		(*p)();
	}
private:
	pipeline_v * p;
};

/* A terminator class has input pushed into it.
 * gen_t: The user-specified output handler.
 * param_t: The gen_t factory. */
template <typename fact_t>
struct terminator {
	terminator(const fact_t & factory) : factory(factory) {
	}
	fact_t factory;
};

/* A generate class pushes input down the pipeline.
 * gen_t: The user-specified class that either generates input or filters input.
 * dest_t: Whatever follows gen_t.
 * param_t: gen_t factory. */
template <typename fact_t>
struct generate {
	typedef fact_t factory_type;

	generate() {
	}

	generate(const fact_t & factory) : factory(factory) {
	}

	template <class fact1_t, class fact2_t>
	struct pair_factory {
		template <typename dest_t>
		struct generated {
			typedef typename fact1_t::template generated<typename fact2_t::template generated<dest_t>::type>::type type;
		};

		pair_factory(const fact1_t & fact1, const fact2_t & fact2)
			: fact1(fact1), fact2(fact2) {
		}

		template <typename dest_t>
		typename generated<dest_t>::type
		construct(const dest_t & dest) const {
			return fact1.construct(fact2.construct(dest));
		}

		fact1_t fact1;
		fact2_t fact2;
	};

	template <class fact1_t, class termfact2_t>
	struct termpair_factory {
		typedef typename fact1_t::template generated<typename termfact2_t::generated_type>::type generated_type;

		termpair_factory(const fact1_t & fact1, const termfact2_t & fact2)
			: fact1(fact1), fact2(fact2) {
			}

		fact1_t fact1;
		termfact2_t fact2;

		generated_type
		construct() const {
			return fact1.construct(fact2.construct());
		}
	};

	/* The pipe operator combines this generator/filter with another filter. */
	template <typename fact2_t>
	generate<pair_factory<fact_t, fact2_t> >
	operator|(const generate<fact2_t> & r) {
		return pair_factory<fact_t, fact2_t>(factory, r.factory);
	}

	/* This pipe operator combines this generator/filter with a terminator to
	 * make a pipeline. */
	template <typename fact2_t>
	pipeline_<termpair_factory<fact_t, fact2_t> >
	operator|(const terminator<fact2_t> & term) {
		return termpair_factory<fact_t, fact2_t>(factory, term.factory);
	}

	fact_t factory;
};

template <typename dest_t>
struct input_t {
	typedef typename dest_t::item_type item_type;

	input_t(const dest_t & dest, file_stream<item_type> & fs) : dest(dest), fs(fs) {
	}

	void operator()() {
		while (fs.can_read()) {
			dest.push(fs.read());
		}
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

	void push(const item_type & item) {
		std::cout << item << std::endl;
		dest.push(item);
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

	void push(const T & item) {
		std::cout << item << std::endl;
		fs.write(item);
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
