//
// File: bte/stream.h (formerly bte.h)
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/9/94
//
// $Id: bte/stream.h,v 1.3 2003-04-17 14:59:16 jan Exp $
//
#ifndef _TPIE_BTE_STREAM_H
#define _TPIE_BTE_STREAM_H

// Get definitions for working with Unix and Windows
#include <portability.h>

#ifndef BTE_VIRTUAL_BASE
#  define BTE_VIRTUAL_BASE 0
#endif

// Get the base class, enums, etc...
#include <bte/stream_base.h>

#ifdef BTE_IMP_UFS
#  define BTE_STREAM_IMP_UFS
#endif
    
#ifdef BTE_IMP_MMB
#  define BTE_STREAM_IMP_MMAP
#endif
    
#ifdef BTE_IMP_STDIO
#  define BTE_STREAM_IMP_STDIO
#endif
    
#ifdef BTE_IMP_USER_DEFINED
#  define BTE_STREAM_IMP_USER_DEFINED
#endif
    
// The number of implementations to be defined.
#define _BTE_STREAM_IMP_COUNT (defined(BTE_STREAM_IMP_USER_DEFINED) + \
			       defined(BTE_STREAM_IMP_STDIO) +	      \
			       defined(BTE_STREAM_IMP_MMAP)   +	      \
			       defined(BTE_STREAM_IMP_UFS) )
    
// Multiple implementations are allowed to coexist, with some
// restrictions.
    
// If the including module did not explicitly ask for multiple
// implementations but requested more than one implementation, issue a
// warning.
#ifndef BTE_STREAM_IMP_MULTI_IMP
#  if (_BTE_STREAM_IMP_COUNT > 1)
#    define BTE_STREAM_IMP_MULTI_IMP
#  endif // (_BTE_STREAM_IMP_COUNT > 1)
#endif // BTE_STREAM_IMP_MULTI_IMP
    
// Make sure at least one implementation was chosen.  If none was, then
// choose one by default, but warn the user.
#if (_BTE_STREAM_IMP_COUNT < 1)
#  define BTE_STREAM_IMP_STDIO
#endif // (_BTE_STREAM_IMP_COUNT < 1)
    
// Now include the definitions of each implementation
// that will be used.
    
#ifdef BTE_STREAM_IMP_MULTI_IMP
#  define BTE_STREAM tpie::bte::stream_base
#endif
    
// User defined implementation.
#if defined(BTE_STREAM_IMP_USER_DEFINED)
// Do nothing.  The user will provide a definition of BTE_STREAM.
#endif
    
// stdio implementation.
#if defined(BTE_STREAM_IMP_STDIO)
#  include <bte/stream_stdio.h>
// If this is the only implementation, then make it easier to get to.
#  ifndef BTE_STREAM_IMP_MULTI_IMP
#    ifdef BTE_STREAM
#      undef BTE_STREAM
#    endif
#    define BTE_STREAM tpie::bte::stream_stdio
#  endif
#endif

// mmap implementation.
#if defined(BTE_STREAM_IMP_MMAP)
#  include <bte/stream_mmap.h>    
// If this is the only implementation, then make it easier to get to.
#  ifndef BTE_STREAM_IMP_MULTI_IMP
#    ifdef BTE_STREAM
#      undef BTE_STREAM
#    endif
#    define BTE_STREAM tpie::bte::stream_mmap
#  endif
#endif

// ufs implementation.
#if defined(BTE_STREAM_IMP_UFS)
#  include <bte/stream_ufs.h>
// If this is the only implementation, then make it easier to get to.
#  ifndef BTE_STREAM_IMP_MULTI_IMP
#    ifdef BTE_STREAM
#      undef BTE_STREAM
#    endif
#    define BTE_STREAM tpie::bte::stream_ufs
#  endif
#endif
    
#endif // _TPIE_BTE_STREAM_H 
