//
// File:    bte_coll.h
// Authors: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: bte_coll.h,v 1.2 2002-01-14 16:15:46 tavi Exp $
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

#if defined(BTE_COLLECTION_IMP_MMB)
#  warning The BTE_COLLECTION_IMP_MMB flag is obsolete. \
           Please use BTE_COLLECTION_IMP_MMAP.
#  warning Implicitly defining BTE_COLLECTION_IMP_MMAP.
#  define BTE_COLLECTION_IMP_MMAP
#endif

#define _BTE_COLL_IMP_COUNT (defined(BTE_COLLECTION_IMP_UFS) + \
                             defined(BTE_COLLECTION_IMP_MMAP) + \
                             defined(BTE_COLLECTION_IMP_USER_DEFINED))

// Multiple implem. are included, but we have to choose a default one.
#if (_BTE_COLL_IMP_COUNT > 1)
#  warning Multiple BTE_COLLECTION_IMP_* defined. \
         Undetermined default implementation.
#  warning Implicitly defining BTE_COLLECTION_IMP_MMAP.
#  define BTE_COLLECTION_IMP_MMAP
#elif (_BTE_COLL_IMP_COUNT == 0)
#  warning No default BTE_COLLECTION implementation defined, using \
         BTE_COLLECTION_IMP_MMAP by default. \
         To avoid this warning, define one of: BTE_COLLECTION_IMP_MMAP,\
         BTE_COLLECTION_IMP_UFS, BTE_COLLECTION_USER_DEFINED \
         in your app_config.h file.
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
