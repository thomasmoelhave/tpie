// Copyright (c) 1999 Octavian Procopiuc
//
// File: ascii2stream.cpp
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 01/26/99
// 
// Transforms an ASCII file containing rectangles (as in rectangle.h)
// into a TPIE stream.
//
#include "common.h"
#include <fstream>
#include "rectangle.h"
#include <tpie/block.h>
#include <tpie/scan_utils.h>


int main(int argc, char **argv) {

  err err;
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
  cxx_istream_scan<rectangle<double, bid_t> > scanner(file_stream);
  stream<rectangle<double, bid_t> > *out_stream;
  out_stream = new stream<rectangle<double, bid_t> >(out_filename);
  out_stream->persist(PERSIST_PERSISTENT);
  
  cerr << "Working...";
  err = scan(&scanner, out_stream);

  delete out_stream;

  if (err != NO_ERROR) {
    cerr << "\nError while parsing data: " << hex 
	 << err << " (see ami_err.H)" << endl;
    exit(-1);
  } else
    cerr << " done.\n";

  return 0;
}
