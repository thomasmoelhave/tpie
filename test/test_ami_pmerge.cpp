// Copyright (c) 1994 Darren Erik Vengroff
//
// File: test_ami_pmerge.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 9/17/94
//
// A test for AMI_generalized_partition_and_merge().


#include <portability.h>

// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

#include <ami_merge.h>
#include <pqueue_heap.h>

#include <versions.h>
VERSION(test_ami_pmerge_cpp,"$Id: test_ami_pmerge.cpp,v 1.28 2005-02-15 00:23:06 tavi Exp $");

// Utitlities for ascii output.
#include <ami_scan_utils.h>

#include "scan_random.h"

// From int_cmp.c
//extern "C" int c_int_cmp(const void *, const void *);
int c_int_cmp(const void *p1, const void *p2)
{
    return *((int *)p1) - *((int *)p2);
}


// A merge object to merge sorted streams.  This code looks a lot like
// what is included as part of the TPIE system for sorting in
// ami_sort_single.h.

class s_merge_manager : public AMI_generalized_merge_base<int> {
private:
    arity_t input_arity;
    pqueue_heap_op<arity_t,int> *pq;
#if DEBUG_ASSERTIONS
    TPIE_OS_OFFSET input_count, output_count;
#endif    
public:
    s_merge_manager(void);
    virtual ~s_merge_manager(void);
    AMI_err initialize(arity_t arity, CONST int * CONST *in,
                       AMI_merge_flag *taken_flags,
                       int &taken_index);
    AMI_err operate(CONST int * CONST *in, AMI_merge_flag *taken_flags,
                    int &taken_index, int *out);
    AMI_err main_mem_operate(int* mm_stream, TPIE_OS_SIZE_T len);
	TPIE_OS_SIZE_T space_usage_overhead(void);
	TPIE_OS_SIZE_T space_usage_per_stream(void);
};


s_merge_manager::s_merge_manager(void)
{
    pq = NULL;
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


TPIE_OS_SIZE_T s_merge_manager::space_usage_overhead(void)
{
    return sizeof(pqueue_heap_op<arity_t,int>);
}


TPIE_OS_SIZE_T s_merge_manager::space_usage_per_stream(void)
{
    return sizeof(arity_t) + sizeof(int);
}


AMI_err s_merge_manager::operate(CONST int * CONST *in,
                                       AMI_merge_flag * /*taken_flags*/,
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


AMI_err s_merge_manager::main_mem_operate(int* mm_stream, TPIE_OS_SIZE_T len)
{
    qsort(mm_stream, len, sizeof(int), c_int_cmp);
    return AMI_ERROR_NO_ERROR;
}


static char def_srf[] = "oss.txt";
static char def_rrf[] = "osr.txt";

static char *sorted_results_filename = def_srf;
static char *rand_results_filename = def_rrf;

static bool report_results_random = false;
static bool report_results_sorted = false;

struct options app_opts[] = {
  { 10, "random-results-filename", "", "R", 1 },
  { 11, "report-results-random", "", "r", 0 },
  { 12, "sorted-results-filename", "", "S", 1 },
  { 13, "report-results-sorted", "", "s", 0 },
  { 0, NULL, NULL, NULL, 0 }
};

void parse_app_opts(int idx, char *opt_arg)
{
    switch (idx) {
        case 10:
            rand_results_filename = opt_arg;
        case 11:
            report_results_random = true;
            break;
        case 12:
            sorted_results_filename = opt_arg;
        case 13:
            report_results_sorted = true;
            break;
    }
}


int main(int argc, char **argv)
{

    AMI_err ae;

    parse_args(argc, argv, app_opts, parse_app_opts);

    if (verbose) {
      cout << "test_size = " << test_size << "." << endl;
      cout << "test_mm_size = " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << "." << endl;
      cout << "random_seed = " << random_seed << "." << endl;
    } else {
        cout << test_size << ' ' << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << ' ' << random_seed;
    }
    
    // Set the amount of main memory:
    MM_manager.set_memory_limit (test_mm_size);
        
    AMI_STREAM<int> amis0;
    AMI_STREAM<int> amis1;
        
    // Write some ints.
    scan_random rnds(test_size,random_seed);
    
    ae = AMI_scan(&rnds, &amis0);

    if (verbose) {
      cout << "Wrote the random values." << endl;
        cout << "Stream length = " << amis0.stream_len() << endl;
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
        ae = AMI_scan(&amis0, rptr);
    }

    s_merge_manager sm;
    
    ae = AMI_generalized_partition_and_merge(&amis0, &amis1,
                                 (s_merge_manager *)&sm);
    
    if (verbose) {
      cout << "Sorted them."<< endl;
        cout << "Sorted stream length = " << amis1.stream_len() << endl;
    }
    
    if (report_results_sorted) {
        ae = AMI_scan(&amis1, rpts);
    }    

    cout << endl;

    return 0;
}
