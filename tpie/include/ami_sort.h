// Copyright (c) 1994 Darren Erik Vengroff
//
// File: ami_sort.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 6/10/94
//
// $Id: ami_sort.h,v 1.1 1994-09-29 12:50:54 darrenv Exp $
//
#ifndef _AMI_SORT_H
#define _AMI_SORT_H

// Declare sort, which a particular implementation will define.  Class
// T must have the operators < and = for comparison and assignment.

template<class T>
AMI_err AMI_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream); 

#ifdef AMI_IMP_SINGLE
#include <ami_sort_single.h>
#endif

#endif // _AMI_SORT_H 
