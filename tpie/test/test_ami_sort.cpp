// Copyright (c) 1994 Darren Erik Vengroff
//
// File: test_ami_sort.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/7/94
//
// A test for AMI_sort().

static char test_ami_sort_id[] = "$Id: test_ami_sort.cpp,v 1.9 1995-06-20 19:00:39 darrenv Exp $";

// This is just to avoid an error message since the string above is never
// refereneced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___test_ami_sort_id_compiler_fooler {
    char *pc;
    ___test_ami_sort_id_compiler_fooler *next;
} the___test_ami_sort_id_compiler_fooler = {
    test_ami_sort_id,
    &the___test_ami_sort_id_compiler_fooler
};

#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>

// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

// Define it all.
#include <ami.h>

#include <ami_kb_sort.h>

// Utitlities for ascii output.
#include <ami_scan_utils.h>

#include "scan_random.h"
#include "scan_diff.h"
#include "merge_random.h"

static char def_srf[] = "/var/tmp/oss.txt";
static char def_rrf[] = "/var/tmp/osr.txt";

static char *sorted_results_filename = def_srf;
static char *rand_results_filename = def_rrf;

static bool report_results_random = false;
static bool report_results_sorted = false;

static bool sort_again = false;

static bool use_operator = false;

static bool kb_sort = false;

static const char as_opts[] = "R:S:rsaok";
void parse_app_opt(char c, char *optarg)
{
    switch (c) {
        case 'R':
            rand_results_filename = optarg;
        case 'r':
            report_results_random = true;
            break;
        case 'S':
            sorted_results_filename = optarg;
        case 's':
            report_results_sorted = true;
            break;
        case 'a':
            sort_again = !sort_again;
            break;
        case 'o':
            use_operator = !use_operator;
            break;
        case 'k':
            kb_sort = !kb_sort;
            break;
    }
}

#if HAVE_GETRUSAGE
#define REPORT_RUSAGE(os, ru, x)				\
{								\
    if (verbose) {						\
    	os << #x " = " << ru.x << ".\n";			\
    } else {							\
    	os << ' ' << ru.x;					\
    }								\
}

#define REPORT_RUSAGE_DIFFERENCE(os, ru0, ru1, x)		\
{								\
    if (verbose) {						\
    	os << #x " = " << ru1.x - ru0.x << ".\n";		\
    } else {							\
    	os << ' ' << ru1.x - ru0.x;				\
    }								\
}
#endif

extern int register_new;

//int cc_int_cmp(const int &i1, const int &i2)
int cc_int_cmp(int &i1, int &i2)
{
    return i1 - i2;
}

static void ___dummy_1() {
    AMI_STREAM<int> *s1 = NULL, *s2 = NULL;
    
    AMI_err ae;

    ae = AMI_sort(s1,s2,cc_int_cmp);
#if 0    
    ae = AMI_sort(s1,s2);
#endif
    ___dummy_1();
}

int main(int argc, char **argv)
{

    AMI_err ae;

#if HAVE_GETRUSAGE
    rusage ru0, ru1;
#endif

    parse_args(argc,argv,as_opts,parse_app_opt);

    if (verbose) {
        cout << "test_size = " << test_size << ".\n";
        cout << "test_mm_size = " << test_mm_size << ".\n";
        cout << "random_seed = " << random_seed << ".\n";
    } else {
        cout << test_size << ' ' << test_mm_size << ' ' << random_seed;
    }
    
    // Set the amount of main memory:
    MM_manager.resize_heap(test_mm_size);
    register_new = 1;
        
    AMI_STREAM<int> amis0((unsigned int)1, test_size);
    AMI_STREAM<int> amis1((unsigned int)1, test_size);
        
    // Write some ints.
    scan_random rnds(test_size,random_seed);
    
    ae = AMI_scan(&rnds, &amis0);

    if (verbose) {
        cout << "Wrote the random values.\n";
        cout << "Stream length = " << amis0.stream_len() << '\n';
    }

    // Streams for reporting random vand/or sorted values to ascii
    // streams.
    
    ofstream *oss;
    cxx_ostream_scan<int> *rpts = NULL;
    ofstream *osr;
    cxx_ostream_scan<int> *rptr = NULL;
    
    if (report_results_random) {
        osr = new ofstream(rand_results_filename);
        rptr = new cxx_ostream_scan<int>(osr);
    }
    
    if (report_results_sorted) {
        oss = new ofstream(sorted_results_filename);
        rpts = new cxx_ostream_scan<int>(oss);
    }
    
    if (report_results_random) {
        ae = AMI_scan(&amis0, rptr);
    }

#if HAVE_GETRUSAGE
    getrusage(RUSAGE_SELF, &ru0);
#endif

    if (kb_sort) {
        key_range range(KEY_MIN, KEY_MAX);
        ae = AMI_kb_sort(amis0, amis1, range);
    } else {
        if (use_operator) {
            ae = AMI_sort(&amis0, &amis1);
        } else {
            ae = AMI_sort(&amis0, &amis1, cc_int_cmp);
        }
    }
    
#if HAVE_GETRUSAGE
    getrusage(RUSAGE_SELF, &ru1);
#endif

    if (verbose) {
        cout << "Sorted them.\n";
        cout << "Sorted stream length = " << amis1.stream_len() << '\n';
    }
    
    if (report_results_sorted) {
        ae = AMI_scan(&amis1, rpts);
    }

#if HAVE_GETRUSAGE
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_utime.tv_sec);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_utime.tv_usec);

    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_stime.tv_sec);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_stime.tv_usec);

    REPORT_RUSAGE(cout, ru1, ru_maxrss);
    REPORT_RUSAGE(cout, ru1, ru_ixrss);
    REPORT_RUSAGE(cout, ru1, ru_idrss);
    REPORT_RUSAGE(cout, ru1, ru_isrss);

    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_minflt);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_majflt);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_nswap);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_inblock);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_oublock);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_msgsnd);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_msgrcv);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_nsignals);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_nvcsw);
    REPORT_RUSAGE_DIFFERENCE(cout, ru0, ru1, ru_nivcsw);
#endif

    cout << '\n';

    if (sort_again) {
        
        AMI_STREAM<int> amis2((unsigned int)0, test_size);
        AMI_STREAM<int> amis3((unsigned int)0, test_size);
        AMI_STREAM<scan_diff_out<int> > amisd((unsigned int)0, test_size);
        
        merge_random<int> mr;
        scan_diff<int> sd(-1);

        ae = AMI_partition_and_merge(&amis1, &amis2,
                                     (AMI_merge_base<int> *)&mr);

        ae = AMI_sort(&amis2, &amis3, cc_int_cmp);
        
        ae = AMI_scan(&amis1, &amis3, &sd, &amisd);

        if (verbose) {
            cout << "Length of diff stream = " <<
                amisd.stream_len() << ".\n";
        }
    }
    
    return 0;
}
    


// Instantiate all the templates we have used.

#ifdef NO_IMPLICIT_TEMPLATES

// Instantiate templates for streams of objects.
TEMPLATE_INSTANTIATE_STREAMS(int)

// Instantiate templates for sorting objects.
TEMPLATE_INSTANTIATE_SORT_OP(int)
TEMPLATE_INSTANTIATE_SORT_CMP(int)

// Instantiate templates for I/O using C++ streams.
TEMPLATE_INSTANTIATE_OSTREAM(int)

// Templated scan/merge management objects used by this program.
TEMPLATE_INSTANTIATE_SCAN_RANDOM
TEMPLATE_INSTANTIATE_SCAN_DIFF(int)
TEMPLATE_INSTANTIATE_MERGE_RANDOM(int)


TEMPLATE_INSTANTIATE_KB_SORT(int)


#endif

