// Copyright (c) 1994 Darren Erik Vengroff
//
// File: mm_register.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/30/94
//
// $Id: mm_register.h,v 1.12 2005-11-15 15:38:10 jan Exp $
//
#ifndef _MM_REGISTER_H
#define _MM_REGISTER_H

/// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
#include <tpie/tpie_log.h>

#define MM_REGISTER_VERSION 2

// To be defined later in this file.
class mm_register_init;



///////////////////////////////////////////////////////////////////////////////
/// \class MM_register 
/// The TPIE memory manager MM_manager, the only instance of class MM_register, 
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

class MM_register {
private:
    /** The number of instances of this class and descendents that exist.*/
    static int instances;

    /** The amount of space remaining to be allocated. */
    TPIE_OS_SIZE_T   remaining;

    /** The user-specified limit on memory. */ 
    TPIE_OS_SIZE_T   user_limit;
    
    /** The amount that has been allocated. */
    TPIE_OS_SIZE_T   used;

    /** The depth of possibly nested "pause"-calls. */
    unsigned long pause_allocation_depth;

public:
  // made public since Linux c++ doesn't like the fact that our new
  // and delete operators don't throw exceptions. [tavi] 
  /** Flag that indicates whether we are keeping track of memory or not.*/
    static MM_mode register_new;

    /** Constructor */
    MM_register();
    /** Destructor */
    ~MM_register(void);

    ///////////////////////////////////////////////////////////////////////////
    /// Checks that new allocation request is below user-defined limit.
    /// This should be a private method, only called by operator new.
    ///////////////////////////////////////////////////////////////////////////
    MM_err register_allocation  (TPIE_OS_SIZE_T sz);

    ///////////////////////////////////////////////////////////////////////////
    /// Does the accounting for a memory deallocation request.
    /// This should be a private method, only called by operators 
    /// delete and delete [].
    ///////////////////////////////////////////////////////////////////////////
    MM_err register_deallocation(TPIE_OS_SIZE_T sz);
#ifdef MM_BACKWARD_COMPATIBLE
// retained for backward compatibility
    MM_err available        (TPIE_OS_SIZE_T *sz);
    MM_err resize_heap      (TPIE_OS_SIZE_T sz);
#endif

    ///////////////////////////////////////////////////////////////////////////
    /// Set the application's memory limit. The memory limit is set to size
    /// bytes. If the specified memory limit is
    /// greater than or equal to the amount of memory already allocated, 
    /// set_memory_limit() returns MM_ERROR_NO_ERROR, otherwise it returns
    /// MM_ERROR_EXCESSIVE_ALLOCATION. By default, successive calls
    /// to operator new() will cause the program to abort if the
    /// resulting memory usage would exceed size bytes. This behavior
    /// can be controlled explicitly by the use of methods
    /// enforce_memory_limit(), warn_memory_limit() and ignore_memory_limit().
    /// \sa \ref alloc_counting "Notes on Allocation Counting"
    /// \param[in] sz The amount of memory in bytes tpie is allowed to use
    /// in the further.
    ///////////////////////////////////////////////////////////////////////////
    MM_err set_memory_limit(TPIE_OS_SIZE_T sz); // dh.

    ///////////////////////////////////////////////////////////////////////////
    /// Instruct TPIE to abort computation when the memory limit is exceeded.
    ///////////////////////////////////////////////////////////////////////////
    void   enforce_memory_limit ();     // dh.

    ///////////////////////////////////////////////////////////////////////////
    /// Instruct TPIE to ignore the memory limit set using set_memory_limit().
    ///////////////////////////////////////////////////////////////////////////
    void   ignore_memory_limit ();      // dh.

    ///////////////////////////////////////////////////////////////////////////
    /// Instruct TPIE to issue a warning when the memory limit is exceeded.
    ///////////////////////////////////////////////////////////////////////////
    void   warn_memory_limit ();        // dh.
    
    ///////////////////////////////////////////////////////////////////////////
    /// Provides the accounting state MM_register#register_new.
    /// \sa \ref alloc_counting "Notes on Allocation Counting"
    ///////////////////////////////////////////////////////////////////////////
    MM_mode get_limit_mode();

    ///////////////////////////////////////////////////////////////////////////
    /// Return the number of bytes of memory which can be allocated before the 
    /// user-specified limit is reached.
    ///////////////////////////////////////////////////////////////////////////
    TPIE_OS_SIZE_T memory_available ();         // dh.
    
    ///////////////////////////////////////////////////////////////////////////
    /// Return the number of bytes of memory currently allocated.
    ///////////////////////////////////////////////////////////////////////////
    TPIE_OS_SIZE_T memory_used ();              // dh.
    
    ///////////////////////////////////////////////////////////////////////////
    /// Return the memory limit as set by the last call to 
    /// method set_memory_limit().
    ///////////////////////////////////////////////////////////////////////////
    TPIE_OS_SIZE_T memory_limit ();             // dh.
    
    ///////////////////////////////////////////////////////////////////////////
    /// Returns the space overhead, that TPIE imposes on each memory
    /// allocation request received by operator
    /// new(). This involves increasing each allocation request by a
    /// fixed number of bytes. The precise size of this increase is machine
    /// dependent, but typically 8 bytes. The method
    /// returns the size of this increase.
    ///////////////////////////////////////////////////////////////////////////
    int    space_overhead ();           // dh.
        
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

    friend class mm_register_init;
    //friend void * operator new(TPIE_OS_SIZE_T);
    //friend void operator delete(void *);
    //friend void operator delete[](void *);
};

inline size_t MM_register::allocation_count_factor() const {
    if (pause_allocation_depth)
	return 0;
    return 1;
}

inline void MM_register::pause_allocation_counting() { 
    if (++pause_allocation_depth == 1) {
        // Tell STL to use realloc for allocation (wherever possible)
        TPIE_OS_UNSET_GLIBCPP_FORCE_NEW;
    };
}

inline void MM_register::resume_allocation_counting() { 
    if (pause_allocation_depth > 0) {
        if (--pause_allocation_depth == 0){
            // Tell STL always to use new/debug for allocation
            TPIE_OS_SET_GLIBCPP_FORCE_NEW;
        };
    }       
    else {  
    	TP_LOG_WARNING("Unmatched MM_register::resume_allocation_counting()");
	    pause_allocation_depth = 0;
    }
}

/** The default amount of memory we will allow to be allocated; set to 40MB. */
#define MM_DEFAULT_MM_SIZE (40<<20)


/** This is the only instance of the MM_register class that should exist in a program. */
extern MM_register MM_manager;

///////////////////////////////////////////////////////////////////////////
/// A class to make sure that MM_manager gets set up properly.  It is
/// based on the code in tpie_log.h that does the same thing for logs,
/// which is in turn based on item 47 from Scott Meyer's book on effective C++.
///////////////////////////////////////////////////////////////////////////
class mm_register_init {
private:
    /** The number of mm_register_init objects that exist. */
    static unsigned int count;

public:
    ///////////////////////////////////////////////////////////////////////////
    // The constructor that ensures that the memory manager is
    // created exactly once.
    ///////////////////////////////////////////////////////////////////////////
    mm_register_init(void);
    ///////////////////////////////////////////////////////////////////////////
    // The constructor that ensures that the memory manager is
    // destroyed when appropriate.
    ///////////////////////////////////////////////////////////////////////////
    ~mm_register_init(void);
};

static mm_register_init source_file_mm_register_init;

#endif // _MM_REGISTER_H 





