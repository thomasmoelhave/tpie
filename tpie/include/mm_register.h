// Copyright (c) 1994 Darren Erik Vengroff
//
// File: mm_register.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/30/94
//
// $Id: mm_register.h,v 1.1 1994-06-03 13:28:26 dev Exp $
//
#ifndef _MM_REGISTER_H
#define _MM_REGISTER_H

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
    size_t remaining;

    // The total amount that can ever be allocated.
    size_t max_sz;
    
public:
    MM_register();
    ~MM_register(void);

    MM_err register_allocation(size_t sz);
    MM_err register_deallocation(size_t sz);
    MM_err available(size_t *sz);

    friend class mm_register_init;
};


// The default amount of memory we will allow to be allocated.

#define MM_DEFAULT_MM_SIZE 4194304   // 4 Meg.

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





