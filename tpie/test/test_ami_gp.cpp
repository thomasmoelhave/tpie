// Copyright (c) 1994 Darren Vengroff
//
// File: test_ami_gp.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/1/94
//
// Tests general permutation using AMI_general_permute() and 
// the AMI_gen_perm_object class. The program generates an input stream
// consisting of sequential integers and outputs a stream consisting of 
// the same integers, in reverse order.
//

#include <portability.h>

#include <versions.h>
VERSION(test_ami_gp_cpp,"$Id: test_ami_gp.cpp,v 1.14 2003-09-27 07:10:42 tavi Exp $");

// Get the application defaults.
#include "app_config.h"

// Get AMI_scan().
#include <ami_scan.h>
// Get ASCII scan objects.
#include <ami_scan_utils.h>
// Get AMI_gen_perm_object.
#include <ami_gen_perm_object.h>
// Get AMI_general_permute().
#include <ami_gen_perm.h>

#include "parse_args.h"
#include "scan_count.h"

static char def_irf[] = "osi.txt";
static char def_frf[] = "osf.txt";

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


class reverse_order : public AMI_gen_perm_object {
private:
    off_t total_size;
public:
    AMI_err initialize(off_t ts) {
        total_size = ts;
        return AMI_ERROR_NO_ERROR;
    };
    off_t destination(off_t source) {
        return total_size - 1 - source;
    };
};


int main(int argc, char **argv)
{
    AMI_err ae;
    
    parse_args(argc,argv,as_opts,parse_app_opt);

    if (verbose) {
      cout << "test_size = " << test_size << "." << endl;
        cout << "test_mm_size = " << test_mm_size << "." << endl;
        cout << "random_seed = " << random_seed << "." << endl;
    } else {
        cout << test_size << ' ' << test_mm_size << ' ' << random_seed;
    }

    TPIE_OS_SRANDOM(random_seed);
    
    // Set the amount of main memory:
    MM_manager.set_memory_limit (test_mm_size);

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
        cout << "Initial stream length = " << amis0.stream_len() << endl;
    }
    
    if (report_results_initial) {
        ae = AMI_scan(&amis0, rpti);
    }

    amis0.seek(0);

    reverse_order ro;
    
    ae = AMI_general_permute(&amis0, &amis1, (AMI_gen_perm_object *)&ro);

    if (verbose) {
        cout << "After reversal, stream length = " 
	     << amis1.stream_len() << endl;
    }

    if (report_results_final) {
        ae = AMI_scan(&amis1, rptf);
    }

    return 0;
}
