// Copyright (c) 1995 Darren Vengroff
//
// File: scan_square_sum.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/23/95
//
// $Id: scan_square_sum.h,v 1.1 1995-04-03 13:18:46 dev Exp $
//
#ifndef _SCAN_SQUARE_SUM_H
#define _SCAN_SQUARE_SUM_H

template<class T>
class scan_square_sum : AMI_scan_object {
private:
    T running_sum;
public:
    scan_square_sum() {};
    virtual ~scan_square_sum() {};
    T result(void)
    {
        return running_sum;
    };
    AMI_err initialize(void)
    {
        running_sum = 0;
        return AMI_ERROR_NO_ERROR;
    };
    inline AMI_err operate(const T &in, AMI_SCAN_FLAG *sf)
    {
        if (sf[0]) {
            running_sum += in * in;
            return AMI_SCAN_CONTINUE;
        } else {
            return AMI_SCAN_DONE;
        }
    };
};


#ifdef NO_IMPLICIT_TEMPLATES
#define TEMPLATE_INSTANTIATE_SCAN_SS(T)					\
template class scan_square_sum<T>;					\
template AMI_err AMI_scan(AMI_STREAM<T> *,				\
                          scan_square_sum<T> *);
#endif

#endif // _SCAN_SQUARE_SUM_H 
