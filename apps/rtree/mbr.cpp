// Copyright (c) 1999 Octavian Procopiuc
//
// File: mbr.cpp
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 01/27/99
//
// $Id: mbr.cpp,v 1.2 2004-02-05 17:54:14 jan Exp $
//
// Performs a scan of a stream of rectangles to find their
// minimum bounding rectangle (MBR).
//

#include <float.h>
// Quick hack.
#define INFINITY DBL_MAX
#define MINUSINFINITY -(DBL_MAX-1)

#include "common.h"
#include "rectangle.h"
#include <tpie/scan.h>
#include <tpie/block.h>
#include <string.h>
#include <fstream>
//#include <limits.h>

template<class coord_t, class oid_t>
class MBRScanner: public scan_object {
protected:
  rectangle<coord_t, oid_t> mbr;
  ofstream *out;
public:
  MBRScanner(std::string out_filename) {
    out = new ofstream(out_filename.c_str());
    mbr.xlo = INFINITY;
    mbr.ylo = INFINITY;
    mbr.xhi = MINUSINFINITY;
    mbr.yhi = MINUSINFINITY;
  }
    
  err initialize() {
	return NO_ERROR;
  }

  err operate(const rectangle<coord_t, oid_t> &in, SCAN_FLAG *sfin) {

    if (*sfin) {
      if (in.xlo < mbr.xlo) mbr.xlo = in.xlo;
      if (in.ylo < mbr.ylo) mbr.ylo = in.ylo;
      if (in.xhi > mbr.xhi) mbr.xhi = in.xhi;
      if (in.yhi > mbr.yhi) mbr.yhi = in.yhi;
    } else {
      out->write((char *) &mbr, sizeof(mbr));
      cerr << " " << mbr.xlo << " " << mbr.ylo 
      	   << " " << mbr.xhi << " " << mbr.yhi << " ";
      return SCAN_DONE;
    }
    return SCAN_CONTINUE; 
  }
};

int main(int argc, char **argv) {

  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <input_file>" << endl;
    exit(-1);
  }

  cerr << "Working...";
  stream<rectangle<double, bid_t> > input_stream(argv[1]);
  input_stream.persist(PERSIST_PERSISTENT);

  cerr << "Stream length : " << input_stream.stream_len() << endl;

  char *output_filename = new char[strlen(argv[1])+5];
  strcpy(output_filename, argv[1]);
  strcat(output_filename, ".mbr");
  MBRScanner<double, bid_t> scan(output_filename);
  err aerr = ami::scan(&input_stream, &scan);
  cerr << "done." << endl;

	return 0;
}
