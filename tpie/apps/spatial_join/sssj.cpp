// Copyright (c) 2002 Octavian Procopiuc
//
// File:         sssj.cpp
// Authors:      Octavian Procopiuc <tavi@cs.duke.edu>
//               and Jan Vahrenhold <jan@cs.duke.edu>
// Created:      01/24/99
// Description:  Join two sets using sort and sweep.
//
// $Id: sssj.cpp,v 1.2 2004-08-12 12:38:53 jan Exp $
//
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::istream;
using std::ostream;

#include "app_config.h"
#include <ami_stream.h>
#include "sorting_adaptor.h"
#include "sortsweep.h"
#include "parse_args.h"
#include "joinlog.h"

// default file names for input and output streams...
static char def_if_red[] = "rectangles.red";
static char def_if_blue[] = "rectangles.blue";
static char def_of[] = "output";

static char *input_filename_red = def_if_red;
static char *input_filename_blue = def_if_blue;
static char *output_filename = def_of;

bool verbose = false;
TPIE_OS_SIZE_T test_mm_size = 64*1024*1024; // Default mem. size.
TPIE_OS_OFFSET test_size = 0; // Not used.
int random_seed = 17;


// set the additional command line parameters to be parsed.

void usage(const char* progname) {
      cerr << "  Usage: sortjoin" << endl 
           << "\t[ -R red_input_stream]" << endl
           << "\t[ -B blue_input_stream ]" << endl
	   << "\t[ -o output_file_name ]" << endl
	   << "\t[ -m memory_size ]" << endl;
}

static const char as_opts[] = "R:B:o:m:z:vh";
void parse_app_opt(char c, char *optarg)
{
    switch (c) {
    case 'o':
      output_filename = optarg;
      break;
    case 'R':
      input_filename_red = optarg;
      break;
    case 'B':
      input_filename_blue = optarg;
      break;
    case 'v':
      verbose = !verbose;
      break;
    case 'm':
      test_mm_size = atol(optarg);
      //istrstream(optarg,strlen(optarg)) >> test_mm_size;
      break;                
    case 'z':
      random_seed = atol(optarg);
      //istrstream(optarg,strlen(optarg)) >> random_seed;
      break;
    case 'h':
      usage("sssj");
      exit(1);
    }
}

AMI_err join() {

  sort_sweep* sweeper;
  AMI_STREAM<pair_of_rectangles>* output_stream;
  AMI_err err;
  TPIE_OS_SIZE_T sz_avail;

  // Create the output stream.
  output_stream = new AMI_STREAM<pair_of_rectangles>(output_filename);
  output_stream->persist(PERSIST_PERSISTENT);

  JoinLog jl("SRTS", input_filename_red,  0, input_filename_blue, 0);
  sz_avail = MM_manager.memory_available();
  cerr << "Beginning. Avail. mem: " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(sz_avail) << endl;
  jl.UsageStart();

  sweeper = new sort_sweep(input_filename_red, input_filename_blue,
			   output_stream);
  sz_avail = MM_manager.memory_available();
  cerr << "Finished sorting. Avail. memory: " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(sz_avail) << endl;

  err = sweeper->run();

  jl.setRedLength(sweeper->redSize());
  jl.setBlueLength(sweeper->blueSize());
  jl.UsageEnd("SortJoin:", output_stream->stream_len());

  cerr << "Output length: " << output_stream->stream_len() << endl;

  delete sweeper;
  delete output_stream;
  return err;
}


int main(int argc, char** argv) {

  AMI_err err;

  tpie_log_init(TPIE_LOG_APP_DEBUG);

  if (argc < 2) {
    usage(argv[0]);
    exit(1);
  }
  // Parse the command line arguments.
  parse_args(argc,argv,as_opts,parse_app_opt);

  // Set the main memory size. 
  MM_manager.set_memory_limit(test_mm_size);
  MM_manager.enforce_memory_limit();

  if ((err = join()) != AMI_ERROR_NO_ERROR)
    cout << "Error: " << err << endl;

  return 0;
}

