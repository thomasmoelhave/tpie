//
// File: bte/err.h
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
//         (from Darren's bte/base_stream.h)
// Created: 12/29/01
// $Id: bte/err.h,v 1.2 2003-04-17 14:56:26 jan Exp $
//
// BTE error codes, moved here from bte/base_stream.h
//

#ifndef _TPIE_BTE_ERR_H
#define _TPIE_BTE_ERR_H

// Get definitions for working with Unix and Windows
#include <portability.h>

namespace bte {
    
//
// BTE error codes are returned using the BTE_err type.
//
    enum err {
	NO_ERROR = 0,
	IO_ERROR,
	END_OF_STREAM,
	READ_ONLY,
	OS_ERROR,
	BASE_METHOD,
	MEMORY_ERROR,
	PERMISSION_DENIED,
	OFFSET_OUT_OF_RANGE,
	OUT_OF_SPACE,
	STREAM_IS_SUBSTREAM,
	WRITE_ONLY,
	BAD_HEADER,
	INVALID_PLACEHOLDER
    };
    
}

#endif // _TPIE_ERR_H
