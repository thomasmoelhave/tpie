//
// File: test_ami_sort.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu> and
//         Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 10/7/94
//
// A test for AMI_sort().
//
#include <sys/types.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <iostream>
#include <fstream>

using std::cout;
using std::cerr;
using std::flush;
using std::ofstream;

// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

// Define it all.
#include <ami_stream.h>
#include <ami_scan.h>
#include <ami_sort.h>
#include <cpu_timer.h>
VERSION(test_ami_sort_cpp,"$Id: test_ami_sort.cpp,v 1.28 2003-09-11 17:18:55 tavi Exp $");

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

static char def_srf[] = "/var/tmp/sorted.txt";
static char def_rrf[] = "/var/tmp/unsorted.txt";

static char istr_name[128]; 
static char ostr_name[128];

static char *sorted_results_filename = def_srf;
static char *rand_results_filename = def_rrf;

static bool report_results_random = false;
static bool report_results_sorted = false;

static bool sort_again = false;
static bool kb_sort = false;
static comparison_mode_t comparison_mode = COMPARISON_OPERATOR;

void print_usage() {
  cerr << "Usage: test_ami_sort \n"
       << "\t-t <test_size> (set input size)\n"
       << "\t[-m <memory_size_in_bytes>] (set TPIE memory size)\n"
       << "\t[-z <random_seed>] (set random seed)\n";

  cerr << "\t[-i <input_stream>]\n"
       << "\t[-o <output_stream>]\n"
       << "\t[-r] (write the unsorted items in " << rand_results_filename << ")\n"
       << "\t[-R <file_name>] (write the unsorted items in the given file)\n"
       << "\t[-s] (write the sorted items in " << sorted_results_filename << ")\n"
       << "\t[-S <file_name> (write the sorted items in the given file)]\n"
       << "\t[-c o|c] (comparison device: Operator|Class)\n"
       << "\t[-a] (sort again with different sorting routine)\n"
    ;
}

// The command line options for this application. See getopt(3).
static const char as_opts[] = "R:S:rsac:ki:o:";
void parse_app_opt(char c, char *optarg)
{
  switch (c) {
  case 'i':
    strncpy(istr_name, optarg, 128);
    break;
  case 'o':
    strncpy(ostr_name, optarg, 128);
    break;
  case 'R':
    rand_results_filename = optarg;
  case 'r':
    report_results_random = true;
    break;
  case 'S':
    sorted_results_filename = optarg;
  case 's':
    report_results_sorted = true;
    break;
  case 'a':
    sort_again = true;
    break;
  case 'c':
    switch (*optarg) {
    case 'o': case 'O':
      comparison_mode = COMPARISON_OPERATOR;
      break;
    case 'c': case 'C':
      comparison_mode = COMPARISON_CLASS;
      break;
    }
    break;
  case 'k':
    kb_sort = !kb_sort;
    break;
  default:
    print_usage();
    exit(1);
    break;
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
  long elapsed;
  AMI_err ae;
  istr_name[0] = ostr_name[0] = '\0';

  // Log debugging info from the application, but not from the library. 
  tpie_log_init(TPIE_LOG_APP_DEBUG); 
 
  test_size = 0;
  parse_args(argc, argv, as_opts, parse_app_opt);
  if (test_size == 0) {
    cerr << argv[0] << ": no input size specified.\n";
    print_usage();
    exit(1);
  }
  // Set the amount of main memory:
  MM_manager.set_memory_limit (test_mm_size);

  //  AMI_STREAM<int> amis0;
  //  AMI_STREAM<int> amis1;
  AMI_STREAM<int>* istr = (istr_name[0] == '\0') ? new AMI_STREAM<int>: new AMI_STREAM<int>(istr_name);
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
    cout << "\n";
    cout << "Comparison device: " 
	 << (comparison_mode == COMPARISON_OPERATOR ? "Operator": "Class")
	 << ".\n";
    cout << "Input size: " << test_size << " items.\n"
	 << "Item size: " << sizeof(int) << " bytes.\n"
	 << "TPIE memory size: " << MM_manager.memory_limit() << " bytes.\n";
    cout << "TPIE free memory: " << MM_manager.memory_available() << " bytes.\n";
  }


  // Write some ints.
  cout << "Generating input (" << test_size << " random integers)..." << flush;
  timer.start();
  scan_random rnds(test_size,random_seed);
  ae = AMI_scan(&rnds, istr);
  timer.stop();
  cout << "Done.\n";
  if (ae != AMI_ERROR_NO_ERROR) {
    cerr << "Error during writing stream.\n"
	 << "Aborting.\n";
    exit(1);
  } else
    cout << "\tInput stream length: " << istr->stream_len() << "\n";
  cout << "\tTime taken: " << timer << "\n";

  timer.reset();
  
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
    cout << "Writing input in ASCII file " 
	 << rand_results_filename << " ..." << flush;
    ae = AMI_scan(istr, rptr);
    cout << "Done.\n";
    if (ae != AMI_ERROR_NO_ERROR) {
      cerr << "Error during writing of input ASCII file.\n";
    }
  }

  if (verbose)
    cout << "TPIE free memory: " << MM_manager.memory_available() << " bytes.\n";
  
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
  cout << "Done.\n";
  if (ae != AMI_ERROR_NO_ERROR) {
    cerr << "Error during sort (check the log).\n"
	 << "Aborting.\n";
    exit(1);
  }
  cout << "\tSorted stream length: " << ostr->stream_len() << '\n';
  cout << "\tTime taken: " << timer << "\n";
  timer.reset();

  if (verbose)
    cout << "TPIE free memory: " << MM_manager.memory_available() << " bytes.\n";

  if (report_results_sorted) {
    cout << "Writing sorted items in ASCII file " 
	 << sorted_results_filename << " ..." << flush;
    ae = AMI_scan(ostr, rpts);
    cout << "Done.\n";
    if (ae != AMI_ERROR_NO_ERROR) {
      cerr << "Error during writing of sorted ASCII file.\n";
    }
    if (verbose)
      cout << "TPIE free memory: " << MM_manager.memory_available() << " bytes.\n";
  }
  
  if (sort_again) {
    
    AMI_STREAM<int> amis2;
    AMI_STREAM<int> amis3;
    AMI_STREAM<scan_diff_out<int> > amisd;
    
    merge_random<int> mr;
    scan_diff<int> sd(-1);
    
    cout << "Sorting again using old sorting routine.\n";
    if (verbose)
      cout << "TPIE free memory: " << MM_manager.memory_available() << " bytes.\n";

    //    ae = AMI_generalized_partition_and_merge(&amis1, &amis2,
    //			 (merge_random<int> *)&mr);
    
    cout << "Sorting input..." << flush;
    timer.start();  
    if (comparison_mode == COMPARISON_OPERATOR) {
      ae = AMI_sort_V1(istr, &amis3);
    } else if (comparison_mode == COMPARISON_CLASS) {
      ae = AMI_sort_V1(istr, &amis3, &int_cmp_obj);
    }
    timer.stop();
    cout << "Done.\n";
    if (ae != AMI_ERROR_NO_ERROR) {
      cerr << "Error during sort (check the log).\n"
	   << "Aborting.\n";
      exit(1);
    }
    cout << "\tSorted stream length: " << amis3.stream_len() << '\n';
    cout << "\tTime taken: " << timer << "\n";
    
    if (verbose)
      cout << "TPIE free memory: " << MM_manager.memory_available() << " bytes.\n";

    ae = AMI_scan(ostr, &amis3, &sd, &amisd);
    
    cout << "Length of diff stream: " <<
      amisd.stream_len() << ".\n";
  }
  delete istr;
  delete ostr;

  return 0;
}
