// Copyright (c) 1995 Darren Vengroff
//
// File: comparator.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/6/95
//
// $Id: comparator.h,v 1.2 1999-12-15 22:08:40 hutchins Exp $
//
#ifndef _COMPARATOR_H
#define _COMPARATOR_H

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
