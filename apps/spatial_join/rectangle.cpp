// Copyright (c) 1999 Octavian Procopiuc
//
// File:         rectangle.cpp
// Author:       Octavian Procopiuc <tavi@cs.duke.edu>
// Created:      02/04/99
// Description:  Definition of output operator for rectangle.
//
// $Id: rectangle.cpp,v 1.1 2003-11-21 17:01:09 tavi Exp $
//
#include <iostream>
using std::ostream;
using std::istream;
using std::endl;
#include "rectangle.h"

ostream& operator<<(ostream& s, const rectangle& r)
{
  return s << r.id << " " << r.xlo << " " << r.ylo << " " 
	   << r.xhi << " " << r.yhi << endl;
}

istream& operator>>(istream& s, rectangle& r) {
#ifdef SEQUOIA_RECTANGLE 
  //sequoia rectangles have the id on the 1st position.
  s >> r.id >> r.xlo >> r.ylo >> r.xhi >> r.yhi;
#else
  s >> r.xlo >> r.ylo >> r.xhi >> r.yhi >> r.id;
#endif
  return s;
}
