// Copyright (c) 1995 Darren Vengroff
//
// File: comparator.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/6/95
//
// $Id: comparator.h,v 1.3 2003-04-17 18:48:39 jan Exp $
//
#ifndef _COMPARATOR_H
#define _COMPARATOR_H

// Get definitions for working with Unix and Windows
#include <portability.h>

// First we define a comparison object.
template<class T>
class comparator
{
public:
#if AMI_VIRTUAL_BASE
  virtual int compare(const T &t1, const T &t2) = 0;
#endif
};

#endif // _COMPARATOR_H 
