// Copyright (c) 1994 Darren Vengroff
//
// File: ami_stream_arith.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/10/94
//
// $Id: ami_stream_arith.h,v 1.1 1995-01-10 16:52:57 darrenv Exp $
//
#ifndef _AMI_STREAM_ARITH_H
#define _AMI_STREAM_ARITH_H

#define SCAN_OPERATOR_DECLARATION(NAME,OP)				\
									\
template<class T> class AMI_scan_ ## NAME : AMI_scan_object {		\
public:									\
    AMI_err initialize(void);						\
    AMI_err operate(const T &op1, const T &op2, AMI_SCAN_FLAG *sfin,	\
                    T *res, AMI_SCAN_FLAG *sfout);			\
};									\
									\
template<class T>							\
AMI_err AMI_scan_ ## NAME<T>::initialize(void)				\
{									\
    return AMI_ERROR_NO_ERROR;						\
}									\
									\
									\
template<class T>							\
AMI_err AMI_scan_ ## NAME<T>::operate(const T &op1, const T &op2,	\
                                  AMI_SCAN_FLAG *sfin,			\
                                  T *res, AMI_SCAN_FLAG *sfout)		\
{									\
    if (*sfout = (sfin[0] && sfin[1])) {				\
        *res = op1 OP op2;						\
        return AMI_SCAN_CONTINUE;					\
    } else {								\
        return AMI_SCAN_DONE;						\
    }									\
}

SCAN_OPERATOR_DECLARATION(add,+)
SCAN_OPERATOR_DECLARATION(sub,-)
SCAN_OPERATOR_DECLARATION(mult,*)
SCAN_OPERATOR_DECLARATION(div,/)

#ifdef NO_IMPLICIT_TEMPLATES

#define TEMPLATE_INSTANTIATE_STREAM_ADD(T)			\
template class AMI_scan_add<T>;					\
template AMI_err AMI_scan(AMI_base_stream<T> *,			\
                          AMI_base_stream<T> *,			\
                          AMI_scan_add<T> *,			\
                          AMI_base_stream<T> *);

#define TEMPLATE_INSTANTIATE_STREAM_SUB(T)			\
template class AMI_scan_sub<T>;					\
template AMI_err AMI_scan(AMI_base_stream<T> *,			\
                          AMI_base_stream<T> *,			\
                          AMI_scan_sub<T> *,			\
                          AMI_base_stream<T> *);

#define TEMPLATE_INSTANTIATE_STREAM_MULT(T)			\
template class AMI_scan_mult<T>;				\
template AMI_err AMI_scan(AMI_base_stream<T> *,			\
                          AMI_base_stream<T> *,			\
                          AMI_scan_mult<T> *,			\
                          AMI_base_stream<T> *);

#define TEMPLATE_INSTANTIATE_STREAM_DIV(T)			\
template class AMI_scan_div<T>;					\
template AMI_err AMI_scan(AMI_base_stream<T> *,			\
                          AMI_base_stream<T> *,			\
                          AMI_scan_div<T> *,			\
                          AMI_base_stream<T> *);

#endif

#endif // _AMI_STREAM_ARITH_H 
