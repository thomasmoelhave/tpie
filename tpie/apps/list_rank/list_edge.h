// Copyright (c) 1994 Darren Vengroff
//
// File: list_edge.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 10/27/94
//
// $Id: list_edge.h,v 1.3 2003-06-03 16:56:24 tavi Exp $
//
// The edge class.  This is what our list ranking function will work on.
//
#ifndef _LIST_EDGE_H
#define _LIST_EDGE_H

#include <iostream>
using std::ostream;

class edge {
public:
    unsigned long int from;        // Node it is from
    unsigned long int to;          // Node it is to
    unsigned long int weight;      // Position when ranked.
    bool flag;            // A flag used to randomly select some edges.

    friend ostream& operator<<(ostream& s, const edge &e);
};    


// Helper functions used to compare to edges to sort them either by 
// the node they are from or the node they are to.

//extern int edgefromcmp(CONST edge &s, CONST edge &t);
//extern int edgetocmp(CONST edge &s, CONST edge &t);
//extern int edgeweightcmp(CONST edge &s, CONST edge &t);

struct edgefromcmp {
  int compare(CONST edge &s, CONST edge &t);
};
struct edgetocmp {
  int compare(CONST edge &s, CONST edge &t);
};
struct edgeweightcmp {
  int compare(CONST edge &s, CONST edge &t);
};
#endif // _LIST_EDGE_H 
