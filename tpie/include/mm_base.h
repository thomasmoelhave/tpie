// Copyright (c) 1994 Darren Erik Vengroff
//
// File: mm_base.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/30/94
//
// $Id: mm_base.h,v 1.4 1994-09-22 14:56:32 darrenv Exp $
//
#ifndef _MM_BASE_H
#define _MM_BASE_H

#include <sys/types.h>

// MM Error codes
enum MM_err {
    MM_ERROR_NO_ERROR = 0,
    MM_ERROR_INSUFFICIENT_SPACE,
    MM_ERROR_UNDERFLOW,
    MM_ERROR_EXCESSIVE_ALLOCATION
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
    MM_STREAM_USAGE_MAXIMUM,
    // Maximum additional amount used by each substream created.
    MM_STREAM_USAGE_SUBSTREAM
};

// The base class for pointers into memory being managed by memory
// managers.  In a uniprocessor, these objects will simply contain
// pointers.  In multiprocessors, they will be more complicated
// descriptions of the layout of memory.
class MM_ptr_base
{
public:
    // This should return 1 to indicate a valid pointer and 0 to
    // indicate an invalid one.  It is usefull for tests and
    // assertions.
    virtual operator int (void) = 0;
};

// The base class for all memory management objects.
class MM_manager_base
{
public:
    // How much is currently available.
    virtual MM_err available(size_t *sz_a) = 0;
#if 0
    // Allocate some space.
    virtual MM_err alloc(size_t req, MM_ptr_base *p) = 0;
    // Free space.
    virtual MM_err free(MM_ptr_base *p) = 0;

    // Registration for main memory usage that cannot be allocated
    // directly through the memory manager.  Use of these is
    // discouraged, but sometimes unavoidable, such as in accounting
    // for space used in the buffer cache.
    
    virtual MM_err register_allocation(size_t sz) = 0;
    virtual MM_err register_deallocation(size_t sz) = 0;
#endif
    
};

#if 0
// A pointer to the one and only memory manager.
extern MM_manager_base *mm_manager;
#endif

#endif // _MM_BASE_H 
