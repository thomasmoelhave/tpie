//
// File:   ami_coll.h
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: ami_coll.h,v 1.5 2002-06-28 23:25:53 tavi Exp $
//
// Front end for the AMI_COLLECTION implementations.
//
#ifndef _AMI_COLL_H
#define _AMI_COLL_H

#include <ami_coll_base.h>

#include <ami_coll_single.h>

// AMI_collection_single is the only implementation, so make it easy
// to get to.
#ifdef BTE_COLLECTION
#  define AMI_COLLECTION AMI_collection_single<BTE_COLLECTION>
#  define AMI_collection AMI_collection_single<BTE_COLLECTION>
#endif

#endif // _AMI_COLL_H
