// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

// A simple registration based memory manager.

//#include <cassert>
#include <tpie/config.h>
#include <tpie/types.h>

#include <tpie/tpie_assert.h>
#include <tpie/tpie_log.h>

#define MM_IMP_REGISTER
#include <tpie/mm.h>
#include <tpie/mm_manager.h>

#ifdef REPORT_LARGE_MEMOPS
#include <iostream>
#endif

#ifdef MM_BACKWARD_COMPATIBLE
extern int register_new;
#endif

#include <cstdlib>

using namespace tpie::mem;
using namespace tpie;

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

err manager::register_allocation(memory_size_type request)
{
  // quick hack to allow operation before limit is set
  // XXX 
    if(!user_limit || pause_allocation_depth) {
	return NO_ERROR;
    }
    
    used      += request;     

    if (request > remaining) {
       TP_LOG_WARNING("Memory allocation request: ");
       TP_LOG_WARNING(static_cast<stream_offset_type>(request));
       TP_LOG_WARNING(": User-specified memory limit exceeded.");
       TP_LOG_FLUSH_LOG;
       remaining = 0;
       return INSUFFICIENT_SPACE;
    }

    remaining -= request; 

    TP_LOG_MEM_DEBUG("manager Allocated ");
    TP_LOG_MEM_DEBUG(static_cast<stream_offset_type>(request));
    TP_LOG_MEM_DEBUG("; ");
    TP_LOG_MEM_DEBUG(static_cast<stream_offset_type>(remaining));
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

err manager::register_deallocation(memory_size_type sz)
{
    remaining += sz;

    if (sz > used) {
       TP_LOG_WARNING("Error in deallocation sz=");
       TP_LOG_WARNING(sz);
       TP_LOG_WARNING(", remaining=");
       TP_LOG_WARNING(remaining);
       TP_LOG_WARNING(", user_limit=");
       TP_LOG_WARNING(user_limit);
       TP_LOG_WARNING("\n");
       TP_LOG_FLUSH_LOG;
       used = 0;
       return EXCESSIVE_DEALLOCATION;
    }

    used      -= sz;    

    TP_LOG_MEM_DEBUG("mm_register De-allocated ");
    TP_LOG_MEM_DEBUG(sz);
    TP_LOG_MEM_DEBUG("; ");
    TP_LOG_MEM_DEBUG(remaining);
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

err manager::available (memory_size_type *sz) {
    *sz = remaining;
    return NO_ERROR;    
}

// resize_heap has the same purpose as set_memory_limit.
// It is retained for backward compatibility. 
// dh. 1999 09 29

err manager::resize_heap(memory_size_type sz) {
   return set_memory_limit(sz);
}
#endif


// User-callable method to set allowable memory size

err manager::set_memory_limit (memory_size_type new_limit)
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
void manager::warn_memory_limit() {
    register_new = WARN_ON_MEMORY_EXCEEDED;
}

// dh. abort if memory limit exceeded
void manager::enforce_memory_limit() {
    register_new = ABORT_ON_MEMORY_EXCEEDED;
}

// dh. ignore memory limit accounting
void manager::ignore_memory_limit() {
    register_new = IGNORE_MEMORY_EXCEEDED;
}

// rw. provide accounting state
mode manager::get_limit_mode() {
  return register_new;
}


// dh. return the amount of memory available before user-specified 
// memory limit exceeded 
memory_size_type manager::memory_available() {
    return remaining;    
}

memory_size_type manager::memory_used() {
    return used;    
}

memory_size_type manager::memory_limit() {
    return user_limit;    
}

memory_size_type manager::consecutive_memory_available(memory_size_type lower_bound, memory_size_type granularity) {
#ifndef TPIE_USE_EXCEPTIONS
	memory_size_type _prevent_compiler_warning = lower_bound + granularity;
	_prevent_compiler_warning++;
	TP_LOG_DEBUG_ID("consecutive_memory_available only works with exceptions\n");
	return memory_available();
#else
	//lower bound of search
	memory_size_type low = lower_bound;

	//don't try to get more than the amount of bytes currently
	//available
	memory_size_type high = memory_available()-space_overhead();

	TP_LOG_DEBUG_ID("\n- - - - - - - MEMORY SEARCH - - - - - -\n");

	if (high< low) {
		low=high;
	}

	//first check quickly if we can get "high" bytes of memory
	//directly.
	try {
		char* mem = new char[high];
		delete[] mem;
		TP_LOG_DEBUG_ID("Successfully allocated " << high << " bytes.\n");
		return high;
	} catch (...) {
		TP_LOG_DEBUG_ID("Failed to get " << high/(1024*1024) << " megabytes of memory. "
			<< "Performing binary search to find largest amount "
			<< "of memory available. This might take a few moments.\n");
	}

	//we should be able to get at least lower_limit bytes
	try {
		char* mem = new char[low];
		delete[] mem;
	} catch (...) {
		TP_LOG_DEBUG_ID("Failed to get lower limit" << low/(1024*1024) << " megabytes of memory. Aborting\n. ");
		return 0;
	}

	//perform a binary search in [low,high] for highest possible 
	//memory allocation within a granularity given by 
	//the "granularity" variable.
	do {
		//middle of search interval, beware of overflows
		size_t mid = size_t((static_cast<stream_offset_type>(low)+high)/2);

		TP_LOG_DEBUG_ID("Search area is  [" << low << "," << high << "]"
			<< " query amount is: " << mid << ":\n");

		if (mid < low || mid > high) {
			throw std::logic_error(
				"Memory interval calculation failed. Try setting the "
				" memory value to something smaller.");
		}

		//try to allocate "mid" bytes of memory
		//TPIE throws an exception if memory allocation fails
		try {
			char* mem = new char[mid];
			low = mid;
			delete[] mem;
		} catch (...) {
			high = mid;
			TP_LOG_DEBUG_ID("failed.\n");
		}
	} while (high - low > granularity);

	TP_LOG_DEBUG_ID("\n- - - - - - - END MEMORY SEARCH - - - - - -\n\n");

	return low;
#endif

}


// Instantiate the actual memory manager, and allocate the 
// its static data members
manager tpie::MM_manager;

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
