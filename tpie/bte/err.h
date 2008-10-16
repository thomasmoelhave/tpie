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
	ERROR_NO_ERROR = 0,
	ERROR_IO_ERROR,
	ERROR_END_OF_STREAM,
	ERROR_READ_ONLY,
	ERROR_OS_ERROR,
	ERROR_BASE_METHOD,
	ERROR_MEMORY_ERROR,
	ERROR_PERMISSION_DENIED,
	ERROR_OFFSET_OUT_OF_RANGE,
	ERROR_OUT_OF_SPACE,
	ERROR_STREAM_IS_SUBSTREAM,
	ERROR_WRITE_ONLY,
	ERROR_BAD_HEADER,
	ERROR_INVALID_PLACEHOLDER
    };
    
}

#endif // _TPIE_ERR_H
