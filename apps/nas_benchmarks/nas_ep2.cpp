// Copyright (c) 1995 Darren Erik Vengroff
//
// File: nas_ep2.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 3/25/95
//

static char nas_ep2_id[] = "$Id: nas_ep2.cpp,v 1.3 1999-02-03 21:58:53 tavi Exp $";

// This is just to avoid an error message since the string above is never
// referenced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___nas_ep2_id_compiler_fooler {
    char *pc;
    ___nas_ep2_id_compiler_fooler *next;
} the___nas_ep2_id_compiler_fooler = {
    nas_ep2_id,
    &the___nas_ep2_id_compiler_fooler
};

#include <iostream.h>
#include <fstream.h>


// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

// Define it all.
#include <ami.h>

// Utitlities for ascii output.
#include <ami_scan_utils.h>

#include <cpu_timer.h>

// Constants specified in the benchmark.

#define COUNT (1024*1024)
#define NAS_S 271828183
#define NAS_A (double(5*5*5*5*5*5*5*5*5*5*5*5*5))

#define TWO_TO_23 (double(2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2))
#define TWO_TO_46 (TWO_TO_23 * TWO_TO_23)

#define TWO_TO_MINUS_23 (1.0 / TWO_TO_23)
#define TWO_TO_MINUS_46 (1.0 / TWO_TO_46)

// A combined scan managemnt object that writes the Gaussians in one
// I/O pass.

class scan_both : AMI_scan_object {
private:
    // The seed.
    double s;

    // The last values from the linear congruential method.
    double xlc1, xlc2;

    // A cache for the multiplicative factor a.
    double a1, a2;
    
    unsigned int max, remaining;
public:
    // The sums of the x and y values output so far.
    double sumx, sumy;

    // Annulus counts.
    unsigned int annulus[10];

    scan_both(double seed = NAS_S,
              unsigned int count = COUNT,
              double a = NAS_A);
                         
    virtual ~scan_both(void);
    AMI_err initialize(void);
    inline AMI_err operate(double *out_x, double *out_y, AMI_SCAN_FLAG *sf);
};


scan_both::scan_both(double seed, unsigned int count, double a) :
        s(seed), 
        max(count)
{
        a1 = floor(TWO_TO_MINUS_23 * a);
        a2 = a - TWO_TO_23 * a1;    
}


scan_both::~scan_both()
{
}

AMI_err scan_both::initialize(void)
{
    xlc2 = s;
    remaining = max;

    sumx = sumy = 0.0;
    for (unsigned int ii = 10; ii--; ) {
        annulus[ii] = 0;
    }    

    return AMI_ERROR_NO_ERROR;
}

inline double next_xlc(double xlc, double a1, double a2)
{    
    register double b1, b2;
    register double t1, t2, t3, t4, t5;

        b1 = floor(TWO_TO_MINUS_23 * xlc);
        b2 = xlc - TWO_TO_23 * b1;

        t1 = a1*b2 + a2*b1;
        t2 = floor(TWO_TO_MINUS_23 * t1);
        t3 = t1 - TWO_TO_23 * t2;
        t4 = TWO_TO_23 * t3 + a2*b2;
        t5 = floor(TWO_TO_MINUS_46 * t4);

        return (t4 - TWO_TO_46 * t5);
}


inline AMI_err scan_both::operate(double *x, double *y,
                                  AMI_SCAN_FLAG *sf)
{
    register double xj, yj;
    register double tj,tj2;
    register double outx, outy;

    // Loop until a pair within the unit circle is found.

    do {

        // Or until the count runs out...
        
        if (!remaining--) {
            sf[0] = sf[1] = 0;
            return AMI_SCAN_DONE;
        }
            
        // Generate the next two outputs of the linear congruential
        // method.
        
        xlc1 = next_xlc(xlc2, a1, a2);
        xlc2 = next_xlc(xlc1, a1, a2);

        xj = 2 * (xlc1 * TWO_TO_MINUS_46) - 1;
        yj = 2 * (xlc2 * TWO_TO_MINUS_46) - 1;

    } while ((tj = xj*xj+yj*yj) > 1.0);
        
    tj2 = sqrt(-2.0*log(tj)/tj);
    *x = outx = xj*tj2;
    *y = outy = yj*tj2;

    // Keep a running total.
    sumx += outx;
    sumy += outy;

    // Count values in succesive square annuli.  Moving
    // outward probably beats binary seach since they are
    // concentrated near the origin.
    {
        outx = fabs(outx);
        outy = fabs(outy);
        register double m = (outx > outy) ? outx : outy;  
        for (unsigned int ii = 0; ii++ < 9; ) {
            if (m < ii) {                   
                annulus[ii-1]++;
                break;
            }
        }
    }

    sf[0] = sf[1] = 1;
    return AMI_SCAN_CONTINUE;

}



//
//
//

static char def_rrf[] = "/var/tmp/osr.txt";
static char def_frf[] = "/var/tmp/osf.txt";

static char *rand_results_filename = def_rrf;
static char *filtered_results_filename = def_frf;

static bool report_results_random = false;
static bool report_results_filtered = false;

static const char as_opts[] = "R:F:rf";

void parse_app_opt(char c, char *optarg)
{
    switch (c) {
        case 'R':
            rand_results_filename = optarg;
        case 'r':
            report_results_random = true;
            break;
        case 'F':
            filtered_results_filename = optarg;
        case 'f':
            report_results_filtered = true;
            break;
    }
}


extern int register_new;

int main(int argc, char **argv)
{
    cpu_timer cput;
    
    AMI_err ae;

    parse_args(argc,argv,as_opts,parse_app_opt);

    // Streams for reporting random vand/or filtered values to ascii
    // streams.
    
    ofstream *osf;
    cxx_ostream_scan<double> *rptf = NULL;
    ofstream *osr;
    cxx_ostream_scan<double> *rptr = NULL;
    
    if (report_results_random) {
        osr = new ofstream(rand_results_filename);
        rptr = new cxx_ostream_scan<double>(osr);
    }
    
    if (report_results_filtered) {
        osf = new ofstream(filtered_results_filename);
        rptf = new cxx_ostream_scan<double>(osf);
    }

    if (verbose) {
        cout << "test_size = " << test_size << ".\n";
        cout << "test_mm_size = " << test_mm_size << ".\n";
    } else {
        cout << test_size << ' ' << test_mm_size << '\n';
    }

    // Set the amount of main memory:
    MM_manager.resize_heap(test_mm_size);
    register_new = 1;
        
    AMI_STREAM<double> amis_x;
    AMI_STREAM<double> amis_y;

    cput.reset();
    cput.start();
    
    scan_both sb(NAS_S, test_size, NAS_A);

    ae = AMI_scan(&sb, &amis_x, &amis_y);

    if (verbose) {
        cout << "Wrote Gaussians; " <<
            "stream lengths = " << amis_x.stream_len() <<
            " and " << amis_y.stream_len() << '\n';        
    }    

    cout << "No. pairs: " << amis_x.stream_len() << '\n';
    
    cout.precision(15);
    
    cout << "Sums: " << sb.sumx << ' ' << sb.sumy << "\nCounts:\n" ;

    for (unsigned int ii = 0; ii < 10; ii++) {
        cout << ii << '\t' << sb.annulus[ii] << '\n';
    }

    cput.stop();

    cout << cput << '\n';
    
    return 0;
}
