// Copyright (c) 1994 Darren Vengroff
//
// File: ami_stack.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/15/94
//
// $Id: ami_stack.h,v 1.3 1999-02-03 17:39:05 tavi Exp $
//
#ifndef _AMI_STACK_H
#define _AMI_STACK_H

template<class T>
class AMI_stack : public AMI_STREAM<T> {
public:
    AMI_stack(); 
    ~AMI_stack(void);
    AMI_err push(const T &t);
    AMI_err pop(T **t);
};


template<class T>
AMI_stack<T>::AMI_stack() :
        AMI_STREAM<T>()
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

#endif // _AMI_STACK_H 
