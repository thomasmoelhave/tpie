// Copyright (c) 1994 Darren Erik Vengroff
//
// File: mm_base.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 9/2/94
//



static char mm_base_id[] = "$Id: mm_base.cpp,v 1.1 1994-10-31 21:46:06 darrenv Exp $";



#include <mm_base.h>

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

void * operator new (size_t sz) {
    void *p;
    
    if ((register_new) && (MM_manager.register_allocation(sz+sizeof(size_t))
                           != MM_ERROR_NO_ERROR)) {
        return (void *)0;
    }
    
    p = malloc(sz + sizeof(size_t));
    *((size_t *)p) = sz;
    return ((size_t *)p) + 1;
}

void operator delete (void *ptr) {
    if (register_new) {
        MM_manager.register_deallocation(*((size_t *)ptr - 1) +
                                         sizeof(size_t));
    }
    free(((char *)ptr) - sizeof(size_t));
}
    
