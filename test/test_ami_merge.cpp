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

#include <tpie/portability.h>


#include <iostream>

#include "app_config.h"        
#include "parse_args.h"

// Define it all.
#include <tpie/stream.h>
#include <tpie/scan.h>
#include <tpie/merge.h>

// Utitlities for ascii output.
#include <tpie/scan_utils.h>

// Get some scanners and a merger.

#include "scan_square.h"
#include "scan_count.h"

#include "merge_interleave.h"

using namespace tpie;

static char def_crf[] = "osc.txt";
static char def_irf[] = "osi.txt";
static char def_frf[] = "osf.txt";

static char *count_results_filename = def_crf;
static char *interleave_results_filename = def_irf;
static char *final_results_filename = def_frf;

static bool report_results_count = false;
static bool report_results_interleave = false;
static bool report_results_final = false;

struct options app_opts[] = {
  { 10, "count-results-filename", "", "C", 1 },
  { 11, "report-results-count", "", "c", 0 },
  { 12, "interleave-results-filename", "", "I", 1 },
  { 13, "report-results-interleave", "", "i", 0 },
  { 14, "final-results-filename", "", "F", 1 },
  { 15, "report-results-final", "", "f", 0 },
  { 0, NULL, NULL, NULL, 0 }
};

void parse_app_opts(int idx, char *opt_arg)
{
    switch (idx) {
        case 10:
            count_results_filename = opt_arg;
        case 11:
            report_results_count = true;
            break;
        case 12:
            interleave_results_filename = opt_arg;
        case 13:
            report_results_interleave = true;
            break;
        case 14:
            final_results_filename = opt_arg;
        case 15:
            report_results_final = true;
            break;
    }
}


int main(int argc, char **argv)
{
    ami::err ae;
    
    parse_args(argc, argv, app_opts, parse_app_opts);
    
    if (verbose) {
        std::cout << "test_size = " << test_size << ".\n";
        std::cout << "test_mm_size = " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << ".\n";
        std::cout << "random_seed = " << random_seed << ".\n";
    } else {
        std::cout << test_size << ' ' << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << ' ' << random_seed;
    }
    
    // Set the amount of main memory:
    MM_manager.set_memory_limit (test_mm_size);
    
    ami::stream<stream_offset_type> amis0;
    ami::stream<stream_offset_type> amis1;
    ami::stream<stream_offset_type> amis2;
    ami::stream<stream_offset_type> amis3;

    // Streams for reporting values to ascii streams.
    
    std::ofstream *osc;
    std::ofstream *osi;
    std::ofstream *osf;
    ami::cxx_ostream_scan<stream_offset_type> *rptc = NULL;
    ami::cxx_ostream_scan<stream_offset_type> *rpti = NULL;
    ami::cxx_ostream_scan<stream_offset_type> *rptf = NULL;
    
    if (report_results_count) {
        osc  = new std::ofstream(count_results_filename);
        rptc = new ami::cxx_ostream_scan<stream_offset_type>(osc);
    }
    
    if (report_results_interleave) {
        osi  = new std::ofstream(interleave_results_filename);
        rpti = new ami::cxx_ostream_scan<stream_offset_type>(osi);
    }
    
    if (report_results_final) {
        osf  = new std::ofstream(final_results_filename);
        rptf = new ami::cxx_ostream_scan<stream_offset_type>(osf);
    }
    
    // Write some ints.
    scan_count sc(test_size);

    ae = ami::scan(&sc, &amis0);
    
    if (verbose) {
        std::cout << "Wrote the initial sequence of values.\n";
        std::cout << "Stopped (didn't write) with ii = "
		  << sc.ii << ". operate() called " << sc.called << " times.\n";
        std::cout << "Stream length = " << amis0.stream_len() << '\n';
    }

    if (report_results_count) {
        ae = ami::scan(&amis0, rptc);
    }
    
    // Square them.
    scan_square<stream_offset_type> ss;
        
    ae = ami::scan(&amis0, &ss, &amis1);

    if (verbose) {
        std::cout << "Squared them; last squared was ii = "
		  << ss.ii << ". operate() called " << ss.called << " times.\n";
        std::cout << "Stream length = " << amis1.stream_len() << '\n';
    }
    
    // Interleave the streams.
    ami::merge_interleave<stream_offset_type> im;

    ami::arity_t arity = 2;
        
    ami::stream<stream_offset_type> *amirs[2];

    amirs[0] = &amis0;
    amirs[1] = &amis1;
    
    ae = ami::single_merge(amirs, arity, &amis2, &im);

    if (verbose) {
        std::cout << "Interleaved them; operate() called " << im.called 
		  << " times.\n";
        std::cout << "Stream length = " << amis2.stream_len() << '\n';
    }
    
    if (report_results_interleave) {
        ae = ami::scan(&amis2, rpti);
    }

    // Divide the stream into two substreams, and interleave them.

    ami::stream<stream_offset_type>* amirs0 = amirs[0]; 
    ami::stream<stream_offset_type>* amirs1 = amirs[1]; 

    ae = amis2.new_substream(ami::READ_STREAM, 0, test_size-1, &amirs0);
    ae = amis2.new_substream(ami::READ_STREAM, 0, 2*test_size-1, &amirs1);

    if (verbose) {
        std::cout << "Created substreams; lengths = " 
		  << amirs[0]->stream_len() 
		  << " and " 
		  << amirs[1]->stream_len() 
		  << '\n';
    }
    
    // Get around the OS (HP_UX in particular) when using BTE_IMP_MMB
    // by seeking back to 0 in the substream, which will force the last
    // block written to be unmapped.
    ae = amis2.seek(0);
    
    ae = ami::single_merge(amirs, arity, &amis3, &im);
    
    if (verbose) {
        std::cout << "Interleaved them; operate() called " << im.called 
		  << " times.\n";        
        std::cout << "Stream length = " << amis3.stream_len() << '\n';
    }
    
    if (report_results_final) {
        ae = ami::scan(&amis3, rptf);
    }
    
    return 0;
}
