// Copyright (c) 1994 Darren Erik Vengroff
//
// File: ami_scan_utils.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 8/31/94
//
// $Id: ami_scan_utils.h,v 1.2 1994-11-02 21:56:17 darrenv Exp $
//
#ifndef _AMI_SCAN_UTILS_H
#define _AMI_SCAN_UTILS_H

#include <iostream.h>

// A scan object class template for reading the contents of an
// ordinary C++ input stream into a TPIE stream.  It works with
// streams of any type for which an >> operator is defined for C++
// stream input.

template<class T> class cxx_istream_scan : AMI_scan_object {
private:
    istream *is;
public:
    cxx_istream_scan(istream *instr = &cin);
    AMI_err initialize(void);
    AMI_err operate(T *out, AMI_SCAN_FLAG *sfout);
};

template<class T>
cxx_istream_scan<T>::cxx_istream_scan(istream *instr) : is(instr)
{
};

template<class T>
AMI_err cxx_istream_scan<T>::initialize(void)
{
    return AMI_ERROR_NO_ERROR;
};

template<class T>
AMI_err cxx_istream_scan<T>::operate(T *out, AMI_SCAN_FLAG *sfout)
{
    if (*sfout = (is >> *out)) {
        return AMI_SCAN_CONTINUE;
    } else {
        return AMI_SCAN_DONE;
    }
};



// A scan object to print the contents of a TPIE stream to a C++
// output stream.  One item per line is written.  It works with
// streams of any type for which an << operator is defined for C++
// stream output.

template<class T> class cxx_ostream_scan : AMI_scan_object {
private:
    ostream *os;
public:
    cxx_ostream_scan(ostream *outstr = &cout);
    AMI_err initialize(void);
    AMI_err operate(const T &in, AMI_SCAN_FLAG *sfin);
};

template<class T>
cxx_ostream_scan<T>::cxx_ostream_scan(ostream *outstr) : os(outstr)
{
};

template<class T>
AMI_err cxx_ostream_scan<T>::initialize(void)
{
    return AMI_ERROR_NO_ERROR;
};

template<class T>
AMI_err cxx_ostream_scan<T>::operate(const T &in, AMI_SCAN_FLAG *sfin)
{
    if (*sfin) {
        *os << in << '\n';
        return AMI_SCAN_CONTINUE;
    } else {
        return AMI_SCAN_DONE;
    }
};

#ifdef NO_IMPLICIT_TEMPLATES

#define TEMPLATE_INSTANTIATE_OSTREAM(T) 				\
template class cxx_ostream_scan<T>; 					\
template AMI_err AMI_scan(AMI_base_stream<T> *, 			\
                          cxx_ostream_scan<T> *);

#define TEMPLATE_INSTANTIATE_ISTREAM(T) \
template class cxx_istream_scan<T>;

#endif

#endif // _AMI_SCAN_UTILS_H 
