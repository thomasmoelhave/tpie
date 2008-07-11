// Copyright (c) 1995 Darren Vengroff
//
// File: test_ami_sm.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/2/95
//


#include <portability.h>

#include "app_config.h"        
#include "parse_args.h"

// Define it all.
#include <ami_scan.h>

VERSION(test_ami_sm_cpp,"$Id: test_ami_sm.cpp,v 1.13 2005-11-16 17:03:52 jan Exp $");

// Utitlities for ascii output.
#include <ami_scan_utils.h>

// Get matrices.

#include <ami_matrix.h>
#include <ami_matrix_fill.h>
#include "fill_value.h"

#include <ami_sparse_matrix.h>

#include "scan_uniform_sm.h"

#include <cpu_timer.h>

static char def_crf[] = "osc.txt";
static char def_irf[] = "osi.txt";
static char def_frf[] = "osf.txt";

static char def_brf[] = "isb.txt";

static char *count_results_filename = def_crf;
static char *intermediate_results_filename = def_irf;
static char *final_results_filename = def_frf;

static char *banded_read_filename = def_brf;

static bool report_results_count = false;
static bool report_results_intermediate = false;
static bool report_results_final = false;

static bool read_banded_matrix = false;

static bool call_mult = false;

static double density = 0.10;

struct options app_opts[] = {
  { 10, "count-results-filename", "", "C", 1 },
  { 11, "report-results-count", "", "c", 0 },
  { 12, "intermediate-results-filename", "", "I", 1 },
  { 13, "report-results-intermediate", "", "i", 0 },
  { 14, "final-results-filename", "", "F", 1 },
  { 15, "report-results-final", "", "f", 0 },
  { 16, "density", "", "d", 1 },
  { 17, "call-multiply-directly", "", "D", 0 },
  { 20, "banded-read-filename", "", "B", 1 },
  { 21, "read_banded_matrix", "", "b", 0 },
  { 0, NULL, NULL, NULL, 0 }
};

void parse_app_opts(int idx, char *opt_arg)
{
    switch (idx) {
    case 20:
      banded_read_filename = opt_arg;
    case 21:
      read_banded_matrix = true;
      break;
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
    case 16:
      density = atof(opt_arg);
      break;
    case 17:
      call_mult = true;
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
      cout << "density = " << density << "." << endl;
      cout << "Call mult directly = " << call_mult << "." << endl;
    } else {
        cout << test_size << ' ' << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << ' ' << random_seed;
    }
    
    // Set the amount of main memory:
    MM_manager.set_memory_limit (test_mm_size);

    // A sparse matrix.
    
    AMI_sparse_matrix<double> esm0(test_size, test_size);

    AMI_sparse_matrix<double> esm0b(test_size, test_size);

    // A vector to multiply the sparse matrix by

    AMI_matrix<double> ev0(test_size, 1);

    // The result vector
    
    AMI_matrix<double> ev1(test_size, 1);

    // Streams for reporting values to ascii streams.
    
    ofstream *osc;
    ofstream *osi;
    ofstream *osf;
    cxx_ostream_scan< AMI_sm_elem<double> > *rptc = NULL;
    cxx_ostream_scan< AMI_sm_elem<double> > *rpti = NULL;
    cxx_ostream_scan<double> *rptf = NULL;

    istream *isb;
    cxx_istream_scan<AMI_sm_elem<double> > *readb = NULL;    
    
    if (report_results_count) {
        osc = new ofstream(count_results_filename);
        rptc = new cxx_ostream_scan< AMI_sm_elem<double> >(osc);
    }

    if (read_banded_matrix) {
        isb = new ifstream(banded_read_filename);
        readb = new cxx_istream_scan<AMI_sm_elem<double> >(isb);
    }
    
    if (report_results_intermediate) {
        osi = new ofstream(intermediate_results_filename);
        rpti = new cxx_ostream_scan< AMI_sm_elem<double> >(osi);
    }
    
    if (report_results_final) {
        osf = new ofstream(final_results_filename);
        rptf = new cxx_ostream_scan<double>(osf);
    }
    
    // Write some elements into the sparse matrix.

    if (!read_banded_matrix) {
        scan_uniform_sm susm(test_size, test_size, density, random_seed);
    
        ae = AMI_scan(&susm, &esm0);

        if (report_results_count) {
            ae = AMI_scan(&esm0, rptc);
        }
    }
    
    // Write the elements of the vector.

    fill_value<double> fv;

    fv.set_value(1.0);

    ae = AMI_matrix_fill(&ev0, &fv);

    // Multiply the two

    if (call_mult) {
        ae = AMI_sparse_mult(esm0, ev0, ev1);
    } else {

        cpu_timer cput0, cput1;
        
		TPIE_OS_SIZE_T rows_per_band;
        TPIE_OS_OFFSET total_bands;
        
        if (read_banded_matrix) {

            cput1.reset();
            cput1.start();

            if (verbose) {
                cout << "Reading banded matrix from \""
		     << banded_read_filename << "\"" << endl;
            }

            // Read in the banded order matrix from an input file.            
            TPIE_OS_SIZE_T file_test_mm_size;
	    TPIE_OS_OFFSET file_test_size;
            TPIE_OS_SIZE_T file_rows_per_band;

            *isb >> file_test_mm_size >> file_test_size
                >> file_rows_per_band;

            rows_per_band = file_rows_per_band;
            
            ae = AMI_scan(readb, &esm0b);

            cput1.stop();

            cout << cput1 << endl;
            
        } else {

            if (verbose) {
	      cout << "Generating banded matrix." << endl;
            }

            cput0.reset();
            cput0.start();

            // Compute band information.

            ae = AMI_sparse_band_info(esm0, rows_per_band, total_bands);

            // Bandify the sparse matrix.
            
            ae = AMI_sparse_bandify(esm0, esm0b, rows_per_band);

            cput0.stop();

            cout << cput0 << endl;
            
        }
        
        if (report_results_intermediate) {
            *osi << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size)
		 << ' ' 
		 << test_size 
		 << ' '
                 << static_cast<TPIE_OS_OUTPUT_SIZE_T>(rows_per_band) << endl;
            ae = AMI_scan(&esm0b, rpti);
        }

        // Do the multiplication.

        if (read_banded_matrix) {
            cput1.reset();
            cput1.start();
        }
        
        ae = AMI_sparse_mult_scan_banded(esm0b, ev0, ev1, test_size,
                                         test_size, rows_per_band);

        if (read_banded_matrix) {
            cput1.stop();
            cout << cput1 << endl;
        }
                
    }
    
    if (verbose) {
      cout << "Multiplied them." << endl;
        cout << "Stream length = " << ev1.stream_len() << endl;
    }
    
    if (report_results_final) {
        ae = AMI_scan(&ev1, rptf);
    }
    
    return 0;
}
