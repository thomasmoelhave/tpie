// Copyright (c) 1994 Darren Erik Vengroff
//
// File: mm_base.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 9/2/94
//

#include <versions.h>
VERSION(mm_base_cpp,"$Id: mm_base.cpp,v 1.23 2000-03-26 05:16:17 rajiv Exp $");

#include "lib_config.h"
#include <mm_base.h>
#include <tpie_log.h>
#include <mm_register.h>

#if(0)
// deadcode
// this is already in mm_register.h -RW

// The actual memory manager.  This is currently a total hack.  The
// proper thing to do is to go through the pointer mm_manager for all
// references to the memory manager.

extern MM_register MM_manager;
#endif


#if(0)
// deadcode
// not being used. -RW

// A pointer to the memory manager currently being used.

MM_manager_base *mm_manager;
#endif

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// support for dmalloc (for tracking memory leaks)
#ifdef USE_DMALLOC
#define DMALLOC_DISABLE
#include <dmalloc.h>
#include <return.h>
#endif

#ifdef MM_BACKWARD_COMPATIBLE
int   register_new = MM_IGNORE_MEMORY_EXCEEDED;
#endif

// SIZE_SPACE is to ensure alignment on quad word boundaries.  It may be
// possible to check whether a machine needs this at configuration
// time or if dword alignment is ok.  On the HP 9000, bus errors occur
// when loading doubles that are not qword aligned.
static const size_t SIZE_SPACE=(sizeof(size_t) > 8 ? sizeof(size_t) : 8);

void *operator new (size_t sz)
{
   void *p;
#ifdef USE_DMALLOC
	char	*file;
	GET_RET_ADDR(file);
#endif

   if ((MM_manager.register_new != MM_IGNORE_MEMORY_EXCEEDED)
       && (MM_manager.register_allocation (sz + SIZE_SPACE) !=
	   MM_ERROR_NO_ERROR)) {
      switch(MM_manager.register_new) {
	  case MM_ABORT_ON_MEMORY_EXCEEDED:
		LOG_FATAL_ID ("In operator new() - allocation request \"");
		LOG_FATAL (sz + SIZE_SPACE);
		LOG_FATAL ("\" plus previous allocation \"");
		LOG_FATAL (MM_manager.memory_used () - (sz + SIZE_SPACE));
		LOG_FATAL ("\" exceeds user-defined limit \"");
		LOG_FATAL (MM_manager.memory_limit ());
		LOG_FATAL ("\" \n");
		LOG_FLUSH_LOG;
		cerr << "memory manager: memory allocation limit exceeded "
			 << "while allocating " << sz << " bytes" << endl;
#ifdef USE_DMALLOC
		dmalloc_shutdown();
#endif
		assert (0);		// core dump if debugging
		exit (1);
		break;
	  case MM_WARN_ON_MEMORY_EXCEEDED:
		LOG_WARNING_ID ("In operator new() - allocation request \"");
		LOG_WARNING (sz + SIZE_SPACE);
		LOG_WARNING ("\" plus previous allocation \"");
		LOG_WARNING (MM_manager.memory_used () - (sz + SIZE_SPACE));
		LOG_WARNING ("\" exceeds user-defined limit \"");
		LOG_WARNING (MM_manager.memory_limit ());
		LOG_WARNING ("\" \n");
		LOG_FLUSH_LOG;
		cerr << "memory manager: memory allocation limit exceeded "
			 << "while allocating " << sz << " bytes" << endl;
		break;
	  case MM_IGNORE_MEMORY_EXCEEDED:
		break;
      }
   }

#ifdef USE_DMALLOC
   p = _malloc_leap(file, 0, sz + SIZE_SPACE);
#else
   p = malloc(sz + SIZE_SPACE);
#endif
   if (!p) {
      LOG_FATAL_ID ("Out of memory. Cannot continue.");
      LOG_FLUSH_LOG;
	  cerr << "out of memory while allocating " << sz << " bytes" << endl;
      perror ("mm_base::new malloc");
	  assert(0);
      exit (1);
   }
   *((size_t *) p) = sz;
   return ((char *) p) + SIZE_SPACE;
}

void operator delete (void *ptr)
{
   if (!ptr) {
      LOG_WARNING_ID ("operator delete was given a NULL pointer");
      return;
   }

   if (MM_manager.register_new != MM_IGNORE_MEMORY_EXCEEDED) {
      if (MM_manager.register_deallocation (
	    *((size_t *) (((char *) ptr) - SIZE_SPACE)) + SIZE_SPACE) 
         != MM_ERROR_NO_ERROR) {
	 LOG_WARNING_ID("In operator delete - MM_manager.register_deallocation failed");
      }
   }
   void *p = ((char *)ptr) - SIZE_SPACE;
#ifdef USE_DMALLOC
	char	*file;
	GET_RET_ADDR(file);
	_free_leap(file, 0, p);    
#else
    free(p);
#endif
}

void operator delete[] (void *ptr) {
   if (!ptr) {
      LOG_WARNING_ID ("operator delete [] was given a NULL pointer");
      return;
   }

   if (MM_manager.register_new != MM_IGNORE_MEMORY_EXCEEDED) {
      if (MM_manager.register_deallocation (
	    *((size_t *) (((char *) ptr) - SIZE_SPACE)) + SIZE_SPACE)
         != MM_ERROR_NO_ERROR) {
	 LOG_WARNING_ID("In operator delete [] - MM_manager.register_deallocation failed");
      }
   }
   void *p = ((char *)ptr) - SIZE_SPACE;
#ifdef USE_DMALLOC
	char	*file;
	GET_RET_ADDR(file);
	_free_leap(file, 0, p);    
#else
    free(p);
#endif
}

// return the overhead on each memory allocation request 
int   MM_register::space_overhead ()
{
   return SIZE_SPACE;
}
