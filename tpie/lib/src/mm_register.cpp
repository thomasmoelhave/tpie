// Copyright (c) 1994 Darren Erik Vengroff
//
// File: mm_register.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/31/94
//

// A simple registration based memory manager.

#include <versions.h>
VERSION(mm_register_cpp,"$Id: mm_register.cpp,v 1.15 2000-04-22 04:03:46 rajiv Exp $");

#include <assert.h>
#include "lib_config.h"

#define MM_IMP_REGISTER
#include <mm.h>
#include <mm_register.h>

#ifdef REPORT_LARGE_MEMOPS
#include <iostream>
#endif

#ifdef MM_BACKWARD_COMPATIBLE
extern int register_new;
#endif

MM_register::MM_register()
{
    instances++;

    tp_assert(instances == 1,
              "Only 1 instance of MM_register_base should exist.");
}
 

MM_register::~MM_register(void)
{
    tp_assert(instances == 1,
              "Only 1 instance of MM_register_base should exist.");

    instances--;
}

// check that new allocation request is below user-defined limit.
// This should be a private method, only called by operator new.

MM_err MM_register::register_allocation(size_t request)
{
  // quick hack to allow operation before limit is set
  // XXX 
  if(!user_limit) {
	return MM_ERROR_NO_ERROR;
  }

    used      += request;     

    if (request > remaining) {
       LOG_WARNING("Memory allocation request: ");
       LOG_WARNING(request);
       LOG_WARNING(": User-specified memory limit exceeded.");
       LOG_FLUSH_LOG;
       remaining = 0;
       return MM_ERROR_INSUFFICIENT_SPACE;
    }

    remaining -= request;

    LOG_DEBUG_INFO("mm_register Allocated ");
    LOG_DEBUG_INFO((unsigned int)request);
    LOG_DEBUG_INFO("; ");
    LOG_DEBUG_INFO((unsigned int)remaining);
    LOG_DEBUG_INFO(" remaining.\n");
    LOG_FLUSH_LOG;

#ifdef REPORT_LARGE_MEMOPS
	if(request > user_limit/10) {
	  cerr << "MEM alloc " << request
		   << " (" << remaining << " remaining)" << endl;
	}
#endif
    
    return MM_ERROR_NO_ERROR;
}

// do the accounting for a memory deallocation request.
// This should be a private method, only called by operators 
// delete and delete [].

MM_err MM_register::register_deallocation(size_t sz)
{
    remaining += sz;

    if (sz > used) {
       LOG_WARNING("Error in deallocation sz=");
       LOG_WARNING(sz);
       LOG_WARNING(", remaining=");
       LOG_WARNING(remaining);
       LOG_WARNING(", user_limit=");
       LOG_WARNING(user_limit);
       LOG_WARNING("\n");
       LOG_FLUSH_LOG;
       used = 0;
       return MM_ERROR_UNDERFLOW;
    }

    used      -= sz;    

    LOG_DEBUG_INFO("mm_register De-allocated ");
    LOG_DEBUG_INFO((unsigned int)sz);
    LOG_DEBUG_INFO("; ");
    LOG_DEBUG_INFO((unsigned int)remaining);
    LOG_DEBUG_INFO(" now available.\n");
    LOG_FLUSH_LOG;
    
#ifdef REPORT_LARGE_MEMOPS
	if(sz > user_limit/10) {
	  cerr << "MEM free " << sz 
		   << " (" << remaining << " remaining)" << endl;
	}
#endif

    return MM_ERROR_NO_ERROR;
}

#ifdef MM_BACKWARD_COMPATIBLE
// (Old) way to query how much memory is available

MM_err MM_register::available (size_t *sz)
{
    *sz = remaining;
    return MM_ERROR_NO_ERROR;    
}

// resize_heap has the same purpose as set_memory_limit.
// It is retained for backward compatibility. 
// dh. 1999 09 29

MM_err MM_register::resize_heap(size_t sz)
{
   return set_memory_limit(sz);
}
#endif


// User-callable method to set allowable memory size

MM_err MM_register::set_memory_limit (size_t new_limit)
{
    // by default, we keep track and abort if memory limit exceeded
    if (register_new == MM_IGNORE_MEMORY_EXCEEDED){
       register_new = MM_ABORT_ON_MEMORY_EXCEEDED;
    }
    // dh. unless the user indicates otherwise
    if (new_limit == 0){
       register_new = MM_IGNORE_MEMORY_EXCEEDED;
       remaining = used = user_limit = 0;
       return MM_ERROR_NO_ERROR;
    } 

    if (used > new_limit) {
        return MM_ERROR_EXCESSIVE_ALLOCATION;
    } else {
        // These are unsigned, so be careful.
        if (new_limit < user_limit) {
            remaining -= user_limit - new_limit;
        } else {
            remaining += new_limit - user_limit;
        }
        user_limit = new_limit;
        return MM_ERROR_NO_ERROR;
    }
}

// dh. only warn if memory limit exceeded
void MM_register::warn_memory_limit()
{
    register_new = MM_WARN_ON_MEMORY_EXCEEDED;
}

// dh. abort if memory limit exceeded
void MM_register::enforce_memory_limit()
{
    register_new = MM_ABORT_ON_MEMORY_EXCEEDED;
}

// dh. ignore memory limit accounting
void MM_register::ignore_memory_limit()
{
    register_new = MM_IGNORE_MEMORY_EXCEEDED;
}

// rw. provide accounting state
MM_mode MM_register::get_limit_mode() {
  return register_new;
}


// dh. return the amount of memory available before user-specified 
// memory limit exceeded 
size_t MM_register::memory_available()
{
    return remaining;    
}

size_t MM_register::memory_used()
{
    return used;    
}

size_t MM_register::memory_limit()
{
    return user_limit;    
}

// Instantiate the actual memory manager, and allocate the 
// its static data members
MM_register MM_manager;
int MM_register::instances = 0; // Number of instances. (init)
// TPIE's "register memory requests" flag
MM_mode MM_register::register_new = MM_ABORT_ON_MEMORY_EXCEEDED; 


// The counter of mm_register_init instances.  It is implicity set to 0.
unsigned int mm_register_init::count;

// The constructor and destructor that ensure that the memory manager is
// created exactly once, and destroyed when appropriate.
mm_register_init::mm_register_init(void)
{
    if (count++ == 0) {
        MM_manager.remaining = MM_manager.user_limit = MM_DEFAULT_MM_SIZE;
    }
}

mm_register_init::~mm_register_init(void)
{
    --count;
}
