// Copyright (c) 1994 Darren Erik Vengroff
//
// File: mm_base.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 9/2/94
//



static char mm_base_id[] = "$Id: mm_base.cpp,v 1.6 1999-03-11 03:13:56 laura Exp $";



#include <mm_base.h>

#include <tpie_log.h>

// A pointer to the memory manager currently being used.

MM_manager_base *mm_manager;

// The actual memory manager.  This is currently a total hack.  The
// proper thing to do is to go through the pointer mm_manager for all
// references to the memory manager.

#include <mm_register.h>
extern MM_register MM_manager;


// My very own new and delete operators.

#include <stdlib.h>

int register_new = 0;

// This is to ensure alignment on quad word boundaries.  It may be
// possible to check whether a machine needs this at configuration
// time or if dword alignment is ok.  On the HP 9000, bus errors occur
// when loading doubles that are not qword aligned.

#define SIZE_SPACE (sizeof(size_t) > 8 ? sizeof(size_t) : 8)

void * operator new (size_t sz) {
    void *p;
    
    if ((register_new) && (MM_manager.register_allocation(sz+SIZE_SPACE)
                           != MM_ERROR_NO_ERROR)) {
        LOG_FATAL("Requested memory allocation \"");
	   LOG_FATAL(sz+SIZE_SPACE);
   	   LOG_FATAL("\" could not be made by new() operator.\n");
        LOG_FLUSH_LOG;
        return (void *)0;
    }
    
    p = malloc(sz + SIZE_SPACE);
    if(!p) {
      LOG_FATAL("Out of Memory");
      LOG_FLUSH_LOG;
      cerr << "Out of Memory" << endl;
      exit(1);
    }
    *((size_t *)p) = sz;
    return ((char *)p) + SIZE_SPACE;
}

void operator delete (void *ptr) {
    if (!ptr) return;
    if (register_new) {
        MM_manager.register_deallocation(*((size_t *)
                                           (((char *)ptr) - SIZE_SPACE)) +
                                         SIZE_SPACE);
    }
    free(((char *)ptr) - SIZE_SPACE);
}
    
