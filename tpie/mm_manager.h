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

#ifndef _MM_MANAGER_H
#define _MM_MANAGER_H

#include <tpie/config.h>

/// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
#include <tpie/tpie_log.h>

#define MM_MANAGER_VERSION 2

namespace tpie {

    namespace mem {

// To be defined later in this file.
	class manager_init;
	


///////////////////////////////////////////////////////////////////////////////
/// \class manager 
/// The TPIE memory manager \ref MM_manager, the only instance of class xmanager, 
/// traps memory allocation and deallocation requests in order to monitor and 
/// enforce memory usage limits.
/// The actual memory allocation requests are done using the standard C++ 
/// operators new() and delete(), which have been replaced with in-house
/// versions that interact with the memory manager. 
///
/// Note that there is no need to include this fille when using the AMI entry
/// points, since it is included by all AMI header files by inclusion of 
/// mm.h.
///
/// \anchor alloc_counting
/// \par Notes on Allocation Counting  
/// When using certain implementations of
/// STL, some dynamic data structures such as stacks or vectors change
/// the size of their scratch space by invoking the system call
/// realloc(). Such calls will invalidate the memory manager's
/// information about how much space is allocated, and eventually will
/// lead the memory manager to loose track of the available space. The
/// suggested solution is to instruct STL not to use realloc but
/// corresponding delete()/new() -calls, and this
/// behavior is implemented by TPIE.
/// However, the performance-oriented programmer may not want to sacrifice
/// potentially fast reallocation, and thus TPIE offers the possibility to
/// switch off and on allocation counting. If allocation counting is
/// switched off, reallocation is re-enabled in STL (if STL's
/// implementation supports this), but TPIE cannot guarantee that the
/// memory limit is respected. Thus, it is the programmer's responsibility
/// to keep track of how much memory is allocated while allocation
/// counting is switched off. Being in ``pause''-mode does not affect
/// correct deallocation of objects that have been allocation with
/// allocation counting switched on (and vice versa).
///////////////////////////////////////////////////////////////////////////////

	class manager {

	private:
	    /** The number of instances of this class and descendents that exist.*/
	    static int instances;
	    
	    /** The amount of space remaining to be allocated. */
	    memory_size_type   remaining;
	    
	    /** The user-specified limit on memory. */ 
	    memory_size_type   user_limit;
	    
	    /** The amount that has been allocated. */
	    memory_size_type   used;
	    
	    /** The depth of possibly nested "pause"-calls. */
	    unsigned long pause_allocation_depth;
	    
	public:
	    // made public since Linux c++ doesn't like the fact that our new
	    // and delete operators don't throw exceptions. [tavi] 
	    /** Flag that indicates whether we are keeping track of memory or not.*/

	    static mode register_new;
	    
	    /** Constructor */
	    manager();
	    /** Destructor */
	    ~manager(void);
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Checks that new allocation request is below user-defined limit.
	    /// This should be a private method, only called by operator new.
	    ///////////////////////////////////////////////////////////////////////////
	    err register_allocation  (memory_size_type sz);
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Does the accounting for a memory deallocation request.
	    /// This should be a private method, only called by operators 
	    /// delete and delete [].
	    ///////////////////////////////////////////////////////////////////////////
	    err register_deallocation(memory_size_type sz);
#ifdef MM_BACKWARD_COMPATIBLE
// retained for backward compatibility
	    err available        (memory_size_type *sz);
	    err resize_heap      (memory_size_type sz);
#endif

	    ///////////////////////////////////////////////////////////////////////////
	    /// Set the application's memory limit. The memory limit is set to size
	    /// bytes. If the specified memory limit is
	    /// greater than or equal to the amount of memory already allocated, 
	    /// set_memory_limit() returns NO_ERROR, otherwise it returns
	    /// EXCESSIVE_ALLOCATION. By default, successive calls
	    /// to operator new() will cause the program to abort if the
	    /// resulting memory usage would exceed size bytes. This behavior
	    /// can be controlled explicitly by the use of methods
	    /// enforce_memory_limit(), warn_memory_limit() and ignore_memory_limit().
	    /// \sa \ref alloc_counting "Notes on Allocation Counting"
	    /// \param[in] sz The amount of memory in bytes tpie is allowed to use
	    /// in the further.
	    ///////////////////////////////////////////////////////////////////////////
	    err set_memory_limit(memory_size_type sz); 
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Instruct TPIE to abort computation when the memory limit is exceeded.
	    ///////////////////////////////////////////////////////////////////////////
	    void   enforce_memory_limit ();     
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Instruct TPIE to ignore the memory limit set using set_memory_limit().
	    ///////////////////////////////////////////////////////////////////////////
	    void   ignore_memory_limit (); 
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Instruct TPIE to issue a warning when the memory limit is exceeded.
	    ///////////////////////////////////////////////////////////////////////////
	    void   warn_memory_limit ();   
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Provides the accounting state manager#register_new.
	    /// \sa \ref alloc_counting "Notes on Allocation Counting"
	    ///////////////////////////////////////////////////////////////////////////
	    mode get_limit_mode();
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Return the number of bytes of memory which can be allocated before the 
	    /// user-specified limit is reached.
	    ///////////////////////////////////////////////////////////////////////////
	    memory_size_type memory_available ();
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Return the number of bytes of memory that can safely be allocated as one
	    /// consecutive array. This is never bigger than the value returned by
	    /// method memory_available(). The time complexity of this method is
	    /// log_2(2^32/granularity).
	    /// \param[in] lower_bound the lower bound on the amount of memory to get
	    /// it TPIE cannot allocate this much it returns 0;
	    /// \param[in] granularity the "resolution" of the search. If the actual
	    /// maximum consecutive memory size is X this method will return Y where
	    /// Y is smaller by X by at most \ref granularity bytes.
	    ///////////////////////////////////////////////////////////////////////////
	    memory_size_type consecutive_memory_available (
	                memory_size_type lower_bound=0,
	                memory_size_type granularity=5*1024*1024
	                );
    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Return the number of bytes of memory currently allocated.
	    ///////////////////////////////////////////////////////////////////////////
	    memory_size_type memory_used ();             
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Return the memory limit as set by the last call to 
	    /// method set_memory_limit().
	    ///////////////////////////////////////////////////////////////////////////
	    memory_size_type memory_limit ();         
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Returns the space overhead, that TPIE imposes on each memory
	    /// allocation request received by operator
	    /// new(). This involves increasing each allocation request by a
	    /// fixed number of bytes. The precise size of this increase is machine
	    /// dependent, but typically 8 bytes. The method
	    /// returns the size of this increase.
	    ///////////////////////////////////////////////////////////////////////////
	    int    space_overhead ();          
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Returns 1 iff allocation is switched on. In all other cases, a value of
	    /// zero is returned.
	    /// \sa \ref alloc_counting "Notes on Allocation Counting"
	    ///////////////////////////////////////////////////////////////////////////
	    size_t allocation_count_factor() const; 
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Instruct the memory manager not to keep track of how much memory is 
	    /// allocated. See below for a more detailled discussion of situtations in 
	    /// which this feature may come in handy.
	    /// \sa \ref alloc_counting "Notes on Allocation Counting"
	    ///////////////////////////////////////////////////////////////////////////
	    void pause_allocation_counting(); 
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Instruct the memory manager to keep track of how much memory is allocated.
	    /// This behavior is the default behavior. See below for a more detailled
	    /// discussion of situtations in which this feature may come in
	    /// handy. Note, that the The pause_allocation_counting() and
	    /// resume resume_allocation_counting calls may be nested.    
	    /// \sa \ref alloc_counting "Notes on Allocation Counting"
	    ///////////////////////////////////////////////////////////////////////////
	    void resume_allocation_counting(); 
	    
	    friend class manager_init;
	};
	
	inline size_t manager::allocation_count_factor() const {
	    if (pause_allocation_depth)
		return 0;
	    return 1;
	}
	
	inline void manager::pause_allocation_counting() { 
	    if (++pause_allocation_depth == 1) {
		// Tell STL to use realloc for allocation (wherever possible)
		TPIE_OS_UNSET_GLIBCPP_FORCE_NEW;
	    };
}

	inline void manager::resume_allocation_counting() { 
	    if (pause_allocation_depth > 0) {
		if (--pause_allocation_depth == 0){
		    // Tell STL always to use new/debug for allocation
		    TPIE_OS_SET_GLIBCPP_FORCE_NEW;
		};
	    }       
	    else {  
		TP_LOG_WARNING("Unmatched MM_manager::resume_allocation_counting()");
		pause_allocation_depth = 0;
	    }
}

/** The default amount of memory we will allow to be allocated; set to 40MB. */
#define MM_DEFAULT_MM_SIZE (40<<20)
	

/** This is the only instance of the MM_manager class that should exist in a program. */
    }  //  mem namespace 

    extern mem::manager MM_manager;

}  //  tpie namespace


namespace tpie {
    
    namespace mem {
	
        ///////////////////////////////////////////////////////////////////////////
        /// A class to make sure that MM_manager gets set up properly.  It is
        /// based on the code in tpie_log.h that does the same thing for logs,
        /// which is in turn based on item 47 from Scott Meyer's book on effective C++.
        ///////////////////////////////////////////////////////////////////////////
	class manager_init {
	private:
	    /** The number of manager_init objects that exist. */
	    static unsigned int count;
	    
	public:
	    ///////////////////////////////////////////////////////////////////////////
	    // The constructor that ensures that the memory manager is
	    // created exactly once.
	    ///////////////////////////////////////////////////////////////////////////
	    manager_init(void);
	    ///////////////////////////////////////////////////////////////////////////
	    // The constructor that ensures that the memory manager is
	    // destroyed when appropriate.
	    ///////////////////////////////////////////////////////////////////////////
	    ~manager_init(void);
	};
	
	static manager_init source_file_manager_init;

    }  //  mem namespace

}  //  tpie namespace

#endif // _TPIE_MEM_MM_MANAGER_ 





