// Copyright (c) 1995 Darren Vengroff
//
// File: test_ami_sort24.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/24/95
//

#include <portability.h>

#include <versions.h>
VERSION(test_ami_sort24_cpp,"$Id: test_ami_sort24.cpp,v 1.18 2005-11-18 12:42:56 jan Exp $");

// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

#include <ami_sort.h>
#include <ami_kb_sort.h>

// Utitlities for ascii output.
#include <ami_scan_utils.h>

#include <cpu_timer.h>

// This is the type of object we will sort.

union sort_obj
{
    kb_key key_val;
    char filler[24];

    // How to extract the key for key bucket sorting.
    inline operator kb_key(void) const
    {
        return key_val;
    }
};



// A scan object to generate random keys.
class scan_random_so : AMI_scan_object {
private:
    TPIE_OS_OFFSET m_max;
    TPIE_OS_OFFSET m_remaining;
public:
    scan_random_so(TPIE_OS_OFFSET count = 1000, int seed = 17);
    virtual ~scan_random_so(void);
    AMI_err initialize(void);
    AMI_err operate(sort_obj *out1, AMI_SCAN_FLAG *sf);
};


scan_random_so::scan_random_so(TPIE_OS_OFFSET count, int seed) : 
    m_max(count), m_remaining(count) {

    TP_LOG_APP_DEBUG("scan_random_so seed = ");
    TP_LOG_APP_DEBUG(seed);
    TP_LOG_APP_DEBUG('\n');

    TPIE_OS_SRANDOM(seed);
}

scan_random_so::~scan_random_so(void)
{
}


AMI_err scan_random_so::initialize(void)
{
    m_remaining = m_max;

    return AMI_ERROR_NO_ERROR;
};

AMI_err scan_random_so::operate(sort_obj *out1, AMI_SCAN_FLAG *sf)
{
    if ((*sf = (m_remaining-- >0))) {
        out1->key_val = TPIE_OS_RANDOM();
        return AMI_SCAN_CONTINUE;
    } else {
        return AMI_SCAN_DONE;
    }
};



static char def_srf[] = "oss.txt";
static char def_rrf[] = "osr.txt";

static char *sorted_results_filename = def_srf;
static char *rand_results_filename = def_rrf;

static bool report_results_random = false;
static bool report_results_sorted = false;

static bool kb_sort = false;

struct options app_opts[] = {
  { 10, "random-results-filename", "", "R", 1 },
  { 11, "report-results-random", "", "r", 0 },
  { 12, "sorted-results-filename", "", "S", 1 },
  { 13, "report-results-sorted", "", "s", 0 },
  { 14, "kb-sort", "", "k", 0 },
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
        case 14:
            kb_sort = !kb_sort;
	    break;
    }
}


int main(int argc, char **argv)
{
    
    AMI_err ae;

    cpu_timer cput;
    
    parse_args(argc, argv, app_opts, parse_app_opts);

    if (verbose) {
      cout << "test_size = " << test_size << "." << endl;
      cout << "test_mm_size = " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << "." << endl;
      cout << "random_seed = " << random_seed << "." << endl;
    } else {
        cout << test_size << ' ' << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << ' ' << random_seed << ' ';
    }
    
    // Set the amount of main memory:
    MM_manager.set_memory_limit (test_mm_size);
        
    AMI_STREAM<sort_obj> amis0;
    AMI_STREAM<sort_obj> amis1;
        
    // Write some ints.
    scan_random_so rnds(test_size,random_seed);
    
    ae = AMI_scan(&rnds, &amis0);

    if (verbose) {
      cout << "Wrote the random values." << endl;
        cout << "Stream length = " << amis0.stream_len() << endl;
    }

    // Streams for reporting random vand/or sorted values to ascii
    // streams.
    
    ofstream *oss;
    cxx_ostream_scan<sort_obj> *rpts = NULL;
    ofstream *osr;
    cxx_ostream_scan<sort_obj> *rptr = NULL;
    
    if (report_results_random) {
        osr = new ofstream(rand_results_filename);
        rptr = new cxx_ostream_scan<sort_obj>(osr);
    }
    
    if (report_results_sorted) {
        oss = new ofstream(sorted_results_filename);
        rpts = new cxx_ostream_scan<sort_obj>(oss);
    }
    
    if (report_results_random) {
        ae = AMI_scan(&amis0, rptr);
    }

    // Make the input stream read-once.

    amis0.persist(PERSIST_READ_ONCE);
    
    cput.reset();
    cput.start();

    if (kb_sort) {
        key_range range(KEY_MIN, KEY_MAX);
        ae = AMI_kb_sort(amis0, amis1, range);
    } else {
        ae = AMI_sort(&amis0, &amis1);
    }

    cput.stop();

    amis0.persist(PERSIST_DELETE);
    
    if (verbose) {
      cout << "Sorted them." << endl;
        cout << "ae = " << ae << endl;
        cout << "Sorted stream length = " << amis1.stream_len() << endl;
    }

    cout << cput;
    
    if (report_results_sorted) {
        ae = AMI_scan(&amis1, rpts);
    }
    cout << endl;
    
    return 0;
}
