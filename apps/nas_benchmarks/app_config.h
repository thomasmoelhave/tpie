// Copyright (c) 1994 Darren Erik Vengroff
//
// File: app_config.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/6/94
//
// $Id: app_config.h,v 1.16 1999-06-18 20:39:54 laura Exp $
//
#ifndef _APP_CONFIG_H
#define _APP_CONFIG_H

// Get the configuration set up by the configure script.
#include <config.h>

#include <sys/types.h> // for size_t
#include <stdlib.h> // for random


/* ---------------------------------------------------------------------- */
/* get tavi to fix the following */

// Use logs if requested.
#if TP_LOG_APPS
#define TPL_LOGGING 1
#endif

// R..
#undef TP_LOG_LIB
#undef TP_LOG_APPS

#include <tpie_log.h>

// Enable assertions if requested.
#if TP_ASSERT_APPS
#define DEBUG_ASSERTIONS 1
#define DEBUG_CERR 1
#define DEBUG_STR 1
#endif
#include <tpie_assert.h>
/* ---------------------------------------------------------------------- */



/* ********************************************************************** */
/* developer use */
/* ********************************************************************** */


/* ********************************************************************** */
/* choose BTE(s) */
/* ********************************************************************** */
// Use the single BTE stream version of AMI streams.
#define AMI_IMP_SINGLE

// Pick a version of BTE streams.
//#define BTE_IMP_MMB
#define BTE_IMP_STDIO
//#define BTE_IMP_UFS


/* ********************************************************************** */
/* choose the block collection class implementation */
/* ********************************************************************** */
//#define BCC_IMP_MMB
//#define BCC_IMP_UFS

/* ********************************************************************** */
/* BTE_MMB configuration options */
/* ********************************************************************** */
#ifdef BTE_IMP_MMB

#ifndef BTE_MMB_LOGICAL_BLOCKSIZE_FACTOR
#define BTE_MMB_LOGICAL_BLOCKSIZE_FACTOR 16
#endif

/* enable/disable read ahead */
//#define BTE_MMB_READ_AHEAD 1


/* prefetch (read ahead) method:
 * USE_LIBAIO - use asynchronous IO
 * else - use mmap
 */
//#define USE_LIBAIO

#endif // BTE_IMP_MMB


/* ********************************************************************** */
/* BTE_UFS configuration options */
/* ********************************************************************** */

#ifdef BTE_IMP_UFS

// The blocksize (corresp to the theoretical I/O model) is 
// BTE_UFS_LOGICAL_BLOCKSIZE_FACTOR * os blocksize 
#ifndef BTE_UFS_LOGICAL_BLOCKSIZE_FACTOR
#define BTE_UFS_LOGICAL_BLOCKSIZE_FACTOR 32
#endif

//In the current version of TPIE, BTE_UFS_READ_AHEAD should be
//defined as 0 and DOUBLE_BUFFER should be defined 0. 
#define BTE_UFS_READ_AHEAD 0
#define DOUBLE_BUFFER 0

// USE_LIBAIO can be set to 1 to trigger off a certain kind of 
// readahead on Solaris machines, but we suggest keeping this 0 as well.
#define USE_LIBAIO 0

// Very often bte_ufs will be used to sequentially access a file;
//for instance this happens with mergesort and scanning. Typical
//filesystems in such situations tend to carryout sequential readahead.
//When BTE_IMPLICIT_FS_READAHEAD is set to 1, we try to account for the
//amount of memory used up by the read-ahead portion (in the filesystem
//buffer cache) by assuming (quick and dirty guess) that the amount of
//read-ahead at any time is equal to the blocksize (corresp to theoretical
// I/O model). If set to 0, we essentially cheat by not accounting at all
//for memory used by readahead. So in applications in which you sequentially
//access streams, BTE_IMPLICIT_FS_READAHEAD shd be set to 1; otherwise for
//tree accesses etc. it should be set to 0.

#define BTE_IMPLICIT_FS_READAHEAD 1
#endif





/* ********************************************************************** */
/* Block collection options : Set similarly to BTE options */
/* ********************************************************************** */

#ifdef BCC_IMP_UFS
#define MMAPPED_BCC 0
#define BCC_LOGICAL_BLOCK_FACTOR 32
#endif

#ifdef BCC_IMP_MMB
#define BCC_LOGICAL_BLOCK_FACTOR 32
#define MMAPPED_BCC 1
#endif



/********************************************************************/

// Set up some defaults for the apps.
#define DEFAULT_TEST_SIZE (1024 * 1024 * 16)
#define DEFAULT_TEST_MM_SIZE (1024 * 1024 * 8)

extern bool verbose;
extern size_t test_mm_size;
extern size_t test_size;
extern int random_seed;



/********************************************************************/
// Don't use virtual interface.
#ifndef AMI_VIRTUAL_BASE
#define AMI_VIRTUAL_BASE 0
#endif
#ifndef BTE_VIRTUAL_BASE 
#define BTE_VIRTUAL_BASE 0
#endif


#endif


