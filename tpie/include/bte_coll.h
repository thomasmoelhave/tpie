//
// File:    bte_coll.h
// Authors: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: bte_coll.h,v 1.3 2003-04-17 14:47:48 jan Exp $
//
// Front end for the BTE collection classes.
//

#ifndef _BTE_COLL_H
#define _BTE_COLL_H

// Get the base class and various definitions.
#include <bte_coll_base.h>

// The MMAP implementation.
#include <bte_coll_mmap.h>

// The UFS implementation.
#include <bte_coll_ufs.h>

// Get definitions for working with Unix and Windows
#include <portability.h>


#if defined(BTE_COLLECTION_IMP_MMB)
	TPIE_OS_UNIX_ONLY_WARNING_BTE_COLLECTION_IMP_MMB_UNIX_ONLY
#  define BTE_COLLECTION_IMP_MMAP
#endif

#define _BTE_COLL_IMP_COUNT (defined(BTE_COLLECTION_IMP_UFS) + \
                             defined(BTE_COLLECTION_IMP_MMAP) + \
                             defined(BTE_COLLECTION_IMP_USER_DEFINED))

// Multiple implem. are included, but we have to choose a default one.
#if (_BTE_COLL_IMP_COUNT > 1)
//	TPIE_OS_UNIX_ONLY_WARNING_MULTIPLE_BTE_COLLECTION_IMP_DEFINED
#  define BTE_COLLECTION_IMP_MMAP
#elif (_BTE_COLL_IMP_COUNT == 0)
//	TPIE_OS_UNIX_ONLY_WARNING_NO_DEFAULT_BTE_COLLECTION
#  define BTE_COLLECTION_IMP_MMAP
#endif

#if defined(BTE_COLLECTION_IMP_MMAP)
#  define BTE_COLLECTION BTE_collection_mmap
#elif defined(BTE_COLLECTION_IMP_UFS)
#  define BTE_COLLECTION BTE_collection_ufs
#elif defined(BTE_COLLECTION_IMP_USER_DEFINED)
   // Do not define BTE_COLLECTION. The user will define it.
#endif

#endif // _BTE_COLL_H
