#ifndef _TPIE_AMI_COLL_H
#define _TPIE_AMI_COLL_H

///////////////////////////////////////////////////////////////////////////
/// \file tpie/coll.h Provides means to choose and set a specific collection type.
/// For now \ref tpie::ami::collection_single is the only implementation.
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <tpie/coll_base.h>
#include <tpie/coll_single.h>

namespace tpie {

    namespace ami {



///////////////////////////////////////////////////////////////////////////
/// As \ref tpie::ami::collection_single is for now the only collection implementation,
/// define collection to point to collection_single.
///////////////////////////////////////////////////////////////////////////
#define AMI_collection collection_single
	
#ifdef BTE_COLLECTION
#  define AMI_COLLECTION collection_single< BTE_COLLECTION >
#endif

    }  //  ami namespace

}  //  tpie namespace

#endif // _TPIE_AMI_COLL_H
