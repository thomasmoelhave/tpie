// Copyright (c) 1994 Darren Vengroff
//
// File: test_ami_stack.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/15/94
//

#include <portability.h>

#include <versions.h>
VERSION(test_ami_stack_cpp,"$Id: test_ami_stack.cpp,v 1.9 2005-02-15 00:23:06 tavi Exp $");

#include "app_config.h"        
#include "parse_args.h"

// Get the AMI_stack definition.
#include <ami_stack.h>

// Utitlities for ascii output.
#include <ami_scan_utils.h>

static char def_crf[] = "osc.txt";
static char def_irf[] = "osi.txt";
static char def_frf[] = "osf.txt";

static char *count_results_filename = def_crf;
static char *intermediate_results_filename = def_irf;
static char *final_results_filename = def_frf;

static bool report_results_count = false;
static bool report_results_intermediate = false;
static bool report_results_final = false;

struct options app_opts[] = {
  { 10, "count-results-filename", "", "C", 1 },
  { 11, "report-results-count", "", "c", 0 },
  { 12, "intermediate-results-filename", "", "I", 1 },
  { 13, "report-results-intermediate", "", "i", 0 },
  { 14, "final-results-filename", "", "F", 1 },
  { 15, "report-results-final", "", "f", 0 },
  { 0, NULL, NULL, NULL, 0 }
};

void parse_app_opts(int idx, char *opt_arg)
{
    switch (idx) {
        case 10:
            count_results_filename = opt_arg;
        case 11:
            report_results_count = true;
            break;
        case 12:
            intermediate_results_filename = opt_arg;
        case 13:
            report_results_intermediate = true;
            break;
        case 14:
            final_results_filename = opt_arg;
        case 15:
            report_results_final = true;
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

    AMI_stack<double> amis0;

    // Streams for reporting values to ascii streams.
        ofstream *osc;
    ofstream *osi;
    ofstream *osf;
    cxx_ostream_scan<double> *rptc = NULL;
    cxx_ostream_scan<double> *rpti = NULL;
    cxx_ostream_scan<double> *rptf = NULL;
    
    if (report_results_count) {
        osc = new ofstream(count_results_filename);
        rptc = new cxx_ostream_scan<double>(osc);
    }
    
    if (report_results_intermediate) {
        osi = new ofstream(intermediate_results_filename);
        rpti = new cxx_ostream_scan<double>(osi);
    }
    
    if (report_results_final) {
        osf = new ofstream(final_results_filename);
        rptf = new cxx_ostream_scan<double>(osf);
    }
    
    // Push values.
    TPIE_OS_OFFSET ii;
    for (ii = test_size; ii--; ) {
        amis0.push((double)ii+0.01);
    }

    if (verbose) {
      cout << "Pushed the initial sequence of values." << endl;
        cout << "Stream length = " << amis0.stream_len() << endl;
    }
        
    if (report_results_count) {
        ae = AMI_scan((AMI_STREAM<double> *)&amis0, rptc);
    }

    // Pop them all off.

    double *jj;
    
    for (ii = 0; ii < test_size; ii++ ) {
        amis0.pop(&jj);
        if (ii  + 0.01 != *jj) {
            cout << ii  + 0.01 << " != " << *jj << endl;
        }
    }

    if (verbose) {
      cout << "Popped the initial sequence of values." << endl;
        cout << "Stream length = " << amis0.stream_len() << endl;
    }
    
    return 0;
}
