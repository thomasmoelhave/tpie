// Copyright (c) 1994 Darren Erik Vengroff
//
// File: pqueue_heap.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/4/94
//
// $Id: pqueue_heap.h,v 1.2 1994-10-10 13:10:20 darrenv Exp $
//
// A priority queue class implemented as a binary heap.
//
#ifndef _PQUEUE_HEAP_H
#define _PQUEUE_HEAP_H


// The virtual base class that defines what priority queues must do.
template <class T, class P>
class pqueue
{
public:
    // Is it full?
    virtual bool full(void) = 0;

    // How many elements?
    virtual unsigned int num_elts(void) = 0;

    // Insert
    virtual bool insert(const T& elt, const P& prio) = 0;

    // Min
    virtual void min(T& elt, P& prio) = 0;
    
    // Extract min.
    virtual bool extract_min(T& elt, P& prio) = 0;
};


#if 0
template<class T>
int op_compare(const T& t1, const T& t2)
{
    return ((t1 < t2) ? -1 :
            (t1 > t2) ? 1 : 0);
};
#endif


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

// A base class for priority queues that use heaps.

template <class T, class P>
class pqueue_heap : public pqueue<T,P>
{
protected:
    // A pointer to the array of elements and their priorities.
    struct q_elt {
        T elt;
        P priority;
    } * elements;

    // The number currently in the queue.

    unsigned int cur_elts;

    // The maximum number the queue can hold.

    unsigned int max_elts;

    // Fix up the heap after a deletion.
    
    virtual void heapify(unsigned int root) = 0;

public:
    pqueue_heap(unsigned int size);

    virtual ~pqueue_heap();

    // Is it full?
    bool full(void);

    // How many elements?
    unsigned int num_elts(void);

    // Min
    void min(T& elt, P& prio);

    // Extract min.
    bool extract_min(T& elt, P& prio);
};


template <class T, class P>
pqueue_heap<T,P>::pqueue_heap(unsigned int size)
{
    elements = new q_elt[max_elts = size];
    cur_elts = 0;
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


// A priority queue that uses a comparison function for comparing
// priorities.
template <class T, class P>
class pqueue_heap_cmp : public pqueue_heap<T,P>
{
private:
    // A pointer to the function used to compare the priorities of
    // elements.
    
    int (*cmp_f)(const P&, const P&);

    void heapify(unsigned int root);

public:
    pqueue_heap_cmp(unsigned int size, int (*cmp)(const P&, const P&));
    virtual ~pqueue_heap_cmp(void) {};

    // Insert
    bool insert(const T& elt, const P& prio);
};


template <class T, class P>
pqueue_heap_cmp<T,P>::pqueue_heap_cmp(unsigned int size,
                                      int (*cmp)(const P&, const P&)) :
                                              pqueue_heap<T,P>(size)

{
    cmp_f = cmp;
}


template <class T, class P>
bool pqueue_heap_cmp<T,P>::insert(const T& elt, const P& prio) {
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
void pqueue_heap_cmp<T,P>::heapify(unsigned int root) {
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



// A priority queue that uses the builtin operator < for comparing
// priorities instead of a comparison function.

template <class T, class P>
class pqueue_heap_op : public pqueue_heap<T,P>
{
private:
    void heapify(unsigned int root);
    
public:
    pqueue_heap_op(unsigned int size);
    virtual ~pqueue_heap_op(void) {};

    // Insert
    bool insert(const T& elt, const P& prio);
};


template <class T, class P>
pqueue_heap_op<T,P>::pqueue_heap_op(unsigned int size) :
        pqueue_heap<T,P>(size)
{
}


template <class T, class P>
bool pqueue_heap_op<T,P>::insert(const T& elt, const P& prio) {
    unsigned int ii;
    
    if (full()) {
        return false;
    }

    for (ii = cur_elts++;
         ii && (elements[parent(ii)].priority > prio);
         ii = parent(ii)) {
        elements[ii] = elements[parent(ii)];
    }
    elements[ii].priority = prio;
    elements[ii].elt = elt;

    return true;
}                                       

template <class T, class P>
void pqueue_heap_op<T,P>::heapify(unsigned int root) {
    unsigned int min_index = root;
    unsigned int lc = lchild(root);
    unsigned int rc = rchild(root);
    
    if ((lc < cur_elts) && (elements[lc].priority <
                            elements[min_index].priority)) {
        min_index = lc;
    }
    if ((rc < cur_elts) && (elements[rc].priority <
                            elements[min_index].priority)) {
        min_index = rc;
    }

    if (min_index != root) {
        q_elt tmp_q = elements[min_index];

        elements[min_index] = elements[root];
        elements[root] = tmp_q;

        heapify(min_index);
    }
}   




// A priority queue that simply uses an array.

template <class T, class P>
class pqueue_array : public pqueue<T,P>
{
private:
    struct Qelt{
	T thing;
	P priority;
    } * elements;                        // array of prioritized stuff
    unsigned int numelts;                // # elements in priority queue
    unsigned int size;                   // capacity of priority queue
    int (*cmp_f)(const P&, const P&);
public:
    pqueue_array(int,int (*)(const P&, const P&));  // initialize to given size
    virtual ~pqueue_array();
    bool insert(const T &, const P&);       // insert T with int priority
    unsigned int num_elts();
    bool full();
    bool extract_min(T& elt, P& prio);
    void min(T &,P &);           // return min elt and priority
};

template <class T, class P> pqueue_array<T,P>::pqueue_array(int n,
                                                int (*cmp)(const P&, const P&))
{
    elements = new Qelt[n+1];
    numelts = 0;
    size = n;
    cmp_f = cmp;
}

template <class T, class P> pqueue_array<T,P>::~pqueue_array()
{
    delete [] elements;
    size = 0;
    numelts = 0;
}

template <class T, class P> unsigned int pqueue_array<T,P>::num_elts()
{
    return numelts;
}
    
template <class T, class P> bool pqueue_array<T,P>::full()
{
    return numelts == size ? 1 : 0;
}

template <class T, class P> bool pqueue_array<T,P>::insert(const T & elt,
                                                           const P &prio)
{
    if (full()) {
        return false;
    } else {
	++numelts;
	elements[numelts].thing  = elt;
	elements[numelts].priority = prio;
        return true;
    }
}

template <class T, class P>
bool pqueue_array<T,P>::extract_min(T& elt, P& prio)
{
    if (!numelts) {
        return false;
    }

    int min=1;
    int k;
    for(k=2; k <= numelts; k++){
        if (cmp_f(elements[k].priority,elements[min].priority) < 0){
            min = k;
        }
    }
    prio = elements[min].priority;
    elt = elements[min].thing;
    for(k=min; k < numelts; k++){
        elements[k] = elements[k+1];
    }
    numelts--;
    
    return true;
}

template <class T, class P>
void pqueue_array<T,P>::min(T& elt, P& prio) 
{
    int min=1;
    int k;
    for(k=2; k <= numelts; k++){
        if (cmp_f(elements[k].priority,elements[min].priority) < 0){
            min = k;
        }
    }
    elt = elements[min].priority;
    prio = elements[min].thing;
}


#endif // _PQUEUE_HEAP_H 
