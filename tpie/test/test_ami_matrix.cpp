// Copyright (c) 1994 Darren Vengroff
//
// File: test_ami_matrix.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/11/94
//

#include <portability.h>

#include <versions.h>
VERSION(test_ami_matrix_cpp,"$Id: test_ami_matrix.cpp,v 1.10 2003-09-27 07:10:42 tavi Exp $");

#include "app_config.h"        
#include "parse_args.h"

// Get AMI_scan().
#include <ami_scan.h>

// Utitlities for ascii output.
#include <ami_scan_utils.h>

// Get matrices.
#include <ami_matrix.h>
#include <ami_matrix_fill.h>
#include "fill_upper_tri.h"

#include <cpu_timer.h>

static char def_crf[] = "osc.txt";
static char def_irf[] = "osi.txt";
static char def_frf[] = "osf.txt";

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
      cout << "test_size = " << test_size << "." << endl;
      cout << "test_mm_size = " << test_mm_size << "." << endl;
      cout << "random_seed = " << random_seed << "." << endl;
    } else {
        cout << test_size << ' ' << test_mm_size << ' ';
    }
    
    // Set the amount of main memory:
    MM_manager.set_memory_limit (test_mm_size);

    AMI_matrix<double> em0(test_size, test_size);
    AMI_matrix<double> em1(test_size, test_size);
    AMI_matrix<double> em2(test_size, test_size);
        
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

#if 1    
    // Write some doubles.

    fill_upper_tri<double> fut(3.1415927);
    
    {
        ae = AMI_matrix_fill(&em0, (AMI_matrix_filler<double> *)&fut);

        if (verbose) {
	  cout << "Wrote the initial sequence of values." << endl;
        }
        
        if (report_results_count) {
            ae = AMI_scan((AMI_STREAM<double> *)&em0, rptc);
            cout << "Stream length = " << em0.stream_len() << endl;
        }
    }

    {
        ae = AMI_matrix_fill(&em1, (AMI_matrix_filler<double> *)&fut);

        if (verbose) {
	  cout << "Wrote the second sequence of values." << endl;
            cout << "Stream length = " << em1.stream_len() << endl;
        }
        
        if (report_results_intermediate) {
            ae = AMI_scan((AMI_STREAM<double> *)&em1, rpti);
        }
    }
#endif

    cpu_timer cput;
    
    cput.reset();
    cput.start();
    
    // Multiply the two

    ae = AMI_matrix_mult(em0, em1, em2);

    cput.stop();

    cout << cput << '\n';
    
    if (verbose) {
      cout << "Multiplied them." << endl;
        cout << "Stream length = " << em2.stream_len() << endl;
    }
    
    if (report_results_final) {
        ae = AMI_scan((AMI_STREAM<double> *)&em2, rptf);
    }
    
    return 0;
}
