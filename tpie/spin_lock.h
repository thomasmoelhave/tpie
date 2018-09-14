// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
//
// Copyright 2017, The TPIE development team
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

///////////////////////////////////////////////////////////////////////////
/// \file tpie/spin_lock A spinlock
///////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_SPIN_LOCK_H__
#define __TPIE_SPIN_LOCK_H__

#include <atomic>
#include <limits>

namespace tpie {

class unique_spin_lock {
public:
	unique_spin_lock(std::atomic_size_t & spin): spin(spin) {
		while (true) {
			size_t expected = 0;
			if (spin.compare_exchange_weak(expected, 0x80000000)) return;
		}
	}
	~unique_spin_lock() {
		spin -= 0x80000000;
	}
private:
	std::atomic_size_t & spin;
};


class shared_spin_lock {
public:
	shared_spin_lock(std::atomic_size_t & spin): spin(spin), locked(false) {
		acquire();
	}
	void release() {
		--spin;
		locked = false;
	}

	void acquire() {
		spin++;
		while (spin.load() >= 0x80000000) {}
		locked = true;
	}
	
	~shared_spin_lock() {
		if (locked) release();
	}
private:
	std::atomic_size_t & spin;
	bool locked;
};

} //namespace tpie
#endif //__TPIE_SPIN_LOCK_H__
