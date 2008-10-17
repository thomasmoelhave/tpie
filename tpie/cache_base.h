//
// Copyright (C) 2001 Octavian Procopiuc
//
// Declaration and definition of AMI_CACHE_MANAGER
//

#ifndef _TPIE_AMI_CACHE_BASE_H
#define _TPIE_AMI_CACHE_BASE_H

#include <tpie/portability.h>

namespace tpie {
    
    namespace ami {

    ////////////////////////////////////////////////////////////////////
    /// Base class for all cache manager implementations.
    /// For now the only cache memory manager implemented is 
    ///  \ref cache_manager_lru.
    ////////////////////////////////////////////////////////////////////
    class cache_manager_base {

	protected:
	    /** Max size in bytes. */
	    TPIE_OS_SIZE_T capacity_;
	    
	    /** Associativity */.
	    TPIE_OS_SIZE_T assoc_;
	    
	    /** Behavior. */
	    int behavior_;
	    
	    ////////////////////////////////////////////////////////////////////
	    ///  Construct a fully-associative cache manager with the given capacity.
	    ////////////////////////////////////////////////////////////////////
	    cache_manager_base(TPIE_OS_SIZE_T capacity, 
			       TPIE_OS_SIZE_T assoc):
		capacity_(capacity), 
		assoc_(assoc), 
		behavior_(0) {
		//  No code in this constructor.
	    }
	    
	public:
	    /** Set behavior. TODO: Expand. */
	    int behavior(int b) { 
		behavior_ = b; 
		return behavior_; 
	    }
	    
	    /**  Inquire behavior. */
	    int behavior() const { 
		return behavior_; 
	    }
	};

    }  //  ami namespace

} //  tpie namespace


#endif // _TPIE_AMI_CACHE_BASE_H
