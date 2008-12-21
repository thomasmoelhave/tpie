// Copyright (c) 1995 Darren Vengroff
//
// File: ami_sparse_matrix.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/2/95
//
// $Id: ami_sparse_matrix.h,v 1.16 2005-11-16 16:53:50 jan Exp $
//
#ifndef _TPIE_APPS_SM_ELEM_H
#define _TPIE_APPS_SM_ELEM_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <iostream>

namespace tpie {

    namespace apps {

// A sparse matrix element is labeled with a row er and a column ec.
// A sparse matrix is simply represented by a colletion of these.
	template <class T>
	class sm_elem {

	public:
	    TPIE_OS_OFFSET er;
	    TPIE_OS_OFFSET ec;
	    T val;

	};

	template <class T>
	std::ostream &operator<<(std::ostream& s, const sm_elem<T> &a) {
	    return s << a.er << ' ' << a.ec << ' ' << a.val;
	};
	
	template <class T>
	std::istream &operator>>(std::istream& s, sm_elem<T> &a) {
	    return s >> a.er >> a.ec >> a.val;
	};
	
    }  //  namespace apps

}  //  namespace tpie 



#endif // _TPIE_APPS_SM_ELEM_H
