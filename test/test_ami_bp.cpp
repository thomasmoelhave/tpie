// Copyright (c) 1995 Darren Vengroff
//
// File: test_ami_bp.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/9/95
//
// Test for AMI_BMMC_permute(). See the Tutorial for an explanation of 
// this particular example.
//

static char test_ami_bp_id[] = "$Id: test_ami_bp.cpp,v 1.5 1999-05-02 16:08:59 tavi Exp $";

// This is just to avoid an error message since the string above is never
// referenced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___test_ami_bp_id_compiler_fooler {
    char *pc;
    ___test_ami_bp_id_compiler_fooler *next;
} the___test_ami_bp_id_compiler_fooler = {
    test_ami_bp_id,
    &the___test_ami_bp_id_compiler_fooler
};




// Get the application defaults.
#include "app_config.h"

// Define it all.
#include <ami.h>

// Utitlities for ascii output.
#include <iostream.h>
#include <fstream.h>
#include <ami_scan_utils.h>

#include "parse_args.h"
#include "scan_count.h"


static char def_irf[] = "/var/tmp/osi.txt";
static char def_frf[] = "/var/tmp/osf.txt";

static char *initial_results_filename = def_irf;
static char *final_results_filename = def_frf;

static bool report_results_initial = false;
static bool report_results_final = false;

static const char as_opts[] = "I:iF:f";
void parse_app_opt(char c, char *optarg)
{
    switch (c) {
        case 'I':
            initial_results_filename = optarg;
        case 'i':
            report_results_initial = true;
            break;
        case 'F':
            final_results_filename = optarg;
        case 'f':
            report_results_final = true;
            break;
    }
}

extern int register_new;

int main(int argc, char **argv)
{
    AMI_err ae;
    int number_of_bits;

    parse_args(argc,argv,as_opts,parse_app_opt);
    
    // Count the bits in test_size.
    for (number_of_bits = 0 ; test_size >= 2; number_of_bits++)
      test_size = test_size >> 1;
    if (number_of_bits == 0)
      number_of_bits = 1;

    // Adjust the test size to be a power of two.
    test_size = 1 << number_of_bits;
    
    if (verbose) {
        cout << "test_size = " << test_size << ".\n";
        cout << "test_mm_size = " << test_mm_size << ".\n";
        cout << "random_seed = " << random_seed << ".\n";
    } else {
        cout << test_size << ' ' << test_mm_size << ' ' << random_seed;
    }

    srandom(random_seed);
    
    // Set the amount of main memory:
    MM_manager.resize_heap(test_mm_size);
    register_new = 1;

    AMI_STREAM<int> amis0;
    AMI_STREAM<int> amis1;

    // Streams for reporting values to ascii streams.
    
    ofstream *osi;
    cxx_ostream_scan<int> *rpti = NULL;

    ofstream *osf;
    cxx_ostream_scan<int> *rptf = NULL;

    if (report_results_initial) {
        osi = new ofstream(initial_results_filename);
        rpti = new cxx_ostream_scan<int>(osi);
    }

    if (report_results_final) {
        osf = new ofstream(final_results_filename);
        rptf = new cxx_ostream_scan<int>(osf);
    }

    scan_count my_scan_count(test_size);

    ae = AMI_scan(&my_scan_count, &amis0);

    if (verbose) {
        cout << "Initial stream length = " << amis0.stream_len() << '\n';
    }
    
    if (report_results_initial) {
        ae = AMI_scan(&amis0, rpti);
    }

    amis0.seek(0);

    bit_matrix A(number_of_bits, number_of_bits);
    bit_matrix c(number_of_bits, 1);

    {
        unsigned int ii,jj;

        for (ii = number_of_bits; ii--; ) {
            c[ii][0] = 0;
            for (jj = number_of_bits; jj--; ) {
                A[number_of_bits-1-ii][jj] = (ii == jj);
            }
        }
    }

    if (verbose) {
        cout << "A = \n" << A << '\n';
        cout << "c = \n" << c << '\n';
    }
    
    AMI_bit_perm_object bpo(A, c);
    
    ae = AMI_BMMC_permute(&amis0, &amis1, (AMI_bit_perm_object *)&bpo);

    if (verbose) {
        cout << "After permutation, stream length = " << amis1.stream_len() <<
            '\n';
    }

    if (report_results_final) {
        ae = AMI_scan(&amis1, rptf);
    }

    return 0;
}
