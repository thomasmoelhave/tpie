// Copyright (c) 1994 Darren Vengroff
//
// File: point.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/16/94
//
// $Id: point.h,v 1.2 1999-02-03 02:56:02 jan Exp $
//
#ifndef _POINT_H
#define _POINT_H

#include <iostream.h>

template<class T>
class point {
public:
    T x;
    T y;
    point() {};
    point(const T &rx, const T &ry) : x(rx), y(ry) {};
    ~point() {};

    inline int operator==(const point<T> &rhs) const {
        return (x == rhs.x) && (y == rhs.y);
    }
    inline int operator!=(const point<T> &rhs) const {
        return (x != rhs.x) || (y != rhs.y);
    }

    // Comparison is done by x.
    int operator<(const point<T> &rhs) const {
        return (x < rhs.x);
    }

    int operator>(const point<T> &rhs) const {
        return (x > rhs.x);
    }
    
    friend ostream& operator<< <> (ostream& s, const point<T> &p);
    friend istream& operator>> <> (istream& s, point<T> &p);
};

template<class T>
ostream& operator<<(ostream& s, const point<T> &p)
{
    return s << p.x << ' ' << p.y;
}

template<class T>
istream& operator>>(istream& s, point<T> &p)
{
    return s >> p.x >> p.y;
}

// ccw returns twice the signed area of the triangle p1,p2,p3.  It is
// > 0 iff p1,p2,p3 is a counterclockwise cycle.  If they are
// colinear, it is 0.
template<class T>
T ccw(const point<T> &p1, const point<T> &p2, const point<T> &p3)
{
    T sa;
    
    sa = ((p1.x * p2.y - p2.x * p1.y) -
          (p1.x * p3.y - p3.x * p1.y) +
          (p2.x * p3.y - p3.x * p2.y));

    return sa;
}

// cw is just the opposite of ccw.  It is > 0 iff p1,p2,p3 is a
// clockwise cycle.
template<class T>
T cw(const point<T> &p1, const point<T> &p2, const point<T> &p3)
{
    return -ccw(p1,p2,p3);
}

#ifdef NO_IMPLICIT_TEMPLATES

#define TEMPLATE_INSTANTIATE_POINT(T)					\
template class point<T>;						\
template T ccw(const point<T> &, const point<T> &, const point<T> &);	\
template T cw(const point<T> &, const point<T> &, const point<T> &);	\
template ostream& operator<<(ostream& s, const point<T> &p);		\
template ostream& operator>>(istream& s, point<T> &p);

#endif


#endif // _POINT_H 
