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
// $Id: ami_merge.h,v 1.1 1994-06-03 13:19:11 dev Exp $
//
#ifndef _AMI_MERGE_H
#define _AMI_MERGE_H

typedef int AMI_merge_flag;
typedef unsigned int arity_t;

// A superclass for merge objects.
template<class T>
class AMI_merge_base {
public:
    virtual AMI_err initialize(arity_t arity, T **in,
                               AMI_merge_flag *taken_flags)
    {
        return AMI_ERROR_BASE_METHOD;
    }
    virtual AMI_err operate(const T **in, AMI_merge_flag *taken_flags,
                            T *out)
    {
        return AMI_ERROR_BASE_METHOD;
    }
};


template<class T, class M>
AMI_err AMI_merge(AMI_STREAM<T> **instreams, arity_t arity,
                  AMI_STREAM<T> *outstream, M *mobj)
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
    
    // Read the first item from every stream.
    for (ii = arity; ii--; ) {
        if ((ami_err = instreams[ii]->read_item(&(in_objects[ii]))) !=
            AMI_ERROR_NO_ERROR) {
            if (ami_err == AMI_ERROR_END_OF_STREAM) {
                in_objects[ii] = NULL;
            }
            return ami_err;
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
                }
                return ami_err;
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
}

#endif // _AMI_MERGE_H 

