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

#include <tpie/array.h>
#include <tpie/bit_array.h>
#include <tpie/array.h>
#include <tpie/concepts.h>
using namespace tpie;

bool basic_test() {
	array<int> hat;
  
	//Resize
	hat.resize(52, 42);
	if (hat.size() != 52) return false;
	for (size_type i=0; i < 52; ++i)
		if (hat[i] != 42) return false;
  
	//Get and set
	for (size_type i=0; i < 52; ++i)
		hat[i] = (i * 104729) % 2251;
  
	const tpie::array<int> & hat2(hat);
	for (size_type i=0; i < 52; ++i)
		if (hat2[i] != (int)((i * 104729) % 2251)) return false;
  
	if (hat.empty()) return false;
	hat.resize(0);
	if (!hat.empty()) return false;
	array<int> a(1,0),b(4,0),c(11,0);
	a[0] = b[0] = c[0] = 1;
	if(!a[0] || !b[0] || ! c[0]) return false;
	a[0] = b[0] = c[0] = 0;
	if(a[0] || b[0] || c[0]) return false;

	return true;
}


class auto_ptr_test_class {
public:
	size_t & dc;
	size_t & cc;
	auto_ptr_test_class(size_t & cc_, size_t & dc_): dc(dc_), cc(cc_) {
		++cc;
	}
	~auto_ptr_test_class() {
		++dc;
	}
	size_t hat() {return 42;}
private:
	auto_ptr_test_class(const auto_ptr_test_class & o): dc(o.dc), cc(o.cc) {}
};


bool auto_ptr_test() {
	size_t s=1234;
	size_t cc=0;
	size_t dc=0;
	array<tpie::auto_ptr<auto_ptr_test_class> > a;
	a.resize(s);
	for(size_t i=0; i < s; ++i) 
		a[i].reset(tpie_new<auto_ptr_test_class, size_t &, size_t &>(cc, dc));
	if (cc != s || dc != 0) return false;
	
	size_t x=0;
	for(size_t i=0; i < s; ++i) 
		x += a[i]->hat();
	if (x != 42*s) return false;

	if (cc != s || dc != 0) return false;

	for(size_t i=0; i < s; ++i) 
		a[i].reset(tpie_new<auto_ptr_test_class>(cc, dc));
	
	if (cc != 2*s || dc != s) return false;
	
	a.resize(0);
	if (cc != 2*s || dc != 2*s) return false;
	
	return true;
}

bool segmented_array_test() {
	array<int> h1;
	array_base<int, true> h2;
	size_t z=8388619;
	h1.resize(z);
	h2.resize(z);
	for (size_type i=0; i < 52; ++i)
		h2[i] = h1[i] = static_cast<int>((i * 833547)%z);

	array<int>::iterator i1=h1.begin();
	array_base<int, true>::iterator i2=h2.begin();
	
	while (i1 != h1.end() || i2 != h2.end()) {
		if (i1 == h1.end() || i2 == h2.end()) return false;
		if (*i1 != *i2) return false;
		i1++;
		i2++;
	}
	return true;
}

bool basic_bool_test() {
	tpie::bit_array hat;
  
	//Resize
	hat.resize(52, 1);
	if (hat.size() != 52) return false;
	for (size_type i=0; i < 52; ++i)
		if (hat[i] != true) return false;
  
	//Get and set
	return true;
	for (size_type i=0; i < 52; ++i)
		hat[i] = static_cast<bool>(((i * 104729)>>3) % 2);
  
	const tpie::bit_array & hat2(hat);
	for (size_type i=0; i < 52; ++i)
		if (hat2[i] != static_cast<bool>(((i * 104729)>>3) % 2)) return false;
  
	if (hat.empty()) return false;
	hat.resize(0);
	if (!hat.empty()) return false;
	bit_array a(1,0),b(4,0),c(11,0);
	a[0] = b[0] = c[0] = true;
	if(!a[0] || !b[0] || ! c[0]) return false;
	a[0] = b[0] = c[0] = false;
	if(a[0] || b[0] || c[0]) return false;

	return true;
}


bool iterator_test() {
	array<int> hat;
	hat.resize(52);

	for (size_type i=0; i < 52; ++i)
		hat[i] = (i * 104729) % 2251;
	{
		array<int>::const_iterator i=hat.begin();
		for (int j=0; j < 52; ++j) {
			if (i == hat.end()) return false;
			if (*i != ((j * 104729) % 2251)) return false;
			++i;
		}
		if (i != hat.end()) return false;
	}
	{
		array<int>::reverse_iterator i=hat.rbegin();
		for (int j=51; j >= 0; --j) {
			if (i == hat.rend()) return false;
			if (*i != ((j * 104729) % 2251)) return false;
			++i;
		}
		if (i != hat.rend()) return false;
	}

	std::sort(hat.begin(), hat.end());
	return true;
}

bool iterator_bool_test() {
	bit_array hat;
	hat.resize(52);

	for (size_type i=0; i < 52; ++i)
		hat[i] = static_cast<bool>(((i * 104729)>>7) % 2);
	{
		bit_array::const_iterator i=hat.begin();
		for (int j=0; j < 52; ++j) {
			if (i == hat.end()) {
				std::cerr << "end too soon" << std::endl;
				return false;
			}
			if (*i != static_cast<bool>(((j * 104729)>>7) % 2)) {
				std::cerr << j << std::endl;
				std::cerr << "Wrong value " << *i << " " << (((j * 104729)>>7) % 2) << std::endl;
				return false;
			}
			++i;
		}
		if (i != hat.end()) return false;
	}
	{
		bit_array::reverse_iterator i=hat.rbegin();
		for (int j=51; j >= 0; --j) {
			if (i == hat.rend()) return false;
			if (*i != static_cast<bool>(((j * 104729)>>7) % 2)) return false;
			++i;
		}
		if (i != hat.rend()) return false;
	}
  	std::sort(hat.begin(), hat.end());
	return true;
}

template <bool seg>
class array_memory_test: public memory_test {
public:
	array_base<int, seg> a;
	virtual void alloc() {a.resize(1024*1024*32);}
	virtual void free() {a.resize(0);}
	virtual size_type claimed_size() {
		return static_cast<size_type>(array_base<int, seg>::memory_usage(1024*1024*32));
	}
};

class array_bool_memory_test: public memory_test {
public:
	bit_array a;
	virtual void alloc() {a.resize(123456);}
	virtual void free() {a.resize(0);}
	virtual size_type claimed_size() {return static_cast<size_type>(bit_array::memory_usage(123456));}
};

int main(int argc, char **argv) {
	tpie_initer _(128);
	BOOST_CONCEPT_ASSERT((linear_memory_structure_concept<array<int> >));
	BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<array<int>::const_iterator>));
	BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<array<int>::const_reverse_iterator>));
	BOOST_CONCEPT_ASSERT((boost::Mutable_RandomAccessIterator<array<int>::iterator>));
	BOOST_CONCEPT_ASSERT((boost::Mutable_RandomAccessIterator<array<int>::reverse_iterator>));
	BOOST_CONCEPT_ASSERT((linear_memory_structure_concept<bit_array >));
	BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<bit_array::const_iterator>));
	BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<bit_array::const_reverse_iterator>));
  
	if(argc != 2) return 1;
	std::string test(argv[1]);
	if (test == "basic")
		return basic_test()?EXIT_SUCCESS:EXIT_FAILURE;
	else if (test == "iterators") 
		return iterator_test()?EXIT_SUCCESS:EXIT_FAILURE;
	else if (test == "auto_ptr")
		return auto_ptr_test()?EXIT_SUCCESS:EXIT_FAILURE;
	else if (test == "memory") 
		return array_memory_test<false>()()?EXIT_SUCCESS:EXIT_FAILURE;
	else if (test == "segmented")
		return segmented_array_test()?EXIT_SUCCESS:EXIT_FAILURE;
	else if (test == "memory_segmented") 
		return array_memory_test<true>()()?EXIT_SUCCESS:EXIT_FAILURE;
	else if (test == "bit_basic")
		return basic_bool_test()?EXIT_SUCCESS:EXIT_FAILURE;
	else if (test == "bit_iterators") 
		return iterator_bool_test()?EXIT_SUCCESS:EXIT_FAILURE;
	else if (test == "bit_memory") 
		return array_bool_memory_test()()?EXIT_SUCCESS:EXIT_FAILURE;
	return EXIT_FAILURE;
}
