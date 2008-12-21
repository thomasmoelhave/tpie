// Copyright (c) 1994 Darren Vengroff
//
// File: ami_matrix_fill.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/12/94
//
// $Id: ami_matrix_fill.h,v 1.9 2005-11-17 17:11:25 jan Exp $
//
#ifndef _TPIE_APPS_MATRIX_FILL_H
#define _TPIE_APPS_MATRIX_FILL_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
// Get the AMI_scan_object definition.
#include <tpie/scan.h>

namespace tpie {
    
    namespace apps {
	
	template<class T>
	class matrix_filler {

	public:
	    virtual ami::err initialize(TPIE_OS_OFFSET rows, TPIE_OS_OFFSET cols) = 0;
	    virtual T element(TPIE_OS_OFFSET row, TPIE_OS_OFFSET col) = 0;
	    virtual ~matrix_filler() {};
	};

    }  //  namespace apps

}  //  namespace tpie

namespace tpie {

    namespace apps {
	
 
	template<class T>
	class matrix_fill_scan : ami::scan_object {

	private:
	    // Prohibit these
	    matrix_fill_scan(const matrix_fill_scan<T>& other);
	    matrix_fill_scan<T>& operator=(const matrix_fill_scan<T>& other);
	    
	    TPIE_OS_OFFSET r, c;
	    TPIE_OS_OFFSET cur_row, cur_col;
	    matrix_filler<T> *pemf;

	public:
	    matrix_fill_scan(matrix_filler<T> *pem_filler,
			     TPIE_OS_OFFSET rows, TPIE_OS_OFFSET cols) :
		r(rows), c(cols), 
		cur_row(0), cur_col(0),
		pemf(pem_filler) {
	    };
	    
	    ami::err initialize(void) {
		cur_row = cur_col = 0;
		return ami::NO_ERROR;
	    };

	    ami::err operate(T *out, ami::SCAN_FLAG *sf) {
		if ((*sf = (cur_row < r))) {
		    *out = pemf->element(cur_row,cur_col);
		    if (!(cur_col = (cur_col+1) % c)) {
			cur_row++;
		    }
		    return ami::SCAN_CONTINUE;
		} else {
		    return ami::SCAN_DONE;
		}        
	    };
	};
    

	template<class T>
	ami::err matrix_fill(matrix<T> *pem, matrix_filler<T> *pemf) {
	    ami::err ae;
	    
	    ae = pemf->initialize(pem->rows(), pem->cols());
	    if (ae != ami::NO_ERROR) {
		return ae;
	    }
	    
	    matrix_fill_scan<T> emfs(pemf, pem->rows(), pem->cols());
	    
	    return ami::scan(&emfs, dynamic_cast<ami::stream<T>*>(pem));
	};

    }  //  namespace apps

}  //  namespace tpie 

#endif // _TPIE_APPS_MATRIX_FILL_H 
