// Copyright (c) 1994 Darren Erik Vengroff
//
// File: ami_sort_single.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 9/28/94
//
// $Id: ami_sort_single.h,v 1.10 1999-06-18 18:06:16 rajiv Exp $
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

#include <pqueue_heap.h>

#include <ami_merge.h>
#include <ami_optimized_merge.h>


// A class of merge objects for merge sorting objects of type T.  We
// will actually use one of two subclasses of this class which use
// either a comparison function or the binary comparison operator <.

template <class T>
class merge_sort_manager : public AMI_merge_base<T> {
private:
    arity_t input_arity;
    bool use_operator;
#if DEBUG_ASSERTIONS
    unsigned int input_count, output_count;
#endif
protected:
    virtual pqueue_heap<arity_t,T> *new_pqueue(arity_t arity) = 0;
    pqueue_heap<arity_t,T> *pq;
public:
    merge_sort_manager(void);
    virtual ~merge_sort_manager(void);
    AMI_err initialize(arity_t arity, CONST T * CONST *in,
                       AMI_merge_flag *taken_flags,
                       int &taken_index);
    AMI_err operate(CONST T * CONST *in, AMI_merge_flag *taken_flags,
                    int &taken_index, T *out);
    virtual AMI_err main_mem_operate(T* mm_stream, size_t len) = 0;
    virtual size_t space_usage_overhead(void) = 0;
    size_t space_usage_per_stream(void);
};


template<class T>
merge_sort_manager<T>::merge_sort_manager(void)
{
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

    bool pqret;
    
    tp_assert(arity > 0, "Input arity is 0.");
    
    if (pq != NULL) {
        delete pq;
        pq = NULL;
    }
    new_pqueue(arity);
    
#if DEBUG_ASSERTIONS
    input_count = output_count = 0;
#endif    
    for (ii = arity; ii--; ) {
        if (in[ii] != NULL) {
            taken_flags[ii] = 1;
            pqret = pq->insert(ii,*in[ii]);
            tp_assert(pqret, "pq->insert() failed.");
#if DEBUG_ASSERTIONS
            input_count++;
#endif                  
        } else {
            taken_flags[ii] = 0;
        }
    }

    taken_index = -1;
    return AMI_MERGE_READ_MULTIPLE;
}

template<class T>
size_t merge_sort_manager<T>::space_usage_per_stream(void)
{
    return sizeof(arity_t) + sizeof(T);
}

template<class T>
AMI_err merge_sort_manager<T>::operate(CONST T * CONST *in,
                                       AMI_merge_flag */*taken_flags*/,
                                       int &taken_index,
                                       T *out)
{
    bool pqret;
    
    // If the queue is empty, we are done.  There should be no more
    // inputs.
    if (!pq->num_elts()) {

#if DEBUG_ASSERTIONS
        arity_t ii;
        
        for (ii = input_arity; ii--; ) {
            tp_assert(in[ii] == NULL, "Empty queue but more input.");
        }

        tp_assert(input_count == output_count,
                  "Merge done, input_count = " << input_count <<
                  ", output_count = " << output_count << '.');
#endif        

        // Delete the queue, which may take up a lot of main memory.
        tp_assert(pq != NULL, "pq == NULL");
        delete pq;
        pq = NULL;
        
        return AMI_MERGE_DONE;

    } else {
        arity_t min_source;
        T min_t;

        pqret = pq->extract_min(min_source,min_t);
        tp_assert(pqret, "pq->extract_min() failed.");
        *out = min_t;
        if (in[min_source] != NULL) {
            pqret = pq->insert(min_source,*in[min_source]);
            tp_assert(pqret, "pq->insert() failed.");
            taken_index = min_source;
            //taken_flags[min_source] = 1;
#if DEBUG_ASSERTIONS
            input_count++;
#endif            
        } else {
            taken_index = -1;
        }
#if DEBUG_ASSERTIONS
        output_count++;
#endif        
        return AMI_MERGE_OUTPUT;
    }
}


// Operator based merge sort manager.

template <class T>
class merge_sort_manager_op : public merge_sort_manager<T> {
private:
    pqueue_heap<arity_t,T> *new_pqueue(arity_t arity);
public:
    merge_sort_manager_op(void);    
    virtual ~merge_sort_manager_op(void);    
    AMI_err main_mem_operate(T* mm_stream, size_t len);
    size_t space_usage_overhead(void);
};    

template<class T>
merge_sort_manager_op<T>::merge_sort_manager_op(void)
{
    pq = NULL;
}

template<class T>
pqueue_heap<arity_t,T> *merge_sort_manager_op<T>::new_pqueue(arity_t arity)
{
    return pq = new pqueue_heap_op<arity_t,T>(arity);
}

template<class T>
merge_sort_manager_op<T>::~merge_sort_manager_op(void)
{
}

template<class T>
AMI_err merge_sort_manager_op<T>::main_mem_operate(T* mm_stream, size_t len)
{
    quicker_sort_op(mm_stream, len);
    return AMI_ERROR_NO_ERROR;
}

template<class T>
size_t merge_sort_manager_op<T>::space_usage_overhead(void)
{
    return sizeof(pqueue_heap_op<arity_t,T>);
}



// Comparison object based merge sort manager.

template <class T>
class merge_sort_manager_obj : public merge_sort_manager<T> {
private:
    comparator<T> *cmp_o;
    pqueue_heap<arity_t,T> *new_pqueue(arity_t arity);
public:
    merge_sort_manager_obj(comparator<T> *cmp);
    virtual ~merge_sort_manager_obj(void);    
    AMI_err main_mem_operate(T* mm_stream, size_t len);
    size_t space_usage_overhead(void);
};   

template<class T>
merge_sort_manager_obj<T>::merge_sort_manager_obj(comparator<T> *cmp)
{
    cmp_o = cmp;
    pq = NULL;
}

template<class T>
pqueue_heap<arity_t,T> *merge_sort_manager_obj<T>::new_pqueue(arity_t arity)
{
    return pq = new pqueue_heap_obj<arity_t,T>(arity,cmp_o);
}

template<class T>
merge_sort_manager_obj<T>::~merge_sort_manager_obj(void)
{
}


template<class T>
AMI_err merge_sort_manager_obj<T>::main_mem_operate(T* mm_stream, size_t len)
{
    quicker_sort_obj(mm_stream, len, cmp_o);
    return AMI_ERROR_NO_ERROR;
}

template<class T>
size_t merge_sort_manager_obj<T>::space_usage_overhead(void)
{
    return sizeof(pqueue_heap_obj<arity_t,T>);
}



// Comparison function based merge sort manager.

template <class T>
class merge_sort_manager_cmp : public merge_sort_manager<T> {
private:
    int (*cmp_f)(CONST T&, CONST T&);
    pqueue_heap<arity_t,T> *new_pqueue(arity_t arity);
public:
    merge_sort_manager_cmp(int (*cmp)(CONST T&, CONST T&));
    virtual ~merge_sort_manager_cmp(void);    
    AMI_err main_mem_operate(T* mm_stream, size_t len);
    size_t space_usage_overhead(void);
};   


template<class T>
merge_sort_manager_cmp<T>::merge_sort_manager_cmp(int (*cmp)(CONST T&,
                                                             CONST T&))
{
    cmp_f = cmp;
    pq = NULL;
}

template<class T>
pqueue_heap<arity_t,T> *merge_sort_manager_cmp<T>::new_pqueue(arity_t arity)
{
    return pq = new pqueue_heap_cmp<arity_t,T>(arity,cmp_f);
}


template<class T>
merge_sort_manager_cmp<T>::~merge_sort_manager_cmp(void)
{
}


template<class T>
AMI_err merge_sort_manager_cmp<T>::main_mem_operate(T* mm_stream, size_t len)
{
    quicker_sort_cmp(mm_stream, len, cmp_f);
    return AMI_ERROR_NO_ERROR;
}

template<class T>
size_t merge_sort_manager_cmp<T>::space_usage_overhead(void)
{
    return sizeof(pqueue_heap_cmp<arity_t,T>);
}


// The actual sort calls.

template<class T>
AMI_err AMI_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
                 int (*cmp)(CONST T&, CONST T&))
{
    merge_sort_manager_cmp<T> msm(cmp);

    return AMI_partition_and_merge(instream, outstream,
                                   (AMI_merge_base<T> *)&msm);
}


template<class T>
AMI_err AMI_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream)
{
    merge_sort_manager_op<T> msm;

    return AMI_partition_and_merge(instream, outstream,
                                   (AMI_merge_base<T> *)&msm);
}


template<class T>
AMI_err AMI_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
                 comparator<T> *cmp)
{
    merge_sort_manager_obj<T> msm(cmp);

    return AMI_partition_and_merge(instream, outstream,
                                   (AMI_merge_base<T> *)&msm);
}
                          
#endif // _AMI_SORT_SINGLE_H 
