// Copyright (c) 1994 Darren Erik Vengroff
//
// File: ami_sort.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 6/10/94
//
// $Id: ami_sort.h,v 1.5 1998-12-11 18:34:04 tavi Exp $
//
#ifndef _AMI_SORT_H
#define _AMI_SORT_H

#define CONST const

// This is #if'ed out because the declarations appear in the
// particular sort implementation.  g++ 2.6.0 gets confused when these
// templates are multiply declared and fails to find matches when
// AMI_sort() is called.

#if 0
// Declare sort, which a particular implementation will define.

template<class T>
AMI_err AMI_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
                 int (*cmp)(CONST T&, CONST T&)); 

template<class T>
AMI_err AMI_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream);
#endif

#ifdef AMI_IMP_SINGLE
#include <ami_sort_single.h>
#ifdef NO_IMPLICIT_TEMPLATES
#define TEMPLATE_INSTANTIATE_SORT_IMP_OP(T) \
TEMPLATE_INSTANTIATE_SORT_SINGLE_IMP_OP(T)
#define TEMPLATE_INSTANTIATE_SORT_IMP_CMP(T) \
TEMPLATE_INSTANTIATE_SORT_SINGLE_IMP_CMP(T)
#define TEMPLATE_INSTANTIATE_SORT_IMP_OBJ(T) \
TEMPLATE_INSTANTIATE_SORT_SINGLE_IMP_OBJ(T)
#endif
#endif

#ifdef NO_IMPLICIT_TEMPLATES
#define TEMPLATE_INSTANTIATE_SORT_OP(T)				\
TEMPLATE_INSTANTIATE_SORT_IMP_OP(T);				\
template AMI_err AMI_sort(AMI_STREAM<T> *instream,		\
                          AMI_STREAM<T> *outstream);		

#define TEMPLATE_INSTANTIATE_SORT_CMP(T)			\
TEMPLATE_INSTANTIATE_SORT_IMP_CMP(T);				\
template AMI_err AMI_sort(AMI_STREAM<T> *instream,		\
                          AMI_STREAM<T> *outstream,		\
                          int (*cmp)(CONST T&, CONST T&));

#define TEMPLATE_INSTANTIATE_SORT_OBJ(T)			\
TEMPLATE_INSTANTIATE_SORT_IMP_OBJ(T);				\
template AMI_err AMI_sort(AMI_STREAM<T> *instream,		\
                          AMI_STREAM<T> *outstream,		\
                          comparator<T> *cmp);
#endif

#endif // _AMI_SORT_H 
