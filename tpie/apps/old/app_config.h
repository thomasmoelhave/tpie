// Copyright (c) 1994 Darren Erik Vengroff
//
// File: app_config.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/6/94
//
// $Id: app_config.h,v 1.12 1999-02-05 22:21:50 rajiv Exp $
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
// Don't use virtual interface.
#ifndef AMI_VIRTUAL_BASE
#define AMI_VIRTUAL_BASE 0
#endif
#ifndef BTE_VIRTUAL_BASE 
#define BTE_VIRTUAL_BASE 0
#endif

/* ********************************************************************** */
/* choose BTE(s) */
/* ********************************************************************** */
// Use the single BTE stream version of AMI streams.
#define AMI_IMP_SINGLE

// Pick a version of BTE streams.
//#define BTE_IMP_MMB
#define BTE_IMP_STDIO
//#define BTE_IMP_CACHE
//#define BTE_IMP_UFS
//#define BTE_IMP_BCS


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
/* BTE_CACHE configuration options */
/* ********************************************************************** */
#ifdef BTE_IMP_CACHE
#define BTE_MMB_CACHE_LINE_SIZE 256
#endif // BTE_IMP_CACHE


/* ********************************************************************** */


// Set up some defaults for the apps.
#define DEFAULT_TEST_SIZE (1024 * 1024 * 8)
#define DEFAULT_TEST_MM_SIZE (1024 * 1024 * 2)

extern bool verbose;
extern size_t test_mm_size;
extern size_t test_size;
extern int random_seed;


#endif

