// Copyright (c) 1994 Darren Erik Vengroff
//
// File: bte_stream.h (formerly bte.h)
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/9/94
//
// $Id: bte_stream.h,v 1.1 2002-01-02 21:48:33 tavi Exp $
//
#ifndef _BTE_STREAM_H
#define _BTE_STREAM_H

#ifndef BTE_VIRTUAL_BASE
#define BTE_VIRTUAL_BASE 0
#endif

// Get the base class, enums, etc...
#include <bte_base_stream.h>

// The number of implementations to be defined.
#define _BTE_IMP_COUNT (defined(BTE_IMP_USER_DEFINED) +	\
			defined(BTE_IMP_STDIO) + \
			defined(BTE_IMP_MMB)   + \
                        defined(BTE_IMP_UFS) )

// Multiple implementations are allowed to coexist, with some
// restrictions.
  
// If the including module did not explicitly ask for multiple
// implementations but requested more than one implementation, issue a
// warning.
#ifndef BTE_IMP_MULTI_IMP

#if (_BTE_IMP_COUNT > 1)
#warning Multiple BTE_IMP_* defined, but BTE_IMP_MULTI_IMP undefined.
#warning Implicitly defining BTE_IMP_MULTI_IMP.
#define BTE_IMP_MULTI_IMP
#endif // (_BTE_IMP_COUNT > 1)

#endif // BTE_IMP_MULTI_IMP


// If we have multiple implementations, set BTE_STREAM to be the base
// class.
#ifdef BTE_IMP_MULTI_IMP
#define BTE_STREAM BTE_base_stream
#endif

// Now include the definitions of each implementation
//that will be used.
  
// Make sure at least one implementation was chosen.  If none was, then
// choose one by default, but warn the user.
#if (_BTE_IMP_COUNT < 1)
#warning No implementation defined.  Using BTE_IMP_STDIO by default.
#define BTE_IMP_STDIO
#endif // (_BTE_IMP_COUNT < 1)


// User defined implementation.

#if defined(BTE_IMP_USER_DEFINED)
// Do nothing.  The user will provide a definition of BTE_STREAM.
#endif // defined(BTE_IMP_USER_DEFINED)


// Stdio implementation.

#if defined(BTE_IMP_STDIO)
#include <bte_stdio.h>
// If this is the only implementation, then make it easier to get to.
#ifndef BTE_IMP_MULTI_IMP
#define BTE_STREAM BTE_stream_stdio
#endif
#endif

// mmb implementation.

#if defined(BTE_IMP_MMB)
#include <bte_mmb.h>
// If this is the only implementation, then make it easier to get to.
#ifndef BTE_IMP_MULTI_IMP
#define BTE_STREAM BTE_stream_mmb
#endif
#endif

// ufs implementation.

#if defined(BTE_IMP_UFS)
#include <bte_ufs.h>
// If this is the only implementation, then make it easier to get to.
#ifndef BTE_IMP_MULTI_IMP
#define BTE_STREAM BTE_single_disk
#endif
#endif

#endif // _BTE_STREAM_H 
