// Copyright (c) 1994 Darren Erik Vengroff
//
// File: partition_and_merge.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 9/17/94
//
// A test for AMI_partition_and_merge().


static char partition_and_merge_id[] = "$Id: test_ami_pmerge.cpp,v 1.1 1994-09-22 14:59:14 darrenv Exp $";

// Use a LEDA priority queue.
//#define MERGE_VIA_LEDA_PRIORITY_QUEUE
//#define LEDA_PQ_IMP k_heap

// Use Owen's priority queue code.
#define MERGE_VIA_TPQUEUE

#ifdef MERGE_VIA_TPQUEUE
#include <tpqueue.h>
#endif // MERGE_VIA_TPQUEUE

#ifdef MERGE_VIA_LEDA_PRIORITY_QUEUE
// Get a priority queue from LEDA
#include <LEDA/prio.h>
#ifdef LEDA_PQ_IMP
#include <LEDA/impl/k_heap.h>
#endif // LEDA_PQ_IMP
#endif // MERGE_VIA_LEDA_PRIORITY_QUEUE

// If you actually want to see all the ints in the stream.
//#define REPORT_RESULTS_RANDOM
//#define REPORT_RESULTS_OUTPUT


//#define BTE_MMB_LOGICAL_BLOCKSIZE_FACTOR 32

//#define TEST_SIZE (1024 * 256)
//#define TEST_SIZE (1024 * 1024)
#define TEST_SIZE (1024 * 1024 * 8)

//#define TEST_MM_SIZE (1024 * 128)
//#define TEST_MM_SIZE (1024 * 256)
//#define TEST_MM_SIZE (1024 * 512)
//#define TEST_MM_SIZE (1024 * 1024)
//#define TEST_MM_SIZE (1024 * 1024 * 2)
//#define TEST_MM_SIZE (1024 * 1024 * 3)
#define TEST_MM_SIZE (1024 * 1024 * 4)
//#define TEST_MM_SIZE (1024 * 1024 * 6)
//#define TEST_MM_SIZE (1024 * 1024 * 36)

#include <stdlib.h>
#include <iostream.h>

// Use logs.
#define TPL_LOGGING 1
#include <tpie_log.h>

// Use the single BTE stream version of AMI streams.
#define AMI_IMP_SINGLE

// Pick a version of BTE streams.
//#define BTE_IMP_MMB
#define BTE_IMP_CACHE
//#define BTE_IMP_STDIO
//#define BTE_IMP_UFS

// Define it all.
#include <ami.h>


#if defined(REPORT_RESULTS_RANDOM) || defined(REPORT_RESULTS_OUTPUT)
#include <ami_scan_utils.h>
#endif //REPORT_RESULTS


// My very own new and delete operators.  The idea is that these should be
// integrated into the memory manager.

static int register_new = 0;

void * operator new (size_t sz) {
    void *p;
    
    if ((register_new) && (MM_manager.register_allocation(sz+sizeof(size_t))
                           != MM_ERROR_NO_ERROR)) {
        return (void *)0;
    }
    
    p = malloc(sz + sizeof(size_t));
    *((size_t *)p) = sz;
    return ((size_t *)p) + 1;
}

void operator delete (void *ptr) {
    if (register_new) {
        MM_manager.register_deallocation(*((size_t *)ptr - 1) +
                                         sizeof(size_t));
    }
    free(((char *)ptr) - sizeof(size_t));
}
    


// random_scan should be moved into it's own source file since it is
// so commonly used.  Perhaps it should be added to a tpie utility
// library.

extern "C" int srandom(int);
extern "C" int random(void);

// A scan object to generate random integers.
class random_scan : AMI_scan_object {
private:
    unsigned int max, remaining;
public:
    random_scan(unsigned int count = 1000, int seed = 17);
    virtual ~random_scan(void);
    AMI_err initialize(void);
    AMI_err operate(int *out1, AMI_SCAN_FLAG *sf);
};

random_scan::random_scan(unsigned int count, int seed) :
max(count), remaining(count)
{
    LOG_INFO("random_scan seed = ");
    LOG_INFO(seed);
    LOG_INFO('\n');

    srandom(seed);
}

random_scan::~random_scan(void)
{
}


AMI_err random_scan::initialize(void)
{
    remaining = max;

    return AMI_ERROR_NO_ERROR;
};

AMI_err random_scan::operate(int *out1, AMI_SCAN_FLAG *sf)
{
    if (*sf = remaining--) {
        *out1 = random();
        return AMI_SCAN_CONTINUE;
    } else {
        return AMI_SCAN_DONE;
    }
};


#ifdef MERGE_VIA_LEDA_PRIORITY_QUEUE
// Basic operations the priority queue needs.
inline void Clear(arity_t)    {}
inline GenPtr Copy(arity_t x)    { return *(GenPtr*)&x; }
inline void Print(arity_t x, ostream& out = cout)         { out << x; }
inline GenPtr Convert(arity_t x)      { return GenPtr(x); }
#endif // MERGE_VIA_LEDA_PRIORITY_QUEUE


// A merge object to merge sorted streams.

class sort_merge : public AMI_merge_base<int> {
private:
    arity_t input_arity;
#ifdef MERGE_VIA_LEDA_PRIORITY_QUEUE
#ifdef LEDA_PQ_IMP
    _priority_queue<arity_t,int,LEDA_PQ_IMP> pq;
#else
    priority_queue<arity_t,int> pq;
#endif
#endif // MERGE_VIA_LEDA_PRIORITY_QUEUE

#ifdef MERGE_VIA_TPQUEUE
    PQueue<arity_t> *pq;
#endif // MERGE_VIA_TPQUEUE
    
public:
    unsigned long int called;

    sort_merge(void);
    ~sort_merge(void);
    AMI_err initialize(arity_t arity, int **in, AMI_merge_flag *taken_flags);
    AMI_err operate(const int **in, AMI_merge_flag *taken_flags, int *out);
    AMI_err main_mem_operate(int* mm_stream, size_t len);

    size_t space_usage_overhead(void);
    size_t space_usage_per_stream(void);
};

sort_merge::sort_merge(void)
{
#ifdef MERGE_VIA_TPQUEUE
    pq = NULL;
#endif // MERGE_VIA_TPQUEUE    
}

sort_merge::~sort_merge(void)
{
#ifdef MERGE_VIA_TPQUEUE
    if (pq != NULL) {
        delete pq;
    }
#endif // MERGE_VIA_TPQUEUE    
}


size_t sort_merge::space_usage_overhead(void)
{
#ifdef MERGE_VIA_TPQUEUE    
    return sizeof(PQueue<arity_t>);
#else
    return 0;
#endif // MERGE_VIA_TPQUEUE    
}

size_t sort_merge::space_usage_per_stream(void)
{
    // How much space will the priority queue use per item?

    return sizeof(arity_t) + sizeof(int);
}


AMI_err sort_merge::initialize(arity_t arity, int **in,
                               AMI_merge_flag *taken_flags)
{
    unsigned int ii;

    called = 0;
    
    input_arity = arity;

    tp_assert(arity > 0, "Input arity is 0.");
    
#ifdef MERGE_VIA_LEDA_PRIORITY_QUEUE
#ifdef LEDA_PQ_IMP
    pq._priority_queue<arity_t,int,LEDA_PQ_IMP>::clear();
#else
    pq.clear();
#endif
#endif

#ifdef MERGE_VIA_TPQUEUE
    if (pq != NULL) {
        delete pq;
    }
    pq = new PQueue<arity_t>(arity);
#endif // MERGE_VIA_TPQUEUE

#if defined(MERGE_VIA_TPQUEUE) || defined(MERGE_VIA_LEDA_PRIORITY_QUEUE)    
    for (ii = arity; ii--; ) {
        if (in[ii] != NULL) {
            taken_flags[ii] = 1;
#ifdef MERGE_VIA_TPQUEUE
            pq->Insert(ii,*in[ii]);
#else            
            pq.insert(ii,*in[ii]);
#endif            
        } else {
            taken_flags[ii] = 0;
        }
    }
#else     
    for (ii = arity; ii--; ) {
        taken_flags[ii] = 0;
    }
#endif // MERGE_VIA_LEDA_PRIORITY_QUEUE

    return AMI_ERROR_NO_ERROR;
}


AMI_err sort_merge::operate(const int **in, AMI_merge_flag *taken_flags,
                            int *out)
{
    int ii, ii_min;
    int min;

    called++;
    
#ifdef MERGE_VIA_LEDA_PRIORITY_QUEUE
    // If the queue is empty, we are done.  There should be no more
    // inputs.
    if (pq.empty()) {
#if DEBUG_ASSERTIONS        
        for (ii = input_arity; ii--; ) {
            tp_assert(in[ii] == NULL, "Empty queue but more input.");
        }
#endif        
        return AMI_MERGE_DONE;
    } else {
        pq_item it = pq.find_min();
        *out = pq.inf(it);
        ii_min = pq.key(it);
        pq.del_item(it);
        if (in[ii_min] != NULL) {
            pq.insert(ii_min,*in[ii_min]);
            taken_flags[ii_min] = 1;
        }
        return AMI_MERGE_OUTPUT;
    }
#elif defined(MERGE_VIA_TPQUEUE)
    // If the queue is empty, we are done.  There should be no more
    // inputs.
    if (!pq->NumElts()) {
#if DEBUG_ASSERTIONS        
        for (ii = input_arity; ii--; ) {
            tp_assert(in[ii] == NULL, "Empty queue but more input.");
        }
#endif        
        return AMI_MERGE_DONE;
    } else {
        arity_t min_source;
        int min_val;

        pq->MinElt(min_source,min_val);
        pq->DeleteMin();
        *out = min_val;
        if (in[min_source] != NULL) {
            pq->Insert(min_source,*in[min_source]);
            taken_flags[min_source] = 1;
        }
        return AMI_MERGE_OUTPUT;
    }
#else    
    tp_assert(input_arity > 0, "Input arity is 0.");

    // Find an input to start with.
    for (ii = input_arity; ii--; ) {
        if (in[ii] != NULL) {
            min = *(in[ii]);
            ii_min = ii;
            break;
        }
    }

    // Did we get one?  If not, then we are done.
    
    if (ii < 0) {
        return AMI_MERGE_DONE;
    }

    // Now look through the rest to see if any are smaller

    while (ii--) {
        if ((in[ii] != NULL) && (*(in[ii]) < min)) {
            min = *(in[ii]);
            ii_min = ii;
        }
    }

    *out = min;
    for (ii = input_arity; ii--; ) {
        taken_flags[ii] = (ii_min == ii);
    }

    return AMI_MERGE_OUTPUT;
#endif // !MERGE_VIA_LEDA_PRIORITY_QUEUE
}


extern "C" int int_cmp(const void *p1, const void *p2);

AMI_err sort_merge::main_mem_operate(int* mm_stream, size_t len)
{
    qsort(mm_stream, len, sizeof(int), int_cmp);
    return AMI_ERROR_NO_ERROR;
}




int main(int argc, char **argv)
{
    AMI_err ae;

    // Set the amount of main memory:
    MM_manager.resize_heap(TEST_MM_SIZE);
    register_new = 1;
        
    AMI_STREAM<int> amis0((unsigned int)0, TEST_SIZE);
    AMI_STREAM<int> amis1((unsigned int)0, TEST_SIZE);
        
    // Write some ints.
    random_scan rnds(TEST_SIZE);
    
    ae = AMI_scan(&rnds, (AMI_base_stream<int> *)&amis0);

    cout << "Wrote the random values.\n";

    cout << "Stream length = " << amis0.stream_len() << '\n';

#ifdef REPORT_RESULTS_RANDOM
    ofstream osr("/var/tmp/osr.ascii");
    cxx_ostream_scan<int> rptr(&osr);
#endif
#ifdef REPORT_RESULTS_OUTPUT
    ofstream oss("/var/tmp/oss.ascii");
    cxx_ostream_scan<int> rpts(&oss);
#endif    
    
#ifdef REPORT_RESULTS_RANDOM
    ae = AMI_scan((AMI_base_stream<int> *)&amis0, &rptr);
#endif // REPORT_RESULTS_RANDOM
    
    sort_merge sm;

    ae = AMI_partition_and_merge(&amis0, &amis1,
                                 (AMI_merge_base<int> *)&sm);


    cout << "Sorted them.\n";

    cout << "Sorted stream length = " << amis1.stream_len() << '\n';

    cout << "sm.operate called " << sm.called << " times.\n";
    
#ifdef REPORT_RESULTS_OUTPUT
    ae = AMI_scan((AMI_base_stream<int> *)&amis1, &rpts);    
#endif // REPORT_RESULTS_OUTPUT

    cout << '\n';
    
    return 0;
}
    
