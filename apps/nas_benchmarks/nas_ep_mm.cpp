// Copyright (c) 1995 Darren Erik Vengroff
//
// File: nas_ep_mm.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 3/25/95
//

// A main memory version that generates and tabulates but does not maintain
// the values it generates.


static char nas_ep_mm_id[] = "$Id: nas_ep_mm.cpp,v 1.1 1995-06-20 18:49:04 darrenv Exp $";

#include <iostream.h>

#include <math.h>

#include <cpu_timer.h>

#include <GetOpt.h>
#include <strstream.h>


// Constants specified in the benchmark.

#define COUNT (1024*1024)
#define NAS_S 271828183
#define NAS_A (double(5*5*5*5*5*5*5*5*5*5*5*5*5))

#define TWO_TO_23 (double(2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2))
#define TWO_TO_46 (TWO_TO_23 * TWO_TO_23)

#define TWO_TO_MINUS_23 (1.0 / TWO_TO_23)
#define TWO_TO_MINUS_46 (1.0 / TWO_TO_46)


// Compute the next uniform value using conjugate gradient.

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

static size_t test_size;

static bool use_array = false;

void parse_args(int argc, char *argv[])
{
    GetOpt go(argc, argv, "at:");
    char c;

    while ((c = go()) != -1) {
        switch (c) {
            case 't':
                istrstream(go.optarg,strlen(go.optarg)) >> test_size;
                break;
            case 'a':
                use_array = !use_array;
                break;
        }
    }
}


int main(int argc, char **argv)
{
    cpu_timer cput;

    parse_args(argc, argv);

    cout << test_size << ' ' << use_array << '\n';
    
    unsigned int pairs = 0;
    unsigned int ii;

    // Variables that were in the class scan_both.

    double s;
    double xlc1, xlc2;
    double a1, a2;
    unsigned int max, remaining;
    double sumx, sumy;
    unsigned int annulus[10];

    // The following was done at construction time in the scan object.

    s = NAS_S;
    max = test_size;
    
    a1 = floor(TWO_TO_MINUS_23 * NAS_A);
    a2 = NAS_A - TWO_TO_23 * a1;    

    // The following is from initialization time.

    xlc2 = s;
    remaining = max;

    sumx = sumy = 0.0;
    for (ii = 10; ii--; ) {
        annulus[ii] = 0;
    }    

    cput.reset();
    cput.start();

    // This loop corresponds to repeated called to operate by AMI_scan().

    double *aX;
    double *aY;
    
    if (use_array) {
        aX = new double[max];
        aY = new double[max];        
    }
    
    while (remaining) {

        register double xj, yj;
        register double tj,tj2;
        register double outx, outy;

        // This loop is broken out from the operate method.
    
        do {

            // Or until the count runs out...
        
            if (!remaining) {
                break;
            } else {
                remaining--;
            }
            
            // Generate the next two outputs of the linear congruential
            // method.
        
            xlc1 = next_xlc(xlc2, a1, a2);
            xlc2 = next_xlc(xlc1, a1, a2);

            xj = 2 * (xlc1 * TWO_TO_MINUS_46) - 1;
            yj = 2 * (xlc2 * TWO_TO_MINUS_46) - 1;

        } while ((tj = xj*xj+yj*yj) > 1.0);
        
        tj2 = sqrt(-2.0*log(tj)/tj);
        outx = xj*tj2;
        outy = yj*tj2;

        if (use_array) {
            aX[pairs] = outx;
            aY[pairs] = outy;            
        }
        
        pairs++;
        
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
            for (ii = 0; ii++ < 9; ) {
                if (m < ii) {                   
                    annulus[ii-1]++;
                    break;
                }
            }
        }
    }

    // This corresponds to reporting the figures at the end.

    cout << "No. pairs: " << pairs << '\n';
    
    cout.precision(15);
    
    cout << "Sums: " << sumx << ' ' << sumy << "\nCounts:\n" ;

    for (ii = 0; ii < 10; ii++) {
        cout << ii << '\t' << annulus[ii] << '\n';
    }

    cput.stop();

    cout << cput << '\n';   
    
    return 0;    
}



