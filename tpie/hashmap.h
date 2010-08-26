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

namespace tpie {

template <typename T>
struct hash {
	inline size_t operator()(const T & e) const {return e *103841;}	
};

template <typename T1, typename T2>
struct hash<std::pair<T1,T2> > {
	hash<T1> h1;
	hash<T2> h2;
	inline size_t operator()(const std::pair<T1,T2> & e) const {
		return h1(e.first) + h2(e.second) * 99181;
	}	
};

template <typename T>
struct default_unused {
	inline static T v() {return std::numeric_limits<T>::max();}
};

template <typename T1, typename T2>
struct default_unused<std::pair<T1, T2> > {
	inline static std::pair<T1,T2> v() {
		return std::pair<T1, T2>(default_unused<T1>::v(), default_unused<T2>::v());
	}
};

template <typename key_t, typename data_t, typename hash_t=hash<key_t>  >
class hash_map {
private:
	static const float sc;

	typedef std::pair<key_t, data_t> value_t;
	array<value_t> elements;
	size_t m_size;
	value_t unused;
	hash_t h;
	
	inline size_t search(const key_t & k) const {
		size_t v = h(k) % elements.size();
		while (elements[v] != unused) {
			if (elements[v].first == k) return v;
			v = (v + 1) % elements.size();
		}
		return elements.size();
	}

	template <typename IT>
	class iter_base {
	private:
		const value_t & unused;
		IT cur;
		IT end;
		iter_base(const IT & c, const IT & e, const value_t & u): unused(u), cur(c), end(e) {};
		friend class iterator;
		friend class hash_map;
	public:
		inline const key_t & key() const {return cur->first;}
		inline const data_t & value() const {return cur->second;}
		inline const value_t & operator*() const {return *cur;}
		inline const value_t & operator->() const {return *end;}
		inline bool operator==(iter_base & o) const {return o.cur == cur;}
		inline bool operator!=(iter_base & o) const {return o.cur != cur;}
		inline void operator++() {
			while (cur != end) {
				++cur;
				if (*cur != unused) break;
			}
		}
	};

public:
	typedef iter_base<typename array<value_t>::const_iterator> const_iterator;

	class iterator: public iter_base<typename array<value_t>::iterator> {
	private:
		typedef iter_base<typename array<value_t>::iterator> p_t;
		friend class hash_map;
		inline iterator(const typename array<value_t>::iterator & a,
				 const typename array<value_t>::iterator & b,
				 const value_t & u): p_t(a,b,u) {}
	public:
		inline key_t & key() {return p_t::cur->first;}
		inline data_t & value() {return p_t::cur->second;}
		inline value_t & operator*() {return *p_t::cur;}
		inline value_t * operator->() {return &(*p_t::cur);}
		inline operator const_iterator() const {return const_iterator(p_t::cur, p_t::end, p_t::unused);}
		inline bool operator==(const const_iterator & o) const {return o.cur == p_t::cur;}
		inline bool operator!=(const const_iterator & o) const {return o.cur != p_t::cur;}
	};

	static offset_type memory_required(offset_type elements) {
		return array<value_t>::memory_required(elements*sc) + sizeof(hash_map) - sizeof(array<value_t>);
	}
	
	static size_t memory_fits(size_t memory) {
		return array<value_t>::memory_fits(memory - sizeof(hash_map) + sizeof(array<value_t>))/sc;
	}

	hash_map(size_t e=0, value_t u=default_unused<value_t>::v()): elements(static_cast<size_t>(e*sc), u), m_size(0), unused(u) {};
	void resize(size_t countelemets) {elements.resize(countelemets*sc, unused);}

	inline bool insert(const key_t & key, const data_t & value) {
		size_t v = h(key) % elements.size();
		while (elements[v] != unused) {
			if (elements[v].first == key) return false;
			v = (v + 1) % elements.size();
		}
		assert(m_size <= elements.size()/sc);
		++m_size;
		elements[v].first = key;
		elements[v].second = value;
		return true;
	}

	inline data_t & operator[](const key_t & key) {
		size_t v = h(key) % elements.size();
		while (elements[v] != unused) {
			if (elements[v].first == key) return elements[v].second;
			v = (v + 1) % elements.size();
		}
		assert(m_size <= elements.size()/sc);
		++m_size;
		elements[v].first = key;
		return elements[v].second;
	}

	inline const data_t & operator[](const key_t & key) const {
		return elements[search(key)];
	}

	
	inline void erase(const iterator & elm) {
		size_t slot = elm.cur - elements.begin();
		size_t cur = (slot+1) % elements.size();
		while (elements[cur] != unused) {	   
			size_t x = h(elements[cur].first) % elements.size();
			if (x <= slot && (cur > slot || x > cur)) {
				elements[slot] = elements[cur];
				slot = cur;
			}
			cur = (cur+1) % elements.size();
		}
		elements[slot] = unused;
		--m_size;
	}
	
	inline void erase(const key_t & key) {
		erase(find(key));
	};
	
	const_iterator find(const key_t & key) const {
		return const_iterator(elements.find(search(key)), elements.end(), unused);
	}
	
	iterator find(const key_t & key) {
		return iterator(elements.find(search(key)), elements.end(), unused);
	}

	const_iterator end() const {
		return const_iterator(elements.end(), elements.end(), unused);
	}
	iterator end() {
		return iterator(elements.end(), elements.end(), unused);
	}
	
	inline size_t size() const {return m_size;}
};
template <typename key_t, typename data_t, typename hash_t>
const float hash_map<key_t, data_t, hash_t>::sc = 1.7f;

}
#endif //__TPIE_HASHMAP_H__
