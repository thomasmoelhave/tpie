// Copyright (c) 1995 Darren Erik Vengroff
//
// File: persist.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 4/7/95
//
// $Id: persist.h,v 1.3 2003-09-13 17:42:27 jan Exp $
//
// Persistence flags for TPIE streams.
//
#ifndef _TPIE_PERSIST_H
#define _TPIE_PERSIST_H

///////////////////////////////////////////////////////////////////////////
/// \file persist.h Declares persistence tags for TPIE streams.
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

namespace tpie {
	
/** Declares for a stream under which circumstances it should be deleted. */
    enum persistence {
	/** Delete the stream from the disk when it is destructed. */
	PERSIST_DELETE = 0,
	/** Do not delete the stream from the disk when it is destructed. */
	PERSIST_PERSISTENT = 1,
	/** Delete each block of data from the disk as it is read.
	 * If not supported by the OS (see portability.h), delete
	 * the stream when it is destructed (see PERSIST_DELETE). */
	PERSIST_READ_ONCE = TPIE_OS_PERSIST_READ_ONCE
    };
    
}  //  tpie namespace 

#endif // _TPIE_AMI_PERSIST_H 
