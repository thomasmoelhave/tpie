// Copyright (c) 2002 Octavian Procopiuc
//
// File:         sortsweep.h
// Author:       Octavian Procopiuc <tavi@cs.duke.edu>
// Created:      01/24/99
// Description:  Internal sweep class.
//
// $Id: sortsweep.h,v 1.1 2003-11-21 17:01:09 tavi Exp $
//
#ifndef _SORTSWEEP_H
#define _SORTSWEEP_H

static const int DELETE_FREQUENCY = 10;

//#include "intertree.h"
#include "sorting_adaptor.h"

class sort_sweep {
private:
  rectangle *prect;
  int crect, ncrect;
  AMI_err err;
  int counter;
  rectangle mbr_;

protected:
  SortingAdaptor *adaptor_[2];
  //  InterTree *iTree_[2];
  AMI_STREAM<pair_of_rectangles> *outStream_;
  long intersection_count;

public:
  sort_sweep(const char* red_filename, const char* blue_filename, 
	     AMI_STREAM<pair_of_rectangles> *outStream);
  ~sort_sweep();
  // run() takes the smallest item from the adaptor and runs 
  // the sweepline algorithm on it.
  AMI_err run();
  long getIntersectionCount() { return intersection_count; }
  off_t redSize() const { return adaptor_[0]->size();}
  off_t blueSize() const { return adaptor_[1]->size();}
  void getMbr(const char *input_filename, rectangle *mbr);
};

#endif // _SORTSWEEP_H
