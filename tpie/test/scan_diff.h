// Copyright (c) 1994 Darren Erik Vengroff
//
// File: scan_diff.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/7/94
//
// $Id: scan_diff.h,v 1.1 1994-10-31 21:34:57 darrenv Exp $
//
// A scanner to diff two streams of the same type of object, to verify that
// they are the same.
//
#ifndef _SCAN_DIFF_H
#define _SCAN_DIFF_H

template<class T> struct scan_diff_out {
    unsigned int index;
    T t0, t1;
};

template<class T> class scan_diff : AMI_scan_object {
private:
    unsigned int input_index;
    T null_t;
public:
    scan_diff(const T &nt);
    virtual ~scan_diff(void);
    AMI_err initialize();
    AMI_err operate(const T &in1, const T &in2, AMI_SCAN_FLAG *sfin,
                    scan_diff_out<T> *out,
                    AMI_SCAN_FLAG *sfout);
};

template<class T>
scan_diff<T>::scan_diff(const T &nt)
{
    null_t = nt;
}

template<class T>
scan_diff<T>::~scan_diff()
{
}

template<class T>
AMI_err scan_diff<T>::initialize(void)
{
    input_index = 0;
    return AMI_ERROR_NO_ERROR;
}

template<class T>
AMI_err scan_diff<T>::operate(const T &in1, const T &in2,
                              AMI_SCAN_FLAG *sfin,
                              scan_diff_out<T> *out,
                              AMI_SCAN_FLAG *sfout)
{
    AMI_err ret;
    
    if (sfin[0] && sfin[1]) {
        if (in1 == in2) {
            *sfout = false;
        } else {
            *sfout = true;
            out->index = input_index;
            out->t0 = in1;
            out->t1 = in2;
        }
        ret =  AMI_SCAN_CONTINUE;
    } else if (!sfin[0] && !sfin[1]) {
        *sfout = false;
        ret = AMI_SCAN_DONE;
    } else {
        ret =  AMI_SCAN_CONTINUE;
        *sfout = true;
        out->index = input_index;
        if (!sfin[0]) {
            out->t0 = null_t;
            out->t1 = in2;
        } else { 
            out->t1 = null_t;
            out->t0 = in1;
        }
    }
    input_index++;
    return ret;
}   


#endif // _SCAN_DIFF_H 
