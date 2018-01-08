// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2016, The TPIE development team
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
#ifndef __TPIE_JSONPRINT_H__
#define __TPIE_JSONPRINT_H__

#include <string>
#include <cstdint>
#include <iosfwd>
namespace tpie {

// predeclare reflection
template <typename R, typename T, typename ... TT>
bool reflect(R & r, T && v, TT && ... vs);

// JSONReflector pimpl
class JSONReflectorP;

/**
 * \brief refletor for json printing to an ostream
 */
struct JSONReflector {
public:
	static constexpr bool write = false;
	static constexpr bool arithmetic = true;
	static constexpr bool string = true;
	static constexpr bool trivialSerializable = false;

	JSONReflector(std::ostream & o, bool pretty);
	~JSONReflector();
	
	void begin(const char *);
	void end();
	void beginArray(size_t);	
	void endArray();
	void name(const char * name);

	void beginStaticArray(size_t x) {beginArray(x);}
	void endStaticArray() {endArray();}
	bool operator()(const uint8_t & v) {writeUint(v); return true;}
	bool operator()(const uint16_t & v) {writeUint(v); return true;}
	bool operator()(const uint32_t & v) {writeUint(v); return true;}
	bool operator()(const uint64_t & v) {writeUint(v); return true;}
	bool operator()(const int8_t & v) {writeInt(v); return true;}
	bool operator()(const int16_t & v) {writeInt(v); return true;}
	bool operator()(const int32_t & v) {writeInt(v); return true;}
	bool operator()(const int64_t & v) {writeInt(v); return true;}
	bool operator()(const float & v) {writeDouble(v); return true;}
	bool operator()(const double & v) {writeDouble(v); return true;}
	bool operator()(const std::string & v) {writeString(v); return true;}
private:
	void writeUint(uint64_t v);
	void writeInt(int64_t v);
	void writeDouble(double v);
	void writeString(const std::string & v);
	
	JSONReflectorP * p;
};

/**
 * \brief helper struct for printing
 */
template <typename T>
class JSONPrinter {
public:
	JSONPrinter(const T & t, bool pretty): t(t), pretty(pretty) {}
	
	friend std::ostream & operator << (std::ostream & o, const JSONPrinter & p) {
		JSONReflector jr(o, p.pretty);
		reflect(jr, p.t);
		return o;
	}
private:
	const T & t;
	bool pretty;
};

/**
 * \brief create a json print wrapper around t
 *
 * Example:
 * std::cout << json_printer(42) << std::endl;
 */
template <typename T>
inline JSONPrinter<T> json_printer(const T & t, bool pretty=true) {return JSONPrinter<T>(t, pretty);}

} //namespace tpie

#endif //__TPIE_JSONPRINT_H__
