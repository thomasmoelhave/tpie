// Copyright (c) 1999 Octavian Procopiuc
//
// File: mbr.cpp
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 01/27/99
//
// $Id: mbr.cpp,v 1.1 2003-11-21 17:26:02 tavi Exp $
//
// Performs a scan of a stream of rectangles to find their
// minimum bounding rectangle (MBR).
//

#include <float.h>
// Quick hack.
#define INFINITY DBL_MAX
#define MINUSINFINITY -(DBL_MAX-1)

#include "app_config.h"
#include "rectangle.h"
#include <ami_scan.h>
#include <ami_block.h>
#include <string.h>
#include <fstream>
//#include <limits.h>

template<class coord_t, class oid_t>
class MBRScanner: public AMI_scan_object {
protected:
  rectangle<coord_t, oid_t> mbr;
  ofstream *out;
public:
  MBRScanner(char *out_filename) {
    out = new ofstream(out_filename);
    mbr.xlo = INFINITY;
    mbr.ylo = INFINITY;
    mbr.xhi = MINUSINFINITY;
    mbr.yhi = MINUSINFINITY;
  }
    
  AMI_err initialize() {

  }

  AMI_err operate(const rectangle<coord_t, oid_t> &in, AMI_SCAN_FLAG *sfin) {

    if (*sfin) {
      if (in.xlo < mbr.xlo) mbr.xlo = in.xlo;
      if (in.ylo < mbr.ylo) mbr.ylo = in.ylo;
      if (in.xhi > mbr.xhi) mbr.xhi = in.xhi;
      if (in.yhi > mbr.yhi) mbr.yhi = in.yhi;
    } else {
      out->write((char *) &mbr, sizeof(mbr));
      cerr << " " << mbr.xlo << " " << mbr.ylo 
      	   << " " << mbr.xhi << " " << mbr.yhi << " ";
      return AMI_SCAN_DONE;
    }
    return AMI_SCAN_CONTINUE; 
  }
};

int main(int argc, char **argv) {

  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <input_file>" << endl;
    exit(-1);
  }

  cerr << "Working...";
  AMI_STREAM<rectangle<double, AMI_bid> > input_stream(argv[1]);
  input_stream.persist(PERSIST_PERSISTENT);

  cerr << "Stream length : " << input_stream.stream_len() << endl;

  char *output_filename = new char[strlen(argv[1])+5];
  strcpy(output_filename, argv[1]);
  strcat(output_filename, ".mbr");
  MBRScanner<double, AMI_bid> scan(output_filename);
  AMI_err err = AMI_scan(&input_stream, &scan);
  cerr << "done." << endl;

}
