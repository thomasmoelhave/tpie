// Copyright (c) 1994 Darren Vengroff
//
// File: ami_stack.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/15/94
//
// $Id: ami_stack.h,v 1.1 1994-12-16 21:47:13 darrenv Exp $
//
#ifndef _AMI_STACK_H
#define _AMI_STACK_H

template<class T>
class AMI_stack : public AMI_STREAM<T> {
public:
    AMI_stack(unsigned int device = 0,
              off_t max_len = 0); 
    ~AMI_stack(void);
    AMI_err push(const T &t);
    AMI_err pop(T **t);
};


template<class T>
AMI_stack<T>::AMI_stack(unsigned int device = 0,
                        off_t max_len = 0) :
                                AMI_STREAM<T>(device, max_len)
{
}

template<class T>
AMI_stack<T>::~AMI_stack(void)
{
}

template<class T>
AMI_err AMI_stack<T>::push(const T &t)
{
    AMI_err ae;
    off_t slen;
    
    ae = truncate((slen = stream_len())+1);
    if (ae != AMI_ERROR_NO_ERROR) {
        return ae;
    }

    ae = seek(slen);
    if (ae != AMI_ERROR_NO_ERROR) {
        return ae;
    }

    return write_item(t);
}


template<class T>
AMI_err AMI_stack<T>::pop(T **t)
{
    AMI_err ae;
    off_t slen;

    slen = stream_len();

    ae = seek(slen-1);
    if (ae != AMI_ERROR_NO_ERROR) {
        return ae;
    }

    ae = read_item(t);
    if (ae != AMI_ERROR_NO_ERROR) {
        return ae;
    }

    return truncate(slen-1);
}

#ifdef NO_IMPLICIT_TEMPLATES

#define TEMPLATE_INSTANTIATE_STACK(T)					\
template class AMI_stack<T>;

#endif

#endif // _AMI_STACK_H 
