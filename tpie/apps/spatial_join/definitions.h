// Copyright (c) 2002 Octavian Procopiuc
//
// File:         definitions.h
// Author:       Octavian Procopiuc <tavi@cs.duke.edu>
// Created:      06/11/97
// Description:  
//
// $Id: definitions.h,v 1.1 2003-11-21 17:01:09 tavi Exp $

#ifndef _DEFINITIONS_H
#define _DEFINITIONS_H

#include <limits.h>
#include <float.h>	    // FreeBSD has DBL_MAX in float.h.

#ifndef MAX_NUMBER_OF_SLABS
#define MAX_NUMBER_OF_SLABS 20
#endif

#define STRIPED_SWEEP
#define SHORT_QUEUE

// #ifndef MIN
// #define MIN(a,b) ((a) <= (b) ? (a) : (b))
// #endif

// #ifndef MAX
// #define MAX(a,b) ((a) >= (b) ? (a) : (b))
// #endif

#define INT 1
#define FLOAT 2
#define DOUBLE 3

#define COORD_T INT

#if (COORD_T==INT)
#define INFINITY      (INT_MAX-1)
#define MINUSINFINITY (1-INT_MAX)
typedef int           coord_t;
#elif (COORD_T==FLOAT)
#define INFINITY (FLT_MAX-1.0)
#define MINUSINFINITY (1.0-FLT_MAX)
typedef float         coord_t;
#elif (COORD_T==DOUBLE)
#define INFINITY (DBL_MAX-1.0)
#define MINUSINFINITY (1.0-DBL_MAX)
typedef double         coord_t;
#endif

typedef unsigned int  oid_t;
//const unsigned short  fanOut = 256;

#endif //_DEFINITIONS_H
