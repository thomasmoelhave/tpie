//
// File: test_ami_merge.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 6/2/94
//

#include <portability.h>

#include <versions.h>
VERSION(test_ami_merge_cpp,"$Id: test_ami_merge.cpp,v 1.20 2005-11-16 17:03:51 jan Exp $");

#include <iostream>

#include "app_config.h"        
#include "parse_args.h"

// Define it all.
#include <ami_stream.h>
#include <ami_scan.h>
#include <ami_merge.h>

// Utitlities for ascii output.
#include <ami_scan_utils.h>

// Get some scanners and a merger.

#include "scan_square.h"
#include "scan_count.h"

#include "merge_interleave.h"

static char def_crf[] = "osc.txt";
static char def_irf[] = "osi.txt";
static char def_frf[] = "osf.txt";

static char *count_results_filename = def_crf;
static char *interleave_results_filename = def_irf;
static char *final_results_filename = def_frf;

static bool report_results_count = false;
static bool report_results_interleave = false;
static bool report_results_final = false;

struct options app_opts[] = {
  { 10, "count-results-filename", "", "C", 1 },
  { 11, "report-results-count", "", "c", 0 },
  { 12, "interleave-results-filename", "", "I", 1 },
  { 13, "report-results-interleave", "", "i", 0 },
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
            interleave_results_filename = opt_arg;
        case 13:
            report_results_interleave = true;
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
        cout << "test_size = " << test_size << ".\n";
        cout << "test_mm_size = " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << ".\n";
        cout << "random_seed = " << random_seed << ".\n";
    } else {
        cout << test_size << ' ' << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << ' ' << random_seed;
    }
    
    // Set the amount of main memory:
    MM_manager.set_memory_limit (test_mm_size);
        
    AMI_STREAM<TPIE_OS_OFFSET> amis0;
    AMI_STREAM<TPIE_OS_OFFSET> amis1;
    AMI_STREAM<TPIE_OS_OFFSET> amis2;
    AMI_STREAM<TPIE_OS_OFFSET> amis3;

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
    
    if (report_results_interleave) {
        osi = new ofstream(interleave_results_filename);
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
        cout << "Wrote the initial sequence of values.\n";
        cout << "Stopped (didn't write) with ii = "
             << sc.ii << ". operate() called " << sc.called << " times.\n";
        cout << "Stream length = " << amis0.stream_len() << '\n';
    }

    if (report_results_count) {
        ae = AMI_scan(&amis0, rptc);
    }
    
    // Square them.
    scan_square<TPIE_OS_OFFSET> ss;
        
    ae = AMI_scan(&amis0, &ss, &amis1);

    if (verbose) {
        cout << "Squared them; last squared was ii = "
             << ss.ii << ". operate() called " << ss.called << " times.\n";
        cout << "Stream length = " << amis1.stream_len() << '\n';
    }
    
    // Interleave the streams.
    merge_interleave<TPIE_OS_OFFSET> im;

    arity_t arity = 2;
        
    AMI_STREAM<TPIE_OS_OFFSET> *amirs[2];

    amirs[0] = &amis0;
    amirs[1] = &amis1;
    
    ae = AMI_generalized_single_merge(amirs, arity, &amis2, &im);

    if (verbose) {
        cout << "Interleaved them; operate() called " << im.called 
             << " times.\n";
        cout << "Stream length = " << amis2.stream_len() << '\n';
    }

    if (report_results_interleave) {
        ae = AMI_scan(&amis2, rpti);
    }

    // Divide the stream into two substreams, and interleave them.

    AMI_STREAM<TPIE_OS_OFFSET>* amirs0 = amirs[0]; 
    AMI_STREAM<TPIE_OS_OFFSET>* amirs1 = amirs[1]; 

    ae = amis2.new_substream(AMI_READ_STREAM, 0, test_size-1, &amirs0);
    ae = amis2.new_substream(AMI_READ_STREAM, 0, 2*test_size-1, &amirs1);

    if (verbose) {
        cout << "Created substreams; lengths = " 
	     << amirs[0]->stream_len() 
	     << " and " 
	     << amirs[1]->stream_len() 
	     << '\n';
    }

    // Get around the OS (HP_UX in particular) when using BTE_IMP_MMB
    // by seeking back to 0 in the substream, which will force the last
    // block written to be unmapped.
    ae = amis2.seek(0);
    
    ae = AMI_generalized_single_merge(amirs, arity, &amis3, &im);

    if (verbose) {
        cout << "Interleaved them; operate() called " << im.called 
             << " times.\n";        
        cout << "Stream length = " << amis3.stream_len() << '\n';
    }
    
    if (report_results_final) {
        ae = AMI_scan(&amis3, rptf);
    }

    delete amirs[0];
    delete amirs[1];
    
    return 0;
}
