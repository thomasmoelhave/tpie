// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
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

#include "common.h"
#include <tpie/memory.h>


struct mtest {
	size_t & r;
	const size_t ve;
	mtest(const size_t & a, const size_t & b, size_t & c): r(c), ve(b) {r=a;}
	~mtest(){r=ve;}
};

struct mtesta {
	virtual ~mtesta() {}
};

struct mtestb: public mtesta {
	size_t & x;
	mtestb(size_t & _): x(_) {}
	virtual ~mtestb() {x=42;}
};

bool basic_test() {
	{
		//Test allocation and deallocation
		size_t a1 = tpie::get_memory_manager().used();
		
		size_t * x = tpie::tpie_new<size_t>(42);
		if (*x != 42) return false;
		size_t a2 = tpie::get_memory_manager().used();
		
		tpie::tpie_delete(x);
		x = 0;
		tpie::tpie_delete(x);
		
		size_t a3 = tpie::get_memory_manager().used();
		if (a2 != a1 + sizeof(size_t) || a1 != a3) return false;
	}

	{
		//Test arrayes
		size_t a1 = tpie::get_memory_manager().used();
		
		size_t * x = tpie::tpie_new_array<size_t>(1234);
		size_t a2 = tpie::get_memory_manager().used();
		tpie::tpie_delete_array(x, 1234);
		size_t a3 = tpie::get_memory_manager().used();
		if (a2 != a1 + sizeof(size_t)*1234 || a1 != a3) return false;
	}
	
	{
		//Test constructors with references and destructors
		size_t foo=1;
		mtest * x = tpie::tpie_new<mtest>(2, 3, foo);
		if (foo != 2) return false;
		tpie::tpie_delete(x);
		if (foo != 3) return false;
	}

	{
		//Test virtual destructors
		size_t a1 = tpie::get_memory_manager().used();
		size_t foo=1;
		mtesta * x = tpie::tpie_new<mtestb>(foo);
		if (tpie::tpie_size(x) != sizeof(mtestb) || tpie::get_memory_manager().used() != sizeof(mtestb) +  a1)
			return false;
		tpie::tpie_delete(x);
		size_t a2 = tpie::get_memory_manager().used();
		if (a1 != a2 || foo != 42) return false;
	}

	{
		//Test auto ptr
		size_t a1 = tpie::get_memory_manager().used();
		{
			tpie::auto_ptr<size_t> x(tpie::tpie_new<size_t>(32));
			tpie::auto_ptr<size_t> y = x;
			y.reset(tpie::tpie_new<size_t>(54));
		}
		size_t a2 = tpie::get_memory_manager().used();
		if (a1 != a2) return false;
	}

	{ 
		//Allocator
		size_t a1;
		{
			a1 = tpie::get_memory_manager().used();
			std::vector<size_t, tpie::allocator<size_t> > myvect;
			myvect.resize(16);
			for(size_t i=0; i < 12345; ++i) {
				if (a1 + myvect.capacity() * sizeof(size_t) != tpie::get_memory_manager().used()) return false;
				myvect.push_back(12);
			}
			for(size_t i=0; i < 12345; ++i) {
				if (a1 + myvect.capacity() * sizeof(size_t) != tpie::get_memory_manager().used()) return false;
				myvect.pop_back();
			}
		}
		if (tpie::get_memory_manager().used() != a1) return false;
	}
		
	return true;
}

int main(int argc, char ** argv) {
  tpie_initer _(128);
  
  if(argc != 2) return 1;
  std::string test(argv[1]);
  
  if (test == "basic") 
    return basic_test()?EXIT_SUCCESS:EXIT_FAILURE;
  
}
