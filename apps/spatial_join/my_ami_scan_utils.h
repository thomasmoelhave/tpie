// Copyright (c) 1994 Darren Erik Vengroff
//
// File: my_ami_scan_utils.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 8/31/94
//
// $Id: my_ami_scan_utils.h,v 1.1 2003-11-21 17:01:09 tavi Exp $
//
#ifndef _AMI_SCAN_UTILS_H
#define _AMI_SCAN_UTILS_H

#include <iostream>

#include "definitions.h"
#include <ami_stream.h>
#include <ami_scan.h>

// A scan object class template for reading the contents of an
// ordinary C++ input stream into a TPIE stream.  It works with
// streams of any type for which an >> operator is defined for C++
// stream input.

template<class T> class my_cxx_istream_scan : AMI_scan_object {
private:
    istream *is;
    coord_t xoffset_, yoffset_;
public:
    my_cxx_istream_scan(istream *instr = &cin, coord_t xoffset = 0, coord_t yoffset = 0);
    AMI_err initialize(void);
    AMI_err operate(T *out, AMI_SCAN_FLAG *sfout);
};

template<class T>
my_cxx_istream_scan<T>::my_cxx_istream_scan(istream *instr, coord_t xoffset, coord_t yoffset) : is(instr), xoffset_(xoffset), yoffset_(yoffset)
{
};

template<class T>
AMI_err my_cxx_istream_scan<T>::initialize(void)
{
    return AMI_ERROR_NO_ERROR;
};

template<class T>
AMI_err my_cxx_istream_scan<T>::operate(T *out, AMI_SCAN_FLAG *sfout)
{
    if (*is >> *out) {
        *sfout = (out->xhi < 0) ? true: false;
	out->xlo += xoffset_; out->ylo += yoffset_;
	out->xhi += xoffset_; out->yhi += yoffset_;
        return AMI_SCAN_CONTINUE;
    } else {
        *sfout = false;
        return AMI_SCAN_DONE;
    }
};



// A scan object to print the contents of a TPIE stream to a C++
// output stream.  One item per line is written.  It works with
// streams of any type for which an << operator is defined for C++
// stream output.

template<class T> class my_cxx_ostream_scan : AMI_scan_object {
private:
    ostream *os;
public:
    my_cxx_ostream_scan(ostream *outstr = &cout);
    AMI_err initialize(void);
    AMI_err operate(const T &in, AMI_SCAN_FLAG *sfin);
};

template<class T>
my_cxx_ostream_scan<T>::my_cxx_ostream_scan(ostream *outstr) : os(outstr)
{
};

template<class T>
AMI_err my_cxx_ostream_scan<T>::initialize(void)
{
    return AMI_ERROR_NO_ERROR;
};

template<class T>
AMI_err my_cxx_ostream_scan<T>::operate(const T &in, AMI_SCAN_FLAG *sfin)
{
    if (*sfin) {
        *os << in << '\n';
        return AMI_SCAN_CONTINUE;
    } else {
        return AMI_SCAN_DONE;
    }
};

#endif // _AMI_SCAN_UTILS_H 
