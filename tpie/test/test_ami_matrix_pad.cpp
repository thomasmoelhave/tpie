// Copyright (c) 1994 Darren Vengroff
//
// File: test_ami_matrix.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/11/94
//

#include <versions.h>
VERSION(test_ami_matrix_cpp,"$Id: test_ami_matrix_pad.cpp,v 1.5 2000-01-11 01:16:45 hutchins Exp $");

#include <iostream.h>
#include <fstream.h>

#include "app_config.h"        
#include "parse_args.h"

// Define it all.
#include <ami.h>

// Utitlities for ascii output.
#include <ami_scan_utils.h>

// Get some scanners.

#include "scan_count.h"

// Get matrices.

#include <ami_matrix.h>

static char def_crf[] = "/var/tmp/osc.txt";
static char def_irf[] = "/var/tmp/osi.txt";
static char def_frf[] = "/var/tmp/osf.txt";

static char *count_results_filename = def_crf;
static char *intermediate_results_filename = def_irf;
static char *final_results_filename = def_frf;

static bool report_results_count = false;
static bool report_results_intermediate = false;
static bool report_results_final = false;

static const char as_opts[] = "C:I:F:cif";
void parse_app_opt(char c, char *optarg)
{
    switch (c) {
        case 'C':
            count_results_filename = optarg;
        case 'c':
            report_results_count = true;
            break;
        case 'I':
            intermediate_results_filename = optarg;
        case 'i':
            report_results_intermediate = true;
            break;
        case 'F':
            final_results_filename = optarg;
        case 'f':
            report_results_final = true;
            break;
    }
}

int main(int argc, char **argv)
{
    AMI_err ae;

    parse_args(argc,argv,as_opts,parse_app_opt);

    if (verbose) {
        cout << "test_size = " << test_size << ".\n";
        cout << "test_mm_size = " << test_mm_size << ".\n";
        cout << "random_seed = " << random_seed << ".\n";
    } else {
        cout << test_size << ' ' << test_mm_size << ' ' << random_seed;
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

    ae = AMI_scan(&sc, (AMI_STREAM<int> *)&em0);

    if (verbose) {
        cout << "Wrote the initial sequence of values.\n";
        cout << "Stopped (didn't write) with ii = "
             << sc.ii << ". operate() called " << sc.called << " times.\n";
        cout << "Stream length = " << em0.stream_len() << '\n';
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
            ae = AMI_scan((AMI_base_stream<int> *)&em2, rpti);
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
