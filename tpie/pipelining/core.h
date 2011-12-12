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

namespace tpie {

/* The only virtual method call in this sublibrary! */
struct pipeline_v {
	virtual void operator()() = 0;
};

template <typename fact_t>
struct pipeline_ : public pipeline_v {
	typedef typename fact_t::generated_type gen_t;
	inline pipeline_(const fact_t & factory) : r(factory.construct()) {}
	void operator()() {
		r();
	}
	inline operator gen_t() {
		return r;
	}
private:
	gen_t r;
};

struct pipeline {
	template <typename T>
	inline pipeline(const T & from) {
		p = new T(from);
	}
	inline void operator()() {
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
	inline terminator(const fact_t & factory) : factory(factory) {
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

	inline generate() {
	}

	inline generate(const fact_t & factory) : factory(factory) {
	}

	template <class fact1_t, class fact2_t>
	struct pair_factory {
		template <typename dest_t>
		struct generated {
			typedef typename fact1_t::template generated<typename fact2_t::template generated<dest_t>::type>::type type;
		};

		inline pair_factory(const fact1_t & fact1, const fact2_t & fact2)
			: fact1(fact1), fact2(fact2) {
		}

		template <typename dest_t>
		inline typename generated<dest_t>::type
		construct(const dest_t & dest) const {
			return fact1.construct(fact2.construct(dest));
		}

		fact1_t fact1;
		fact2_t fact2;
	};

	template <class fact1_t, class termfact2_t>
	struct termpair_factory {
		typedef typename fact1_t::template generated<typename termfact2_t::generated_type>::type generated_type;

		inline termpair_factory(const fact1_t & fact1, const termfact2_t & fact2)
			: fact1(fact1), fact2(fact2) {
			}

		fact1_t fact1;
		termfact2_t fact2;

		inline generated_type
		construct() const {
			return fact1.construct(fact2.construct());
		}
	};

	/* The pipe operator combines this generator/filter with another filter. */
	template <typename fact2_t>
	inline generate<pair_factory<fact_t, fact2_t> >
	operator|(const generate<fact2_t> & r) {
		return pair_factory<fact_t, fact2_t>(factory, r.factory);
	}

	/* This pipe operator combines this generator/filter with a terminator to
	 * make a pipeline. */
	template <typename fact2_t>
	inline pipeline_<termpair_factory<fact_t, fact2_t> >
	operator|(const terminator<fact2_t> & term) {
		return termpair_factory<fact_t, fact2_t>(factory, term.factory);
	}

	fact_t factory;
};

}

#endif
