//
// File: test_ami_matrix.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/11/94
//

#include <portability.h>

#include <versions.h>
VERSION(test_ami_matrix_cpp,"$Id: test_ami_matrix_pad.cpp,v 1.11 2005-02-15 00:23:06 tavi Exp $");

#include "app_config.h"        
#include "parse_args.h"

// Define it all.
#include <ami_scan.h>

// Utitlities for ascii output.
#include <ami_scan_utils.h>

// Get some scanners.
#include "scan_count.h"

// Get matrices.
#include <ami_matrix.h>

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

	test_size = 128 * 1024;

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

    AMI_matrix<int> em0(test_size, test_size);
        
    // Streams for reporting values to ascii streams.
    
    ofstream *osc;
    ofstream *osi;
    ofstream *osf;
    cxx_ostream_scan<int> *rptc = NULL;
    cxx_ostream_scan<int> *rpti = NULL;
    cxx_ostream_scan<int> *rptf = NULL;
    
    if (report_results_count) {
        osc = new ofstream(count_results_filename);
        rptc = new cxx_ostream_scan<int>(osc);
    }
    
    if (report_results_intermediate) {
        osi = new ofstream(intermediate_results_filename);
        rpti = new cxx_ostream_scan<int>(osi);
    }
    
    if (report_results_final) {
        osf = new ofstream(final_results_filename);
        rptf = new cxx_ostream_scan<int>(osf);
    }
    
    // Write some ints.
    scan_count sc(test_size*test_size);

    ae = AMI_scan(&sc, (AMI_STREAM<TPIE_OS_OFFSET> *)&em0);

    if (verbose) {
      cout << "Wrote the initial sequence of values." << endl;
        cout << "Stopped (didn't write) with ii = "
             << sc.ii << ". operate() called " 
	     << sc.called << " times." << endl;
        cout << "Stream length = " << em0.stream_len() << endl;
    }

    if (report_results_count) {
        ae = AMI_scan((AMI_STREAM<int> *)&em0, rptc);
    }

    {
        // Pad the matrix.

        AMI_matrix_pad<int> smp(test_size, test_size, 7);

        AMI_matrix<int> em1(7 * ((em0.rows() - 1)/7 + 1),
                            7 * ((em0.cols() - 1)/7 + 1));

        ae = AMI_scan((AMI_STREAM<int> *)&em0, &smp, (AMI_STREAM<int> *)&em1);

        
        // Block permute the matrix.

        AMI_matrix<int> em1p(7 * ((em0.rows() - 1)/7 + 1),
                             7 * ((em0.cols() - 1)/7 + 1));

        perm_matrix_into_blocks pmib1(7 * ((em0.rows() - 1)/7 + 1),
                                      7 * ((em0.cols() - 1)/7 + 1),
                                      7);

        ae = AMI_general_permute((AMI_STREAM<int> *)&em1,
                                 (AMI_STREAM<int> *)&em1p,
                                 (AMI_gen_perm_object *)&pmib1); 

        if (report_results_intermediate) {
            ae = AMI_scan((AMI_STREAM<int> *)&em1p, rpti);
        }
        
        // Un block permute it.

        AMI_matrix<int> em2(7 * ((em0.rows() - 1)/7 + 1),
                            7 * ((em0.cols() - 1)/7 + 1));

        perm_matrix_outof_blocks pmob1(7 * ((em0.rows() - 1)/7 + 1),
                                       7 * ((em0.cols() - 1)/7 + 1),
                                       7);

        ae = AMI_general_permute((AMI_STREAM<int> *)&em1p,
                                 (AMI_STREAM<int> *)&em2,
                                 (AMI_gen_perm_object *)&pmob1); 

#if 0        
        if (report_results_intermediate) {
            ae = AMI_scan((AMI_stream_base<int> *)&em2, rpti);
        }        
#endif
        
        // Unpad the matrix.

        AMI_matrix_unpad<int> smup(test_size, test_size, 7);

        AMI_matrix<int> em3(em0.rows(), em0.cols());

        ae = AMI_scan((AMI_STREAM<int> *)&em2, &smup,
                      (AMI_STREAM<int> *)&em3);

        if (report_results_final) {
            ae = AMI_scan((AMI_STREAM<int> *)&em3, rptf);
        }

    }
    
    return 0;
}
