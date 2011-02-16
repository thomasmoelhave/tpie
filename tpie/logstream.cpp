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

// The logstream class, for writing to the log.
#include <tpie/logstream.h>


using namespace tpie;

// Constructor.
logstream::logstream(log_target * target, log_level p,
					 log_level tp): priority(p), threshold(tp), m_target(target), disable(false) { log_initialized = true; } 

bool logstream::log_initialized = false;

// Destructor.
logstream::~logstream() {
  log_initialized = false;
}

bool logstream::flush() {
	bool d=true;										
	std::swap(d,disable);								
	if (d) return true;
	if (m_sstream.str().size())
		(*m_target)(priority, m_sstream.str().c_str());
	m_sstream.clear();
	disable=d;
	return true;
}

// Output operators.

// A macro to define a log stream output operator for a given type.
// The type can be any type that has an ofstream output operator.
#define _DEFINE_LOGSTREAM_OUTPUT_OPERATOR(T)					\
    tpie::logstream& tpie::logstream::operator<<(const T x) {	\
		if (priority <= threshold) {							\
			bool d=true;										\
			std::swap(d,disable);								\
			if(!d) m_sstream << x;								\
			disable=d;											\
		}														\
		return *this;											\
	}

_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(const char *)
_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(const std::string &)
_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(char)
_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(int)
_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(unsigned int)
_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(long int)
_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(unsigned long)
_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(float)
_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(double)
#ifdef _WIN64
_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(size_t)
#endif


//"LONGLONG" and "long long" are different in Win32/Unix
TPIE_OS_DEFINE_LOGSTREAM_LONGLONG

// Setting priority and threshold on the fly with manipulators.

logstream& tpie::manip_priority(logstream& tpl, log_level p)
{
	tpl.flush();
    tpl.priority = p;
    return tpl;
}

logmanip<log_level> tpie::setpriority(log_level p)
{
   return logmanip<log_level>(&manip_priority, p);
} 

logstream& tpie::manip_threshold(logstream& tpl, log_level p)
{
    tpl.threshold = p;
    return tpl;
}

logmanip<log_level> tpie::setthreshold(log_level p)
{
   return logmanip<log_level>(&manip_threshold, p);
} 


