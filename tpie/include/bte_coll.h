// Copyright (C) 2001 Octavian Procopiuc
//
// File:    bte_coll.h
// Authors: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: bte_coll.h,v 1.1 2001-05-17 19:29:14 tavi Exp $
//
// Front end for the BTE collection classes.
//

#ifndef _BTE_COLL_H
#define _BTE_COLL_H

// Get the base class and various definitions.
#include <bte_coll_base.h>

// The MMB implementation.
#include <bte_coll_mmb.h>

// The UFS implementation.
#include <bte_coll_ufs.h>

#define _BTE_COLL_IMP_COUNT (defined(BTE_COLLECTION_IMP_UFS) + \
                             defined(BTE_COLLECTION_IMP_MMB) + \
                             defined(BTE_COLLECTION_IMP_USER_DEFINED))

// All implementations are included, but we have to choose a default
// one.

#if (_BTE_COLL_IMP_COUNT > 1)
#warning Multiple BTE_COLLECTION_IMP_* defined. \
         Undetermined default implementation.
#endif

#if (_BTE_COLL_IMP_COUNT == 0)
#warning No default BTE_COLLECTION implementation defined, using \
         BTE_COLLECTION_IMP_MMB by default. \
         To avoid this warning, define either BTE_COLLECTION_IMP_MMB \
         or BTE_COLLECTION_IMP_UFS in your app_config.h file.
#define BTE_COLLECTION_IMP_MMB
#endif

#if defined(BTE_COLLECTION_IMP_MMB)
#define BTE_COLLECTION BTE_collection_mmb
#elif defined(BTE_COLLECTION_IMP_UFS)
#define BTE_COLLECTION BTE_collection_ufs
#elif defined(BTE_COLLECTION_IMP_USER_DEFINED)
// Do not define BTE_COLLECTION. The user will define it.
#endif

#endif // _BTE_COLL_H
