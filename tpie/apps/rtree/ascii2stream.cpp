// Copyright (c) 1999 Octavian Procopiuc
//
// File: ascii2stream.cpp
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 01/26/99
// 
// Transforms an ASCII file containing rectangles (as in rectangle.h)
// into a TPIE stream.
//
#include "app_config.h"
#include <fstream>
#include "rectangle.h"
#include <ami_block.h>
#include <ami_scan_utils.h>


int main(int argc, char **argv) {

  AMI_err err;
  istream *file_stream;
  char *out_filename;
  if (argc < 2) {
    cerr << "Transforms ASCII rectangles file into TPIE stream.\n" 
	 << "Usage: " << argv[0] << " [ <input_file> ] <output_file>\n";
    exit(-1);
  } else if (argc == 2) {
    file_stream = &cin;
    out_filename = argv[1];
  } else {
    file_stream = new ifstream(argv[1]);
    out_filename = argv[2];
  }
  cxx_istream_scan<rectangle<double, AMI_bid> > scanner(file_stream);
  AMI_STREAM<rectangle<double, AMI_bid> > *out_stream;
  out_stream = new AMI_STREAM<rectangle<double, AMI_bid> >(out_filename);
  out_stream->persist(PERSIST_PERSISTENT);
  
  cerr << "Working...";
  err = AMI_scan(&scanner, out_stream);

  delete out_stream;

  if (err != AMI_ERROR_NO_ERROR) {
    cerr << "\nError while parsing data: " << hex 
	 << err << " (see ami_err.H)" << endl;
    exit(-1);
  } else
    cerr << " done.\n";
}
