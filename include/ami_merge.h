// Copyright (c) 1994 Darren Erik Vengroff
//
// File: ami_merge.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/31/94
//
// First cut at a merger.  Obviously missing is code to verify that
// lower level streams will use appropriate levels of buffering.  This
// will be more critical for parallel disk implementations.
//
// $Id: ami_merge.h,v 1.3 1994-09-16 13:24:48 darrenv Exp $
//
#ifndef _AMI_MERGE_H
#define _AMI_MERGE_H

#include <ami_ptr.h>

typedef int AMI_merge_flag;
typedef unsigned int arity_t;

// A superclass for merge objects.
template<class T>
class AMI_merge_base {
public:
    virtual AMI_err initialize(arity_t arity, T **in,
                               AMI_merge_flag *taken_flags) = 0;
    virtual AMI_err operate(const T **in, AMI_merge_flag *taken_flags,
                            T *out) = 0;
};



template<class T, class M>
static AMI_err AMI_recursive_merge(pp_AMI_bs<T> instreams, arity_t arity,
                                   AMI_base_stream<T> *outstream, M *mobj)
{
    size_t sz_avail;
    size_t sz_stream;
    unsigned int ii, iistart;
    
    // How much main memory is available?
    if (MM_manager.available(&sz_avail) != MM_ERROR_NO_ERROR) {
        return AMI_ERROR_MM_ERROR;
    }
    
    ii = 0;

    do {

        iistart = ii;
        
        // Iterate through the streams until we have as many as can
        // fit in main memory.

        do {
            instreams[ii]->main_memory_usage(&sz_stream,
                                             MM_STREAM_USAGE_MAXIMUM);
            sz_avail -= sz_stream;
            instreams[ii]->main_memory_usage(&sz_stream,
                                             MM_STREAM_USAGE_CURRENT);
            sz_avail += sz_stream;
            ii++;
        } while ((sz_stream > 0) && (ii < arity));                             

        // If they all fit, then we should not have called the
        // recursive version of merge.
        tp_assert(iistart || (ii < arity),
                  "All the streams fit, but recursive merge was called.");

        // Next time through the inner loop we will start with the stream
        // that caused us to go over.

        ii--;

        // Make sure we make some progress, and are merging at least two
        // streams.  Hopefully we are doing much better than that.
        if (ii <= iistart + 1) {
            LOG_ERROR("Insuficent main memory to perform a merge.\n");
            return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
        }

        // Merge the series of streams into a substream of the output
        // stream.
        
    } while (ii < arity);
    
};


template<class T, class M>
static AMI_err AMI_single_merge(pp_AMI_bs<T> instreams, arity_t arity,
                  AMI_base_stream<T> *outstream, M *mobj)
{
    unsigned int ii;
    AMI_err ami_err;

    // Create an array of pointers for the input.
    T *in_objects[arity];

    // Create an array of flags the merge object can use to ask for
    // more input from specific streams.
    AMI_merge_flag taken_flags[arity];

    // Output of the merge object.
    T merge_out;
    
    // Rewind and read the first item from every stream.
    for (ii = arity; ii--; ) {
        if ((ami_err = instreams[ii]->seek(0)) != AMI_ERROR_NO_ERROR) {
            return ami_err;
        }
        if ((ami_err = instreams[ii]->read_item(&(in_objects[ii]))) !=
            AMI_ERROR_NO_ERROR) {
            if (ami_err == AMI_ERROR_END_OF_STREAM) {
                in_objects[ii] = NULL;
            } else {
                return ami_err;
            }
            // Set the taken flags to 0 before we call intialize()
            taken_flags[ii] = 0;
        }
    }

    // Initialize the merge object.
    if (mobj->initialize(arity, in_objects, taken_flags) !=
        AMI_ERROR_NO_ERROR) {
        return AMI_ERROR_OBJECT_INITIALIZATION;
    }

    // Now simply call the merge object repeatedly until it claims to
    // be done or generates an error.

    while (1) {
        for (ii = arity; ii--; ) {
            if (taken_flags[ii] &&
                ((ami_err = instreams[ii]->read_item(&(in_objects[ii]))) !=
                 AMI_ERROR_NO_ERROR)) {
                if (ami_err == AMI_ERROR_END_OF_STREAM) {
                    in_objects[ii] = NULL;
                } else {
                    return ami_err;
                }
            }
            // Clear all flags before operate is called.
            taken_flags[ii] = 0;
        }
        ami_err = mobj->operate(in_objects, taken_flags, &merge_out);
        if (ami_err == AMI_MERGE_DONE) {
            break;
        } else if (ami_err == AMI_MERGE_OUTPUT) {
            if ((ami_err = outstream->write_item(merge_out)) !=
                AMI_ERROR_NO_ERROR) {
                return ami_err;
            }            
        } else if (ami_err != AMI_MERGE_CONTINUE) {
            return ami_err;
        }
    }
    
    return AMI_ERROR_NO_ERROR;
};


template<class T, class M>
AMI_err AMI_merge(pp_AMI_bs<T> instreams, arity_t arity,
                  AMI_base_stream<T> *outstream, M *mobj)
{
    size_t sz_avail;
    size_t sz_stream, sz_needed;
    unsigned int ii;
    
    // Iterate through the streams, finding out how much additional
    // memory each stream will need in the worst case.

    for (ii = 0, sz_needed = 0; ii < arity; ii++) {
        instreams[ii]->main_memory_usage(&sz_stream,
                                         MM_STREAM_USAGE_MAXIMUM);
        sz_needed += sz_stream;
        instreams[ii]->main_memory_usage(&sz_stream,
                                         MM_STREAM_USAGE_CURRENT);
        sz_needed -= sz_stream;
    }                              
    
    // How much main memory is available?

    if (MM_manager.available(&sz_avail) != MM_ERROR_NO_ERROR) {
        return AMI_ERROR_MM_ERROR;
    }
    
    // If more memory will be needed that we have available, then
    // we will have to use a recursive merge.  Otherwise, a single
    // level merge is OK.

    if (sz_needed > sz_avail) {
        
        // Make sure we have a temporary stream to merge to before we
    	// do a recursive merge.
        return AMI_recursive_merge(instreams, arity, outstream, mobj);

    } else {
        return AMI_single_merge(instreams, arity, outstream, mobj);
    }

};



// Recursive division of a stream and then merging back together.
template<class T, class M>
AMI_err AMI_partition_and_merge(AMI_STREAM<T> *instream,
                                AMI_STREAM<T> *outstream, M *mobj) 
{
    AMI_err ae;
    off_t len;
    size_t sz_avail, sz_stream;
    
    // Figure out how much memory we've got to work with.

    if (MM_manager.available(&sz_avail) != MM_ERROR_NO_ERROR) {
        return AMI_ERROR_MM_ERROR;
    }

    // If the whole input stream can fit in main memory, then load it
    // and call the bottoming out function.

    len = instream->stream_len();

    if ((len * sizeof(T)) <= sz_avail) {
        MM_ptr<T> mm_stream;

        if (!(mm_stream = MM_manager->alloc(len * sizeof(T)))) {
            return AMI_ERROR_MM_ERROR;
        }

        if ((ae = instream->read_array(mm_stream, &len1)) !=
            AMI_ERROR_NO_ERROR) {
            return ae;
        }
        
        tp_assert(len1 = len, "Did not read the right amount; "
                  "Allocated space for " << len << ", read " << len1 << '.');

        if ((ae = (*mm_operate)(mm_stream, len)) !=
            AMI_ERROR_NO_ERROR) {
            return ae;
        }

        if ((ae = outstream->write_array(mm_stream, len)) !=
            AMI_ERROR_NO_ERROR) {
            return ae;
        }

        MM_manager->free(mm_stream);

        return AMI_ERROR_NO_ERROR;

    } else {

        // It won't all fit, so we have to recurse.

        tp_assert(0, "Recursive part not implemented yet.");

        return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
    }
};


#if 0
    // How much memory do we need for each substream?

    if ((ae = instream->main_memory_usage(&sz_stream,
                                          MM_STREAM_USAGE_SUBSTREAM)) !=
                                          AMI_ERROR_NO_ERROR) {
        return ae;
    }                                     
    
    // Determine how many substreams we can merge at a time.

    substream_arity = sz_avail / sz_stream;

    // Determine the depth of recursion.  What is important is whether
    // it is even or odd, since that will determine whether we start
    // out going to the output stream or to the shadow stream.

    
    
    // If the recursion depth exceeds 1 the create a shadow stream as
    // large as the input stream.

    // Partition the input stream into substreams that fit into memory,
    // read each in as it is created, process it, and send it to the
    // appropriate substream of the shadow stream or output stream.

    // Iterate through the levels of recursion.  At each level, create
    // the appropriate substreams of the output (resp. shadow) stream
    // and merge them into a substream of the shadow (resp. output)
    // stream.
    
    return AMI_ERROR_NO_ERROR;
};
#endif

#endif // _AMI_MERGE_H 


