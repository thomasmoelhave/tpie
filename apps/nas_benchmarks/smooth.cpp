// Copyright (c) 1995 Darren Erik Vengroff
//
// File: smooth.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 3/29/95
//

#include <portability.h>

#include "app_config.h"        
#include "parse_args.h"

// Define it all.
#include <ami.h>
VERSION(smooth_cpp,"$Id: smooth.cpp,v 1.6 2003-09-12 01:58:14 tavi Exp $");

// Utitlities for ascii output.
#include <ami_scan_utils.h>

// Get matrices.

#include <ami_matrix.h>
#include <ami_matrix_fill.h>
#include "fill_value.h"

#include <ami_sparse_matrix.h>

#include "scan_build_smoother.h"

#include <cpu_timer.h>

// The uniform random variate generator from the NAS parallel
// benchmarks.

// Constants specified in the benchmark.

#define COUNT (1024*1024)
#define NAS_S 271828183
#define NAS_A (double(5*5*5*5*5*5*5*5*5*5*5*5*5))

#define TWO_TO_23 (double(2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2))
#define TWO_TO_46 (TWO_TO_23 * TWO_TO_23)

#define TWO_TO_MINUS_23 (1.0 / TWO_TO_23)
#define TWO_TO_MINUS_46 (1.0 / TWO_TO_46)

// The scan managemnt object that writes the random numbers.

class scan_nas_psuedo_rand : AMI_scan_object {
private:
    // The seed.
    double s;

    // The last value output.
    double x;

    // A cache for the multiplicative factor a.
    double a1, a2;
    
    unsigned int max, remaining;
public:
    scan_nas_psuedo_rand(double seed = NAS_S,
                         unsigned int count = COUNT,
                         double a = NAS_A);
                         
    virtual ~scan_nas_psuedo_rand(void);
    AMI_err initialize(void);
    inline AMI_err operate(double *out, AMI_SCAN_FLAG *sf);
};


scan_nas_psuedo_rand::scan_nas_psuedo_rand(double seed,
                                           unsigned int count,
                                           double a) :
                                                   s(seed),
                                                   max(count)
{
        a1 = floor(TWO_TO_MINUS_23 * a);
        a2 = a - TWO_TO_23 * a1;    
}


scan_nas_psuedo_rand::~scan_nas_psuedo_rand()
{
}

AMI_err scan_nas_psuedo_rand::initialize(void)
{
    x = s;
    remaining = max;

    return AMI_ERROR_NO_ERROR;
}

inline AMI_err scan_nas_psuedo_rand::operate(double *out, AMI_SCAN_FLAG *sf)
{
    register double b1, b2;
    register double t1, t2, t3, t4, t5;

    if ((*sf = remaining--)) {
        b1 = floor(TWO_TO_MINUS_23 * x);
        b2 = x - TWO_TO_23 * b1;

        t1 = a1*b2 + a2*b1;
        t2 = floor(TWO_TO_MINUS_23 * t1);
        t3 = t1 - TWO_TO_23 * t2;
        t4 = TWO_TO_23 * t3 + a2*b2;
        t5 = floor(TWO_TO_MINUS_46 * t4);

        x = t4 - TWO_TO_46 * t5;

        *out = x * TWO_TO_MINUS_46;
             
        return AMI_SCAN_CONTINUE;

    } else {
        return AMI_SCAN_DONE;
    }
}

// End of the uniform scanner.


static char def_srf[] = "osc.txt";
static char def_brf[] = "osi.txt";
static char def_frf[] = "osf.txt";

static char *smooth_results_filename = def_srf;
static char *banded_results_filename = def_brf;
static char *final_results_filename = def_frf;

static bool report_results_smooth = false;
static bool report_results_banded = false;
static bool report_results_final = false;

static unsigned int niter = 1;

static const char as_opts[] = "sS:bB:fF:N:";
void parse_app_opt(char c, char *optarg)
{
    switch (c) {
        case 'S':
            smooth_results_filename = optarg;
        case 's':
            report_results_smooth = true;
            break;
        case 'B':
            banded_results_filename = optarg;
        case 'b':
            report_results_banded = true;
            break;
        case 'F':
            final_results_filename = optarg;
        case 'f':
            report_results_final = true;
            break;
        case 'N':
            istrstream(optarg,strlen(optarg)) >> niter;
            break;
    }
}


int main(int argc, char **argv)
{
    AMI_err ae;

    parse_args(argc,argv,as_opts,parse_app_opt);

    // The actual size of the vector ans matrices is test_size^3.

    unsigned int ts3 = test_size * test_size * test_size;
    
    if (verbose) {
      cout << "test_size = " << test_size << "." << endl;
      cout << "test_mm_size = " << test_mm_size << "." << endl;
      cout << "random_seed = " << random_seed << "." << endl;
    } else {
        cout << test_size << ' ' << test_mm_size << ' ' << ts3 << ' '
             << niter << ' ';
    }
    
    // Set the amount of main memory:
    MM_manager.set_memory_limit (test_mm_size);

    // A sparse matrix smoother.
    
    AMI_sparse_matrix<double> smoother(ts3, ts3);
    AMI_sparse_matrix<double> smoother_b(ts3, ts3);

    // A vector to multiply the sparse matrix by

    AMI_matrix<double> ev0(ts3, 1);

    // The result vector
    
    AMI_matrix<double> ev1(ts3, 1);

    // Streams for reporting values to ascii streams.
    
    ofstream *oss;
    ofstream *osb;
    ofstream *osf;
    cxx_ostream_scan< AMI_sm_elem<double> > *rpts = NULL;
    cxx_ostream_scan< AMI_sm_elem<double> > *rptb = NULL;
    cxx_ostream_scan<double> *rptf = NULL;
    
    if (report_results_smooth) {
        oss = new ofstream(smooth_results_filename);
        rpts = new cxx_ostream_scan< AMI_sm_elem<double> >(oss);
    }

    if (report_results_banded) {
        osb = new ofstream(banded_results_filename);
        rptb = new cxx_ostream_scan< AMI_sm_elem<double> >(osb);
    }
    
    if (report_results_final) {
        osf = new ofstream(final_results_filename);
        rptf = new cxx_ostream_scan<double>(osf);
    }
    
    // Write the smoother matrix.

    scan_build_smoother sbs(test_size);

    ae = AMI_scan(&sbs, (AMI_STREAM< AMI_sm_elem<double> > *)&smoother);

    if (report_results_smooth) {
        ae = AMI_scan((AMI_STREAM< AMI_sm_elem<double> > *)&smoother,
                      rpts);
    }
    
    // Write the elements of the vector.

#if 0    
    fill_value<double> fv;

    fv.set_value(3.14159);

    ae = AMI_matrix_fill(&ev0, (AMI_matrix_filler<double> *)&fv);
#else

    scan_nas_psuedo_rand spr(NAS_S, ts3, NAS_A);
    
    ae = AMI_scan(&spr, (AMI_STREAM<double> *)&ev0);

#endif
    
    // Multiply the two

    cpu_timer cput0, cput1;
        
    unsigned int rows_per_band, total_bands;
        
    if (verbose) {
      cout << "Generating banded matrix." << endl;
    }

    cput0.reset();
    cput0.start();

    // Compute band information.

    ae = AMI_sparse_band_info(smoother, rows_per_band, total_bands);

    // Bandify the sparse matrix.
            
    ae = AMI_sparse_bandify(smoother, smoother_b, rows_per_band);

    if (report_results_banded) {
        cput0.stop();
        ae = AMI_scan((AMI_STREAM< AMI_sm_elem<double> > *)&smoother_b,
                      rptb);
        cput0.start();
    }

    // Do the multiplication.

    cput1.reset();
    cput1.start();

    for (unsigned int ii = 0; ii < niter; ii++ ) {
        if (verbose) {
            cout << "Iteration " << ii+1 << endl;
        }
        if (ii & 1) {
            ae = AMI_sparse_mult_scan_banded(smoother_b, ev1, ev0, ts3,
                                             ts3, rows_per_band);
        } else {
            ae = AMI_sparse_mult_scan_banded(smoother_b, ev0, ev1, ts3,
                                             ts3, rows_per_band);
        }
        
        if (verbose) {
	  cout << "Multiplied them." << endl;
            cout << "Stream length = " << ev1.stream_len() << endl;
        }
    }
    
    cput1.stop();
    cput0.stop();

    if (report_results_final) {
        if (niter & 1) {
            ae = AMI_scan((AMI_STREAM<double> *)&ev1, rptf);
        } else {
            ae = AMI_scan((AMI_STREAM<double> *)&ev0, rptf);
        }
    }
    
    cout << cput0 << ' ' << cput1 << endl;
            
    return 0;
}




