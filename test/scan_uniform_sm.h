// Copyright (c) 1995 Darren Vengroff
//
// File: scan_uniform_sm.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/6/95
//
// $Id: scan_uniform_sm.h,v 1.4 1999-02-03 22:17:08 tavi Exp $
//
#ifndef _SCAN_UNIFORM_SM_H
#define _SCAN_UNIFORM_SM_H

extern "C" void srandom(unsigned int);
extern "C" long int random(void);

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
