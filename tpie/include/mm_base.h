// Copyright (c) 1994 Darren Erik Vengroff
//
// File: mm_base.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/30/94
//
// $Id: mm_base.h,v 1.2 1994-08-31 19:28:03 darrenv Exp $
//
#ifndef _MM_BASE_H
#define _MM_BASE_H

// MM Error codes
enum MM_err {
    MM_ERROR_NO_ERROR = 0,
    MM_ERROR_INSUFFICIENT_SPACE,
    MM_ERROR_UNDERFLOW
};

// types of memory usage queries we can make on streams (either BTE or MM)
enum MM_stream_usage {
    // Overhead of the object without the buffer
    MM_STREAM_USAGE_OVERHEAD = 1,
    // Max amount ever used by a buffer
    MM_STREAM_USAGE_BUFFER,
    // Amount currently in use.
    MM_STREAM_USAGE_CURRENT,
    // Max amount that will ever be used.
    MM_STREAM_USAGE_MAXIMUM
};


// The base class for all memory management objects.
class MM_base_manager
{
};

#endif // _MM_BASE_H 
