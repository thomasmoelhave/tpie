// Copyright (c) 1995 Darren Vengroff
//
// File: nas_cg.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/23/95
//

// From the benchmark specification.
#define PROBLEM_SIZE 1400
#define NITER 15
#define LAMBDA 10
#define ZETA_REF (8.59717750786234)

// The file to read A from.
#define A_INPUT_FILENAME "A.sample.txt"

#include <iostream>
#include <fstream>
#include <strstream>

using std::cout;
using std::cerr;
using std::ofstream;
using std::ifstream;
using std::istrstream;

#include <cpu_timer.h>

// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

// Define it all.
#include <ami.h>
VERSION(nas_cg_cpp,"$Id: nas_cg.cpp,v 1.4 2003-04-20 23:51:40 tavi Exp $");

// Utitlities for ascii output.
#include <ami_scan_utils.h>

// Dense matric support.
#include <ami_matrix.h>

// Sparse matrix support.
#include <ami_sparse_matrix.h>


#include "scan_value.h"
#include "scan_inner_product.h"
#include "scan_copy2.h"
#include "scan_awc.h"
#include "scan_square_sum.h"

AMI_err solve_system(AMI_sparse_matrix<double> &A,
                     AMI_matrix<double> &z,
                     AMI_matrix<double> &x,
                     double &rnorm, unsigned int n,
                     unsigned int rpb)
{
    AMI_err ae;
    
    AMI_matrix<double> z0(n, 1), z1(n, 1);
    AMI_matrix<double> p0(n, 1), p1(n, 1);
    AMI_matrix<double> r0(n, 1), r1(n, 1);

    AMI_matrix<double> q(n, 1);

    double rho, rho0, alpha, beta;

    static scan_inner_product<double> sip;
    static scan_add_with_coefficient<double> sawc;
    static scan_square_sum<double> sss;
    
    // Fill z with zeroes.
    {
        scan_value<double> sv0(0.0, n);

        ae = AMI_scan(&sv0, (AMI_STREAM<double> *)&z0);
    }
    
    // Copy x into r and p.
    {
        scan_copy2<double> sc2;
    
        ae = AMI_scan((AMI_STREAM<double> *)&x, &sc2,
                      (AMI_STREAM<double> *)&r0,
                      (AMI_STREAM<double> *)&p0);
    }

    // Compute the initial value of rho.  We take the inner product of
    // r and p, both of which were jsut set to x.
    
    {
        ae = AMI_scan((AMI_STREAM<double> *)&r0,
                      (AMI_STREAM<double> *)&p0, &sip);
        rho = sip.result();
    }

    // Streams to be used for input and ouput of each of the following
    // variables during a given interation.
    
    AMI_matrix<double> *z_in, *z_out, *r_in, *r_out, *p_in, *p_out;

    // The main loop.
    
    for (unsigned int ii = 25; ii--; ) {

        // Select the proper input and ouput streams.
        if (ii & 0x1) {
            z_in = &z1;
            r_in = &r1;
            p_in = &p1;
            z_out = &z0;
            r_out = &r0;
            p_out = &p0;            
        } else {
            z_in = &z0;
            r_in = &r0;
            p_in = &p0;

            r_out = &r1;
            p_out = &p1;

            if (ii) {
                z_out = &z1;
            } else {
                // The final iteration is special, since it provides the
                // solution back to the caller.
                z_out = &z;
            }
        }
        
        // q = A p

        ae = AMI_sparse_mult_scan_banded(A, *p_in, q, n, n, rpb);
                
        // alpha = rho / ( pT q )

        ae = AMI_scan((AMI_STREAM<double> *)p_in,
                      (AMI_STREAM<double> *)&q, &sip);
        alpha = rho / sip.result();

        // z = z + alpha p

        sawc.set_coefficient(alpha);        
        ae = AMI_scan((AMI_STREAM<double> *)z_in,
                      (AMI_STREAM<double> *)p_in, &sawc,
                      (AMI_STREAM<double> *)z_out);

        // The following are not needed on the last iteration.

        if (ii) {

            // rho0 = rho

            rho0 = rho;

            // r = r - alpha q

            sawc.set_coefficient(-alpha);        
            ae = AMI_scan((AMI_STREAM<double> *)r_in,
                          (AMI_STREAM<double> *)&q,
                          &sawc, (AMI_STREAM<double> *)r_out);

            // rho = rT r

            ae = AMI_scan((AMI_STREAM<double> *)r_out, &sss);
            rho = sss.result();

            // beta = rho / rho0
            
            beta = rho / rho0;

            // p = r + beta p

            sawc.set_coefficient(beta);        
            ae = AMI_scan((AMI_STREAM<double> *)r_out,
                          (AMI_STREAM<double> *)p_in,
                          &sawc, (AMI_STREAM<double> *)p_out);

        }
        
    }

    // Compute the residual norm explicitly

    {
        static AMI_matrix<double> Az(n, 1);
        static AMI_matrix<double> x_minus_Az(n, 1);

        AMI_scan_sub<double> ssub;
        
        ae = AMI_sparse_mult_scan_banded(A, z, Az, n, n, rpb);

        ae = AMI_scan((AMI_STREAM<double> *)&x,
                      (AMI_STREAM<double> *)&Az, &ssub,
                      (AMI_STREAM<double> *)&x_minus_Az);

        ae = AMI_scan((AMI_STREAM<double> *)&x_minus_Az, &sss);
        
        rnorm = sqrt(sss.result());

    }

    return AMI_ERROR_NO_ERROR;
}


// Input file name.

static char default_filename[] = A_INPUT_FILENAME;

static char *input_filename = default_filename;

// Problem size.

static unsigned int problem_size = PROBLEM_SIZE;

static double zeta_ref = ZETA_REF;

static unsigned int lambda = LAMBDA;

static unsigned int niter = NITER;

// Application specific options.

static const char as_opts[] = "I:n:Z:L:N:";

void parse_app_opt(char c, char *optarg)
{
    switch (c) {
        case 'I':
            input_filename = optarg;
            break;
        case 'N':
            istrstream(optarg,strlen(optarg)) >> niter;
            break;            
        case 'n':
            istrstream(optarg,strlen(optarg)) >> problem_size;
            break;            
        case 'Z':
            istrstream(optarg,strlen(optarg)) >> zeta_ref;
            break;            
        case 'L':
            istrstream(optarg,strlen(optarg)) >> lambda;
            break;            
    }
}


int main(int argc, char **argv)
{
    unsigned int n;
    
    AMI_err ae;

    cpu_timer cput;
    
    parse_args(argc,argv,as_opts,parse_app_opt);

    // Set the amount of main memory:
    MM_manager.set_memory_limit (test_mm_size);

    n = problem_size;
    
    cout << test_mm_size << ' ' << n << '\n';
    
    AMI_matrix<double> x(n, 1);
    AMI_matrix<double> z(n, 1);

    AMI_sparse_matrix<double> A(n,n), A_raw(n,n);

    scan_inner_product<double> sip;
    scan_square_sum<double> sss;

    unsigned int rpb, tb;
    
    double rnorm, zeta;
    
    // Fill the vector x.

    {
        scan_value<double> sv1(1.0, n);

        ae = AMI_scan(&sv1, (AMI_STREAM<double> *)&x);
    }
    
    // Read the sparse matrix A.

    {
        ifstream isA(input_filename);
        cxx_istream_scan<AMI_sm_elem<double> > readA(&isA);
    
        ae = AMI_scan(&readA, (AMI_STREAM< AMI_sm_elem<double> > *)&A_raw);
    }

    if (verbose) {
        cerr << "Read raw A; stream length = " << A_raw.stream_len() << '\n';
    }

    // Preprocess A into band(s).
    
    ae = AMI_sparse_band_info(A_raw, rpb, tb);
    
    ae = AMI_sparse_bandify(A_raw, A, rpb);
    
    if (verbose) {
        cerr << "Banded A; stream length = " << A.stream_len() << '\n';
        cerr << "Rows per band = " << rpb << '\n';
        cerr << "Total bands = " << tb << '\n';
    }

    // High precision output.
    cout.precision(15);

    cput.reset();
    cput.start();
    
    // Iterate to solve the problem.

    for (unsigned int ii = 1; ii <= niter; ii++ ) {

        if (verbose) {
            cerr << "Iteration " << ii << '\n';
        }
        
        ae = solve_system(A, z, x, rnorm, n, rpb);

        ae = AMI_scan((AMI_STREAM<double> *)&x,
                      (AMI_STREAM<double> *)&z, &sip);

        zeta = lambda + 1.0 / sip.result();

        cout << ii << ' ' << rnorm << ' ' << zeta << '\n';

        ae = AMI_scan((AMI_STREAM<double> *)&z, &sss);

        double znorminv = 1.0 / sqrt(sss.result());

        AMI_scan_scalar_mult<double> ssmult(znorminv);
        
        ae = AMI_scan((AMI_STREAM<double> *)&z, &ssmult,
                      (AMI_STREAM<double> *)&x);
    }

    cput.stop();
    
    cout << "zeta = " << zeta << '\n';
    cout << "ref. = " << zeta_ref << '\n';
    cout << "diff = " << zeta - zeta_ref << '\n';

    cout << cput << '\n';
    
    return 0;
    
}
