// Copyright (c) 1994 Darren Vengroff
//
// File: test_ami_arith.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/10/94
//

#include <portability.h>




#include "app_config.h"        
#include "parse_args.h"

// Get AMI_scan().
#include <scan.h>

// Get utitlities for ascii output.
#include <scan_utils.h>

// Get some scanners.
#include "scan_square.h"
#include "scan_count.h"

// Get stream arithmetic.
#include <stream_arith.h>

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
        
    AMI_STREAM<TPIE_OS_OFFSET> amis0;
    AMI_STREAM<TPIE_OS_OFFSET> amis1;
    AMI_STREAM<TPIE_OS_OFFSET> amis2;

    // Streams for reporting values to ascii streams.
    
    ofstream *osc;
    ofstream *osi;
    ofstream *osf;
    cxx_ostream_scan<TPIE_OS_OFFSET> *rptc = NULL;
    cxx_ostream_scan<TPIE_OS_OFFSET> *rpti = NULL;
    cxx_ostream_scan<TPIE_OS_OFFSET> *rptf = NULL;
    
    if (report_results_count) {
        osc = new ofstream(count_results_filename);
        rptc = new cxx_ostream_scan<TPIE_OS_OFFSET>(osc);
    }
    
    if (report_results_intermediate) {
        osi = new ofstream(intermediate_results_filename);
        rpti = new cxx_ostream_scan<TPIE_OS_OFFSET>(osi);
    }
    
    if (report_results_final) {
        osf = new ofstream(final_results_filename);
        rptf = new cxx_ostream_scan<TPIE_OS_OFFSET>(osf);
    }
    
    // Write some ints.
    scan_count sc(test_size);

    ae = AMI_scan(&sc, &amis0);

    if (verbose) {
      cout << "Wrote the initial sequence of values." << endl;
        cout << "Stopped (didn't write) with ii = "
             << sc.ii << ". operate() called " 
	     << sc.called << " times." << endl;
        cout << "Stream length = " << amis0.stream_len() << endl;
    }

    if (report_results_count) {
        ae = AMI_scan(&amis0, rptc);
    }
    
    // Square them.
    scan_square<TPIE_OS_OFFSET> ss;
        
    ae = AMI_scan(&amis0, &ss, &amis1);

    if (verbose) {
        cout << "Squared them; last squared was ii = "
             << ss.ii << ". operate() called " 
	     << ss.called << " times." << endl;
        cout << "Stream length = " << amis1.stream_len() << endl;
    }
    
    AMI_scan_div<TPIE_OS_OFFSET> sd;
    
    ae = AMI_scan(&amis1, &amis0, &sd, &amis2);
        
    if (verbose) {
      cout << "Divided them." << endl
	   << "Stream length = " << amis2.stream_len() << endl;
    }
    
    if (report_results_final) {
        ae = AMI_scan(&amis2, rptf);
    }
    
    return 0;
}
