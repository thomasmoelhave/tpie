// Copyright (c) 1994 Darren Erik Vengroff
//
// File: partition_and_merge.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 9/17/94
//
// A test for AMI_partition_and_merge().

static char test_ami_pmerge_id[] = "$Id: test_ami_pmerge.cpp,v 1.8 1994-10-11 12:54:55 dev Exp $";

// This is just to avoid an error message since the string above is never
// referenced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___test_ami_pmerge_id_compiler_fooler {
    char *pc;
    ___test_ami_pmerge_id_compiler_fooler *next;
} the___test_ami_pmerge_id_compiler_fooler = {
    test_ami_pmerge_id,
    &the___test_ami_pmerge_id_compiler_fooler
};

#include <tpqueue.h>

#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>

// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

// Define it all.
#include <ami.h>

// Utitlities for ascii output.
#include <ami_scan_utils.h>

#include "scan_random.h"

// From int_cmp.c
extern "C" int c_int_cmp(const void *, const void *);

// A merge object to merge sorted streams.  This code looks a lot like
// what is included as part of the TPIE system for sorting in
// ami_sort_single.h.


class s_merge_manager : public AMI_merge_base<int> {
private:
    arity_t input_arity;
    pqueue_heap_op<arity_t,int> *pq;
#if DEBUG_ASSERTIONS
    unsigned int input_count, output_count;
#endif    
public:
    s_merge_manager(void);
    virtual ~s_merge_manager(void);
    AMI_err initialize(arity_t arity, CONST int * CONST *in,
                       AMI_merge_flag *taken_flags,
                       int &taken_index);
    AMI_err operate(CONST int * CONST *in, AMI_merge_flag *taken_flags,
                    int &taken_index, int *out);
    AMI_err main_mem_operate(int* mm_stream, size_t len);
    size_t space_usage_overhead(void);
    size_t space_usage_per_stream(void);
};


s_merge_manager::s_merge_manager(void)
{
}


s_merge_manager::~s_merge_manager(void)
{
    if (pq != NULL) {
        delete pq;
    }
}


AMI_err s_merge_manager::initialize(arity_t arity, CONST int * CONST *in,
                                          AMI_merge_flag *taken_flags,
                                          int &taken_index)
{
    arity_t ii;

    input_arity = arity;

    bool pqret;
    
    tp_assert(arity > 0, "Input arity is 0.");
    
    if (pq != NULL) {
        delete pq;
    }
    pq = new pqueue_heap_op<arity_t,int>(arity);

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


size_t s_merge_manager::space_usage_overhead(void)
{
    return sizeof(PQUEUE<arity_t,int>);
}


size_t s_merge_manager::space_usage_per_stream(void)
{
    return sizeof(arity_t) + sizeof(int);
}


AMI_err s_merge_manager::operate(CONST int * CONST *in,
                                       AMI_merge_flag *taken_flags,
                                       int &taken_index,
                                       int *out)
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
        int min_t;

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


AMI_err s_merge_manager::main_mem_operate(int* mm_stream, size_t len)
{
    qsort(mm_stream, len, sizeof(int), c_int_cmp);
    return AMI_ERROR_NO_ERROR;
}


static char def_srf[] = "/var/tmp/oss.txt";
static char def_rrf[] = "/var/tmp/osr.txt";

static char *sorted_results_filename = def_srf;
static char *rand_results_filename = def_rrf;

static bool report_results_random = false;
static bool report_results_sorted = false;

static const char as_opts[] = "R:S:rs";
void parse_app_opt(char c, char *optarg)
{
    switch (c) {
        case 'R':
            rand_results_filename = optarg;
        case 'r':
            report_results_random = true;
            break;
        case 'S':
            sorted_results_filename = optarg;
        case 's':
            report_results_sorted = true;
            break;
    }
}

#if !(defined(__sun__) && defined(__svr4__))
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
#endif

extern int register_new;


int main(int argc, char **argv)
{

    AMI_err ae;

#if !(defined(__sun__) && defined(__svr4__))
    rusage ru0, ru1;
#endif

    parse_args(argc,argv,as_opts,parse_app_opt);

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
        
    AMI_STREAM<int> amis0((unsigned int)1, test_size);
    AMI_STREAM<int> amis1((unsigned int)1, test_size);
        
    // Write some ints.
    scan_random rnds(test_size,random_seed);
    
    ae = AMI_scan(&rnds, (AMI_base_stream<int> *)&amis0);

    if (verbose) {
        cout << "Wrote the random values.\n";
        cout << "Stream length = " << amis0.stream_len() << '\n';
    }

    // Streams for reporting random and/or sorted values to ascii
    // streams.
    
    ofstream *oss;
    cxx_ostream_scan<int> *rpts = NULL;
    ofstream *osr;
    cxx_ostream_scan<int> *rptr = NULL;
    
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

    s_merge_manager sm;
    
#if !(defined(__sun__) && defined(__svr4__))
    getrusage(RUSAGE_SELF, &ru0);
#endif

    ae = AMI_partition_and_merge(&amis0, &amis1,
                                 (AMI_merge_base<int> *)&sm);
    
#if !(defined(__sun__) && defined(__svr4__))
    getrusage(RUSAGE_SELF, &ru1);
#endif

    if (verbose) {
        cout << "Sorted them.\n";
        cout << "Sorted stream length = " << amis1.stream_len() << '\n';
    }
    
    if (report_results_sorted) {
        ae = AMI_scan((AMI_base_stream<int> *)&amis1, rpts);
    }

#if !(defined(__sun__) && defined(__svr4__))
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
#endif

    cout << '\n';

    return 0;
}
    
