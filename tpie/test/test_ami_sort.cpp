//
// File: test_ami_sort.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu> and
//         Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 10/7/94
//
// A test for AMI_sort().
//

// Get definitions for working with Unix and Windows
#include <portability.h>

#include <stdlib.h>

// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

// Define it all.
#include <ami_stream.h>
#include <ami_scan.h>
#include <ami_sort.h>
#include <cpu_timer.h>
VERSION(test_ami_sort_cpp,"$Id: test_ami_sort.cpp,v 1.31 2004-08-12 15:15:12 jan Exp $");

#include <ami_kb_sort.h>

// Utilities for ascii output.
#include <ami_scan_utils.h>

#include "scan_random.h"
#include "scan_diff.h" 
#include "merge_random.h"

enum comparison_mode_t {
  COMPARISON_OPERATOR,
  COMPARISON_CLASS
};

static char def_srf[] = "sorted.txt";
static char def_rrf[] = "unsorted.txt";

static char istr_name[128]; 
static char ostr_name[128];

static char *sorted_results_filename = def_srf;
static char *rand_results_filename = def_rrf;

static bool report_results_random = false;
static bool report_results_sorted = false;

static bool sort_again = false;
static bool kb_sort = false;
static comparison_mode_t comparison_mode = COMPARISON_OPERATOR;


// The command line options for this application.
struct options app_opts[] = {
  { 10, "input-stream", "Read items from given stream", "i", 1 },
  { 11, "output-sream", "Write sorted items into given stream", "o", 1 },
  { 12, "write-input-ascii", "Write unsorted items as plain text into the given file", NULL, 1 },
  { 13, "write-output-ascii", "Write sorted items as plain text into the given file", NULL, 1 },
  { 14, "comparison", "Comparison device: [o]perator or [c]lass", "c", 1 },
  { 15, "again", "Sort again with different routine (AMI_sort_V1)", "a", 0 },
  { 0, NULL, NULL, NULL, 0 }
};

void parse_app_opts(int idx, char *opt_arg) {
  switch (idx) {
  case 10:
    strncpy(istr_name, opt_arg, 128);
    break;
  case 11:
    strncpy(ostr_name, opt_arg, 128);
    break;
  case 12:
    rand_results_filename = opt_arg;
    //  case 'r':
    report_results_random = true;
    break;
  case 13:
    sorted_results_filename = opt_arg;
    //  case 's':
    report_results_sorted = true;
    break;
  case 15:
    sort_again = true;
    break;
  case 14:
    switch (opt_arg[0]) {
    case 'o': case 'O':
      comparison_mode = COMPARISON_OPERATOR;
      break;
    case 'c': case 'C':
      comparison_mode = COMPARISON_CLASS;
      break;
    default:
      cerr << "Invalid comparison device. Valid options are [o]perator and [c]lass." << endl;
      exit(1);
    }
    break;
    //  case 'k':
    //    kb_sort = !kb_sort;
    //    break;

  }
}


class int_cmp_class {
public:
  int compare(CONST int &i1, CONST int &i2) {
    return i1 - i2;
  }
};

int main(int argc, char **argv) 
{
  int_cmp_class int_cmp_obj;
  cpu_timer timer;
  AMI_err ae;
  bool random_input;

  // Log debugging info from the application, but not from the library. 
  tpie_log_init(TPIE_LOG_APP_DEBUG); 

  test_size = 0;
  istr_name[0] = ostr_name[0] = '\0';

  //  parse_args(argc, argv, as_opts, parse_app_opt);
  parse_args(argc, argv, app_opts, parse_app_opts);

  if (test_size == 0 && istr_name[0] == '\0') {
    cerr << argv[0] << ": No input size or input file specified. Use -h for help." << endl;
    exit(1);
  }

  TP_LOG_APP_DEBUG_ID("Boo");

  random_input = (istr_name[0] == '\0');

  // Set the amount of main memory:
  MM_manager.set_memory_limit (test_mm_size);

  //  AMI_STREAM<int> amis0;
  //  AMI_STREAM<int> amis1;
  AMI_STREAM<int>* istr = (istr_name[0] == '\0') ? new AMI_STREAM<int>: new AMI_STREAM<int>(istr_name);
  if (!istr->is_valid()) {
    cerr << argv[0] << ": Error while initializing input stream. Aborting." << endl;
    exit(2);
  }
  AMI_STREAM<int>* ostr = NULL;

  if (verbose) {
    cout << "BTE: ";
#ifdef BTE_STREAM_IMP_MMAP
    cout << "BTE_STREAM_IMP_MMAP " << BTE_STREAM_MMAP_BLOCK_FACTOR;
#endif
#ifdef BTE_STREAM_IMP_STDIO
    cout << "BTE_STREAM_IMP_STDIO ";
#endif
#ifdef BTE_STREAM_IMP_UFS
    cout << "BTE_STREAM_IMP_UFS " << BTE_STREAM_UFS_BLOCK_FACTOR;
#endif
#ifdef BTE_MMB_READ_AHEAD
    cout << " BTE_MMB_READ_AHEAD ";	  
#endif
    cout << endl;
    cout << "Comparison device: " 
	 << (comparison_mode == COMPARISON_OPERATOR ? "Operator": "Class")
	 << "." << endl;
    cout << "Input size: " << test_size << " items." << endl
	 << "Item size: " << sizeof(int) << " bytes." << endl
	 << "TPIE memory size: " << (TPIE_OS_LONGLONG)MM_manager.memory_limit() << " bytes." << endl;
    cout << "TPIE free memory: " << (TPIE_OS_LONGLONG)MM_manager.memory_available() << " bytes." << endl;
  }

  if (random_input) {
    // Write some ints.
    cout << "Generating input (" << test_size << " random integers)..." << flush;
    timer.start();
    scan_random rnds(test_size,random_seed);
    ae = AMI_scan(&rnds, istr);
    timer.stop();
    cout << "Done." << endl;
    if (ae != AMI_ERROR_NO_ERROR) {
      cerr << argv[0] << ": Error while generating input. Aborting." << endl;
      exit(2);
    } else {
      if (verbose) {
	cout << "Input stream length: " << istr->stream_len() << "\n";
      }
    }
    if (verbose) {
      cout << "Time taken: " << timer << endl;
    }
    timer.reset();
  } else {
    test_size = istr->stream_len();
  }

  // Streams for reporting random and/or sorted values to ascii
  // streams.    
  ofstream *oss;
  cxx_ostream_scan<int> *rpts = NULL;
  ofstream *osr;
  cxx_ostream_scan<int> *rptr = NULL;
  
  if (report_results_random) {
    osr = new ofstream(rand_results_filename);
    rptr = new cxx_ostream_scan<int>(osr);
  }
  
  if (report_results_sorted) {
    oss = new ofstream(sorted_results_filename);
    rpts = new cxx_ostream_scan<int>(oss);
  }
  
  if (report_results_random) {
    cout << "Writing input in ASCII file " << rand_results_filename << " ..." << flush;
    ae = AMI_scan(istr, rptr);
    cout << "Done." << endl;
    if (ae != AMI_ERROR_NO_ERROR) {
      cerr << argv[0] << ": Error while writing input ASCII file." << endl;
    }
  }

  if (verbose) {
    cout << "TPIE free memory: " << (TPIE_OS_LONGLONG)MM_manager.memory_available() << " bytes.\n";
  }
  cout << "Sorting input..." << flush;
  timer.start();  
  ostr = (ostr_name[0] == '\0') ? new AMI_STREAM<int>: new AMI_STREAM<int>(ostr_name); 
  if (kb_sort) {
    key_range range(KEY_MIN, KEY_MAX);
    ae = AMI_kb_sort(*istr, *ostr, range);
  } else if (comparison_mode == COMPARISON_OPERATOR) {
    ae = AMI_sort(istr, ostr);
  } else if (comparison_mode == COMPARISON_CLASS) {
    ae = AMI_sort(istr, ostr, &int_cmp_obj);
  }
  timer.stop();
  cout << "Done." << endl;
  if (ae != AMI_ERROR_NO_ERROR) {
    cerr << argv[0] << ": Error during sort (check the log). Aborting." << endl;
    exit(3);
  }
  if (verbose) {
    cout << "Sorted stream length: " << ostr->stream_len() << endl;
    cout << "Time taken: " << timer << endl;
  }
  timer.reset();

  if (verbose) {
    cout << "TPIE free memory: " << (TPIE_OS_LONGLONG)MM_manager.memory_available() << " bytes." << endl;
  }
  if (report_results_sorted) {
    cout << "Writing sorted items in ASCII file " 
	 << sorted_results_filename << " ..." << flush;
    ae = AMI_scan(ostr, rpts);
    cout << "Done.\n";
    if (ae != AMI_ERROR_NO_ERROR) {
      cerr << argv[0] << ": Error during writing of sorted ASCII file." << endl;
    }
    if (verbose) {
      cout << "TPIE free memory: " << (TPIE_OS_LONGLONG)MM_manager.memory_available() << " bytes." << endl;
    }
  }
  
  if (sort_again) {
    
    AMI_STREAM<int> amis2;
    AMI_STREAM<int> amis3;
    AMI_STREAM<scan_diff_out<int> > amisd;
    
    merge_random<int> mr;
    scan_diff<int> sd(-1);
    
    cout << "Sorting again using old sorting routine." << endl;
    if (verbose)
      cout << "TPIE free memory: " << (TPIE_OS_LONGLONG)MM_manager.memory_available() << " bytes." << endl;

    cout << "Sorting input..." << flush;
    timer.start();  
    if (comparison_mode == COMPARISON_OPERATOR) {
      ae = AMI_sort_V1(istr, &amis3);
    } else if (comparison_mode == COMPARISON_CLASS) {
      ae = AMI_sort_V1(istr, &amis3, &int_cmp_obj);
    }
    timer.stop();
    cout << "Done." << endl;

    if (ae != AMI_ERROR_NO_ERROR) {
      cerr << argv[0] << "Error during sort (check the log). Aborting." << endl;
      exit(3);
    }
    cout << "Sorted stream length: " << amis3.stream_len() << endl;
    if (verbose) {
      cout << "Time taken: " << timer << endl;
      cout << "TPIE free memory: " << (TPIE_OS_LONGLONG)MM_manager.memory_available() << " bytes.\n";
    }

    ae = AMI_scan(ostr, &amis3, &sd, &amisd);
    
    cout << "Length of diff stream: " << amisd.stream_len() << "." << endl;
  }

  delete istr;
  delete ostr;

  return 0;
}
