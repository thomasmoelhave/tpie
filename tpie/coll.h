//
// File:   ami_coll.h
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: ami_coll.h,v 1.8 2003-05-08 22:12:21 tavi Exp $
//
// Front end for the AMI_COLLECTION implementations.
//
#ifndef _AMI_COLL_H
#define _AMI_COLL_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <tpie/coll_base.h>
#include <tpie/coll_single.h>

///////////////////////////////////////////////////////////////////////////
/// \file tpie/coll.h Provides means to choose and set a specific collection type.
/// For now \ref tpie::ami::collection_single is the only implementation.
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
/// As \ref tpie::ami::collection_single is for now the only collection implementation,
/// define collection to point to collection_single.
///////////////////////////////////////////////////////////////////////////
#define AMI_collection AMI_collection_single

#ifdef BTE_COLLECTION
#  define AMI_COLLECTION AMI_collection_single< BTE_COLLECTION >
#endif

#endif // _AMI_COLL_H
