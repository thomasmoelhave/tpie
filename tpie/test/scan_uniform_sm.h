// Copyright (c) 1995 Darren Vengroff
//
// File: scan_uniform_sm.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/6/95
//
// $Id: scan_uniform_sm.h,v 1.6 2003-09-12 14:59:06 tavi Exp $
//
#ifndef _SCAN_UNIFORM_SM_H
#define _SCAN_UNIFORM_SM_H

#include <ami_sparse_matrix.h>

class scan_uniform_sm : public AMI_scan_object {
private:
    unsigned int r,c, rmax, cmax;
    double d;
public:
    scan_uniform_sm(unsigned int rows, unsigned int cols,
                    double density, int seed);
    virtual ~scan_uniform_sm(void);
    AMI_err initialize(void);
    AMI_err operate(AMI_sm_elem<double> *out, AMI_SCAN_FLAG *sf);
};

#endif // _SCAN_UNIFORM_SM_H 
