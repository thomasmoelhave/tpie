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

#ifndef __TPIE_PIPELINING_PAIR_FACTORY_H__
#define __TPIE_PIPELINING_PAIR_FACTORY_H__

#include <tpie/types.h>
#include <tpie/pipelining/priority_type.h>

namespace tpie {

namespace pipelining {

namespace bits {

template <typename child_t>
struct pair_factory_base {
	inline double memory() const {
		return self().fact1.memory() + self().fact2.memory();
	}

	inline void name(const std::string & n, priority_type) {
		push_breadcrumb(n);
	}

	void push_breadcrumb(const std::string & n) {
		self().fact1.push_breadcrumb(n);
		self().fact2.push_breadcrumb(n);
	}

private:
	inline child_t & self() {return *static_cast<child_t*>(this);}
	inline const child_t & self() const {return *static_cast<const child_t*>(this);}
};

template <class fact1_t, class fact2_t>
struct pair_factory : public pair_factory_base<pair_factory<fact1_t, fact2_t> > {
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
struct termpair_factory : public pair_factory_base<termpair_factory<fact1_t, termfact2_t> > {
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

} // namespace bits

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_PAIR_FACTORY_H__
