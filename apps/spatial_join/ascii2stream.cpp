// Copyright (c) 1999 Octavian Procopiuc
//
// File: ascii2stream.cpp
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 01/26/99
// 
// Transforms an ASCII file containing rectangles (as in rectangle.H)
// into a TPIE stream.
//
#include "app_config.h"
#include <fstream>
#include "rectangle.h"
#include "my_ami_scan_utils.h"

int main(int argc, char **argv)
{

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
  my_cxx_istream_scan<rectangle> *scanner;
  AMI_STREAM<rectangle> *out_stream;
  out_stream = new AMI_STREAM<rectangle>(out_filename);
  out_stream->persist(PERSIST_PERSISTENT);
  cerr << "Working...";

  scanner = new my_cxx_istream_scan<rectangle>(file_stream);
  err = AMI_scan(scanner, out_stream);
  delete scanner;
  delete out_stream;

  if (err != AMI_ERROR_NO_ERROR) {
    cerr << "\nError while parsing data: " << hex 
	 << err << " (see ami_base.H)" << endl;
    exit(-1);
  } else
    cerr << " done.\n";
}
