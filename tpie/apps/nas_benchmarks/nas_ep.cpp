// Copyright (c) 1995 Darren Vengroff
//
// File: nas_ep.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/22/95
//

static char nas_ep_id[] = "$Id: nas_ep.cpp,v 1.2 1995-05-02 00:56:48 dev Exp $";

// This is just to avoid an error message since the string above is never
// referenced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___nas_ep_id_compiler_fooler {
    char *pc;
    ___nas_ep_id_compiler_fooler *next;
} the___nas_ep_id_compiler_fooler = {
    nas_ep_id,
    &the___nas_ep_id_compiler_fooler
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

    if (*sf = remaining--) {
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



// The scan management object that generates the gaussian deviates.


class scan_gauss : public AMI_scan_object {
private:
    // Are we about to read the i'th input where i is even?
    bool even;
    // The input values from the last two times.
    double xj,yj;
public:
    // The sums of the x and y values output so far.
    double sumx, sumy;

    // Annulus counts.
    unsigned int annulus[10];

    scan_gauss();
    virtual ~scan_gauss();
    AMI_err initialize(void);
    inline AMI_err operate(const double &r, AMI_SCAN_FLAG *sfin,
                           double *x, double *y, AMI_SCAN_FLAG *sfout);
};



scan_gauss::scan_gauss()
{
}

scan_gauss::~scan_gauss()
{
}

AMI_err scan_gauss::initialize(void)
{
    even = false;
    sumx = sumy = 0.0;
    for (unsigned int ii = 10; ii--; ) {
        annulus[ii] = 0;
    }    
    return AMI_ERROR_NO_ERROR;
}


inline AMI_err scan_gauss::operate(const double &r, AMI_SCAN_FLAG *sfin,
                                   double *x, double *y, AMI_SCAN_FLAG *sfout)
{
    register double tj,tj2;
    register double outx, outy;
    
    if (*sfin) {
        if (even) {
            yj = 2*r - 1;
            if ((tj = xj*xj+yj*yj) <= 1.0) {
                // In the unit circle.
                tj2 = sqrt(-2.0*log(tj)/tj);
                *x = outx = xj*tj2;
                *y = outy = yj*tj2;
                sfout[0] = sfout[1] = 1;

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
               
            } else {
                sfout[0] = sfout[1] = 0;
            }
        } else {
            sfout[0] = sfout[1] = 0;
            xj = 2*r - 1;
        }
        even = !even;
        return AMI_SCAN_CONTINUE;
    } else {
        sfout[0] = sfout[1] = 0;
        return AMI_SCAN_DONE;
    }
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
        
    AMI_STREAM<double> amis_r((unsigned int)0, 2*test_size);

    cput.reset();
    cput.start();
    
    // Write the random values.

    scan_nas_psuedo_rand spr(NAS_S, 2*test_size, NAS_A);

    ae = AMI_scan(&spr, &amis_r);

    if (verbose) {
        cout << "Wrote psuedo-random numbers; " <<
            "stream length = " << amis_r.stream_len() << '\n';
    }
    
    if (report_results_random) {
        ae = AMI_scan(&amis_r, rptr);
    }

    // Make the streams read once so they give up their blocks if the
    // OS supports it.

    amis_r.persist(PERSIST_READ_ONCE);

    // Scan the stream, evaluating pairs and producing independent
    // gaussian deviates for acceptable pairs.

    AMI_STREAM<double> amis_x((unsigned int)1, test_size);
    AMI_STREAM<double> amis_y((unsigned int)1, test_size);

    scan_gauss sg;

    ae = AMI_scan(&amis_r, &sg, &amis_x, &amis_y);

    if (verbose) {
        cout << "Wrote Gaussians; " <<
            "stream lengths = " << amis_x.stream_len() <<
            " and " << amis_y.stream_len() << '\n';        
    }    

    cout << "No. pairs: " << amis_x.stream_len() << '\n';
    
    cout.precision(15);
    
    cout << "Sums: " << sg.sumx << ' ' << sg.sumy << "\nCounts:\n" ;

    for (unsigned int ii = 0; ii < 10; ii++) {
        cout << ii << '\t' << sg.annulus[ii] << '\n';
    }

    cput.stop();

    cout << cput << '\n';
    
    return 0;
}


#ifdef NO_IMPLICIT_TEMPLATES

// Instantiate templates for streams of objects.
TEMPLATE_INSTANTIATE_STREAMS(double)

// Instantiate templates for I/O using C++ streams.
TEMPLATE_INSTANTIATE_OSTREAM(double)

template AMI_err AMI_scan(scan_nas_psuedo_rand *, AMI_STREAM<double> *);

template AMI_err AMI_scan(AMI_STREAM<double> *, scan_gauss *,
                          AMI_STREAM<double> *, AMI_STREAM<double> *);

#endif

