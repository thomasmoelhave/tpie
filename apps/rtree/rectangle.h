// Copyright (c) 1997 Octavian Procopiuc
//
// File: rectangle.h
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 03/22/97
// Last Modified: 02/04/99
//
// $Id: rectangle.h,v 1.2 2004-02-05 17:54:14 jan Exp $
//
// rectangle and pair_of_rectangles classes for rectangle intersection
//


#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_

#include "common.h"

#include <iostream>
// Include min and max from STL.
#include <algorithm>

//
// Class rectangle:
// An abstraction of a 2-dim. rectangle.
//
template <typename coord_t, typename oid_t>
class rectangle {
public:
  oid_t   id;
  coord_t xlo;
  coord_t ylo; 
  coord_t xhi;
  coord_t yhi;

  // Constructor.
  rectangle(oid_t aid = (oid_t) 0, 
	    coord_t axlo = (coord_t) 0.0, 
	    coord_t aylo = (coord_t) 0.0,
	    coord_t axhi = (coord_t) 0.0, 
	    coord_t ayhi = (coord_t) 0.0): 
    id(aid) {
    xlo = (axlo<axhi ? axlo : axhi);
    xhi = (axlo<axhi ? axhi : axlo);
    ylo = (aylo<ayhi ? aylo : ayhi);
    yhi = (aylo<ayhi ? ayhi : aylo);
  }

  // Copy constructor.
  rectangle(const rectangle<coord_t, oid_t>& r) {
    *this = r;  
  }

  bool intersects(const rectangle<coord_t, oid_t>& r) const {
    if (((xlo <= r.xlo && r.xlo <= xhi) || 
	 (r.xlo <= xlo && xlo <= r.xhi)) 
	&& ((ylo <= r.ylo && r.ylo <= yhi) || 
	    (r.ylo <= ylo && ylo <= r.yhi) )) {
      return true;
    } else {
      return false;
    }
  }

  bool operator<(const rectangle<coord_t, oid_t> &rhs) const {
    return  ylo < rhs.ylo;
  }

  bool operator>(const rectangle<coord_t, oid_t> &rhs) const {
    return ylo > rhs.ylo;
  }

  bool operator==(const rectangle<coord_t, oid_t> &rhs) const {
    return ((id == rhs.id) && (xlo == rhs.xlo) && 
	    (ylo == rhs.ylo) && (xhi == rhs.xhi) && (yhi == rhs.yhi));
  }

  bool operator!=(const rectangle<coord_t, oid_t> &rhs) const {
    return !(*this == rhs);
  }

  rectangle<coord_t, oid_t>& operator=(const rectangle<coord_t, oid_t>& rhs) {
    if (this != &rhs) {
      id  = rhs.id;
      xlo = rhs.xlo;
      xhi = rhs.xhi;
      ylo = rhs.ylo;
      yhi = rhs.yhi;
      //    memcpy((void*) &id, &rhs.id, sizeof(id) + 4*sizeof(coord_t));
    }
    return (*this);
  }


  // The coordinates of the rectangle.

  coord_t left() const { return xlo; }
  coord_t right() const { return xhi; }
  coord_t lower() const { return ylo; }
  coord_t upper() const { return yhi; }

  //. The width and height and the area (as the product of width and 
  //. height) can be inquired. One can compute the "extended" area, 
  //. that is the bounding box of the union of the current object and
  //. another instance of BoundingBox, and the "overlap" area,
  //. that is the bounding box of the intersection of two bounding
  //. boxes.
  coord_t width() const { return (xhi - xlo); }
  coord_t height() const { return (yhi - ylo); }
  coord_t area() const { return (xhi - xlo) * (yhi - ylo); }


  coord_t extendedArea(const rectangle<coord_t, oid_t>& r) const {
    return ((max(xhi,r.xhi) - min(xlo,r.xlo)) * 
	    (max(yhi,r.yhi) - min(ylo,r.ylo)));
  }

  coord_t overlapArea(const rectangle<coord_t, oid_t>& r) const {
    return (intersects(r) ? ((min(xhi,r.xhi) - max(xlo,r.xlo)) * 
	      (min(yhi,r.yhi) - max(ylo,r.ylo))) : (coord_t) 0.0);
  }

  void setID(oid_t ID) { id = ID; }
  oid_t getID() const { return id; }

  // Checks whether the projections on the x-axis of 
  // the 2 rectangles have a non-empty intersection.
  bool xOverlaps(const rectangle<coord_t, oid_t>& r) const {
    return ((xlo <= r.xlo && r.xlo <= xhi) ||
	    (r.xlo <= xlo && xlo <= r.xhi));
  }

  // Checks whether the projections on the y-axis of 
  // the 2 rectangles have a non-empty intersection.  
  bool yOverlaps(const rectangle<coord_t, oid_t>& r) const {
    return ((ylo <= r.ylo && r.ylo <= yhi) ||
	    (r.ylo <= ylo && ylo <= r.yhi));
  }

  // Extend the rectangle to include the given point.
  void extend(coord_t x, coord_t y) {
    xlo = min(x, xlo);
    ylo = min(y, ylo);
    xhi = max(x, xhi);
    yhi = max(y, yhi);
  }

  // Extend the rectangle to include the given rectangle.
  void extend(const rectangle<coord_t, oid_t>& r) {
    xlo = min(xlo, r.xlo);
    ylo = min(ylo, r.ylo);
    xhi = max(xhi, r.xhi);
    yhi = max(yhi, r.yhi);
  }

};

// Output operator.
template <class coord_t, class oid_t>
ostream& operator<<(ostream& s, const rectangle<coord_t, oid_t>& r) {
  return s << r.id << " " << r.xlo << " " << r.ylo << " " << r.xhi << " " << r.yhi << "\n";
}

// Input operator.
template <class coord_t, class oid_t>
istream& operator>>(istream& s, rectangle<coord_t, oid_t>& r) {
  s.precision(7);
  s >> r.xlo >> r.ylo >> r.xhi >> r.yhi >> r.id;
  return s;
}


//
// class pair_of_rectangles:
// A pair of rectangles for reporting intersections.
//
template<class oid_t> 
class pair_of_rectangles {
public:
  oid_t first;		// id of first rectangle
  oid_t second;		// id of second rectangle
};


#endif //_RECTANGLE_H_
