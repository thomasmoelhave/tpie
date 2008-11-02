///////////////////////////////////////////////////////////////////////////
/// \file cache.h 
/// Declaration and definition of CACHE_MANAGER implementation(s).
/// Provides means to choose and set a specific cache manager/
/// For now the only cache memory manager implemented is 
///  \ref cache_manager_lru.
///////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_AMI_CACHE_H
#define _TPIE_AMI_CACHE_H

#include <tpie/cache_base.h>
#include <tpie/cache_lru.h>

namespace tpie {
    
    namespace ami {
	
/** The only cache manager implementation so far is cache_manager_lru. */
#define CACHE_MANAGER cache_manager_lru

    }  //  ami namespace

}  //  tpie namespace


#endif // _TPIE_AMI_CACHE_H
