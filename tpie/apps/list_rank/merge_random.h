// Copyright (c) 1994 Darren Erik Vengroff
//
// File: merge_random.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/5/94
//
// $Id: merge_random.h,v 1.7 1999-02-03 21:53:41 tavi Exp $
//
// A merge managment object that reorders the input stream in a random
// way.

#ifndef _MERGE_RANDOM_H
#define _MERGE_RANDOM_H

#if 0
extern "C" void srandom(unsigned int);
// Linux defines this random as a macro.
#ifndef random
extern "C" long int random(void);
#endif
#endif

template<class T>
class merge_random : public AMI_merge_base<T> {
private:
    arity_t input_arity;
    pqueue_heap_op<arity_t,int> *pq;
#if DEBUG_ASSERTIONS
    unsigned int input_count, output_count;
#endif    
public:
    merge_random(int seed = 0);
    virtual ~merge_random(void);
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
merge_random<T>::merge_random(int seed)
{
    if (seed) {
        srandom(seed);
    }
    pq = NULL;
}

template<class T>
merge_random<T>::~merge_random(void)
{
    if (pq != NULL) {
        delete pq;
    }
}

template<class T>
AMI_err merge_random<T>::initialize(arity_t arity,
                                    CONST T * CONST *in,
                                    AMI_merge_flag */*taken_flags*/,
                                    int &taken_index)
{
    arity_t ii;

    input_arity = arity;

    tp_assert(arity > 0, "Input arity is 0.");
    
    if (pq != NULL) {
        delete pq;
    }
    pq = new pqueue_heap_op<arity_t,int>(arity);

    // Insert an element with random priority for each non-empty stream.
    for (ii = arity; ii--; ) {
        if (in[ii] != NULL) {
            pq->insert(ii,random());
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
                                 AMI_merge_flag */*taken_flags*/,
                                 int &taken_index, T *out)
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

        return AMI_MERGE_DONE;

    } else {
        arity_t min_source;
        int min;

        pqret = pq->extract_min(min_source, min);
        tp_assert(pqret, "pq->extract_min() failed.");

        // If there is something in the stream of lowest priority,
        // then send it to the output and put a new element with the
        // same source and a random priority into the tree.

        if (in[min_source] != NULL) {
            *out = *(in[min_source]);
            pqret = pq->insert(min_source, random());
            tp_assert(pqret, "pq->insert() failed.");
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
        rand_index = ii + (random() % (len - ii));
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
    return sizeof(int) + sizeof(arity_t);
}

#endif // _MERGE_RANDOM_H 
