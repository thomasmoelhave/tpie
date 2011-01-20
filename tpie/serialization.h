// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2010, The TPIE development team
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
#ifndef __TPIE_SERIALIZATION_H__
#define __TPIE_SERIALIZATION_H__
#include <tpie/portability.h>
#include <vector>
#include <utility>

#include <boost/type_traits/is_fundamental.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/utility/enable_if.hpp>
#include <istream>
#include <ostream>

namespace tpie {

template <bool b1, bool b2>
struct _disjunction: public boost::true_type {};

template <>
struct _disjunction<false, false>: public boost::false_type {};

template <typename T1, typename T2>
struct disjunction: public _disjunction<T1::value, T2::value> {};

struct serialization_error: public std::runtime_error {
	explicit serialization_error(const std::string & what): std::runtime_error(what) {}
};

class serializer {
public:
	serializer(std::ostream & out): m_out(out) {
		*this << "TPIE Serialization" 
			  << (uint16_t)1
			  << false;
	}

	template <typename T>
	inline serializer & write(const T * data, size_t l) {
		*this << (boost::uint16_t)l;
		for (size_t i=0; i < l; ++i)
			*this << data[i];
		return *this;
	}

	template <typename T>
	inline typename boost::enable_if<disjunction<boost::is_fundamental<T>, boost::is_enum<T> > ,
									 serializer &>::type operator << (const T & x) {
		m_out.write(reinterpret_cast<const char*>(&x), sizeof(T));
		return * this;
	}

	template <typename T1, typename T2>
	inline serializer & operator <<(const std::pair<T1, T2> & p) {
		return *this << p.first << p.second;
	}

	template <typename T>
	inline serializer & operator <<(const std::vector<T> & v) {
		*this << (boost::uint16_t)v.size();
		for (size_t i=0; i < v.size(); ++i)
			*this << v[i];
		return *this;
	}

	inline serializer & operator <<(const char * data) {return write(data, strlen(data));}
	inline serializer & operator <<(const std::string & s) {return write(s.c_str(), s.size());}

private:
	std::ostream & m_out;
};

class unserializer {
public:
	unserializer(std::istream & in): m_in(in) {
		//Validate header;
		*this << "TPIE Serialization" 
			  << (uint16_t)1
			  << false;
	}

	template <typename T>
	inline unserializer & operator <<(const T & x) {
		T y;
		*this >> y;
		if (y != x) throw serialization_error("Verification failed");
		return *this;
	}

	inline unserializer & operator <<(const char * x) {
		std::string y;
		*this >> y;
		if (y != x) throw serialization_error("Verification failed");
		return *this;
	}

	template <typename T>
	inline unserializer & read(T * array, size_t & size) {
		boost::uint16_t x;
		*this >> x;
		if (x > size) throw serialization_error("array to short");
		size=x;
		for (size_t i=0; i < size; ++i)
			*this >> array[i];
		return *this;
	}

	template <typename T>
	inline typename boost::enable_if<disjunction<boost::is_fundamental<T>, boost::is_enum<T> >, unserializer &>::type operator >> (T & x) {
		char * y = reinterpret_cast<char*>(&x);
		m_in.read(y, sizeof(T));
		if (m_in.eof() || m_in.fail()) throw serialization_error("Out of bytes");
		return *this;
	}

	template <typename T1, typename T2>
	inline unserializer & operator >>(std::pair<T1, T2> & p) {
		return *this >> p.first >> p.second;
	}

	template <typename T>
	inline unserializer & operator >> (std::vector<T> & v) {
		boost::uint16_t size;
		*this >> size;
		v.clear();
		for (size_t i=0; i < size; ++i) {
			v.push_back(T());
			*this >> v.back();
		}
		return *this;
	}

	inline unserializer & operator >>(std::string & s) {
		s.clear();
		boost::uint16_t size;
		*this >> size;
		for (size_t i=0; i < size; ++i) {
			char x;
			*this >> x;
			s.push_back(x);
		}
		return *this;
	}

private:
	std::istream & m_in;
};

} //namespace tpie
#endif /*__TPIE_SERIALIZATION_H__*/
