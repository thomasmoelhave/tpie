// Copyright (c) 1994 Darren Erik Vengroff
//
// File: app_config.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/6/94
//
// $Id: app_config.h,v 1.2 1994-10-11 12:53:34 dev Exp $
//
#ifndef _APP_CONFIG_H
#define _APP_CONFIG_H

// for size_t
#include <sys/types.h>

// Use logs.
#include <tpie_log.h>

#define DEFAULT_TEST_SIZE (1024 * 1024 * 8)

#define DEFAULT_TEST_MM_SIZE (1024 * 1024 * 2)

// Use the single BTE stream version of AMI streams.
#define AMI_IMP_SINGLE

// Pick a version of BTE streams.
#define BTE_IMP_MMB
//#define BTE_IMP_CACHE
//#define BTE_IMP_STDIO
//#define BTE_IMP_UFS

#ifdef BTE_IMP_MMB
#define BTE_MMB_LOGICAL_BLOCKSIZE_FACTOR 1
#endif

#ifdef BTE_IMP_CACHE
#define BTE_MMB_CACHE_LINE_SIZE 256
#endif

extern bool verbose;

extern size_t test_mm_size;
extern size_t test_size;
extern int random_seed;



#endif // _APP_CONFIG_H 
