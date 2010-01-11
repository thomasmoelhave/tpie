// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

// Copyright (c) 1994 Darren Vengroff
//
// File: convex_hull.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/16/94
//
// 2-D convex hull
//

#include <portability.h>

// Get the application defaults.
#include "app_config.h"

#include <ami_scan.h>
#include <ami_sort.h>
#include <ami_stack.h>

VERSION(convex_hull_cpp,"$Id: convex_hull.cpp,v 1.2 2004-08-12 12:36:23 jan Exp $");

// Utitlities for ascii output.
#include <ami_scan_utils.h>

#include "parse_args.h"
#include "point.h"
#include "scan_random_point.h"

// A scan object that produces both the upper and lower hull as stacks.

template<class T>
class scan_ul_hull : AMI_scan_object {
public:
    AMI_stack< point <T> > *uh_stack, *lh_stack;

    scan_ul_hull(void);
    virtual ~scan_ul_hull(void);
    AMI_err initialize(void);
    AMI_err operate(const point<T> &in, AMI_SCAN_FLAG *sfin);
};

template<class T>
scan_ul_hull<T>::scan_ul_hull(void) : uh_stack(NULL), lh_stack(NULL)
{
}

template<class T>
scan_ul_hull<T>::~scan_ul_hull(void)
{
}

template<class T>
AMI_err scan_ul_hull<T>::initialize(void)
{
    return AMI_ERROR_NO_ERROR;
}


template<class T>
AMI_err scan_ul_hull<T>::operate(const point<T> &in,
                                 AMI_SCAN_FLAG *sfin)
{
    AMI_err ae;

    // If there is no more input we are done.
    if (!*sfin) {
        return AMI_SCAN_DONE;
    }

    if (!uh_stack->stream_len()) {

        // If there is nothing on the stacks then put the first point
        // on them.
        ae = uh_stack->push(in);
        if (ae != AMI_ERROR_NO_ERROR) {
            return ae;
        }

        ae = lh_stack->push(in);
        if (ae != AMI_ERROR_NO_ERROR) {
            return ae;
        }

    } else {

        // Add to the upper hull.

        {
            // Pop the last two points off.

            point<T> *p1, *p2;

            tp_assert(uh_stack->stream_len() >= 1, "Stack is empty.");
            
            uh_stack->pop(&p2);

            // If the point just popped is equal to the input, then we
            // are done.  There is no need to have both on the stack.
            
            if (*p2 == in) {
                uh_stack->push(*p2);
                return AMI_SCAN_CONTINUE;
            }
            
            if (uh_stack->stream_len() >= 1) {
                uh_stack->pop(&p1);
            } else {
                p1 = p2;
            }
            
            // While the turn is counter clockwise and the stack is
            // not empty pop another point.
            
            while (1) {                
                if (ccw(*p1,*p2,in) >= 0) {
                    // It does not turn the right way.  The points may
                    // be colinear.
                    if (uh_stack->stream_len() >= 1) {
                        // Move backwards to check another point.
                        p2 = p1;
                        uh_stack->pop(&p1);
                    } else {
                        // Nothing left to pop, so we can't move
                        // backwards.  We're done.
                        uh_stack->push(*p1);
                        if (in != *p1) {
                            uh_stack->push(in);
                        }
                        break;
                    }
                } else {
                    // It turns the right way.  We're done.
                    uh_stack->push(*p1);
                    uh_stack->push(*p2);
                    uh_stack->push(in);
                    break;
                }
            }
        }

        // Add to the lower hull.

        {
            // Pop the last two points off.

            point<T> *p1, *p2;

            tp_assert(lh_stack->stream_len() >= 1, "Stack is empty.");
            
            lh_stack->pop(&p2);

            // If the point just popped is equal to the input, then we
            // are done.  There is no need to have both on the stack.
            
            if (*p2 == in) {
                lh_stack->push(*p2);
                return AMI_SCAN_CONTINUE;
            }
            
            if (lh_stack->stream_len() >= 1) {
                lh_stack->pop(&p1);
            } else {
                p1 = p2;
            }
            
            // While the turn is clockwise and the stack is
            // not empty pop another point.
            
            while (1) {                
                if (cw(*p1,*p2,in) >= 0) {
                    // It does not turn the right way.  The points may
                    // be colinear.
                    if (lh_stack->stream_len() >= 1) {
                        // Move backwards to check another point.
                        p2 = p1;
                        lh_stack->pop(&p1);
                    } else {
                        // Nothing left to pop, so we can't move
                        // backwards.  We're done.
                        lh_stack->push(*p1);
                        if (in != *p1) {
                            lh_stack->push(in);
                        }
                        break;
                    }
                } else {
                    // It turns the right way.  We're done.
                    lh_stack->push(*p1);
                    lh_stack->push(*p2);
                    lh_stack->push(in);
                    break;
                }
            }
        }

        
    }

    return AMI_SCAN_CONTINUE;    
}


// Compute the convex hull of a set of points.

template<class T>
AMI_err convex_hull(AMI_STREAM< point<T> > *instream,
                    AMI_STREAM< point<T> > *outstream)
{
    AMI_err ae;

    point<T> *pt;

    AMI_stack< point<T> > uh;
    AMI_stack< point<T> > lh;

    AMI_STREAM< point<T> > in_sort;
        
    // Sort the points by x.

    ae = AMI_sort(instream, &in_sort);
    
    // Compute the upper hull and lower hull in a single scan.

    scan_ul_hull<T> sulh;

    sulh.uh_stack = &uh;
    sulh.lh_stack = &lh;
    
    ae = AMI_scan(&in_sort, &sulh);

    // Copy the upper hull to the output.

    uh.seek(0);
    
    while (1) {
        ae = uh.read_item(&pt);
        if (ae == AMI_ERROR_END_OF_STREAM) {
            break;
        } else if (ae != AMI_ERROR_NO_ERROR) {
            return ae;
        }

        ae = outstream->write_item(*pt);
        if (ae != AMI_ERROR_NO_ERROR) {
            return ae;
        }
    }
    
    // Reverse the lower hull, concatenating it onto the upper hull.

    while (lh.pop(&pt) == AMI_ERROR_NO_ERROR) {
        ae = outstream->write_item(*pt);
        if (ae != AMI_ERROR_NO_ERROR) {
            return ae;
        }
    }

    return AMI_ERROR_NO_ERROR;
}



static char def_rif[] = "isr.txt";
static char def_irf[] = "osi.txt";
static char def_frf[] = "osf.txt";

static char *read_input_filename = def_rif;
static char *initial_results_filename = def_irf;
static char *final_results_filename = def_frf;

static bool read_input = false;
static bool report_results_initial = false;
static bool report_results_final = false;

static const char as_opts[] = "I:iF:fR:r";
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
        case 'R':
            read_input_filename = optarg;
        case 'r':
            read_input = true;
            break;
    }
}

int main(int argc, char **argv)
{
    AMI_err ae;
    
    parse_args(argc,argv,as_opts,parse_app_opt);

    if (verbose) {
      cout << "test_size = " << test_size << "." << endl;
      cout << "test_mm_size = " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << "." << endl;
      cout << "random_seed = " << random_seed << "." << endl;
    } else {
        cout << test_size << ' ' << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << ' ' << random_seed;
    }

    seed_random(random_seed);
    
    // Set the amount of main memory:
    MM_manager.set_memory_limit (test_mm_size);
        
    AMI_STREAM< point<int> > amis0, amis1;

    // Streams for reporting values to ascii streams.

    ifstream *isi;
    cxx_istream_scan< point<int> > *readi = NULL;
    
    ofstream *osi;
    cxx_ostream_scan< point<int> > *rpti = NULL;

    ofstream *osf;
    cxx_ostream_scan< point<int> > *rptf = NULL;

    if (read_input) {
        isi = new ifstream(read_input_filename);
        readi = new cxx_istream_scan< point<int> >(isi);
    }
    
    if (report_results_initial) {
        osi = new ofstream(initial_results_filename);
        rpti = new cxx_ostream_scan< point<int> >(osi);
    }

    if (report_results_final) {
        osf = new ofstream(final_results_filename);
        rptf = new cxx_ostream_scan< point<int> >(osf);
    }

    if (read_input) {
        ae = AMI_scan(readi, &amis0);
    } else {
        scan_random_point rnds(test_size, random_seed);

        ae = AMI_scan(&rnds, &amis0);    
    }

    if (verbose) {
      cout << "Wrote the initial values." << endl;
        cout << "Stream length = " << amis0.stream_len() << endl;
    }

    if (report_results_initial) {
        ae = AMI_scan(&amis0, rpti);
    }

    ae = convex_hull(&amis0, &amis1);
    
    if (verbose) {
      cout << "Computed convex hull." << endl;
        cout << "Stream length = " << amis1.stream_len() << endl;
    }

    if (report_results_final) {
        ae = AMI_scan(&amis1, rptf);
    }

    return 0;
}
