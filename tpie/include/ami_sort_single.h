// Copyright (c) 1994 Darren Erik Vengroff
//
// File: ami_sort_single.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 9/28/94
//
// $Id: ami_sort_single.h,v 1.2 1994-10-04 19:08:05 darrenv Exp $
//
// Merge sorting for the AMI_IMP_SINGLE implementation.
//
#ifndef _AMI_SORT_SINGLE_H
#define _AMI_SORT_SINGLE_H

#ifndef AMI_IMP_SINGLE
#warning Including __FILE__ when AMI_IMP_SINGLE undefined.
#endif

// For use in core by main_mem_operate().
#include <quicksort.h>

// For the priority queue to do the merge.
#define SORT_PQUEUE_HEAP
#ifdef SORT_PQUEUE_HEAP
#include <pqueue_heap.h>
#define PQUEUE pqueue_heap
#else
#include <pqueue.h>
#define PQUEUE pqueue
#endif

// A class of merge objects for merge sorting objects of type T.

template <class T>
class merge_sort_manager : public AMI_merge_base<T> {
private:
    int (*cmp_f)(CONST T&, CONST T&);
    arity_t input_arity;
    PQUEUE<arity_t,T> *pq;
public:
    merge_sort_manager(int (*cmp)(CONST T&, CONST T&));
    virtual ~merge_sort_manager(void);
    AMI_err initialize(arity_t arity, CONST T * CONST *in,
                       AMI_merge_flag *taken_flags,
                       int &taken_index);
    AMI_err operate(CONST T * CONST *in, AMI_merge_flag *taken_flags,
                    int &taken_index, T *out);
    AMI_err main_mem_operate(T* mm_stream, size_t len);
    size_t space_usage_overhead(void);
    size_t space_usage_per_stream(void);
};


template<class T>
merge_sort_manager<T>::merge_sort_manager(int (*cmp)(CONST T&, CONST T&))
{
    cmp_f = cmp;
    pq = NULL;
}

template<class T>
merge_sort_manager<T>::~merge_sort_manager(void)
{
    if (pq != NULL) {
        delete pq;
    }
}

template<class T>
AMI_err merge_sort_manager<T>::initialize(arity_t arity, CONST T * CONST *in,
                                          AMI_merge_flag *taken_flags,
                                          int &taken_index)
{
    arity_t ii;

    input_arity = arity;

    tp_assert(arity > 0, "Input arity is 0.");
    
    if (pq != NULL) {
        delete pq;
    }
    pq = new PQUEUE<arity_t,T>(arity,cmp_f);

    for (ii = arity; ii--; ) {
        if (in[ii] != NULL) {
            taken_flags[ii] = 1;
            pq->insert(ii,*in[ii]);
        } else {
            taken_flags[ii] = 0;
        }
    }

    taken_index = -1;
    return AMI_MERGE_READ_MULTIPLE;
}

template<class T>
size_t merge_sort_manager<T>::space_usage_overhead(void)
{
    return sizeof(PQUEUE<arity_t,T>);
}

template<class T>
size_t merge_sort_manager<T>::space_usage_per_stream(void)
{
    return sizeof(arity_t) + sizeof(T);
}

template<class T>
AMI_err merge_sort_manager<T>::operate(CONST T * CONST *in,
                                       AMI_merge_flag *taken_flags,
                                       int &taken_index,
                                       T *out)
{
    // If the queue is empty, we are done.  There should be no more
    // inputs.
    if (!pq->num_elts()) {

#if DEBUG_ASSERTIONS
        arity_t ii;
        
        for (ii = input_arity; ii--; ) {
            tp_assert(in[ii] == NULL, "Empty queue but more input.");
        }
#endif        

        return AMI_MERGE_DONE;

    } else {
        arity_t min_source;
        T min_t;

        pq->extract_min(min_source,min_t);
        *out = min_t;
        if (in[min_source] != NULL) {
            pq->insert(min_source,*in[min_source]);
            taken_index = min_source;
            //taken_flags[min_source] = 1;
        }
        return AMI_MERGE_OUTPUT;
    }
}

template<class T>
AMI_err merge_sort_manager<T>::main_mem_operate(T* mm_stream, size_t len)
{
    quicker_sort(mm_stream, len, cmp_f);
    return AMI_ERROR_NO_ERROR;
}




template<class T>
AMI_err AMI_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
                 int (*cmp)(CONST T&, CONST T&))
{
    merge_sort_manager<T> msm(cmp);

    return AMI_partition_and_merge(instream, outstream, &msm);
}



#endif // _AMI_SORT_SINGLE_H 



