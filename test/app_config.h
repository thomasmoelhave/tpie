// Copyright (c) 1994 Darren Erik Vengroff
//
// File: app_config.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/6/94
//
// $Id: app_config.h,v 1.6 1995-03-07 14:54:22 darrenv Exp $
//
#ifndef _APP_CONFIG_H
#define _APP_CONFIG_H

// Get the configuration set up by the configure script.
#include <config.h>

// for size_t
#include <sys/types.h>

// Many apps use random numbers.
extern "C" int srandom(int);
extern "C" int random(void);

// Use logs if requested.
#if TP_LOG_APPS
#define TPL_LOGGING 1
#endif
#include <tpie_log.h>

// Enable assertions if requested.
#if TP_ASSERT_APPS
#define DEBUG_ASSERTIONS 1
#define DEBUG_CERR 1
#define DEBUG_STR 1
#endif
#include <tpie_assert.h>

// Set up some defaults for the apps.

#define DEFAULT_TEST_SIZE (1024 * 1024 * 8)

#define DEFAULT_TEST_MM_SIZE (1024 * 1024 * 2)

// Use the single BTE stream version of AMI streams.
#define AMI_IMP_SINGLE

// Pick a version of BTE streams.
#define BTE_IMP_MMB
//#define BTE_IMP_CACHE
//#define BTE_IMP_STDIO
//#define BTE_IMP_UFS
//#define BTE_IMP_BCS

#ifdef BTE_IMP_MMB
#define BTE_MMB_LOGICAL_BLOCKSIZE_FACTOR 16
#endif

#ifdef BTE_IMP_CACHE
#define BTE_MMB_CACHE_LINE_SIZE 256
#endif

extern bool verbose;

extern size_t test_mm_size;
extern size_t test_size;
extern int random_seed;



#endif // _APP_CONFIG_H 
