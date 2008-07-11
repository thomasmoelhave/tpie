// Copyright (c) 1994 Darren Vengroff
//
// File: list_edge.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 10/27/94
//
// $Id: list_edge.h,v 1.5 2004-08-12 12:36:45 jan Exp $
//
// The edge class.  This is what our list ranking function will work on.
//
#ifndef _LIST_EDGE_H
#define _LIST_EDGE_H

#include <portability.h>

class edge {
public:
    TPIE_OS_OFFSET from;        // Node it is from
    TPIE_OS_OFFSET to;          // Node it is to
    TPIE_OS_OFFSET weight;      // Position when ranked.
    bool flag;                  // A flag used to randomly select some edges.

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
