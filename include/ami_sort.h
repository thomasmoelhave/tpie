// Copyright (c) 1994 Darren Erik Vengroff
//
// File: ami_sort.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 6/10/94
//
// $Id: ami_sort.h,v 1.2 1994-10-04 19:07:52 darrenv Exp $
//
#ifndef _AMI_SORT_H
#define _AMI_SORT_H

#define CONST 

// Declare sort, which a particular implementation will define.

template<class T>
AMI_err AMI_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
                 int (*cmp)(CONST T&, CONST T&)); 

#ifdef AMI_IMP_SINGLE
#include <ami_sort_single.h>
#endif

#endif // _AMI_SORT_H 
