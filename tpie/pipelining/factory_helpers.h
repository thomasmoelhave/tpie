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

#ifndef __TPIE_PIPELINING_FACTORY_HELPERS_H__
#define __TPIE_PIPELINING_FACTORY_HELPERS_H__

namespace tpie {

namespace pipelining {

template <typename child_t>
struct factory_base {
	factory_base() : amount(0) {
	}

	inline void memory(double amount) {
		this->amount = amount;
	}
	
	inline double memory() const {
		return amount;
	}

private:
	double amount;
};

///////////////////////////////////////////////////////////////////////////////
/// \class factory_0
/// Push segment factory for 0-argument generator.
///
/// The factory classes are factories that take the destination
/// class as a template parameter and constructs the needed user-specified
/// filter.
///////////////////////////////////////////////////////////////////////////////
template <template <typename dest_t> class R>
struct factory_0 : public factory_base<factory_0<R> > {
	template<typename dest_t>
	struct generated {
		typedef R<dest_t> type;
	};

	template <typename dest_t>
	inline R<dest_t> construct(const dest_t & dest) const {
		R<dest_t> r(dest);
		r.set_memory_fraction(this->memory());
		return r;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \class factory_1
/// Push segment factory for 1-argument generator.
///////////////////////////////////////////////////////////////////////////////
template <template <typename dest_t> class R, typename T1>
struct factory_1 : public factory_base<factory_1<R, T1> > {
	template<typename dest_t>
	struct generated {
		typedef R<dest_t> type;
	};

	inline factory_1(T1 t1) : t1(t1) {}

	template <typename dest_t>
	inline R<dest_t> construct(const dest_t & dest) const {
		R<dest_t> r(dest, t1);
		r.set_memory_fraction(this->memory());
		return r;
	}
private:
	T1 t1;
};

///////////////////////////////////////////////////////////////////////////////
/// \class factory_2
/// Push segment factory for 2-argument generator.
///////////////////////////////////////////////////////////////////////////////
template <template <typename dest_t> class R, typename T1, typename T2>
struct factory_2 : public factory_base<factory_2<R, T1, T2> > {
	template<typename dest_t>
	struct generated {
		typedef R<dest_t> type;
	};

	inline factory_2(T1 t1, T2 t2) : t1(t1), t2(t2) {}

	template <typename dest_t>
	inline R<dest_t> construct(const dest_t & dest) const {
		R<dest_t> r(dest, t1, t2);
		r.set_memory_fraction(this->memory());
		return r;
	}
private:
	T1 t1;
	T2 t2;
};

///////////////////////////////////////////////////////////////////////////////
/// \class termfactory_0
/// Final push segment factory for 0-argument terminator.
///////////////////////////////////////////////////////////////////////////////
template <typename R>
struct termfactory_0 : public factory_base<termfactory_0<R> > {
	typedef R generated_type;
	inline R construct() const {
		R r;
		r.set_memory_fraction(this->memory());
		return r;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \class termfactory_1
/// Final push segment factory for 1-argument terminator.
///////////////////////////////////////////////////////////////////////////////
template <typename R, typename T1>
struct termfactory_1 : public factory_base<termfactory_1<R, T1> > {
	typedef R generated_type;
	inline termfactory_1(T1 t1) : t1(t1) {}
	inline R construct() const {
		R r(t1);
		r.set_memory_fraction(this->memory());
		return r;
	}
private:
	T1 t1;
};

///////////////////////////////////////////////////////////////////////////////
/// \class termfactory_2
/// Final push segment factory for 2-argument terminator.
///////////////////////////////////////////////////////////////////////////////
template <typename R, typename T1, typename T2>
struct termfactory_2 : public factory_base<termfactory_2<R, T1, T2> > {
	typedef R generated_type;
	inline termfactory_2(T1 t1, T2 t2) : t1(t1), t2(t2) {}
	inline R construct() const {
		R r(t1, t2);
		r.set_memory_fraction(this->memory());
		return r;
	}
private:
	T1 t1;
	T2 t2;
};

}

}

#endif
