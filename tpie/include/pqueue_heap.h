// Copyright (c) 1994 Darren Erik Vengroff
//
// File: pqueue_heap.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/4/94
//
// $Id: pqueue_heap.h,v 1.1 1994-10-04 19:02:53 darrenv Exp $
//
// A priority queue class implemented as a binary heap.
//
#ifndef _PQUEUE_HEAP_H
#define _PQUEUE_HEAP_H

// Helper functions for navigating through a binary heap.

// The children of an element of the heap.
    
static inline unsigned int lchild(unsigned int index) {
    return 2 * index;
}

static inline unsigned int rchild(unsigned int index) {
    return 2 * index + 1;
}

// The parent of an element.

static inline unsigned int parent(unsigned int index) {
    return index >> 1;
}


template <class T, class P> class pqueue_heap
{
private:
    // A pointer to the array of elements and their priorities.
    struct q_elt {
        T elt;
        P priority;
    } * elements;

    // The number currently in the queue.

    unsigned int cur_elts;

    // The maximum number the queue can hold.

    unsigned int max_elts;

    // A pointer to the function used to compare the priorities of
    // elements.
    
    int (*cmp_f)(const P&, const P&);

    void heapify(unsigned int root);

public:
    // At construction time, initialize to a given size and set the
    // comparison function.
    
    pqueue_heap(unsigned int size, int (*cmp)(const P&, const P&));  

    ~pqueue_heap();

    // Is it full?
    bool full(void);

    // How many elements?
    unsigned int num_elts(void);

    // Insert
    bool insert(const T& elt, const P& prio);

    // Min
    void min(T& elt, P& prio);
    
    // Extract min.
    bool extract_min(T& elt, P& prio);
};

template <class T, class P>
pqueue_heap<T,P>::pqueue_heap(unsigned int size,
                              int (*cmp)(const P&, const P&))
{
    elements = new q_elt[max_elts = size];
    cur_elts = 0;
    cmp_f = cmp;
}

template <class T, class P>
pqueue_heap<T,P>::~pqueue_heap() {
    delete elements;
    cur_elts = 0;
    max_elts = 0;
    return;
}

template <class T, class P>
bool pqueue_heap<T,P>::full(void) {
    return cur_elts == max_elts;
}

template <class T, class P>
unsigned int pqueue_heap<T,P>::num_elts(void) {
    return cur_elts;
}

template <class T, class P>
bool pqueue_heap<T,P>::insert(const T& elt, const P& prio) {
    unsigned int ii;
    
    if (full()) {
        return false;
    }

    for (ii = cur_elts++;
         ii && (cmp_f(elements[parent(ii)].priority, prio) > 0);
         ii = parent(ii)) {
        elements[ii] = elements[parent(ii)];
    }
    elements[ii].priority = prio;
    elements[ii].elt = elt;

    return true;
}                                       
    
template <class T, class P>
void pqueue_heap<T,P>::min(T& elt, P& prio) {
    elt = elements->elt;
    prio = elements->priority;
}
    
template <class T, class P>
bool pqueue_heap<T,P>::extract_min(T& elt, P& prio) {
    if (!cur_elts) {
        return false;
    }
    elt = elements->elt;
    prio = elements->priority;
    elements[0] = elements[--cur_elts];
    heapify(0);

    return true;
}

template <class T, class P>
void pqueue_heap<T,P>::heapify(unsigned int root) {
    unsigned int min_index = root;
    unsigned int lc = lchild(root);
    unsigned int rc = rchild(root);
    
    if ((lc < cur_elts) && (cmp_f(elements[lc].priority,
                                  elements[min_index].priority) < 0)) {
        min_index = lc;
    }
    if ((rc < cur_elts) && (cmp_f(elements[rc].priority,
                                  elements[min_index].priority) < 0)) {
        min_index = rc;
    }

    if (min_index != root) {
        q_elt tmp_q = elements[min_index];

        elements[min_index] = elements[root];
        elements[root] = tmp_q;

        heapify(min_index);
    }
}   

#endif // _PQUEUE_HEAP_H 
