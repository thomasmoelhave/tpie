//
// File: ami_stream_base.h (formerly ami_base.h)
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/19/94
//
// $Id: ami_stream_base.h,v 1.3 2002-03-14 20:25:59 tavi Exp $
//
#ifndef _AMI_STREAM_BASE_H
#define _AMI_STREAM_BASE_H

#define A_INLINE inline

#include <tpie_assert.h>
#include <ami_err.h>
#include <persist.h>


// AMI stream types passed to constructors
enum AMI_stream_type {
    AMI_READ_STREAM = 1,	// Open existing stream for reading
    AMI_WRITE_STREAM,		// Open for writing.  Create if non-existent
    AMI_APPEND_STREAM,		// Open for writing at end.  Create if needed.
    AMI_READ_WRITE_STREAM	// Open to read and write.
};

// AMI stream status.
enum AMI_stream_status {
  AMI_STREAM_STATUS_VALID = 0,
  AMI_STREAM_STATUS_INVALID = 1,
};


// An abstract class template which implements a stream of objects 
// of type T within the AMI.  This is the superclass of all actual 
// implementations of streams of T within the AMI (e.g. single device
// streams, single CPU/many disk streams, and distributed streams).
template<class T> class AMI_stream_base {
protected:

  AMI_stream_status status_;

public:

  AMI_stream_base(void) { status_ = AMI_STREAM_STATUS_INVALID; }

  // Inquire the status.
  AMI_stream_status status() const { return status_; }
  bool is_valid() const { return status_ == AMI_STREAM_STATUS_VALID; }
  bool operator!() const { return !is_valid(); }

  // TODO: Does this need to be virtual?
  virtual ~AMI_stream_base(void) {}
  
#if AMI_VIRTUAL_BASE
    
    // A virtual psuedo-constructor for substreams.
    virtual AMI_err new_substream(AMI_stream_type st,
                                  off_t sub_begin,
                                  off_t sub_end,
                                  AMI_stream_base<T> **sub_stream) = 0;

    // Access methods.

    virtual A_INLINE AMI_err write_item(const T &tin) = 0;
    virtual A_INLINE AMI_err read_item(T **tout) = 0;

    virtual A_INLINE AMI_err read_array(T *mm_space, off_t *len) = 0;
    virtual A_INLINE AMI_err write_array(const T *mm_space, off_t len) = 0;
    
    // Misc. 
    virtual AMI_err main_memory_usage(size_t *usage,
                                      MM_stream_usage usage_type) = 0;
    
    virtual off_t stream_len(void) = 0;

    virtual AMI_err name(char **stream_name) = 0;
    
    virtual AMI_err seek(off_t offset) = 0;

    virtual AMI_err truncate(off_t offset) = 0;

    virtual int available_streams(void) = 0;

    virtual off_t chunk_size(void) = 0;

    virtual void persist(persistence) = 0;

#endif // AMI_VIRTUAL_BASE
};

#endif // _AMI_STREAM_BASE_H 
