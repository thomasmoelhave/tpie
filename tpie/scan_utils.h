// Copyright (c) 1994 Darren Erik Vengroff
//
// File: ami_scan_utils.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 8/31/94
//
// $Id: ami_scan_utils.h,v 1.12 2005-11-18 12:29:52 jan Exp $
//
#ifndef _TPIE_AMI_SCAN_UTILS_H
#define _TPIE_AMI_SCAN_UTILS_H

#include <iostream>

// Get definitions for working with Unix and Windows.
#include <portability.h>
// Get the AMI_scan_object definition.
#include <scan.h>

namespace tpie {

    namespace ami {

// A scan object class template for reading the contents of an
// ordinary C++ input stream into a TPIE stream.  It works with
// streams of any type for which an >> operator is defined for C++
// stream input.
	
	template<class T> class cxx_istream_scan : scan_object {
	    
	private:
	    // Prohibit these.
	    cxx_istream_scan(const cxx_istream_scan<T>& other);
	    cxx_istream_scan<T>& operator=(const cxx_istream_scan<T>& other);
	    std::istream *is;

	public:
	    cxx_istream_scan(std::istream *instr = &std::cin);
	    err initialize(void);
	    err operate(T *out, SCAN_FLAG *sfout);
	};
	
	template<class T>
	cxx_istream_scan<T>::cxx_istream_scan(std::istream *instr) : is(instr) {
	    //  No code in this constructor.
	};

	template<class T>
	err cxx_istream_scan<T>::initialize(void) {
	    return NO_ERROR;
	};

	template<class T>
	err cxx_istream_scan<T>::operate(T *out, SCAN_FLAG *sfout) {
	    if (*is >> *out) {
		*sfout = true;
		return SCAN_CONTINUE;
	    } 
	    else {
		*sfout = false;
		return SCAN_DONE;
	    }
	};

    }  //  ami namespace

}  //  tpie namespace 


namespace tpie {
    
    namespace ami {
	
	
// A scan object to print the contents of a TPIE stream to a C++
// output stream.  One item per line is written.  It works with
// streams of any type for which an << operator is defined for C++
// stream output.
	
	template<class T> class cxx_ostream_scan : scan_object {

	private:
	    // Prohibit these.
	    cxx_ostream_scan(const cxx_ostream_scan<T>& other);
	    cxx_ostream_scan<T>& operator=(const cxx_ostream_scan<T>& other);
	    std::ostream *os;

	public:
	    cxx_ostream_scan(std::ostream *outstr = &std::cout);
	    err initialize(void);
	    err operate(const T &in, SCAN_FLAG *sfin);
	};
	
	template<class T>
	cxx_ostream_scan<T>::cxx_ostream_scan(std::ostream *outstr) : os(outstr) {
	    //  No code in this constructor.
	};
	
	template<class T>
	err cxx_ostream_scan<T>::initialize(void) {
	    return NO_ERROR;
	};
	
	template<class T>
	err cxx_ostream_scan<T>::operate(const T &in, SCAN_FLAG *sfin) {
	    if (*sfin) {
		*os << in << '\n';
		return SCAN_CONTINUE;
	    } 
	    else {
		return SCAN_DONE;
	    }
	};
	
    }  //  ami namespace
    
}  //  tpie namespace 


#endif // _TPIE_AMI_SCAN_UTILS_H 
