// Copyright (C) 2001 Octavian Procopiuc
//
// File:   ami_coll_base.h
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: ami_coll_base.h,v 1.2 2001-05-29 15:42:04 tavi Exp $
//
// Basic definitions for all AMI_COLLECTION implementations.
//
#ifndef _AMI_COLL_BASE_H
#define _AMI_COLL_BASE_H


// AMI collection types passed to constructors
enum AMI_collection_type {
    AMI_READ_COLLECTION = 1,	// Open existing collection for reading
    AMI_WRITE_COLLECTION,      	// Open for writing.  Create if non-existent
    AMI_READ_WRITE_COLLECTION	// Open to read and write.
};

// AMI collection status.
enum AMI_collection_status {
  AMI_COLLECTION_STATUS_VALID = 0,
  AMI_COLLECTION_STATUS_INVALID = 1,
};

#endif // _AMI_COLL_BASE_H
