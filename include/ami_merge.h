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
// $Id: ami_merge.h,v 1.19 1999-03-09 01:45:56 natsev Exp $
//
#ifndef _AMI_MERGE_H
#define _AMI_MERGE_H

#include <ami_ptr.h>

// For log() and such as needed to compute tree heights.
#include <math.h>

enum AMI_merge_output_type {
    AMI_MERGE_OUTPUT_OVERWRITE = 1,
    AMI_MERGE_OUTPUT_APPEND
};

typedef int AMI_merge_flag;
typedef unsigned int arity_t;

#define CONST const

// A superclass for merge objects.
template<class T>
class AMI_merge_base {
public:
    virtual AMI_err initialize(arity_t arity,
                               CONST T *
                               CONST * in,
                               AMI_merge_flag *taken_flags,
                               int &taken_index) = 0;
    virtual AMI_err operate(CONST T * CONST *in,
                            AMI_merge_flag *taken_flags,
                            int &taken_index,
                            T *out) = 0;
    virtual AMI_err main_mem_operate(T* mm_stream, size_t len) = 0;
    virtual size_t space_usage_overhead(void) = 0;
    virtual size_t space_usage_per_stream(void) = 0;
};



template<class T, class M>
static AMI_err AMI_recursive_merge(AMI_STREAM<T> **instreams, arity_t arity,
                                   AMI_STREAM<T> *outstream, M *m_obj)
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
AMI_err AMI_single_merge(AMI_STREAM<T> **instreams, arity_t arity,
                         AMI_STREAM<T> *outstream, M *m_obj)
{
    unsigned int ii;
    AMI_err ami_err;

    // Create an array of pointers for the input.
    T *in_objects[arity];

    // Create an array of flags the merge object can use to ask for
    // more input from specific streams.
    AMI_merge_flag taken_flags[arity];

    // An index to speed things up when the merge object takes only
    // from one index.
    int taken_index;
    
    //Output of the merge object.
    T merge_out;

#if DEBUG_PERFECT_MERGE
    unsigned int input_count = 0, output_count = 0;
#endif    
    
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
        } else {
#if DEBUG_PERFECT_MERGE
    input_count++;
#endif                
        }
    }

    // Initialize the merge object.
    if (((ami_err = m_obj->initialize(arity, in_objects, taken_flags,
                                     taken_index)) !=
                                     AMI_ERROR_NO_ERROR) &&
                                      (ami_err != AMI_MERGE_READ_MULTIPLE)) {
        return AMI_ERROR_OBJECT_INITIALIZATION;
    }      

    // Now simply call the merge object repeatedly until it claims to
    // be done or generates an error.

    while (1) {
        if (ami_err == AMI_MERGE_READ_MULTIPLE) {
            for (ii = arity; ii--; ) {
                if (taken_flags[ii]) {
                    ami_err = instreams[ii]->read_item(&(in_objects[ii]));
                    if (ami_err != AMI_ERROR_NO_ERROR) {
                        if (ami_err == AMI_ERROR_END_OF_STREAM) {
                            in_objects[ii] = NULL;
                        } else {
                            return ami_err;
                        }
                    } else {
#if DEBUG_PERFECT_MERGE                    
                    input_count++;
#endif
                    }
                }
                // Clear all flags before operate is called.
                taken_flags[ii] = 0;
            }
        } else {
            // The last call took at most one item.
            if (taken_index >= 0) {
                ami_err = instreams[taken_index]->
                    read_item(&(in_objects[taken_index]));
                if (ami_err != AMI_ERROR_NO_ERROR) {
                    if (ami_err == AMI_ERROR_END_OF_STREAM) {
                        in_objects[taken_index] = NULL;
                    } else {
                        return ami_err;
                    }
                } else {
#if DEBUG_PERFECT_MERGE                    
                    input_count++;
#endif
                }
                taken_flags[taken_index] = 0;
            }
        }
        ami_err = m_obj->operate(in_objects, taken_flags, taken_index,
                                 &merge_out);
        if (ami_err == AMI_MERGE_DONE) {
            break;
        } else if (ami_err == AMI_MERGE_OUTPUT) {
#if DEBUG_PERFECT_MERGE
            output_count++;
#endif                    
            if ((ami_err = outstream->write_item(merge_out)) !=
                AMI_ERROR_NO_ERROR) {
                return ami_err;
            }            
        } else if ((ami_err != AMI_MERGE_CONTINUE) &&
                   (ami_err != AMI_MERGE_READ_MULTIPLE)) {
            return ami_err;
        }
    }

#if DEBUG_PERFECT_MERGE
        tp_assert(input_count == output_count,
                  "Merge done, input_count = " << input_count <<
                  ", output_count = " << output_count << '.');
#endif
    
    return AMI_ERROR_NO_ERROR;
};


template<class T, class M>
AMI_err AMI_merge(AMI_STREAM<T> **instreams, arity_t arity,
                  AMI_STREAM<T> *outstream, M *m_obj)
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
        return AMI_recursive_merge(instreams, arity, outstream, m_obj);

    } else {
        return AMI_single_merge(instreams, arity, outstream, m_obj);
    }

};


template<class T, class M>
AMI_err AMI_main_mem_merge(AMI_STREAM<T> *instream,
                           AMI_STREAM<T> *outstream, M *m_obj) 
{
    AMI_err ae;
    off_t len;
    size_t sz_avail;
    
    // Figure out how much memory we've got to work with.

    if (MM_manager.available(&sz_avail) != MM_ERROR_NO_ERROR) {
        return AMI_ERROR_MM_ERROR;
    }

    // If the whole input can fit in main memory then just call
    // AMI_main_mem_merge() to deal with it by loading it once and
    // processing it.

    len = instream->stream_len();

    if ((len * sizeof(T)) <= sz_avail) {

        T *mm_stream;
        off_t len1;

        instream->seek(0);
        
        // This code is sloppy and has to be rewritten correctly for
        // parallel buffer allocation.  It will not work with anything
        // other than a registration based memory manager.
        
        if ((mm_stream = new T[len]) == NULL) {
            return AMI_ERROR_MM_ERROR;
        };

        len1 = len;
        
        if ((ae = instream->read_array(mm_stream, &len1)) !=
            AMI_ERROR_NO_ERROR) {
            return ae;
        }
        
        tp_assert(len1 == len, "Did not read the right amount; "
                  "Allocated space for " << len << ", read " << len1 << '.');

        if ((ae = m_obj->main_mem_operate(mm_stream, len)) !=
            AMI_ERROR_NO_ERROR) {
            return ae;
        }

        if ((ae = outstream->write_array(mm_stream, len)) !=
            AMI_ERROR_NO_ERROR) {
            return ae;
        }

        delete [] mm_stream;

        return AMI_ERROR_NO_ERROR;

    } else {

        // Something went wrong.  We should not have called this
        // function, since we don't have enough mein memory.

        return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
    }
};


// Recursive division of a stream and then merging back together.
template<class T, class M>
AMI_err AMI_partition_and_merge(AMI_STREAM<T> *instream,
                                AMI_STREAM<T> *outstream, M *m_obj) 
{
    AMI_err ae;
    off_t len;
    size_t sz_avail, sz_stream;
    unsigned int ii;
    int jj;
    
    // Figure out how much memory we've got to work with.

    if (MM_manager.available(&sz_avail) != MM_ERROR_NO_ERROR) {
        return AMI_ERROR_MM_ERROR;
    }

    // If the whole input can fit in main memory then just call
    // AMI_main_mem_merge() to deal with it by loading it once and
    // processing it.

    len = instream->stream_len();

    if ((len * sizeof(T)) <= sz_avail) {

        return AMI_main_mem_merge(instream, outstream, m_obj);

    } else {

        // The number of substreams that the original input stream
        // will be split into.
        
        arity_t original_substreams;

        // The length, in terms of stream objects of type T, of the
        // original substreams of the input stream.  The last one may
        // be shorter than this.
        
        size_t sz_original_substream;

        // The initial temporary stream, to which substreams of the
        // original input stream are written.

        AMI_STREAM<T> *initial_tmp_stream;
        
        // The number of substreams that can be merged together at once.

        arity_t merge_arity;

        // A pointer to the buffer in main memory to read a memory load into.
        T *mm_stream;
        
        // Loop variables:

        // The stream being read at the current level.
        
        AMI_STREAM<T> *current_input;

        // The output stream for the current level if it is not outstream.

        AMI_STREAM<T> *intermediate_tmp_stream;
        
        // The size of substreams of *current_input that are being
        // merged.  The last one may be smaller.  This value should be
        // sz_original_substream * (merge_arity ** k) where k is the
        // number of iterations the loop has gone through.
        
        size_t current_substream_len;

        // The exponenent used to verify that current_substream_len is
        // correct.
        
        unsigned int k;

        off_t sub_start, sub_end;

        // How many substreams will there be?  The main memory
        // available to us is the total amount available, minus what
        // is needed for the input stream and the temporary stream.

        if ((ae = instream->main_memory_usage(&sz_stream,
                                              MM_STREAM_USAGE_MAXIMUM)) !=
                                              AMI_ERROR_NO_ERROR) {
            return ae;
        }                                     

        if (sz_avail <= 2 * sz_stream + sizeof(T)) {
            return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
        }
        
        sz_avail -= 2 * sz_stream;

        sz_original_substream = sz_avail / sizeof(T);

        // Round the original substream length off to an integral
        // number of chunks.  This is for systems like HP-UX that
        // cannot map in overlapping regions.  It is also required for
        // BTE's that are capable of freeing chunks as they are
        // read.

        {
            size_t sz_chunk_size = instream->chunk_size();
            
            sz_original_substream = sz_chunk_size *
                ((sz_original_substream + sz_chunk_size - 1) /
                 sz_chunk_size);
        }

        original_substreams = (len + sz_original_substream - 1) /
            sz_original_substream;
        
        // Account for the space that a merge object will use.

        {
            size_t sz_avail_during_merge = sz_avail -
                m_obj->space_usage_overhead();
            size_t sz_stream_during_merge =sz_stream +
                m_obj->space_usage_per_stream();
           
            merge_arity = (sz_avail_during_merge +
                           sz_stream_during_merge - 1) /
                sz_stream_during_merge;

        }

        // Make sure that the AMI is willing to provide us with the
        // number of substreams we want.  It may not be able to due to
        // operating system restrictions, such as on the number of
        // regions that can be mmap()ed in.

        {
            int ami_available_streams = instream->available_streams();

            if (ami_available_streams != -1) {
                if (ami_available_streams <= 4) {
                    return AMI_ERROR_INSUFFICIENT_AVAILABLE_STREAMS;
                }
                
                if (merge_arity > (arity_t)ami_available_streams - 2) {
                    merge_arity = ami_available_streams - 2;
                    LOG_INFO("Reduced merge arity due to AMI restrictions.\n");
                }
            }
        }
        
        LOG_INFO("AMI_partition_and_merge(): merge arity = " <<
                 merge_arity << ".\n");
        
        if (merge_arity < 2) {
            return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
        }

//#define MINIMIZE_INITIAL_SUBSTREAM_LENGTH
#ifdef MINIMIZE_INITIAL_SUBSTREAM_LENGTH
        
        // Make the substreams as small as possible without increasing
        // the height of the merge tree.

        {
            // The tree height is the ceiling of the log base merge_arity
            // of the number of original substreams.
            
            double tree_height = log((double)original_substreams) /
                log((double)merge_arity);

            tp_assert(tree_height > 0,
                      "Negative or zero tree height!");
           
            tree_height = ceil(tree_height);

            // See how many substreams we could possibly fit in the
            // tree without increasing the height.

            double max_original_substreams = pow((double)merge_arity,
                                                 tree_height);

            tp_assert(max_original_substreams >= original_substreams,
                      "Number of permitted substreams was reduced.");

            // How big will such substreams be?

            double new_sz_original_substream = ceil((double)len /
                                                    max_original_substreams);

            tp_assert(new_sz_original_substream <= sz_original_substream,
                      "Size of original streams increased.");

            sz_original_substream = (size_t)new_sz_original_substream;

            LOG_INFO("Memory constraints set original substreams = " <<
                     original_substreams << '\n');
            
            original_substreams = (len + sz_original_substream - 1) /
                sz_original_substream;

            LOG_INFO("Tree height constraints set original substreams = " <<
                     original_substreams << '\n');
        }
                
#endif // MINIMIZE_INITIAL_SUBSTREAM_LENGTH

        // Create a temporary stream, then iterate through the
        // substreams, processing each one and writing it to the
        // corresponding substream of the temporary stream.
        
        initial_tmp_stream = new AMI_STREAM<T>;
        
        mm_stream = new T[sz_original_substream];

        tp_assert(mm_stream != NULL, "Misjudged available main memory.");

        if (mm_stream == NULL) {
            return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
        }

        instream->seek(0);

        tp_assert(original_substreams * sz_original_substream - len <
                  sz_original_substream,
                  "Total substream length too long or too many.");

        tp_assert(len - (original_substreams - 1) * sz_original_substream <=
                  sz_original_substream,
                  "Total substream length too short or too few.");        
            
        for (ii = 0; ii++ < original_substreams; ) {
            off_t mm_len;

            if (ii == original_substreams) {
                mm_len = len % sz_original_substream;
                // If it is an exact multiple, then the mod will come
                // out 0, which is wrong.
                if (!mm_len) {
                    mm_len = sz_original_substream;
                }
            } else {
                mm_len = sz_original_substream;
            }
            
#if DEBUG_ASSERTIONS
            off_t mm_len_bak = mm_len;
#endif
            
            // Read a memory load out of the input stream.
            ae = instream->read_array(mm_stream, &mm_len);
            if (ae != AMI_ERROR_NO_ERROR) {
                return ae;
            }

            tp_assert(mm_len == mm_len_bak,
                      "Did not read the requested number of objects." <<
                      "\n\tmm_len = " << mm_len <<
                      "\n\tmm_len_bak = " << mm_len_bak << '.');
                      
            // Solve in main memory.
            m_obj->main_mem_operate(mm_stream, mm_len);

            // Write the result out to the temporary stream.
            ae = initial_tmp_stream->write_array(mm_stream, mm_len);
            if (ae != AMI_ERROR_NO_ERROR) {
                return ae;
            }            
        }

        delete [] mm_stream;

        // Make sure the total length of the temporary stream is the
        // same as the total length of the original input stream.

        tp_assert(instream->stream_len() == initial_tmp_stream->stream_len(),
                  "Stream lengths do not match:" <<
                  "\n\tinstream->stream_len() = " << instream->stream_len() <<
                  "\n\tinitial_tmp_stream->stream_len() = " <<
                  initial_tmp_stream->stream_len() << ".\n");

        // Set up the loop invariants for the first iteration of hte
        // main loop.

        current_input = initial_tmp_stream;
        current_substream_len = sz_original_substream;

        // Pointers to the substreams that will be merged.
        
        AMI_STREAM<T> **the_substreams = new (AMI_STREAM<T> *)[merge_arity];

        k = 0;
        
        // The main loop.  At the outermost level we are looping over
        // levels of the merge tree.  Typically this will be very
        // small, e.g. 1-3.

        for( ; current_substream_len < (size_t)len;
               current_substream_len *= merge_arity) {

            // The number of substreams to be processed at this level.
            
            arity_t substream_count;
            
            // Set up to process a given level.

            tp_assert(len == current_input->stream_len(),
                      "Current level stream not same length as input." <<
                      "\n\tlen = " << len <<
                      "\n\tcurrent_input->stream_len() = " <<
                      current_input->stream_len() << ".\n");

            // Do we have enough main memory to merge all the
            // substreams on the current level into the output stream?
            // If so, then we will do so, if not then we need an
            // additional level of iteration to process the substreams
            // in groups.

            substream_count = (len + current_substream_len - 1) /
                current_substream_len;
            
            if (substream_count <= merge_arity) {

                LOG_INFO("Merging substreams directly to the output stream.\n");
                // Create all the substreams

                for (sub_start = 0, ii = 0 ;
                     ii < substream_count;
                     sub_start += current_substream_len, ii++) {

                    sub_end = sub_start + current_substream_len - 1;
                    if (sub_end >= len) {
                        sub_end = len - 1;
                    }
                    current_input->new_substream(AMI_READ_STREAM,
                                                 sub_start,
                                                 sub_end,
                                                 (AMI_base_stream<T> **)
                                                 (the_substreams + ii));

                    // The substreams are read-once.

                    the_substreams[ii]->persist(PERSIST_READ_ONCE);

                }               

                tp_assert(((int) sub_start >= (int) len) &&
                          ((int) sub_start < (int) len + (int) current_substream_len),
                          "Loop ended in wrong location.");

                // Fool the OS into unmapping the current block of the
                // input stream so that blocks of the substreams can
                // be mapped in without overlapping it.  This is
                // needed for correct execution on HP-UX.

                current_input->seek(0);
                
                // Merge them into the output stream.

                ae = AMI_single_merge(the_substreams,
                                      substream_count,
                                      outstream, m_obj);
                if (ae != AMI_ERROR_NO_ERROR) {
                    return ae;
                }
                
                // Delete the substreams.

                for (ii = 0; ii < substream_count; ii++) {
                    delete the_substreams[ii];
                }

                // And the current input, which is an intermediate stream
                // of some kind.

                delete current_input;
                
            } else {

                LOG_INFO("Merging substreams to an intermediate stream.\n");

                // Create the next intermediate stream.

                intermediate_tmp_stream = new AMI_STREAM<T>;

                // Fool the OS into unmapping the current block of the
                // input stream so that blocks of the substreams can
                // be mapped in without overlapping it.  This is
                // needed for correct execution on HU-UX.

                current_input->seek(0);

                // Loop through the substreams of the current stream,
                // merging as many as we can at a time until all are
                // done with.

                for (sub_start = 0, ii = 0, jj = 0;
                     ii < substream_count;
                     sub_start += current_substream_len, ii++, jj++) {

                    sub_end = sub_start + current_substream_len - 1;
                    if (sub_end >= len) {
                        sub_end = len - 1;
                    }

                    current_input->new_substream(AMI_READ_STREAM,
                                                 sub_start,
                                                 sub_end,
                                                 (AMI_base_stream<T> **)
                                                 (the_substreams + jj));

                    // The substreams are read-once.

                    the_substreams[jj]->persist(PERSIST_READ_ONCE);
                    
                    // If we've got all we can handle or we've seen
                    // them all, then merge them.
                    
                    if ((jj >= (int) merge_arity - 1) ||
                        (ii == substream_count - 1)) {
                        
                        tp_assert(jj <= (int) merge_arity - 1,
                                  "Index got too large.");

#if DEBUG_ASSERTIONS
                        // Check the lengths before the merge.
                        
                        size_t sz_output, sz_output_after_merge;
                        size_t sz_substream_total;

                        {
                            unsigned int kk;

                            sz_output = intermediate_tmp_stream->stream_len();
                            sz_substream_total = 0;
                            
                            for (kk = jj+1; kk--; ) {
                                sz_substream_total +=
                                    the_substreams[kk]->stream_len();
                            }                          
                                
                        }
#endif // DEBUG_ASSERTIONS
                        
                        // This should append to the stream, since
                        // AMI_single_merge() does not rewind the
                        // output before merging.
                        ae = AMI_single_merge(the_substreams,
                                              jj+1,
                                              intermediate_tmp_stream,
                                              m_obj);
                        
                        if (ae != AMI_ERROR_NO_ERROR) {
                            return ae;
                        }

#if DEBUG_ASSERTIONS
                        // Verify the total lengths after the merge.

                        sz_output_after_merge =
                            intermediate_tmp_stream->stream_len();

                        tp_assert(sz_output_after_merge - sz_output ==
                                  sz_substream_total,
                                  "Stream lengths do not add up: " <<
                                  sz_output_after_merge - sz_output <<
                                  " written when " <<
                                  sz_substream_total <<
                                  " were to have been read.");
                                  
#endif // DEBUG_ASSERTIONS                        
                        
                        // Delete the substreams.  jj is currently the index
                        // of the largest, so we want to bump it up before the
                        // idiomatic loop.

                        for (jj++; jj--; ) {
                            delete the_substreams[jj];
                        }

                        // Now jj should be -1 so that it gets bumped
                        // back up to 0 before the next iteration of
                        // the outer loop.
                        tp_assert((jj == -1), "Index not reduced to -1.");
                        
                    }               

                }

                // Get rid of the current input stream and use the next one.

                delete current_input;
                current_input = intermediate_tmp_stream;
                
            }
            
            k++;

        }

        delete [] the_substreams;
        
        return AMI_ERROR_NO_ERROR;
    }
}

#endif //_AMI_MERGE_H
