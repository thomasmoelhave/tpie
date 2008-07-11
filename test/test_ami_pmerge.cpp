// Copyright (c) 1994 Darren Erik Vengroff
//
// File: test_ami_pmerge.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 9/17/94
//
// A test for AMI_partition_and_merge().


#include <portability.h>

// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

#include <ami_merge.h>
#include <mergeheap.h>

#include <versions.h>
VERSION(test_ami_pmerge_cpp,"$Id: test_ami_pmerge.cpp,v 1.33 2005-12-19 03:11:19 adanner Exp $");

// Utitlities for ascii output.
#include <ami_scan_utils.h>

#include "scan_random.h"

// From int_cmp.c
//extern "C" int c_int_cmp(const void *, const void *);
int c_int_cmp(const void *p1, const void *p2)
{
    return *(static_cast<const int*>(p1)) - *(static_cast<const int*>(p2));
}


// A merge object to merge sorted streams.  This code looks a lot like
// what is included as part of the TPIE system for sorting in
// ami_sort_single.h.

class s_merge_manager : public AMI_merge_base<int> {
private:
    merge_heap_op<int> *mheap;
    TPIE_OS_SIZE_T input_arity;
#if DEBUG_ASSERTIONS
    TPIE_OS_OFFSET input_count, output_count;
#endif    
    // Prohibit using the next two.
    s_merge_manager(const s_merge_manager& other);
    s_merge_manager& operator=(const s_merge_manager& other);
public:
    s_merge_manager(void);
    virtual ~s_merge_manager(void);
    AMI_err initialize(TPIE_OS_SIZE_T arity, int **in,
                       AMI_merge_flag *taken_flags,
                       int &taken_index);
    AMI_err operate(int **in, AMI_merge_flag *taken_flags,
                    int &taken_index, int *out);
    AMI_err main_mem_operate(int* mm_stream, TPIE_OS_SIZE_T len);
	TPIE_OS_SIZE_T space_usage_overhead(void);
	TPIE_OS_SIZE_T space_usage_per_stream(void);
};


s_merge_manager::s_merge_manager(void) : mheap(NULL), input_arity(0)
#if DEBUG_ASSERTIONS
				       , input_count(0), output_count(0)
#endif
{
    // Do nothing.
}


s_merge_manager::~s_merge_manager(void)
{
    if (mheap != NULL) {
        mheap->deallocate();
        delete mheap;
    }
}


AMI_err s_merge_manager::initialize(TPIE_OS_SIZE_T arity, int **in,
                                          AMI_merge_flag *taken_flags,
                                          int &taken_index)
{
    TPIE_OS_SIZE_T ii;

    input_arity = arity;

    tp_assert(arity > 0, "Input arity is 0.");
    
    if (mheap != NULL) {
        mheap->deallocate();
        delete mheap;
    }
    mheap = new merge_heap_op<int>();
    mheap->allocate(arity);

#if DEBUG_ASSERTIONS
    input_count = output_count = 0;
#endif    
    for (ii = arity; ii--; ) {
        if (in[ii] != NULL) {
            taken_flags[ii] = 1;
            mheap->insert(in[ii],ii);
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
    return mheap->space_overhead();
}


TPIE_OS_SIZE_T s_merge_manager::space_usage_per_stream(void)
{
    return sizeof(TPIE_OS_SIZE_T) + sizeof(int);
}


AMI_err s_merge_manager::operate(int **in,
                                       AMI_merge_flag * /*taken_flags*/,
                                       int &taken_index,
                                       int *out)
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
        int min_t;

        mheap->extract_min(min_t, min_source);
        *out = min_t;
        if (in[min_source] != NULL) {
            mheap->insert(in[min_source], min_source);
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
    
    ae = AMI_partition_and_merge(&amis0, &amis1, &sm);
    
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
