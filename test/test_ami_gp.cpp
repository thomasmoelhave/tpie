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

// Tests general permutation using AMI_general_permute() and 
// the AMI_gen_perm_object class. The program generates an input stream
// consisting of sequential integers and outputs a stream consisting of 
// the same integers, in reverse order.

#include <tpie/portability.h>

// Get the application defaults.
#include "app_config.h"

// Get AMI_scan().
#include <tpie/scan.h>
// Get ASCII scan objects.
#include <tpie/scan_utils.h>
// Get AMI_gen_perm_object.
#include <tpie/gen_perm_object.h>
// Get AMI_general_permute().
#include <tpie/gen_perm.h>

#include "parse_args.h"
#include "scan_count.h"

using namespace tpie;

static char def_irf[] = "osi.txt";
static char def_frf[] = "osf.txt";

static char *initial_results_filename = def_irf;
static char *final_results_filename = def_frf;

static bool report_results_initial = false;
static bool report_results_final = false;

struct options app_opts[] = {
  { 12, "initial-results-filename", "", "I", 1 },
  { 13, "report-results-initial", "", "i", 0 },
  { 14, "final-results-filename", "", "F", 1 },
  { 15, "report-results-final", "", "f", 0 },
  { 0, NULL, NULL, NULL, 0 }
};

void parse_app_opts(int idx, char *opt_arg)
{
    switch (idx) {
        case 12:
            initial_results_filename = opt_arg;
        case 13:
            report_results_initial = true;
            break;
        case 14:
            final_results_filename = opt_arg;
        case 15:
            report_results_final = true;
            break;
    }
}

class reverse_order : public ami::gen_perm_object {
private:
    stream_offset_type total_size;
public:
    reverse_order() : total_size(0) {};
    
    ami::err initialize(stream_offset_type ts) {
        total_size = ts;
        return ami::NO_ERROR;
    };

    stream_offset_type destination(stream_offset_type source) {
        return total_size - 1 - source;
    };
};


int main(int argc, char **argv) {

    ami::err ae;
    
    parse_args(argc, argv, app_opts, parse_app_opts);
    
    if (verbose) {
	std::cout << "test_size = " << test_size << "." << std::endl;
        std::cout << "test_mm_size = " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << "." << std::endl;
        std::cout << "random_seed = " << random_seed << "." << std::endl;
    }
    else {
        std::cout << test_size << ' ' << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << ' ' << random_seed;
    }
    
    seed_random(random_seed);
    
    // Set the amount of main memory:
    MM_manager.set_memory_limit (test_mm_size);
    
    ami::stream<stream_offset_type> amis0;
    ami::stream<stream_offset_type> amis1;
    
    // Streams for reporting values to ascii streams.
    
    std::ofstream *osi;
    ami::cxx_ostream_scan<stream_offset_type> *rpti = NULL;

    std::ofstream *osf;
    ami::cxx_ostream_scan<stream_offset_type> *rptf = NULL;

    if (report_results_initial) {
        osi  = new std::ofstream(initial_results_filename);
        rpti = new ami::cxx_ostream_scan<stream_offset_type>(osi);
    }

    if (report_results_final) {
        osf  = new std::ofstream(final_results_filename);
        rptf = new ami::cxx_ostream_scan<stream_offset_type>(osf);
    }
    
    scan_count my_scan_count(test_size);
    
    ae = ami::scan(&my_scan_count, &amis0);
    
    if (verbose) {
        std::cout << "Initial stream length = " << amis0.stream_len() << std::endl;
    }
    
    if (report_results_initial) {
        ae = ami::scan(&amis0, rpti);
    }
    
    amis0.seek(0);
    
    reverse_order ro;
    
    ae = ami::general_permute(&amis0, &amis1, &ro);
    
    if (verbose) {
        std::cout << "After reversal, stream length = " 
		  << amis1.stream_len() << std::endl;
    }
    
    if (report_results_final) {
        ae = ami::scan(&amis1, rptf);
    }
    
    return 0;
}
