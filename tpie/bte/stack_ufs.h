//
// File:     bte/stack_ufs.h
// Author:   Octavian Procopiuc <tavi@cs.duke.edu>
// Created:  09/15/03
//
// A stack implemented using BTE_stream_ufs. It is used by
// BTE_collection_base to implement deletions.  
//
// $Id: bte/stack_ufs.h,v 1.2 2005-01-14 18:47:22 tavi Exp $
//

#ifndef _TPIE_BTE_STACK_UFS_H
#define _TPIE_BTE_STACK_UFS_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <tpie/bte/stream_ufs.h>

namespace tpie {

    namespace bte {
    
	template<class T>
	class stack_ufs : public stream_ufs<T> {

	public:
	    using stream_ufs<T>::stream_len;
	    using stream_ufs<T>::seek;
	    using stream_ufs<T>::truncate;
	
	    // Construct a new stack with the given name and access type.
	    stack_ufs(char *path, stream_type type = WRITE_STREAM); 

	    // Destroy this object.
	    ~stack_ufs(void);

	    // Push an element on top of the stack.
	    err push(const T &t);
	
	    // Pop an element from the top of the stack.
	    err pop(T **t);

	};
    
    
	template<class T>
	stack_ufs<T>::stack_ufs(char *path, 
				stream_type type) :
	    stream_ufs<T>(path, type, 1) {
	    // No code in this constructor.
	}
    
	template<class T>
	stack_ufs<T>::~stack_ufs(void) {
	    // No code in this destructor.
	}
    
	template<class T>
	err stack_ufs<T>::push(const T &t) {

	    err retval = NO_ERROR;
	    TPIE_OS_OFFSET slen = stream_len();
           
	    if ((retval = truncate(slen+1)) != NO_ERROR) {
		return retval;
	    }

	    if ((retval = seek(slen)) != NO_ERROR) {
		return retval;
	    }
	
	    return write_item(t);
	}
    
    
	template<class T>
	err stack_ufs<T>::pop(T **t) {

	    err retval = NO_ERROR;
	    TPIE_OS_OFFSET slen = stream_len();

	    if ((retval = seek(slen-1)) != NO_ERROR) {
		return retval;
	    }
  
	    if ((retval =  read_item(t)) != NO_ERROR) {
		return retval;
	    }
	
	    return truncate(slen-1);
	}
    
    }  //  bte namespace

}  //  tpie namespace

#endif // _TPIE_BTE_STACK_UFS_H 
