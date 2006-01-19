// Copyright (c) 1994 Darren Vengroff
//
// File: ami_stack.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/15/94
//
// $Id: ami_stack.h,v 1.10 2006-01-19 18:25:11 jan Exp $
//
#ifndef _AMI_STACK_H
#define _AMI_STACK_H

// Get definitions for working with Unix and Windows
#include <portability.h>
// Get the AMI_STREAM definition.
#include <ami_stream.h>

template<class T>
class AMI_stack {
  public:
    AMI_stack(); 
    AMI_stack(const char* path, 
        AMI_stream_type type = AMI_READ_WRITE_STREAM);
    ~AMI_stack(void);
    AMI_err push(const T &t);
    AMI_err pop(T **t);

    TPIE_OS_OFFSET size() const {
	return m_size;
    }

    //  This should go as soon as all old code has been migrated.
    TPIE_OS_OFFSET stream_len() const {
	cerr << "Using AMI_stack<T>::stream_len() is deprecated." << endl;
	return m_size;
    }

protected:
    AMI_STREAM<T>* m_amiStream;
    TPIE_OS_OFFSET m_size;
};


template<class T>
AMI_stack<T>::AMI_stack() : 
    m_amiStream(NULL), 
    m_size(0) {

    // No error checking done for the time being.
    m_amiStream = new AMI_STREAM<T>();
}

template<class T>
AMI_stack<T>::AMI_stack(const char* path, AMI_stream_type type) :
    m_amiStream(NULL), 
    m_size(0) {

    // No error checking done for the time being.
    m_amiStream = new AMI_STREAM<T>(path, type);

    // Set the size of the stack to be the number of items present 
    // in the underlying stream file. 
    m_size = m_amiStream->stream_len();
}

template<class T>
AMI_stack<T>::~AMI_stack() {

    // Make sure there are no left-overs.
    m_amiStream->truncate(m_size);

    delete m_amiStream;
}

template<class T>
AMI_err AMI_stack<T>::push(const T &t) {

    AMI_err ae = m_amiStream->write_item(t);

    if (ae == AMI_ERROR_NO_ERROR) {
	m_size++;
    }

    return ae;
}


template<class T>
AMI_err AMI_stack<T>::pop(T **t)
{
    AMI_err ae = AMI_ERROR_END_OF_STREAM;

    if (m_size) {

	ae = m_amiStream->seek(m_size-1);
	
	if (ae != AMI_ERROR_NO_ERROR) {
	    return ae;
	}
	
	ae = m_amiStream->read_item(t);
	
	if (ae != AMI_ERROR_NO_ERROR) {
	    return ae;
	}
	
	ae = m_amiStream->seek(m_size-1);
	
	if (ae == AMI_ERROR_NO_ERROR) {
	    m_size--;
	}
    }

    return ae;
}

#endif // _AMI_STACK_H 
