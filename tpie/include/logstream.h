// Copyright (c) 1994 Darren Erik Vengroff
//
// File: logstream.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//
// $Id: logstream.h,v 1.9 1999-04-08 01:39:13 rajiv Exp $
//
#ifndef _LOGSTREAM_H
#define _LOGSTREAM_H

#include <fstream.h>

// For size_t
#include <sys/types.h>

// A macro for declaring output operators for log streams.
#define _DECLARE_LOGSTREAM_OUTPUT_OPERATOR(T)	\
    logstream& operator<<(T)

// A log is like a regular output stream, but it also supports messages
// at different priorities.  If a message's priority is at least as high
// as the current priority threshold, then it appears in the log.  
// Otherwise, it does not.  Lower numbers have higher priority; 0 is
// the highest.  1 is the default if not 

class logstream : public ofstream {

  public:
    unsigned int priority;
    unsigned int threshold;

    logstream(const char *fname, unsigned int p = 0, unsigned int tp = 0);

    // Output operators

    _DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const char *);
    _DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const char);
    _DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const int);
    _DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const unsigned int);
    _DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const long int);
    _DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const long unsigned int);
    _DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const float);
    _DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const double);
#ifdef _BSD_OFF_T_
  _DECLARE_LOGSTREAM_OUTPUT_OPERATOR(off_t);
#endif
};


// The logmanip template is based on the omanip template from iomanip.h 
// in the libg++ sources.

template <class TP> class logmanip {
    logstream& (*_f)(logstream&, TP);
    TP _a;
public:
    logmanip(logstream& (*f)(logstream&, TP), TP a) : _f(f), _a(a) {}

    friend logstream& operator<< <> (logstream& o, const logmanip<TP>& m);
};

template<class TP>
logstream& operator<<(logstream& o, const logmanip<TP>& m)
{
    return (*m._f)(o, m._a);
}

logmanip<unsigned int> setpriority(unsigned int p);
logmanip<unsigned int> setthreshold(unsigned int p);

#endif // _LOGSTREAM_H 
