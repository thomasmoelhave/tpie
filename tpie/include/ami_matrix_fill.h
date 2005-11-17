// Copyright (c) 1994 Darren Vengroff
//
// File: ami_matrix_fill.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/12/94
//
// $Id: ami_matrix_fill.h,v 1.9 2005-11-17 17:11:25 jan Exp $
//
#ifndef _AMI_MATRIX_FILL_H
#define _AMI_MATRIX_FILL_H

// Get definitions for working with Unix and Windows
#include <portability.h>
// Get the AMI_scan_object definition.
#include <ami_scan.h>

template<class T>
class AMI_matrix_filler {
public:
    virtual AMI_err initialize(TPIE_OS_OFFSET rows, TPIE_OS_OFFSET cols) = 0;
    virtual T element(TPIE_OS_OFFSET row, TPIE_OS_OFFSET col) = 0;
};

template<class T>
class AMI_matrix_fill_scan : AMI_scan_object {
private:
    // Prohibit these
    AMI_matrix_fill_scan(const AMI_matrix_fill_scan<T>& other);
    AMI_matrix_fill_scan<T>& operator=(const AMI_matrix_fill_scan<T>& other);

    TPIE_OS_OFFSET r, c;
    TPIE_OS_OFFSET cur_row, cur_col;
    AMI_matrix_filler<T> *pemf;
public:
    AMI_matrix_fill_scan(AMI_matrix_filler<T> *pem_filler,
                         TPIE_OS_OFFSET rows, TPIE_OS_OFFSET cols) :
	r(rows), c(cols), 
	cur_row(0), cur_col(0),
	pemf(pem_filler)
    {
    };
    AMI_err initialize(void)
    {
        cur_row = cur_col = 0;
        return AMI_ERROR_NO_ERROR;
    };
    AMI_err operate(T *out, AMI_SCAN_FLAG *sf)
    {
        if ((*sf = (cur_row < r))) {
            *out = pemf->element(cur_row,cur_col);
            if (!(cur_col = (cur_col+1) % c)) {
                cur_row++;
            }
            return AMI_SCAN_CONTINUE;
        } else {
            return AMI_SCAN_DONE;
        }        
    };
};

template<class T>
AMI_err AMI_matrix_fill(AMI_matrix<T> *pem, AMI_matrix_filler<T> *pemf)
{
    AMI_err ae;
    
    ae = pemf->initialize(pem->rows(), pem->cols());
    if (ae != AMI_ERROR_NO_ERROR) {
        return ae;
    }
    
    AMI_matrix_fill_scan<T> emfs(pemf, pem->rows(), pem->cols());
    
    return AMI_scan(&emfs, dynamic_cast<AMI_STREAM<T>*>(pem));
};

#endif // _AMI_MATRIX_FILL_H 
