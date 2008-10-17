// Copyright (C) 2001 Octavian Procopiuc
//
// File:   ami_coll_base.h
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: ami_coll_base.h,v 1.4 2004-08-17 16:47:45 jan Exp $
//
// Basic definitions for all AMI_COLLECTION implementations.
//
#ifndef _TPIE_AMI_COLL_BASE_H
#define _TPIE_AMI_COLL_BASE_H

///////////////////////////////////////////////////////////////////////////
/// \file tpie/coll_base.h 
/// Definitions and enumeration types for TPIE block collection.
/// \sa tpie::ami::collection_single< BTECOLL >
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

namespace tpie {

  namespace ami {

  /** TPIE block  collection types passed to constructors */
  enum collection_type {
    /** Open existing collection for reading */
    READ_COLLECTION = 1,   
    /**  Open for writing.  Create if non-existent */
    WRITE_COLLECTION,      
    /** Open to read and write.  */  
    READ_WRITE_COLLECTION  
  };

/** TPIE block collection validity status */
	enum collection_status {
	    /** Signaling validity of the collection. */
	    COLLECTION_STATUS_VALID = 0,
      /** Signaling invalidity of the collection. 
	     * The only operation that can leave the collection invalid 
	     * is the constructor (if that happens, the log file contains more i
	     * nformation). No blocks should be read from or written to an invalid 
	     *  collection. */
	    COLLECTION_STATUS_INVALID = 1
	};
	
    }  //  ami namespace
    
}  //  tpie namespace

/** TPIE block  collection types passed to constructors */
enum AMI_collection_type {
  /** Open existing collection for reading */
  AMI_READ_COLLECTION = 1,	
  /**  Open for writing.  Create if non-existent */
  AMI_WRITE_COLLECTION,
  /** Open to read and write.  */  
  AMI_READ_WRITE_COLLECTION	
};

/**  TPIE collection validity status. */
enum AMI_collection_status {
  AMI_COLLECTION_STATUS_VALID = 0,
  AMI_COLLECTION_STATUS_INVALID = 1
};

#endif // _TPIE_AMI_COLL_BASE_H
