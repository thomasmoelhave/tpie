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
#ifndef __TPIE_HASHMAP_H__
#define __TPIE_HASHMAP_H__

#include <tpie/array.h>
#include <tpie/unused.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <tpie/prime.h>

namespace tpie {

template <typename T>
struct hash {
	inline size_t operator()(const T & e) const {return e * 103841;}	
};

template <typename T1, typename T2>
struct hash<std::pair<T1,T2> > {
	hash<T1> h1;
	hash<T2> h2;
	inline size_t operator()(const std::pair<T1,T2> & e) const {
		return h1(e.first) + h2(e.second) * 99181;
	}	
};

template <typename value_t, typename hash_t, typename equal_t>
class chaining_hash_table {
private:
 	static const float sc;
	
 	struct bucket_t {
 		value_t value;
 		size_t next;
 	};
	
	size_t first_free;
	array<size_t> list;
	array<bucket_t> buckets;

  	hash_t h;
 	equal_t e;
public:
 	size_t size;
  	value_t unused;

	static double memory_coefficient() {
		return array<size_t>::memory_coefficient() *sc + array<bucket_t>::memory_coefficient();
	}

	static double memory_overhead() {
		return array<size_t>::memory_coefficient() * 100.0 
			+ array<size_t>::memory_overhead() + sizeof(chaining_hash_table) - sizeof(array<value_t>);
	}
	
	inline value_t & get(size_t idx) {return buckets[idx].value;}
	inline const value_t & get(size_t idx) const {return buckets[idx].value;}

 	void resize(size_t z) {
 		buckets.resize(z);
 		first_free = 0;
 		for (size_t i=0; i < z; ++i) buckets[i].next = i+1;
		size_t x=size_t(99+z*sc)|1;
		while (!is_prime(x)) x -= 2;
 		list.resize(x, std::numeric_limits<size_t>::max());
 	}

	chaining_hash_table(size_t e=0, value_t u=default_unused<value_t>::v()): size(0), unused(u) {resize(e);};

	inline size_t begin() {
		if (size == 0) return buckets.size();
		for(size_t i=0; true; ++i)
			if (buckets[i].value != unused) return i;
	}

	inline size_t end() const {return buckets.size();}	

 	inline size_t find(const value_t & value) const {
 		size_t v = list[h(value) % list.size()];
		while (v != std::numeric_limits<size_t>::max()) {
			if (e(buckets[v].value, value)) return v;
			v = buckets[v].next;
		}
 		return buckets.size();
 	}

	inline std::pair<size_t, bool> insert(const value_t & val) {
		size_t hv = h(val) % list.size();
 		size_t v = list[hv];
		while (v != std::numeric_limits<size_t>::max()) {
			if (e(buckets[v].value, val)) return std::make_pair(v, false);
			v = buckets[v].next;
		}
		v = first_free;
		first_free = buckets[v].next;
		buckets[v].value = val;
		buckets[v].next = list[hv];
		list[hv] = v;
 		++size;
		return std::make_pair(v, true);
	}


 	inline void erase(const value_t & val) {
		size_t hv = h(val) % list.size();
		size_t cur = list[hv];
		size_t prev = std::numeric_limits<size_t>::max();

		while (!e(buckets[cur].value, val)) {
			prev = cur;
			cur = buckets[cur].next;
		}
		
		if (prev == std::numeric_limits<size_t>::max())
			list[hv] = buckets[cur].next;
		else
			buckets[prev].next = buckets[cur].next;

		buckets[cur].next = first_free;
		buckets[cur].value = unused;
		first_free = cur;
 		--size;
 	}
};

template <typename value_t, typename hash_t, typename equal_t>
class linear_probing_hash_table {
private:
	static const float sc;
 	array<value_t> elements;
  	hash_t h;
 	equal_t e;
public:
 	size_t size;
  	value_t unused;

	static double memory_coefficient() {
		return array<value_t>::memory_coefficient() *sc;
	}

	static double memory_overhead() {
		return array<value_t>::memory_coefficient() * 100.0 
			+ array<value_t>::memory_overhead() + sizeof(linear_probing_hash_table) - sizeof(array<value_t>);
	}

	void resize(size_t element_count) {
		size_t x=size_t(99+element_count*sc)|1;
		while (!is_prime(x)) x -= 2;
		elements.resize(x, unused);
	}

	linear_probing_hash_table(size_t e=0, value_t u=default_unused<value_t>::v()):
		size(0), unused(u) {resize(e);}

 	inline size_t find(const value_t & value) const {
 		size_t v = h(value) % elements.size();
 		while (elements[v] != unused) {
 			if (e(elements[v], value)) return v;
 			v = (v + 1) % elements.size();
 		}
 		return elements.size();
 	}


	inline size_t end() const {return elements.size();}
	inline size_t begin() const {
		if (size == 0) return elements.size();
		for(size_t i=0; true; ++i)
			if (elements[i] != unused) return i;
	}

	value_t & get(size_t idx) {return elements[idx];}
	const value_t & get(size_t idx) const {return elements[idx];}

	inline std::pair<size_t, bool> insert(const value_t & val) {
 		size_t v = h(val) % elements.size();
 		while (elements[v] != unused) {
			if (e(elements[v],val)) {return std::make_pair(v, false);}
 			v = (v + 1) % elements.size();
 		}
 		++size;
		elements[v] = val;
		return std::make_pair(v, true);
	}

 	inline void erase(const value_t & val) {
		size_t slot = find(val);
 		size_t cur = (slot+1) % elements.size();
		assert(size < elements.size());
 		while (elements[cur] != unused) {	   
 			size_t x = h(elements[cur]) % elements.size();
 			if (x <= slot && (cur > slot || x > cur)) {
 				elements[slot] = elements[cur];
 				slot = cur;
 			}
 			cur = (cur+1) % elements.size();
 		}
 		elements[slot] = unused;
 		--size;
 	}
};

template <typename key_t, 
		  typename data_t, 
		  typename hash_t=hash<key_t>,
		  typename equal_t=std::equal_to<key_t>, 
		  template <typename value_t, typename hash_t, typename equal_t> class table_t = chaining_hash_table
		  >
class hash_map: public linear_memory_base< hash_map<key_t, data_t, hash_t, equal_t, table_t> > {
public:
	typedef std::pair<key_t, data_t> value_t;
private:
	struct key_equal_t {
		equal_t e;
		inline bool operator()(const value_t & a, const value_t & b) const {
			return e(a.first, b.first);
		}
	};

	struct key_hash_t {
		hash_t h;
		inline size_t operator()(const value_t & a) const {
			return h(a.first);
		}
	};

	typedef table_t<value_t, key_hash_t, key_equal_t> tbl_t;

	tbl_t tbl;

 	template <typename IT>
	class iter_base {
	protected:
		IT & tbl;
		size_t cur;
 		iter_base(IT & t, size_t c): tbl(t), cur(c) {};
 		friend class iterator;
 		friend class hash_map;
 	public:
 		inline const key_t & key() const {return tbl.get(cur)->first;}
 		inline const data_t & value() const {return tbl.get(cur)->second;}
 		inline const value_t & operator*() const {return tbl.get(cur);}
 		inline const value_t * operator->() const {return &tbl.get(cur);}
 		inline bool operator==(iter_base & o) const {return o.cur == cur;}
 		inline bool operator!=(iter_base & o) const {return o.cur != cur;}
 		inline void operator++() {
 			while (cur != tbl.end()) {
 				++cur;
				if (tbl.get(cur) != tbl.unused) break;
 			}
 		}
 	};
public:
	typedef iter_base<const tbl_t> const_iterator;
	
	class iterator: public iter_base<tbl_t> {
 	private:
		typedef iter_base<tbl_t> p_t;
 		friend class hash_map;
		inline iterator(tbl_t & tbl, size_t cur): p_t(tbl, cur) {}
		using p_t::tbl;
		using p_t::cur;
 	public:
 		inline key_t & key() {return tbl.get(cur)->first;}
 		inline data_t & value() {return tbl.get(cur)->second;}
 		inline value_t & operator*() {return tbl.get(cur);}
		inline value_t * operator->() {return &tbl.get(cur);}
 		inline operator const_iterator() const {return const_iterator(tbl, cur);}
 		inline bool operator==(const const_iterator & o) const {return o.cur == cur;}
 		inline bool operator!=(const const_iterator & o) const {return o.cur != cur;}
 	};


	static double memory_coefficient() {
		return tbl_t::memory_coefficient();
	}

	static double memory_overhead() {
		return tbl_t::memory_overhead() - sizeof(tbl_t) + sizeof(hash_map);
	}

	inline hash_map(size_t size=0): tbl(size) {}
	inline void resize(size_t size) {tbl.resize(size);}

	inline void erase(const key_t & key) {
		tbl.erase(value_t(key, tbl.unused.second));
	}
	
	inline void erase(const iterator & iter) {erase(iter.key());}

	inline bool insert(const key_t & key, const data_t & data) {
		return tbl.insert(value_t(key, data)).second;
	}

	inline data_t & operator[](const key_t & key) {
		return tbl.get(tbl.insert(value_t(key, tbl.unused.second)).first).second;
	}
	
	inline const data_t & operator[](const key_t & key) const {
		return tbl.get(tbl.find(key))->second;
	}
	
	inline iterator find(const key_t & key) {
		return iterator(tbl, tbl.find(value_t(key, tbl.unused.second)));
	}

	inline const_iterator find(const key_t & key) const {
		return const_iterator(tbl, tbl.find(key));
	}

	inline bool contains(const key_t & key) const {
		return tbl.find(value_t(key, tbl.unused.second)) != tbl.end();
	}
	inline iterator begin() {return iterator(tbl, tbl.begin());}
	inline const_iterator begin() const {return const_iterator(tbl, tbl.begin());}
	inline const_iterator cbegin() const {return const_iterator(tbl, tbl.begin());}

	inline iterator end() {return iterator(tbl, tbl.end());}
	inline const_iterator end() const {return const_iterator(tbl, tbl.end());}
	inline const_iterator cend() const {return const_iterator(tbl, tbl.end());}
	inline size_t size() const {return tbl.size;}
};

template <typename key_t,
		  typename hash_t=hash<key_t>,
		  typename equal_t=std::equal_to<key_t>,
		  template <typename value_t, typename hash_t, typename equal_t> class table_t=linear_probing_hash_table>
class hash_set {
private:
	typedef table_t<key_t, hash_t, equal_t> tbl_t;
	tbl_t tbl;
	typedef key_t value_t;

 	template <typename IT>
	class iter_base {
	protected:
		IT & tbl;
		size_t cur;
 		iter_base(IT & t, size_t c): tbl(t), cur(c) {};
 		friend class iterator;
 		friend class hash_set;
 	public:
 		inline const value_t & operator*() const {return tbl.get(cur);}
 		inline const value_t * operator->() const {return &tbl.get(cur);}
 		inline bool operator==(iter_base & o) const {return o.cur == cur;}
 		inline bool operator!=(iter_base & o) const {return o.cur != cur;}
 		inline void operator++() {
 			while (cur != tbl.end()) {
 				++cur;
				if (tbl.get(cur) != tbl.unused) break;
 			}
 		}
 	};
public:
	typedef iter_base<const tbl_t> const_iterator;
	
	class iterator: public iter_base<tbl_t> {
 	private:
		typedef iter_base<tbl_t> p_t;
 		friend class hash_set;
		inline iterator(tbl_t & tbl, size_t cur): p_t(tbl, cur) {}
		using p_t::tbl;
		using p_t::cur;
 	public:
 		inline value_t & operator*() {return tbl.get(cur);}
		inline value_t * operator->() {return &tbl.get(cur);}
 		inline operator const_iterator() const {return const_iterator(tbl, cur);}
 		inline bool operator==(const const_iterator & o) const {return o.cur == cur;}
 		inline bool operator!=(const const_iterator & o) const {return o.cur != cur;}
 	};


	static double memory_coefficient() {
		return tbl_t::memory_coefficient();
	}

	static double memory_overhead() {
		return tbl_t::memory_overhead() - sizeof(tbl_t) + sizeof(hash_set);
	}

	inline hash_set(size_t size=0): tbl(size) {}
	inline void resize(size_t size) {tbl.resize(size);}
	inline void erase(const key_t & key) {tbl.erase(key);}
	inline void erase(const iterator & iter) {erase(iter.key());}
	inline bool insert(const key_t & key) {
		return tbl.insert(key).second;
	}
	inline iterator find(const key_t & key) {
		return iterator(tbl, tbl.find(key));
	}

	inline const_iterator find(const key_t & key) const {
		return const_iterator(tbl, tbl.find(key));
	}

	inline bool contains(const key_t & key) const {return tbl.find(key) != tbl.end();}

	inline iterator begin() {return iterator(tbl, tbl.begin());}
	inline const_iterator begin() const {return const_iterator(tbl, tbl.begin());}
	inline const_iterator cbegin() const {return const_iterator(tbl, tbl.begin());}

	inline iterator end() {return iterator(tbl, tbl.end());}
	inline const_iterator end() const {return const_iterator(tbl, tbl.end());}
	inline const_iterator cend() const {return const_iterator(tbl, tbl.end());}
	inline size_t size() const {return tbl.size;}

};


template <typename value_t, typename hash_t, typename equal_t>
const float linear_probing_hash_table<value_t, hash_t, equal_t>::sc = 2.0f;

template <typename value_t, typename hash_t, typename equal_t>
const float chaining_hash_table<value_t, hash_t, equal_t>::sc = 2.f;

}
#endif //__TPIE_HASHMAP_H__
