// Copyright (c) 1994 Darren Erik Vengroff
//
// File: merge_random.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/5/94
//
// $Id: merge_random.h,v 1.15 2005-12-19 03:11:18 adanner Exp $
//
// A merge managment object that reorders the input stream in a random
// way.

#ifndef _MERGE_RANDOM_H
#define _MERGE_RANDOM_H

#include <portability.h>
#include <merge.h>
#include <mergeheap.h>

template<class T>
class merge_random : public AMI_merge_base<T> {
private:
    // Prohibit these
    merge_random(const merge_random<T>& other);
    merge_random<T>& operator=(const merge_random<T>& other);

    TPIE_OS_SIZE_T input_arity;
    merge_heap_op<int> *mheap;
#if DEBUG_ASSERTIONS
    unsigned int input_count, output_count;
#endif    
public:
    merge_random(int seed = 0);
    virtual ~merge_random(void);
    AMI_err initialize(TPIE_OS_SIZE_T arity, CONST T * CONST *in,
                       AMI_merge_flag *taken_flags,
                       int &taken_index);
    AMI_err operate(CONST T * CONST *in, AMI_merge_flag *taken_flags,
                    int &taken_index, T *out);
    AMI_err main_mem_operate(T* mm_stream, size_t len);
    size_t space_usage_overhead(void);
    size_t space_usage_per_stream(void);
};
    
template<class T>
merge_random<T>::merge_random(int seed) : 
    input_arity(0),  
    mheap(NULL)
#if DEBUG_ASSERTIONS
					, input_count(0)
					, output_count(0)
#endif    
{
    if (seed) {
        TPIE_OS_SRANDOM(seed);
    }
}

template<class T>
merge_random<T>::~merge_random(void)
{
    if (mheap != NULL) {
      mheap->deallocate();
      delete mheap;
    }
}

template<class T>
AMI_err merge_random<T>::initialize(TPIE_OS_SIZE_T arity,
                                    CONST T * CONST *in,
                                    AMI_merge_flag * /*taken_flags*/,
                                    int &taken_index)
{
    TPIE_OS_SIZE_T ii;
    int rnum;
    
    input_arity = arity;

    tp_assert(arity > 0, "Input arity is 0.");
    
    if (mheap != NULL) {
        mheap->deallocate();
        delete mheap;
    }
    mheap = new merge_heap_op<int>();
    mheap->allocate(arity);

    // Insert an element with random priority for each non-empty stream.
    for (ii = arity; ii--; ) {
        if (in[ii] != NULL) {
            rnum=TPIE_OS_RANDOM();
            mheap->insert(&rnum,ii);
        }
    }
    
#if DEBUG_ASSERTIONS
    input_count = output_count = 0;
#endif    

    taken_index = -1;
    return AMI_ERROR_NO_ERROR;
}

template<class T>
AMI_err merge_random<T>::operate(CONST T * CONST *in,
                                 AMI_merge_flag * /*taken_flags*/,
                                 int &taken_index, T *out)
{
    
    // If the queue is empty, we are done.  There should be no more
    // inputs.
    if (!mheap->sizeofheap()) {

#if DEBUG_ASSERTIONS
        TPIE_OS_SIZE_T ii;
        
        for (ii = input_arity; ii--; ) {
            tp_assert(in[ii] == NULL, "Empty queue but more input.");
        }

        tp_assert(input_count == output_count,
                  "Merge done, input_count = " << input_count <<
                  ", output_count = " << output_count << '.');
#endif        

        return AMI_MERGE_DONE;

    } else {
        TPIE_OS_SIZE_T min_source;
        int min;

        mheap->extract_min(min,min_source);

        // If there is something in the stream of lowest priority,
        // then send it to the output and put a new element with the
        // same source and a random priority into the tree.

        if (in[min_source] != NULL) {
            *out = *(in[min_source]);
            min=TPIE_OS_RANDOM();
            mheap->insert(&min,min_source);
            taken_index = min_source;
            return AMI_MERGE_OUTPUT;
        } else {
            taken_index = -1;
            return AMI_MERGE_CONTINUE;
        }
    }
}




// Randomly shuffle a small file in main memory.

template<class T>
AMI_err merge_random<T>::main_mem_operate(T* mm_stream,
                                          size_t len)
{
    size_t ii;
    T temp;
    int rand_index;
    
    for (ii = 0; ii < len - 1; ii++) {
        rand_index = ii + (TPIE_OS_RANDOM() % (len - ii));
        temp = mm_stream[ii];
        mm_stream[ii] = mm_stream[rand_index];
        mm_stream[rand_index] = temp;
    }
    return AMI_ERROR_NO_ERROR;
}

template<class T>
size_t merge_random<T>::space_usage_overhead(void)
{
    return sizeof(*this);
}

template<class T>
size_t merge_random<T>::space_usage_per_stream(void)
{
    return sizeof(int) + sizeof(TPIE_OS_SIZE_T);
}

#endif // _MERGE_RANDOM_H 
