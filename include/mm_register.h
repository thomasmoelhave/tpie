// Copyright (c) 1994 Darren Erik Vengroff
//
// File: mm_register.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/30/94
//
// $Id: mm_register.h,v 1.9 2003-04-17 19:42:08 jan Exp $
//
#ifndef _MM_REGISTER_H
#define _MM_REGISTER_H

// Get definitions for working with Unix and Windows
#include <portability.h>

#define MM_REGISTER_VERSION 2

// Get size_t
#include <sys/types.h>

// To be defined later in this file.
class mm_register_init;

// Declarations of a very simple memory manager desgined to work with
// BTEs that rely on the underlying OS to manage physical memory.
// Examples include BTEs based on mmap() and the stdio library.
// Another type of BTE this MM would be useful for is one which is
// designed to make efficient use of a cache for programs running
// entirely in main memory.

class MM_register {
private:
    // The number of instances of this class and descendents that exist.
    static int instances;

    // The amount of space remaining to be allocated.
    size_t   remaining;

    // The user-specified limit on memory. 
    size_t   user_limit;
    
    // the amount that has been allocated.
    size_t   used;


public:
    // made public since Linux c++ doesn't like the fact that our new
    // and delete operators don't throw exceptions. [tavi] 
    // flag indicates whether we are keeping track of memory or not
    static MM_mode register_new;

    MM_register();
    ~MM_register(void);

    MM_err register_allocation  (size_t sz);
    MM_err register_deallocation(size_t sz);
#ifdef MM_BACKWARD_COMPATIBLE
// retained for backward compatibility
    MM_err available        (size_t *sz);
    MM_err resize_heap      (size_t sz);
#endif
    MM_err set_memory_limit(size_t sz); // dh.

    void   enforce_memory_limit ();     // dh.
    void   ignore_memory_limit ();      // dh.
    void   warn_memory_limit ();        // dh.
    MM_mode get_limit_mode();

    size_t memory_available ();         // dh.
    size_t memory_used ();              // dh.
    size_t memory_limit ();             // dh.
    int    space_overhead ();           // dh.
        
    friend class mm_register_init;
    //friend void * operator new(size_t);
    //friend void operator delete(void *);
    //friend void operator delete[](void *);
};


// The default amount of memory we will allow to be allocated.
// 40MB
#define MM_DEFAULT_MM_SIZE (40<<20)


// Here is the single memory management object.
extern MM_register MM_manager;


// A class to make sure that MM_manager gets set up properly.  It is
// based on the code in tpie_log.h that does the same thing for logs,
// which is in turn based on item 47 from sdm's book.
class mm_register_init {
private:
    // The number of mm_register_init objects that exist.
    static unsigned int count;

public:
    mm_register_init(void);
    ~mm_register_init(void);
};

static mm_register_init source_file_mm_register_init;

#endif // _MM_REGISTER_H 





