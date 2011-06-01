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
#include <tpie/static_string_stream.h>
#include <tpie/tpie_assert.h>
#include <tpie/tpie_log.h>
#include <tpie/util.h>

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
#include <cstring>

using namespace tpie::mem;

manager::manager() : 
    user_limit(0), used(0), global_overhead (0), pause_allocation_depth (0) {
    instances++;
    tp_assert(instances == 1,
              "Only 1 instance of manager_base should exist.");
#ifdef TPIE_THREADSAFE_MEMORY_MANAGEMNT
	mm_mutex = 0; //This is very important
	mm_mutex = new boost::recursive_mutex(); //This will call register_allocation on a uninitialized this
#endif
}

manager::~manager(void)
{
    tp_assert(instances == 1,
              "Only 1 instance of manager_base should exist.");

    instances--;
#ifdef TPIE_THREADSAFE_MEMORY_MANAGEMNT
	mm_mutex = 0;
#endif
}

// check that new allocation request is below user-defined limit.
// This should be a private method, only called by operator new.
err manager::register_allocation(TPIE_OS_SIZE_T request)
{
#ifdef TPIE_THREADSAFE_MEMORY_MANAGEMNT
	if (!mm_mutex) return NO_ERROR;
	boost::recursive_mutex::scoped_lock lock(*mm_mutex);
#endif
	// quick hack to allow operation before limit is set
	// XXX 	
	if(!user_limit || pause_allocation_depth) {
		return NO_ERROR;
	}

	if (request > remaining()) {
		used += request;
#ifdef TPIE_THREADSAFE_MEMORY_MANAGEMNT
		lock.unlock();
#endif
		return INSUFFICIENT_SPACE;
	}
	used += request;
    return NO_ERROR;
}

// do the accounting for a memory deallocation request.
// This should be a private method, only called by operators 
// delete and delete [].

err manager::register_deallocation(TPIE_OS_SIZE_T sz)
{
#ifdef TPIE_THREADSAFE_MEMORY_MANAGEMNT
	if (!mm_mutex) return NO_ERROR;
	boost::recursive_mutex::scoped_lock lock(*mm_mutex);
#endif

	if(!user_limit || pause_allocation_depth)
		return NO_ERROR;

	if (sz > used) {
#ifdef TPIE_THREADSAFE_MEMORY_MANAGEMNT
		lock.unlock();
#endif
		log_error() << "Error in deallocation sz=" << static_cast<TPIE_OS_LONG>(sz)
					<< ", remaining=" << static_cast<TPIE_OS_LONG>(remaining()) 
					<< ", user_limit=" << static_cast<TPIE_OS_LONG>(user_limit) << std::endl;
		used = 0;
		return EXCESSIVE_DEALLOCATION;
	}
	used -= sz;    
	
	// TP_LOG_MEM_DEBUG("mm_register De-allocated ");
	// TP_LOG_MEM_DEBUG(static_cast<TPIE_OS_LONG>(sz));
	// TP_LOG_MEM_DEBUG("; ");
	// TP_LOG_MEM_DEBUG(static_cast<TPIE_OS_LONG>(remaining));
	// TP_LOG_MEM_DEBUG(" now available.\n");
	// TP_LOG_FLUSH_LOG;
    
#ifdef REPORT_LARGE_MEMOPS
	if(sz > user_limit/10) {
	  std::cerr << "MEM free " << sz 
				<< " (" << remaining() << " remaining)" << endl;
	}
#endif

	return NO_ERROR;
}

#ifdef MM_BACKWARD_COMPATIBLE
// (Old) way to query how much memory is available

err manager::available (TPIE_OS_SIZE_T *sz) {
    *sz = memory_available();
    return NO_ERROR;    
}

// resize_heap has the same purpose as set_memory_limit.
// It is retained for backward compatibility. 
// dh. 1999 09 29

err manager::resize_heap(TPIE_OS_SIZE_T sz) {
   return set_memory_limit(sz);
}
#endif

TPIE_OS_SIZE_T manager::get_memory_limit() {
	return user_limit;
}

// User-callable method to set allowable memory size

err manager::set_memory_limit (TPIE_OS_SIZE_T new_limit)
{
#ifdef TPIE_THREADSAFE_MEMORY_MANAGEMNT
	// memory manager should be initialized before calling set_memory_limit (as opposed to (de)allocate)
	if (!mm_mutex) return MUTEX_FAILURE;
	boost::recursive_mutex::scoped_lock lock(*mm_mutex);
#endif
    // by default, we keep track and abort if memory limit exceeded
    if (register_new == IGNORE_MEMORY_EXCEEDED)
		register_new = ABORT_ON_MEMORY_EXCEEDED;

    // dh. unless the user indicates otherwise
    if (new_limit == 0){
		register_new = IGNORE_MEMORY_EXCEEDED;
		global_overhead = used = user_limit = 0;
		return NO_ERROR;
    } 

    if (used > new_limit) return EXCESSIVE_ALLOCATION;
	user_limit = new_limit;
	return NO_ERROR;
    
}

// Add to the global overhead
void manager::add_to_global_overhead (TPIE_OS_SIZE_T sz) {
#ifdef TPIE_THREADSAFE_MEMORY_MANAGEMNT
	if (!mm_mutex) return;
	boost::recursive_mutex::scoped_lock lock(*mm_mutex);
#endif
	global_overhead += sz;
}

// Subtract from the global overhead
void manager::subtract_from_global_overhead (TPIE_OS_SIZE_T sz) {
#ifdef TPIE_THREADSAFE_MEMORY_MANAGEMNT
	if (!mm_mutex) return;
	boost::recursive_mutex::scoped_lock lock(*mm_mutex);
#endif
	if(global_overhead < sz)
		global_overhead = 0;
	else
		global_overhead -= sz;
}

TPIE_OS_SIZE_T manager::get_global_overhead () const{
	return global_overhead;
}

// dh. only warn if memory limit exceeded
void manager::warn_memory_limit() {
#ifdef TPIE_THREADSAFE_MEMORY_MANAGEMNT
	if (!mm_mutex) return;
	boost::recursive_mutex::scoped_lock lock(*mm_mutex);
#endif
    register_new = WARN_ON_MEMORY_EXCEEDED;
}

// dh. abort if memory limit exceeded
void manager::enforce_memory_limit() {
#ifdef TPIE_THREADSAFE_MEMORY_MANAGEMNT
	if (!mm_mutex) return;
	boost::recursive_mutex::scoped_lock lock(*mm_mutex);
#endif
    register_new = ABORT_ON_MEMORY_EXCEEDED;
}

// dh. ignore memory limit accounting
void manager::ignore_memory_limit() {
#ifdef TPIE_THREADSAFE_MEMORY_MANAGEMNT
	if (!mm_mutex) return;
	boost::recursive_mutex::scoped_lock lock(*mm_mutex);
#endif
    register_new = IGNORE_MEMORY_EXCEEDED;
}

// rw. provide accounting state
mode manager::get_limit_mode() {
  return register_new;
}


TPIE_OS_SIZE_T manager::remaining() {
	if(used > user_limit) return 0;
	return user_limit - used;
}

// dh. return the amount of memory available before user-specified 
// memory limit exceeded 
TPIE_OS_SIZE_T manager::memory_available() {
	if(remaining() < global_overhead)
		return 0;
	return remaining() - global_overhead;
}

TPIE_OS_SIZE_T manager::memory_used() {
    return used;    
}

TPIE_OS_SIZE_T manager::memory_limit() {
    return user_limit;    
}

struct log_flusher {
	tpie::static_string_stream buf;
	~log_flusher() {
		if(strlen(buf.c_str())) {
			tpie::log_debug() << buf.c_str();
			tpie::log_debug().flush();
		}
	}
};

TPIE_OS_SIZE_T manager::consecutive_memory_available(TPIE_OS_SIZE_T lower_bound, TPIE_OS_SIZE_T granularity) {
#ifndef TPIE_USE_EXCEPTIONS
	unused(lower_bound);
	unused(granularity);
	TP_LOG_DEBUG_ID("consecutive_memory_available only works with exceptions\n");
	return memory_available();
#else
	log_flusher lf;
	scoped_log_enabler le(false);
#ifdef TPIE_THREADSAFE_MEMORY_MANAGEMNT
	boost::recursive_mutex::scoped_lock lock(*mm_mutex);
#endif // TPIE_THREADSAFE_MEMORY_MANAGEMNT
	tpie::scoped_change<mode> c(register_new, ABORT_ON_MEMORY_EXCEEDED);

	//lower bound of search
	TPIE_OS_SIZE_T low = lower_bound;

	//don't try to get more than the amount of bytes currently
	//available
	TPIE_OS_SIZE_T high = remaining();
	if(high > static_cast<TPIE_OS_SIZE_T>(space_overhead()))
		high -= static_cast<TPIE_OS_SIZE_T>(space_overhead());
	else
		high = 0;

	if (high< low) {
		low=high;
	}

	//first check quickly if we can get "high" bytes of memory
	//directly.
	try {
		char* mem = new char[high];
		delete[] mem;
		return (high > global_overhead)?high-global_overhead:0;
	} catch (std::bad_alloc) {
		lf.buf << "Failed to get " << high/(1024*1024) << " megabytes of memory. "
			   << "Performing binary search to find largest amount "
			   << "of memory available. This might take a few moments.\n";
	}

	//we should be able to get at least lower_limit bytes
	try {
		char* mem = new char[low];
		delete[] mem;
	} catch (std::bad_alloc) {
		lf.buf << "Failed to get lower limit" << low/(1024*1024) << " megabytes of memory.\n";
		return 0;
	}

	//perform a binary search in [low,high] for highest possible 
	//memory allocation within a granularity given by 
	//the "granularity" variable.
	do {
		//middle of search interval, beware of overflows
		size_t mid = size_t((static_cast<TPIE_OS_OFFSET>(low)+high)/2);

		lf.buf << "Search area is  [" << low << "," << high << "]"
			   << " query amount is: " << mid << ":\n";
		if (mid < low || mid > high) {
			throw std::logic_error(
				"Memory interval calculation failed. Try setting the "
				" memory value to something smaller.");
		}
		
		//try to allocate "mid" bytes of memory
		//TPIE throws an exception if memory allocatio fails
		try {
			char* mem = new char[mid];
			low = mid;
			delete[] mem;
		} catch (std::bad_alloc) {
			high = mid;
			lf.buf << "failed.\n";
		}
	} while (high - low > granularity);
	
	lf.buf << "- - - - - - - END MEMORY SEARCH - - - - - -\n";
	return (high > global_overhead)?high-global_overhead:0;
#endif

}
int manager::instances = 0; // Number of instances. (init)
// TPIE's "register memory requests" flag
mode manager::register_new = ABORT_ON_MEMORY_EXCEEDED; 

// Instantiate the actual memory manager, and allocate the 
// its static data members
manager tpie::MM_manager;

void tpie::init_memory_manager() {
	MM_manager.set_memory_limit(MM_DEFAULT_MM_SIZE);
    // Tell STL always to use new/debug for allocation
    TPIE_OS_SET_GLIBCPP_FORCE_NEW;
}

void tpie::finish_memory_manager() {
}
	
