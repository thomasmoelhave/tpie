// Copyright (c) 2003 Octavian Procopiuc
//
// File: scan_universal.h
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 04/23/03
//
// $Id: scan_universal.h,v 1.4 2004-08-12 15:15:11 jan Exp $
//

#ifndef _SCAN_UNIVERSAL_H
#define _SCAN_UNIVERSAL_H

// Get the STL min fonction.
#include <algorithm>
// Get the AMI_scan_object definition.
#include <ami_scan.h>

template <int sz>
struct ifoo_t {
  int i;
  char el[sz-sizeof(int)];
  // Default constructor.
  ifoo_t() { i = 0; }
  // This class also acts as comparison class for MAI_sort.
  int compare(const ifoo_t<sz>& lhi, const ifoo_t<sz>& rhi) const {
    return (lhi.i > rhi.i ? 1: lhi.i < rhi.i ? -1: 0);
  }
  // Comparison operator.
  bool operator<(const ifoo_t<sz>& rhi) const {
    return i < rhi.i;
  }
};

// A scan object to generate random integers.
template <int sz>
class scan_universal : AMI_scan_object {
private:
  unsigned int _max, _remaining;
  int _even, _odd;
  int _switches;
  bool _have_prev;
public:
  scan_universal(unsigned int count = 1000, int seed = 17): 
    _max(count), _remaining(count) {
    TP_LOG_APP_DEBUG_ID("scan_universal random seed:");
    TP_LOG_APP_DEBUG_ID(seed);
    TPIE_OS_SRANDOM(seed);     
  }

  virtual ~scan_universal(void) {}

  AMI_err initialize(void) {
    _have_prev = false;
    _remaining = _max;
    _even = _odd = 0;
    _switches = 0;
    return AMI_ERROR_NO_ERROR;
  }

  // Generating random ints.
  AMI_err operate(int *out0, AMI_SCAN_FLAG *sf) {
    if ((*sf = (_remaining-- != 0))) {
      *out0 = TPIE_OS_RANDOM();
      return AMI_SCAN_CONTINUE;
    } else {
      return AMI_SCAN_DONE;
    }
  }

  // Generate ifoo_t's with random ints.
  AMI_err operate(ifoo_t<sz>* out0, AMI_SCAN_FLAG *sf) {
    if ((*sf = (_remaining-- != 0))) {
      out0->i = TPIE_OS_RANDOM();
      //    out0->el[0] = char (out0.i % 128);
      return AMI_SCAN_CONTINUE;
    } else {
      return AMI_SCAN_DONE;
    }
  }

  // Counting switches, even, and odd in stream of ints.
  AMI_err operate(const int& in0, AMI_SCAN_FLAG* sfin) {
    static int prev;
    if (*sfin) {
      if (in0 % 2 == 0)
	_even++;
      else
	_odd++;
      if (_have_prev && in0 < prev)
	_switches++;
      prev = in0;
      _have_prev = true;
      return AMI_SCAN_CONTINUE;
    } else {
      return AMI_SCAN_DONE;
    }
  }

  // Counting switches in stream of ifoo_t's
  AMI_err operate(const ifoo_t<sz>& in0, AMI_SCAN_FLAG* sfin) {
    static ifoo_t<sz> prev;
    if (*sfin) {
      if (_have_prev && in0 < prev)
	_switches++;
      prev = in0;
      _have_prev = true;
      return AMI_SCAN_CONTINUE;
    } else {
      return AMI_SCAN_DONE;
    }    
  }

  // Halving each int.
  AMI_err operate(const int& in0, AMI_SCAN_FLAG *sfin,
		  int *out0, AMI_SCAN_FLAG *sfout) {
    if (*sfout = *sfin) {
      *out0 = in0 / 2;
      return AMI_SCAN_CONTINUE;
    } else {
      return AMI_SCAN_DONE;
    }
  }

  // Taking min of each pair.
  AMI_err operate(const int& in0, const int& in1, AMI_SCAN_FLAG *sfin,
		  int *out0, AMI_SCAN_FLAG *sfout) {
    if (*sfout = sfin[0] || sfin[1]) {
      if (sfin[0] && sfin[1])
	*out0 = min(in0, in1);
      else 
	*out0 = (sfin[0] ? in0: in1);
      return AMI_SCAN_CONTINUE;
    } else {
      return AMI_SCAN_DONE;
    }
  }

  // Even ints in first stream and odd ints in second stream.
  AMI_err operate(const int& in0, const int& in1, AMI_SCAN_FLAG *sfin,
		  int *out0, int *out1, AMI_SCAN_FLAG *sfout) {
    sfout[0] = sfout[1] = 0;
    
    if (sfin[0])
      if (in0 % 2 == 0) {
	*out0 = in0;
	sfout[0] = 1;
      } else {
	*out1 = in0;
	sfout[1] = 1;
      }
    
    if (sfin[1])
      if (in1 % 2 == 0)
	if (!sfout[0]) {
	  *out0 = in1;
	  sfout[0] = 1;
	} else
	  sfin[1] = 0;
      else 
	if (!sfout[1]) {
	  *out1 = in1;
	  sfout[1] = 1;
	} else
	  sfin[1] = 0;
    
    if (sfout[0] || sfout[1])
      return AMI_SCAN_CONTINUE;
    else
      return AMI_SCAN_DONE;    
  }
  
  // Outputs: avg, min, max.
  AMI_err operate(const int& in0, const int& in1, 
		  const int& in2, const int& in3, AMI_SCAN_FLAG *sfin,
		  int *out0, int *out1, int *out2,
		  AMI_SCAN_FLAG *sfout) {
    if (sfout[0] = sfout[1] = sfout[2] = sfin[0] || sfin[1] || sfin[2] || sfin[3]) {
      int c = 0;
      if (sfin[0]) c++;
      if (sfin[1]) c++;
      if (sfin[2]) c++;
      if (sfin[3]) c++;
      
      *out0 = (sfin[0] ? in0: 0) / c + (sfin[1] ? in1: 0) / c + 
	(sfin[2] ? in2: 0) / c + (sfin[3] ? in3: 0) / c;
      
      *out1 = sfin[0] ? in0: sfin[1] ? in1: sfin[2] ? in2: in3;
      if (sfin[1]) *out1 = min(*out1, in1);
      if (sfin[2]) *out1 = min(*out1, in2);
      if (sfin[3]) *out1 = min(*out1, in3);
      
      *out2 = sfin[0] ? in0: sfin[1] ? in1: sfin[2] ? in2: in3;
      if (sfin[1]) *out2 = max(*out2, in1);
      if (sfin[2]) *out2 = max(*out2, in2);
      if (sfin[3]) *out2 = max(*out2, in3);
      
      return AMI_SCAN_CONTINUE;
    } else
      return AMI_SCAN_DONE;    
  }
  
  int even() const { return _even; }
  int odd() const { return _odd; }
  int switches() const { return _switches; }
};

#endif // _SCAN_UNIVERSAL_H 
