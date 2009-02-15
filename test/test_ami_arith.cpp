#include <tpie/portability.h>

#include "app_config.h"        
#include "parse_args.h"

// Get AMI_scan().
#include <tpie/scan.h>

// Get utitlities for ascii output.
#include <tpie/scan_utils.h>

// Get some scanners.
#include "scan_square.h"
#include "scan_count.h"

// Get stream arithmetic.
#include <tpie/stream_arith.h>

using namespace tpie;

static char def_crf[] = "osc.txt";
static char def_irf[] = "osi.txt";
static char def_frf[] = "osf.txt";

static char *count_results_filename = def_crf;
static char *intermediate_results_filename = def_irf;
static char *final_results_filename = def_frf;

static bool report_results_count = false;
static bool report_results_intermediate = false;
static bool report_results_final = false;

struct options app_opts[] = {
  { 10, "count-results-filename", "", "C", 1 },
  { 11, "report-results-count", "", "c", 0 },
  { 12, "intermediate-results-filename", "", "I", 1 },
  { 13, "report-results-intermediate", "", "i", 0 },
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
            intermediate_results_filename = opt_arg;
        case 13:
            report_results_intermediate = true;
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
	std::cout << "test_size = " << test_size << "." << std::endl;
        std::cout << "test_mm_size = " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << "." << std::endl;
        std::cout << "random_seed = " << random_seed << "." << std::endl;
    } 
    else {
        std::cout << test_size << ' ' << static_cast<TPIE_OS_OUTPUT_SIZE_T>(test_mm_size) << ' ' << random_seed;
    }
    
    // Set the amount of main memory:
    MM_manager.set_memory_limit (test_mm_size);
    
    ami::stream<TPIE_OS_OFFSET> amis0;
    ami::stream<TPIE_OS_OFFSET> amis1;
    ami::stream<TPIE_OS_OFFSET> amis2;
    
    // Streams for reporting values to ascii streams.
    
    std::ofstream *osc;
    std::ofstream *osi;
    std::ofstream *osf;
    ami::cxx_ostream_scan<TPIE_OS_OFFSET> *rptc = NULL;
    ami::cxx_ostream_scan<TPIE_OS_OFFSET> *rpti = NULL;
    ami::cxx_ostream_scan<TPIE_OS_OFFSET> *rptf = NULL;
    
    if (report_results_count) {
        osc  = new std::ofstream(count_results_filename);
        rptc = new ami::cxx_ostream_scan<TPIE_OS_OFFSET>(osc);
    }
    
    if (report_results_intermediate) {
        osi  = new std::ofstream(intermediate_results_filename);
        rpti = new ami::cxx_ostream_scan<TPIE_OS_OFFSET>(osi);
    }
    
    if (report_results_final) {
        osf  = new std::ofstream(final_results_filename);
        rptf = new ami::cxx_ostream_scan<TPIE_OS_OFFSET>(osf);
    }
    
    // Write some ints.
    scan_count sc(test_size);

    ae = ami::scan(&sc, &amis0);

    if (verbose) {
	std::cout << "Wrote the initial sequence of values." << std::endl;
        std::cout << "Stopped (didn't write) with ii = "
		  << sc.ii << ". operate() called " 
		  << sc.called << " times." << std::endl;
        std::cout << "Stream length = " << amis0.stream_len() << std::endl;
    }
    
    if (report_results_count) {
        ae = ami::scan(&amis0, rptc);
    }
    
    // Square them.
    scan_square<TPIE_OS_OFFSET> ss;
    
    ae = ami::scan(&amis0, &ss, &amis1);
    
    if (verbose) {
        std::cout << "Squared them; last squared was ii = "
		  << ss.ii << ". operate() called " 
		  << ss.called << " times." << std::endl;
        std::cout << "Stream length = " << amis1.stream_len() << std::endl;
    }
    
    ami::scan_div<TPIE_OS_OFFSET> sd;
    
    ae = ami::scan(&amis1, &amis0, &sd, &amis2);
    
    if (verbose) {
	std::cout << "Divided them." << std::endl
		  << "Stream length = " << amis2.stream_len() << std::endl;
    }
    
    if (report_results_final) {
        ae = ami::scan(&amis2, rptf);
    }
    
    return 0;
}
