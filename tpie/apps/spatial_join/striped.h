// Copyright (c) 2002 Octavian Procopiuc
//
// File:         striped.h
// Author:       Octavian Procopiuc <tavi@cs.duke.edu>
// Created:      01/24/99
// Description:  
//
// $Id: striped.h,v 1.1 2003-11-21 17:01:09 tavi Exp $
//
#ifndef _STRIPED_H
#define _STRIPED_H

#define MAX_STRIPS     128       /* max number of strips */

/***********************************/
/* Settings and internal constants */
/***********************************/

#define C_SIZE 32          /* number of rectangles per linked list element */


/***********************************/
/* Data structure for linked lists */
/***********************************/

typedef struct _ch {
  _ch *next; 
  int num;
  rectangle rects[C_SIZE];    /* array of rectangles */
} chunk;


#endif //_STRIPED_H
