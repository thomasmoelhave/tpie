// Copyright (c) 1994 Darren Vengroff
//
// File: list_edge.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 10/27/94
//

#include <versions.h>
VERSION(list_edge_cpp,"$Id: list_edge.cpp,v 1.3 2000-01-11 01:37:29 hutchins Exp $");

// A hack for now until const handling improves.
#define CONST const

#include "list_edge.h"

// An output operator for edges.

ostream& operator<<(ostream& s, const edge &e)
{
    s << e.from << " -> " << e.to << " (" << e.weight << ") ";
    s << '[' << e.flag << ']';
    return s;
}

// Helper functions used to compare to edges to sort them either by 
// the node they are from or the node they are to.

int edgefromcmp(CONST edge &s, CONST edge &t)
{
    return (s.from < t.from) ? -1 : ((s.from > t.from) ? 1 : 0);
}
  
int edgetocmp(CONST edge &s, CONST edge &t)
{
    return (s.to < t.to) ? -1 : ((s.to > t.to) ? 1 : 0);
}

int edgeweightcmp(CONST edge &s, CONST edge &t)
{
    return (s.weight < t.weight) ? -1 : ((s.weight > t.weight) ? 1 : 0);
}
