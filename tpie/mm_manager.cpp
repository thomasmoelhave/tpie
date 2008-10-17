// Copyright (c) 1994 Darren Erik Vengroff
//
// File: mm_manager.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/31/94
//

// A simple registration based memory manager.

//#include <cassert>
#include "lib_config.h"

#define MM_IMP_REGISTER
#include <tpie/mm.h>
#include <tpie/mm_register.h>

#ifdef REPORT_LARGE_MEMOPS
#include <iostream>
#endif

#ifdef MM_BACKWARD_COMPATIBLE
extern int register_new;
#endif

#include <cstdlib>

using namespace tpie::mem;

manager::manager() : 
    remaining (0), user_limit(0), used(0), pause_allocation_depth (0) {
    instances++;

    tp_assert(instances == 1,
              "Only 1 instance of manager_base should exist.");
}
 

manager::~manager(void)
{
    tp_assert(instances == 1,
              "Only 1 instance of manager_base should exist.");

    instances--;
}

// check that new allocation request is below user-defined limit.
// This should be a private method, only called by operator new.

err manager::register_allocation(TPIE_OS_SIZE_T request)
{
  // quick hack to allow operation before limit is set
  // XXX 
    if(!user_limit || pause_allocation_depth) {
	return NO_ERROR;
    }
    
    used      += request;     

    if (request > remaining) {
       TP_LOG_WARNING("Memory allocation request: ");
       TP_LOG_WARNING(static_cast<TPIE_OS_OUTPUT_SIZE_T>(request));
       TP_LOG_WARNING(": User-specified memory limit exceeded.");
       TP_LOG_FLUSH_LOG;
       remaining = 0;
       return INSUFFICIENT_SPACE;
    }

    remaining -= request; 

    TP_LOG_MEM_DEBUG("manager Allocated ");
    TP_LOG_MEM_DEBUG(static_cast<TPIE_OS_OUTPUT_SIZE_T>(request));
    TP_LOG_MEM_DEBUG("; ");
    TP_LOG_MEM_DEBUG(static_cast<TPIE_OS_OUTPUT_SIZE_T>(remaining));
    TP_LOG_MEM_DEBUG(" remaining.\n");
    TP_LOG_FLUSH_LOG;

#ifdef REPORT_LARGE_MEMOPS
	if(request > user_limit/10) {
	  std::cerr << "MEM alloc " << request
		   << " (" << remaining << " remaining)" << endl;
	}
#endif
    
    return NO_ERROR;
}

// do the accounting for a memory deallocation request.
// This should be a private method, only called by operators 
// delete and delete [].

err manager::register_deallocation(TPIE_OS_SIZE_T sz)
{
    remaining += sz;

    if (sz > used) {
       TP_LOG_WARNING("Error in deallocation sz=");
       TP_LOG_WARNING(static_cast<TPIE_OS_LONG>(sz));
       TP_LOG_WARNING(", remaining=");
       TP_LOG_WARNING(static_cast<TPIE_OS_LONG>(remaining));
       TP_LOG_WARNING(", user_limit=");
       TP_LOG_WARNING(static_cast<TPIE_OS_LONG>(user_limit));
       TP_LOG_WARNING("\n");
       TP_LOG_FLUSH_LOG;
       used = 0;
       return UNDERFLOW;
    }

    used      -= sz;    

    TP_LOG_MEM_DEBUG("mm_register De-allocated ");
    TP_LOG_MEM_DEBUG(static_cast<TPIE_OS_LONG>(sz));
    TP_LOG_MEM_DEBUG("; ");
    TP_LOG_MEM_DEBUG(static_cast<TPIE_OS_LONG>(remaining));
    TP_LOG_MEM_DEBUG(" now available.\n");
    TP_LOG_FLUSH_LOG;
    
#ifdef REPORT_LARGE_MEMOPS
	if(sz > user_limit/10) {
	  std::cerr << "MEM free " << sz 
		   << " (" << remaining << " remaining)" << endl;
	}
#endif

    return NO_ERROR;
}

#ifdef MM_BACKWARD_COMPATIBLE
// (Old) way to query how much memory is available

err manager::available (TPIE_OS_SIZE_T *sz)
{
    *sz = remaining;
    return NO_ERROR;    
}

// resize_heap has the same purpose as set_memory_limit.
// It is retained for backward compatibility. 
// dh. 1999 09 29

err manager::resize_heap(TPIE_OS_SIZE_T sz)
{
   return set_memory_limit(sz);
}
#endif


// User-callable method to set allowable memory size

err manager::set_memory_limit (TPIE_OS_SIZE_T new_limit)
{
    // by default, we keep track and abort if memory limit exceeded
    if (register_new == IGNORE_MEMORY_EXCEEDED){
       register_new = ABORT_ON_MEMORY_EXCEEDED;
    }
    // dh. unless the user indicates otherwise
    if (new_limit == 0){
       register_new = IGNORE_MEMORY_EXCEEDED;
       remaining = used = user_limit = 0;
       return NO_ERROR;
    } 

    if (used > new_limit) {
        return EXCESSIVE_ALLOCATION;
    } else {
        // These are unsigned, so be careful.
        if (new_limit < user_limit) {
            remaining -= user_limit - new_limit;
        } else {
            remaining += new_limit - user_limit;
        }
        user_limit = new_limit;
        return NO_ERROR;
    }
}

// dh. only warn if memory limit exceeded
void manager::warn_memory_limit()
{
    register_new = WARN_ON_MEMORY_EXCEEDED;
}

// dh. abort if memory limit exceeded
void manager::enforce_memory_limit()
{
    register_new = ABORT_ON_MEMORY_EXCEEDED;
}

// dh. ignore memory limit accounting
void manager::ignore_memory_limit()
{
    register_new = IGNORE_MEMORY_EXCEEDED;
}

// rw. provide accounting state
mode manager::get_limit_mode() {
  return register_new;
}


// dh. return the amount of memory available before user-specified 
// memory limit exceeded 
TPIE_OS_SIZE_T manager::memory_available()
{
    return remaining;    
}

size_t manager::memory_used()
{
    return used;    
}

size_t manager::memory_limit()
{
    return user_limit;    
}

// Instantiate the actual memory manager, and allocate the 
// its static data members
manager MM_manager;

int manager::instances = 0; // Number of instances. (init)
// TPIE's "register memory requests" flag
mode manager::register_new = ABORT_ON_MEMORY_EXCEEDED; 

// The counter of mm_register_init instances. 
unsigned int manager_init::count = 0;

// The constructor and destructor that ensure that the memory manager is
// created exactly once, and destroyed when appropriate.
manager_init::manager_init(void)
{
    if (count++ == 0) {
	MM_manager.set_memory_limit(MM_DEFAULT_MM_SIZE);
    // Tell STL always to use new/debug for allocation
    TPIE_OS_SET_GLIBCPP_FORCE_NEW;
    }
}

manager_init::~manager_init(void)
{
    --count;
}
