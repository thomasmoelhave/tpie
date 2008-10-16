//
// File:    bte/coll.h
// Authors: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: bte/coll.h,v 1.4 2003-04-29 05:29:42 tavi Exp $
//
// Front end for the BTE collection classes.
//

#ifndef _TPIE_BTE_COLL_H
#define _TPIE_BTE_COLL_H

// Get the base class and various definitions.
#include <bte/coll_base.h>

// The MMAP implementation.
#include <bte/coll_mmap.h>

// The UFS implementation.
#include <bte/coll_ufs.h>

// Get definitions for working with Unix and Windows
#include <portability.h>

namespace tpie {

    namespace bte {
		
#define _BTE_COLL_IMP_COUNT (defined(COLLECTION_IMP_UFS) +		\
                             defined(COLLECTION_IMP_MMAP) +		\
                             defined(COLLECTION_IMP_USER_DEFINED))
	
// Multiple implem. are included, but we have to choose a default one.
#if (_BTE_COLL_IMP_COUNT > 1)
#  define COLLECTION_IMP_MMAP
#elif (_BTE_COLL_IMP_COUNT == 0)
#  define COLLECTION_IMP_MMAP
#endif
	
#define COLLECTION_MMAP collection_mmap<TPIE_BLOCK_ID_TYPE>
#define COLLECTION_UFS  collection_ufs<TPIE_BLOCK_ID_TYPE>
	
#if defined(COLLECTION_IMP_MMAP)
#  define COLLECTION COLLECTION_MMAP
#elif defined(COLLECTION_IMP_UFS)
#  define COLLECTION COLLECTION_UFS
#elif defined(COLLECTION_IMP_USER_DEFINED)
   // Do not define BTE_COLLECTION. The user will define it.
#endif

    }  //  bte namespace

}  //  tpie namespace
	
#endif // _TPIE_BTE_COLL_H
