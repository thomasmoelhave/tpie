// Copyright (c) 1995 Darren Vengroff
//
// File: scan_copy2.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/23/95
//
// $Id: scan_copy2.h,v 1.1 1995-04-03 13:17:22 dev Exp $
//
#ifndef _SCAN_COPY2_H
#define _SCAN_COPY2_H

template<class T>
class scan_copy2 : AMI_scan_object {
public:
    scan_copy2() {};
    virtual ~scan_copy2() {};
    AMI_err initialize(void)
    {
        return AMI_ERROR_NO_ERROR;
    };
    inline AMI_err operate(const T &in, AMI_SCAN_FLAG *sfin,
                           T *out1, T *out2, AMI_SCAN_FLAG *sfout)
    {
        if (sfout[0] = sfout[1] = *sfin) {
            *out1 = *out2 = in;
            return AMI_SCAN_CONTINUE;
        } else {
            return AMI_SCAN_DONE;
        }
    };
};

#ifdef NO_IMPLICIT_TEMPLATES
#define TEMPLATE_INSTANTIATE_SCAN_COPY2(T)				\
template class scan_copy2<T>;						\
template AMI_err AMI_scan(AMI_STREAM<T> *, scan_copy2<T> *,		\
                          AMI_STREAM<T> *, AMI_STREAM<T> *);
#endif

#endif // _SCAN_COPY2_H 
