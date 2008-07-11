//
// File: app_config.h
// Created: 10/23/97
// Last modified: 10/23/97
//
// Test program for bte_col_rtree.H

#define LOG_BLK_SZ 8*1024  /* HERE WE NEED TO PUT SOMETHING MEANINGFUL */

#ifndef _APP_CONFIG_H
#define _APP_CONFIG_H

// Get the configuration set up by the configure script.
#include <config.h>

#define LOG_BLK_SZ 8*1024  /* HERE WE NEED TO PUT SOMETHING MEANINGFUL */


// <><><><><><><><><><><><><><><><><><><><><><> //
// <><><> Choose default BTE COLLECTION  <><><> //
// <><><><><><><><><><><><><><><><><><><><><><> //

#if (!defined(BTE_COLLECTION_IMP_MMAP) && !defined(BTE_COLLECTION_IMP_UFS) && !defined(BTE_COLLECTION_IMP_USER_DEFINED))
// Define only one (default is BTE_COLLECTION_IMP_MMAP)
#  define BTE_COLLECTION_IMP_MMAP
//#  define BTE_COLLECTION_IMP_UFS
//#  define BTE_COLLECTION_IMP_USER_DEFINED
#endif

// <><><><><><><><><><><><><><><><><><><><><><> //
// <><><><><><> Choose BTE STREAM  <><><><><><> //
// <><><><><><><><><><><><><><><><><><><><><><> //

// Define only one (default is BTE_STREAM_IMP_UFS)
#define BTE_STREAM_IMP_UFS
//#define BTE_STREAM_IMP_MMAP
//#define BTE_STREAM_IMP_STDIO
//#define BTE_STREAM_IMP_USER_DEFINED


// <><><><><><><><><><><><><><><><><><><><><><><><> //
// <> BTE_COLLECTION_MMAP configuration options  <> //
// <><><><><><><><><><><><><><><><><><><><><><><><> //

// Define write behavior.
// Allowed values:
//  0    (synchronous writes)
//  1    (asynchronous writes using MS_ASYNC - see msync(2))
//  2    (asynchronous bulk writes) [default]
#ifndef BTE_COLLECTION_MMAP_LAZY_WRITE
#  define BTE_COLLECTION_MMAP_LAZY_WRITE 2
#endif

// <><><><><><><><><><><><><><><><><><><><><><><><> //
// <><> BTE_STREAM_MMAP configuration options  <><> //
// <><><><><><><><><><><><><><><><><><><><><><><><> //

#ifdef BTE_STREAM_IMP_MMAP
   // Define logical blocksize factor (default is 32)
#  ifndef BTE_STREAM_MMAP_BLOCK_FACTOR
#    define BTE_STREAM_MMAP_BLOCK_FACTOR 32
#  endif
   // Enable/disable TPIE read ahead; default is enabled (set to 1)
#  define BTE_STREAM_MMAP_READ_AHEAD 1
   /* read ahead method, ignored unless BTE_STREAM_MMAP_READ_AHEAD is set to 1;
   if USE_LIBAIO is enabled, use asynchronous IO read ahead; otherwise
   use use mmap-based read ahead; default is mmap-based read ahead
   (USE_LIBAIO not defined) */
   //#define USE_LIBAIO
#endif


// <><><><><><><><><><><><><><><><><><><><><><><><> //
// <><> BTE_STREAM_UFS configuration options <><><> //
// <><><><><><><><><><><><><><><><><><><><><><><><> //

#ifdef BTE_STREAM_IMP_UFS
   // Define logical blocksize factor (default is 32)
#  ifndef BTE_STREAM_UFS_BLOCK_FACTOR
#    define BTE_STREAM_UFS_BLOCK_FACTOR 32
#  endif
   // Enable/disable TPIE read ahead; default is disabled (set to 0)
#  define BTE_STREAM_UFS_READ_AHEAD 0
   /* read ahead method, ignored unless BTE_STREAM_UFS_READ_AHEAD is set to 1;
   if USE_LIBAIO is set to 1, use asynchronous IO read ahead;
   otherwise no TPIE read ahead is done; default is disabled (set to
   0) */
#  define USE_LIBAIO 0
#endif


// Use logs if requested.
#if TP_LOG_APPS
#define TPL_LOGGING 1
#include <tpie_log.h>
#endif

// Enable assertions if requested.
#if TP_ASSERT_APPS
#define DEBUG_ASSERTIONS 1
#define DEBUG_CERR 1
#define DEBUG_STR 1
#include <tpie_assert.h>
#endif


#endif
