// Copyright (c) 2003 Octavian Procopiuc
//
// File: scan_universal.h
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 04/23/03
//
// $Id: scan_universal.h,v 1.1 2003-04-24 23:50:00 tavi Exp $
//

#ifndef _SCAN_UNIVERSAL_H
#define _SCAN_UNIVERSAL_H
// Get the AMI_scan_object definition.
#include <ami_scan.h>

// A scan object to generate random integers.
class scan_universal : AMI_scan_object {
private:
  unsigned int max, remaining;
  int _even, _odd;
public:
  scan_universal(unsigned int count = 1000, int seed = 17);
  virtual ~scan_universal(void);
  AMI_err initialize(void);
  // Generating random ints.
  AMI_err operate(int *out1, AMI_SCAN_FLAG *sf);
  // Counting even and odd ints.
  AMI_err operate(const int& in0, AMI_SCAN_FLAG* sfin);
  // Halving each int.
  AMI_err operate(const int& in0, AMI_SCAN_FLAG *sfin,
		  int *out0, AMI_SCAN_FLAG *sfout);
  // Taking min of each pair.
  AMI_err operate(const int& in0, const int& in1, AMI_SCAN_FLAG *sfin,
		  int *out0, AMI_SCAN_FLAG *sfout);
  // Even ints in first stream and odd ints in second stream.
  AMI_err operate(const int& in0, const int& in1, AMI_SCAN_FLAG *sfin,
		  int *out0, int *out1, AMI_SCAN_FLAG *sfout);
  // Outputs: avg, min, max.
  AMI_err operate(const int& in0, const int& in1, 
		  const int& in2, const int& in3, AMI_SCAN_FLAG *sfin,
		  int *out0, int *out1, int *out2,
		  AMI_SCAN_FLAG *sfout);
  int even() const { return _even; }
  int odd() const { return _odd; }
};

#endif // _SCAN_UNIVERSAL_H 
