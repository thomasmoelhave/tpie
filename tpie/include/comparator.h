// Copyright (c) 1995 Darren Vengroff
//
// File: comparator.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/6/95
//
// $Id: comparator.h,v 1.1 1995-03-07 15:00:46 darrenv Exp $
//
#ifndef _COMPARATOR_H
#define _COMPARATOR_H

// First we define a comparison object.
template<class T>
class comparator
{
public:
    virtual int compare(const T &t1, const T &t2) = 0;
};

#endif // _COMPARATOR_H 
