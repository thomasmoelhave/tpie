// Copyright (c) 1995 Darren Vengroff
//
// File: scan_uniform_sm.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/6/95
//
// $Id: scan_uniform_sm.h,v 1.1 1995-03-07 15:14:26 darrenv Exp $
//
#ifndef _SCAN_UNIFORM_SM_H
#define _SCAN_UNIFORM_SM_H

extern "C" int srandom(int);
extern "C" int random(void);

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

#ifdef NO_IMPLICIT_TEMPLATES

#define TEMPLATE_INSTANTIATE_SCAN_UNIFORM_SM				\
template AMI_err AMI_scan(scan_uniform_sm *,				\
                          AMI_base_stream< AMI_sm_elem<double> > *);

#endif


#endif // _SCAN_UNIFORM_SM_H 
