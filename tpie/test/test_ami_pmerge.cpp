// Copyright (c) 1994 Darren Erik Vengroff
//
// File: partition_and_merge.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 9/17/94
//
// A test for AMI_partition_and_merge().


static char partition_and_merge_id[] = "$Id: test_ami_pmerge.cpp,v 1.2 1994-09-26 19:22:03 darrenv Exp $";

// Use Owen's priority queue code.
#define MERGE_VIA_TPQUEUE

#ifdef MERGE_VIA_TPQUEUE
#include <tpqueue.h>
#endif // MERGE_VIA_TPQUEUE


#ifdef BTE_IMP_MMB
#define BTE_MMB_LOGICAL_BLOCKSIZE_FACTOR 4
#endif

#ifdef BTE_IMP_CACHE
#define BTE_MMB_CACHE_LINE_SIZE 256
#endif

#define DEFAULT_TEST_SIZE (1024 * 1024 * 8)

#define DEFAULT_TEST_MM_SIZE (1024 * 1024 * 2)


#include <stdlib.h>
#include <sys/resource.h>
#include <iostream.h>
#include <strstream.h>

#include <GetOpt.h>

// Use logs.
//#define TPL_LOGGING 1
#include <tpie_log.h>

// Use the single BTE stream version of AMI streams.
#define AMI_IMP_SINGLE

// Pick a version of BTE streams.
#define BTE_IMP_MMB
//#define BTE_IMP_CACHE
//#define BTE_IMP_STDIO
//#define BTE_IMP_UFS

// Define it all.
#include <ami.h>


// Utitlities for ascii output.
#include <ami_scan_utils.h>


// Some defaults.
static size_t test_mm_size = DEFAULT_TEST_MM_SIZE;
static size_t test_size = DEFAULT_TEST_SIZE;
static int random_seed = 17;


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



// A merge object to merge sorted streams.

class sort_merge : public AMI_merge_base<int> {
private:
    arity_t input_arity;
#ifdef MERGE_VIA_TPQUEUE
    PQueue<arity_t> *pq;
#endif // MERGE_VIA_TPQUEUE
    
public:
    unsigned long int called;

    sort_merge(void);
    virtual ~sort_merge(void);
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
    
#ifdef MERGE_VIA_TPQUEUE
    if (pq != NULL) {
        delete pq;
    }
    pq = new PQueue<arity_t>(arity);
#endif // MERGE_VIA_TPQUEUE

#ifdef MERGE_VIA_TPQUEUE
    for (ii = arity; ii--; ) {
        if (in[ii] != NULL) {
            taken_flags[ii] = 1;
            pq->Insert(ii,*in[ii]);
        } else {
            taken_flags[ii] = 0;
        }
    }
#else     
    for (ii = arity; ii--; ) {
        taken_flags[ii] = 0;
    }
#endif // MERGE_VIA_TPQUEUE

    return AMI_ERROR_NO_ERROR;
}


AMI_err sort_merge::operate(const int **in, AMI_merge_flag *taken_flags,
                            int *out)
{
    int ii;
#ifndef MERGE_VIA_TPQUEUE        
    int min, ii_min;
#endif // !MERGE_VIA_TPQUEUE
    
    called++;
    
#ifdef MERGE_VIA_TPQUEUE
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
#endif // !MERGE_VIA_TPQUEUE
}


extern "C" int int_cmp(const void *p1, const void *p2);

AMI_err sort_merge::main_mem_operate(int* mm_stream, size_t len)
{
    qsort(mm_stream, len, sizeof(int), int_cmp);
    return AMI_ERROR_NO_ERROR;
}

static bool verbose = false;

static char def_srf[] = "/var/tmp/oss.txt";
static char def_rrf[] = "/var/tmp/osr.txt";

static char *sorted_results_filename = def_srf;
static char *rand_results_filename = def_rrf;

static bool report_results_random = false;
static bool report_results_sorted = false;

void parse_args(int argc, char **argv)
{
    GetOpt go(argc, argv, "m:t:z:vbR:rS:s");
    char c;
    
    while ((c = go()) != -1) {
        switch (c) {
            case 'v':
                verbose = true;
                break;
            case 'b':
                verbose = false;
                break;
            case 'm':
                istrstream(go.optarg,strlen(go.optarg)) >> test_mm_size;
                break;                
            case 't':
                istrstream(go.optarg,strlen(go.optarg)) >> test_size;
                break;
            case 'z':
                istrstream(go.optarg,strlen(go.optarg)) >> random_seed;
                break;
            case 'R':
                rand_results_filename = go.optarg;
            case 'r':
                report_results_random = true;
                break;
            case 'S':
                sorted_results_filename = go.optarg;
            case 's':
                report_results_sorted = true;
                break;
        }
    }
}

#define REPORT_RUSAGE(os, ru, x)				\
{								\
    if (verbose) {						\
    	os << #x " = " << ru.x << ".\n";			\
    } else {							\
    	os << ' ' << ru.x;					\
    }								\
}

#define REPORT_RUSAGE_DIFFERENCE(os, ru0, ru1, x)		\
{								\
    if (verbose) {						\
    	os << #x " = " << ru1.x - ru0.x << ".\n";		\
    } else {							\
    	os << ' ' << ru1.x - ru0.x;				\
    }								\
}

int main(int argc, char **argv)
{

    AMI_err ae;

    rusage ru0, ru1;

    parse_args(argc,argv);

    if (verbose) {
        cout << "test_size = " << test_size << ".\n";
        cout << "test_mm_size = " << test_mm_size << ".\n";
        cout << "random_seed = " << random_seed << ".\n";
    } else {
        cout << test_size << ' ' << test_mm_size << ' ' << random_seed;
    }
    
    // Set the amount of main memory:
    MM_manager.resize_heap(test_mm_size);
    register_new = 1;
        
    AMI_STREAM<int> amis0((unsigned int)0, test_size);
    AMI_STREAM<int> amis1((unsigned int)0, test_size);
        
    // Write some ints.
    random_scan rnds(test_size,random_seed);
    
    ae = AMI_scan(&rnds, (AMI_base_stream<int> *)&amis0);

    if (verbose) {
        cout << "Wrote the random values.\n";
        cout << "Stream length = " << amis0.stream_len() << '\n';
    }

    // Streams for reporting random vand/or sorted values to ascii
    // streams.
    
    ofstream *oss;
    cxx_ostream_scan<int> *rpts;
    ofstream *osr;
    cxx_ostream_scan<int> *rptr;
    
    if (report_results_random) {
        osr = new ofstream(rand_results_filename);
        rptr = new cxx_ostream_scan<int>(osr);
    }
    
    if (report_results_sorted) {
        oss = new ofstream(sorted_results_filename);
        rpts = new cxx_ostream_scan<int>(oss);
    }
    
    if (report_results_random) {
        ae = AMI_scan((AMI_base_stream<int> *)&amis0, rptr);
    }
    
    sort_merge sm;

    getrusage(RUSAGE_SELF, &ru0);
    
    ae = AMI_partition_and_merge(&amis0, &amis1,
                                 (AMI_merge_base<int> *)&sm);

    getrusage(RUSAGE_SELF, &ru1);

    if (verbose) {
        cout << "Sorted them.\n";
        cout << "Sorted stream length = " << amis1.stream_len() << '\n';
        cout << "sm.operate called " << sm.called << " times.\n";
    }
    
    if (report_results_sorted) {
        ae = AMI_scan((AMI_base_stream<int> *)&amis1, rpts);
    }

    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_utime.tv_sec);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_utime.tv_usec);

    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_stime.tv_sec);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_stime.tv_usec);

    REPORT_RUSAGE(cout, ru1, ru_maxrss);
    REPORT_RUSAGE(cout, ru1, ru_ixrss);
    REPORT_RUSAGE(cout, ru1, ru_idrss);
    REPORT_RUSAGE(cout, ru1, ru_isrss);

    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_minflt);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_majflt);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_nswap);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_inblock);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_oublock);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_msgsnd);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_msgrcv);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_nsignals);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_nvcsw);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_nivcsw);

    cout << '\n';

    return 0;
}
    
