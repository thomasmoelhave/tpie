// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2010, 2011, 2012 The TPIE development team
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
#ifndef __TPIE_HASH_H__
#define __TPIE_HASH_H__

#include <tpie/util.h>
#include <cstring>

namespace tpie {

namespace hash_bits {

extern size_t hash_codes[sizeof(size_t)][256];

} // namespace hash_bits

void init_hash();

///////////////////////////////////////////////////////////////////////////////
/// \brief Default tabulation-hashing function for integral (size_t-castable)
/// types.
/// \tparam T Type of value to hash.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
struct hash {
private:
	size_t m_matrix[sizeof(size_t)][256];
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate integer hash using tabulation hashing
	///////////////////////////////////////////////////////////////////////////
	inline size_t operator()(const T & e) const {
		size_t key = e;
		size_t result = 0;
		for(size_t i = 0; i < sizeof(size_t); ++i) {
			// use the least significant byte as key in the matrix
			result ^= hash_bits::hash_codes[i][key & 0xFF];
			key >>= 8;
		}

		return result;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Default hashing function for std::pair.
/// \tparam T1 First part of std::pair.
/// \tparam T2 Second part of std::pair.
///////////////////////////////////////////////////////////////////////////////
template <typename T1, typename T2>
struct hash<std::pair<T1,T2> > {
	hash<T1> h1;
	hash<T2> h2;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate std::pair hash.
	/// \param s Pair to hash.
	///////////////////////////////////////////////////////////////////////////
	inline size_t operator()(const std::pair<T1,T2> & e) const {
		return h1(e.first) + h2(e.second) * 99181;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Default hashing function for C-style strings.
///////////////////////////////////////////////////////////////////////////////
template <>
struct hash<const char *> {
	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate string hash.
	/// \param s String to hash.
	///////////////////////////////////////////////////////////////////////////
	inline size_t operator()(const char * s) const {
		uint32_t r = 1;
		for(int i=0; s[i]; i++){
			r = r*13+s[i]*7;
		}
		return r;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Default hashing function for std::string.
///////////////////////////////////////////////////////////////////////////////
template <>
struct hash<std::string> {
	hash<const char * > h;
	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate string hash by using std::string::c_str().
	/// \param s String to hash.
	///////////////////////////////////////////////////////////////////////////
	inline size_t operator()(const std::string & s) const {
		return h(s.c_str());
	}
};


// Predeclare reflect
template <typename R, typename T, typename ... TT>
bool reflect(R & r, T && v, TT && ... vs);

struct BufferedHash {
public:
	BufferedHash(size_t seed=0);
	BufferedHash(BufferedHash && o) {copy(o);}
	BufferedHash(const BufferedHash & o) {copy(o);}
	BufferedHash & operator=(BufferedHash && o) {copy(o); return *this;}
	BufferedHash & operator=(const BufferedHash & o) {copy(o); return *this;}

	void add(const char * data, size_t length) {
		const char * istart = data;
		const char * iend = data + length;
		while (iend - istart > stop - cur) {
			memcpy(cur, istart, stop - cur);
			istart += stop - cur;
			cur = stop;
			flush();
		}
		memcpy(cur, istart, iend - istart);
		cur += (iend - istart);
	}

	template <typename T>
	void add(const T & v) {
		add(reinterpret_cast<const char *>(&v), sizeof(v));
	}

	uint32_t finalize();
private:
	void flush();
	void copy(const BufferedHash & o);

	char buff[1024];
	char * stop;
	char * cur;
	uint32_t value;
};

template <typename H, typename T>
inline void ghash(H & h, const T & t);

///////////////////////////////////////////////////////////////////////////////
/// \brief Buffer based hash reflector
///////////////////////////////////////////////////////////////////////////////
template <typename H>
struct HashReflector {
public:
	static constexpr bool write = false;
	static constexpr bool arithmetic = true;
	static constexpr bool string = true;
	static constexpr bool trivialSerializable = false;

	HashReflector(H & h): h(h) {}

	void begin(const char *) {}
	void end() {}
	void beginArray(size_t x) {h.add(x);}
	void endArray() {}
	void name(const char *) {}
	void beginStaticArray(size_t) {}
	void endStaticArray() {}

	template <typename T>
	bool operator()(const T & v) {
		h.add(v);
		return true;
	}

	bool operator()(const std::string & v) {
		h.add(v.size());
		h.add(v.c_str(), v.size());
		return true;
	}

	template <typename T>
	bool apply(const T & v) {
		ghash(h, v);
		return true;
	}
private:
	H & h;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Hash t using reflection
///////////////////////////////////////////////////////////////////////////////
template <typename H, typename T>
inline void ghash(H & h, const T & t) {
	HashReflector<H> r(h);
	reflect(r, t);
}

} // namespace tpie

#endif //__TPIE_HASH_H__
