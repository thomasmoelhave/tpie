// Copyright (c) 1999 Octavian Procopiuc
//
// File: mbr.cpp
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 01/27/99
//
// $Id: mbr.cpp,v 1.1 2003-11-21 17:01:09 tavi Exp $
//
// Performs a scan of a stream of rectangles to find their minimum
// bounding rectangle (MBR). The MBR is written on cout, as well as in
// a file with the same name as the input file, but with added
// extension .mbr
//

#include <string.h>
#include <fstream>

#include "app_config.h"
#include <ami.h>
#include "rectangle.h"

class MBRScanner: AMI_scan_object {
protected:
  rectangle mbr;
  ofstream out;
public:
  MBRScanner(char *out_filename): out(out_filename) {
    //out = new ofstream(out_filename);
    mbr.xlo = INFINITY;
    mbr.ylo = INFINITY;
    mbr.xhi = -INFINITY;
    mbr.yhi = -INFINITY;
  }

  AMI_err initialize() {
  }

    AMI_err operate(const rectangle &in, AMI_SCAN_FLAG *sfin) {

    if (*sfin) {
      if (in.xlo < mbr.xlo) mbr.xlo = in.xlo;
      if (in.ylo < mbr.ylo) mbr.ylo = in.ylo;
      if (in.xhi > mbr.xhi) mbr.xhi = in.xhi;
      if (in.yhi > mbr.yhi) mbr.yhi = in.yhi;
    } else {
      out.write((char *) &mbr, sizeof(mbr));
      cout << mbr.xlo << " " << mbr.ylo 
      	   << " " << mbr.xhi << " " << mbr.yhi << endl;
      return AMI_SCAN_DONE;
    }
    return AMI_SCAN_CONTINUE; 
  }
};

int main(int argc, char **argv) {

  if (argc < 2 || argv[1][0] == '-') {
    cerr << "Usage: " << argv[0] << " <TPIE_stream_input> [<MBR_output_file>]" << endl
	 << "(if the output file name is missing, the .mbr extension is added to the input file name)" << endl;
    exit(-1);
  }

  char *output_filename = new char[strlen(argv[1])+5];

  if (argc == 3 && argv[2][0] != '-') {
    strcpy(output_filename, argv[2]);
  } else {
    strcpy(output_filename, argv[1]);
    strcat(output_filename, ".mbr");
  }

  cerr << "Working...";
  AMI_STREAM<rectangle> input_stream(argv[1], AMI_READ_STREAM);
  input_stream.persist(PERSIST_PERSISTENT);

  cerr << "Stream length : " << input_stream.stream_len() << endl;

  MBRScanner scan(output_filename);
  AMI_scan(&input_stream, &scan);
  cerr << "done." << endl;

}
