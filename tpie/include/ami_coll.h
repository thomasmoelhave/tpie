// Copyright (C) 2001 Octavian Procopiuc
//
// File:   ami_coll.h
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: ami_coll.h,v 1.1 2001-05-17 19:31:47 tavi Exp $
//
// Front end for the AMI_COLLECTION implementations.
//
#ifndef _AMI_COLL_H
#define _AMI_COLL_H

#include <ami_coll_base.h>

#include <ami_coll_single.h>

// AMI_collection_single is the only implementation, so make it easy
// to get to.
#define AMI_COLLECTION AMI_collection_single

#endif // _AMI_COLL_H
