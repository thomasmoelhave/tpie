// Copyright (c) 1994 Darren Vengroff
//
// File: test_ami_gp.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/1/94
//

static char test_ami_gp_id[] = "$Id: test_ami_gp.cpp,v 1.3 1995-06-20 20:15:45 darrenv Exp $";

// This is just to avoid an error message since the string above is never
// referenced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___test_ami_gp_id_compiler_fooler {
    char *pc;
    ___test_ami_gp_id_compiler_fooler *next;
} the___test_ami_gp_id_compiler_fooler = {
    test_ami_gp_id,
    &the___test_ami_gp_id_compiler_fooler
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
static char def_rrf[] = "/var/tmp/osr.txt";
static char def_frf[] = "/var/tmp/osf.txt";

static char *initial_results_filename = def_irf;
static char *rand_results_filename = def_rrf;
static char *final_results_filename = def_frf;

static bool report_results_initial = false;
static bool report_results_random = false;
static bool report_results_final = false;

static const char as_opts[] = "I:iR:rF:f";
void parse_app_opt(char c, char *optarg)
{
    switch (c) {
        case 'I':
            initial_results_filename = optarg;
        case 'i':
            report_results_initial = true;
            break;
        case 'R':
            rand_results_filename = optarg;
        case 'r':
            report_results_random = true;
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

extern int register_new;

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

    srandom(random_seed);
    
    // Set the amount of main memory:
    MM_manager.resize_heap(test_mm_size);
    register_new = 1;

    AMI_STREAM<int> amis0((unsigned int)0, test_size);
    AMI_STREAM<int> amis1((unsigned int)0, test_size);

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

    reverse_order ro;
    
    ae = AMI_general_permute(&amis0, &amis1, (AMI_gen_perm_object *)&ro);

    if (verbose) {
        cout << "After reversal, stream length = " << amis1.stream_len() <<
            '\n';
    }

    if (report_results_final) {
        ae = AMI_scan(&amis1, rptf);
    }

    return 0;
}


// Instantiate all the templates we have used.

#ifdef NO_IMPLICIT_TEMPLATES

// Instantiate templates for streams of objects.
TEMPLATE_INSTANTIATE_STREAMS(int)

// Instantiate templates for I/O using C++ streams.
TEMPLATE_INSTANTIATE_OSTREAM(int)

// Instantiate AMI_gen_permute()
TEMPLATE_INSTANTIATE_GEN_PERM(int)

// Calls to AMI_scan using various object types.
template AMI_err AMI_scan(scan_count *, AMI_STREAM<int> *);

#endif
