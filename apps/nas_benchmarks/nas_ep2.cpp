// Copyright (c) 1995 Darren Erik Vengroff
//
// File: nas_ep2.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 3/25/95
//

#include <portability.h>

// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

// Define it all.
#include <ami.h>
VERSION(nas_ep2_cpp,"$Id: nas_ep2.cpp,v 1.9 2004-08-12 12:37:04 jan Exp $");

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
    
    TPIE_OS_OFFSET max, remaining;
public:
    // The sums of the x and y values output so far.
    double sumx, sumy;

    // Annulus counts.
    unsigned int annulus[10];

    scan_both(double seed = NAS_S,
              TPIE_OS_OFFSET count = COUNT,
              double a = NAS_A);
                         
    virtual ~scan_both(void);
    AMI_err initialize(void);
    inline AMI_err operate(double *out_x, double *out_y, AMI_SCAN_FLAG *sf);
};


scan_both::scan_both(double seed, TPIE_OS_OFFSET count, double a) {
    this->s = seed; 
    this->max = count;
    a1 = floor(TWO_TO_MINUS_23 * a);
    a2 = a - TWO_TO_23 * a1;    
}


scan_both::~scan_both()
{
}

AMI_err scan_both::initialize(void)
{
    xlc2 = s;
    this->remaining = this->max;

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

static char def_rrf[] = "osr.txt";
static char def_frf[] = "osf.txt";

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
      cout << "test_size = " << test_size << "." << endl;
      cout << "test_mm_size = " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << "." << endl;
    } else {
        cout << test_size << ' ' << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << endl;
    }

    // Set the amount of main memory:
    MM_manager.set_memory_limit (test_mm_size);
        
    AMI_STREAM<double> amis_x;
    AMI_STREAM<double> amis_y;

    cput.reset();
    cput.start();
    
    scan_both sb(NAS_S, test_size, NAS_A);

    ae = AMI_scan(&sb, &amis_x, &amis_y);

    if (verbose) {
        cout << "Wrote Gaussians; " <<
            "stream lengths = " << amis_x.stream_len() <<
            " and " << amis_y.stream_len() << endl;        
    }    

    cout << "No. pairs: " << amis_x.stream_len() << endl;
    
    cout.precision(15);
    
    cout << "Sums: " << sb.sumx << ' ' << sb.sumy << endl << "Counts:" << endl;

    for (unsigned int ii = 0; ii < 10; ii++) {
        cout << ii << '\t' << sb.annulus[ii] << endl;
    }

    cput.stop();

    cout << cput << endl;
    
    return 0;
}
