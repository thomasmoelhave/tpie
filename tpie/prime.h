// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
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
#ifndef __TPIE_PRIME_H__
#define __TPIE_PRIME_H__
#include <tpie/array.h>
#include <tpie/util.h>

///////////////////////////////////////////////////////////////////////////
/// \file prime.h
/// \brief Contains computations related to prime numbers
///////////////////////////////////////////////////////////////////////////

namespace tpie {

///////////////////////////////////////////////////////////////////////////
/// Functor class to calculate if a given number i a prime
///////////////////////////////////////////////////////////////////////////
struct is_prime_t {
private:
	const size_type m;
	const size_type mr;
	array<size_type> m_pr;
public:
	is_prime_t();

	///////////////////////////////////////////////////////////////////////
	/// \brief Check if i is a prime.
	///
	/// \param i number to check, must be less then 4294967295
	/// \return true if and only if i is a prime number
	///////////////////////////////////////////////////////////////////////
	inline bool operator()(size_type i) { 
		for(size_type j =0; m_pr[j] * m_pr[j] <= i; ++j) {
			if (i % m_pr[j] == 0) return false;
		}
		return true;
	}
};

///////////////////////////////////////////////////////////////////////////
/// Functor object to check if a given number is a prime
///////////////////////////////////////////////////////////////////////////
extern is_prime_t is_prime;
}

#endif //__TPIE_PRIME_H__
