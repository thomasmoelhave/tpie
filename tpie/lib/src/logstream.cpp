// Copyright (c) 1994 Darren Erik Vengroff
//
// File: logstream.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//
// The logstream class, for writing to the log.
//

#include <versions.h>
VERSION(logstream_cpp,"$Id: logstream.cpp,v 1.17 2004-05-06 16:04:01 adanner Exp $");

#include <logstream.h>

// Constructor.
logstream::logstream(const char *fname, 
		     unsigned int p,
		     unsigned int tp) 
#ifdef UNIFIED_LOGGING
: ofstream(2), priority(p), threshold(tp) { log_initialized = true; } 
#else
: ofstream(fname), priority(p), threshold(tp) { log_initialized = true; } 
#endif

bool logstream::log_initialized = false;

// Destructor.
logstream::~logstream() {
  log_initialized = false;
}

// Output operators.

// A macro to define a log stream output operator for a given type.
// The type can be any type that has an ofstream output operator.
#define _DEFINE_LOGSTREAM_OUTPUT_OPERATOR(T)		\
logstream& logstream::operator<<(const T x)		\
{						       	\
    if (priority <= threshold) {		       	\
	ofstream::operator<<(x);			\
    }						       	\
    return *this;					\
}


logstream& logstream::operator<<(const char *x)
{									
    if (priority <= threshold) {					
	ofstream::operator<<(x);					
    }							
    return *this;							
}


//_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(char *)
_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(char)
_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(int)
_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(unsigned int)
_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(long int)
_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(long unsigned int)
_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(float)
_DEFINE_LOGSTREAM_OUTPUT_OPERATOR(double)

//"LONGLONG" and "long long" are different in Win32/Unix
TPIE_OS_DEFINE_LOGSTREAM_LONGLONG

// Setting priority and threshold on the fly with manipulators.

logstream& manip_priority(logstream& tpl, unsigned int p)
{
    tpl.priority = p;
    return tpl;
}

logmanip<unsigned int> setpriority(unsigned int p)
{
   return logmanip<unsigned int>(&manip_priority, p);
} 

logstream& manip_threshold(logstream& tpl, unsigned int p)
{
    tpl.threshold = p;
    return tpl;
}

logmanip<unsigned int> setthreshold(unsigned int p)
{
   return logmanip<unsigned int>(&manip_threshold, p);
} 
