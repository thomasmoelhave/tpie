// Copyright (c) 1994 Darren Vengroff
//
// File: ami_gen_perm_object.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/15/94
//
// $Id: ami_gen_perm_object.h,v 1.2 1998-12-11 18:38:20 tavi Exp $
//
#ifndef _AMI_GEN_PERM_OBJECT_H
#define _AMI_GEN_PERM_OBJECT_H

// For AMI_err.
#include <ami_base.h>

// A class of object that computes permutation destinations.
class AMI_gen_perm_object {
public:
    virtual AMI_err initialize(off_t len) = 0;
    virtual off_t destination(off_t src) = 0;
};

#endif // _AMI_GEN_PERM_OBJECT_H 
