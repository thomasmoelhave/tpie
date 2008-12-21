// Copyright (c) 1994 Darren Vengroff
//
// File: matrix.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/4/94
//
// $Id: matrix.h,v 1.12 2005-11-17 17:11:25 jan Exp $
//

#ifndef _TPIE_APPS_MM_COLREF_H
#define _TPIE_APPS_MM_COLREF_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

namespace tpie {
    
    namespace apps { 
	
	template<class T> class mm_matrix_base;
	template<class T> class mm_matrix;
	
	template<class T> 
	class mm_colref {

	private:
	    mm_matrix_base<T> &m;
	    TPIE_OS_SIZE_T c;

	public:
	    mm_colref(mm_matrix_base<T> &amatrix, TPIE_OS_SIZE_T col);
	    ~mm_colref(void);
	    
	    T &operator[](const TPIE_OS_SIZE_T col) const;
	    
	    friend class mm_matrix_base<T>;
	    friend class mm_matrix<T>;
	};



	template<class T>
	mm_colref<T>::mm_colref(mm_matrix_base<T> &amatrix, TPIE_OS_SIZE_T col) :
	    m(amatrix),
	    c(col) {
	}
	
	template<class T>
	mm_colref<T>::~mm_colref(void) {
	}
	
	template<class T>
	T &mm_colref<T>::operator[](const TPIE_OS_SIZE_T row) const {
	    return m.elt(row,c);
	}
	
    }  //  namespace apps

}  //  namespace tpie 

#endif // _TPIE_APPS_MM_COLREF_H 
