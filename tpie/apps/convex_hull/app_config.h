// Copyright (c) 1994 Darren Erik Vengroff
//
// File: app_config.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/6/94
//
// $Id: app_config.h,v 1.19 1999-06-29 19:58:34 large Exp $
//
#ifndef _APP_CONFIG_H
#define _APP_CONFIG_H

// Get the configuration as set up by the TPIE configure script.
#include <config.h>

/* ********************************************************************** */
/*                      developer use                                     */
/* ********************************************************************** */


/* ********************************************************************** */
/*                       choose BTE                                       */
/* ********************************************************************** */

/* Pick a version of BTE streams; default is BTE_IMP_UFS */
//#define BTE_IMP_MMB
//#define BTE_IMP_STDIO
#define BTE_IMP_UFS


/* ********************************************************************** */
/*                      configure BTE                                     */
/* ********************************************************************** */


/* ********************************************************************** */
/* BTE_MMB configuration options */
/* ********************************************************************** */
#ifdef BTE_IMP_MMB

/* define logical blocksize; default is 32 * operating system blocksize */
#define BTE_MMB_LOGICAL_BLOCKSIZE_FACTOR 32

/* enable/disable TPIE read ahead; default is enabled (set to 1) */
#define BTE_MMB_READ_AHEAD 1

/* read ahead method, ignored unless BTE_MMB_READ_AHEAD is set to 1;
   if USE_LIBAIO is enabled, use asynchronous IO read ahead; otherwise
   use use mmap-based read ahead; default is mmap-based read ahead
   (USE_LIBAIO not defined) */
//#define USE_LIBAIO

#endif


/* ********************************************************************** */
/* BTE_UFS configuration options */
/* ********************************************************************** */
#ifdef BTE_IMP_UFS

/* define logical blocksize; default is 32 * operating system blocksize */
#define BTE_UFS_LOGICAL_BLOCKSIZE_FACTOR 32

/* enable/disable TPIE read ahead; default is disabled (set to 0) */
#define BTE_UFS_READ_AHEAD 0

/* read ahead method, ignored unless BTE_UFS_READ_AHEAD is set to 1;
   if USE_LIBAIO is set to 1, use asynchronous IO read ahead;
   otherwise no TPIE read ahead is done; default is disabled (set to
   0) */
#define USE_LIBAIO 0

#endif
/********************************************************************/



/********************************************************************/
/*              Set up some defaults for the apps                   */
/********************************************************************/
#include <sys/types.h> // for size_t
#include <stdlib.h> // for random

#define DEFAULT_TEST_SIZE (1024 * 1024 * 16)
#define DEFAULT_TEST_MM_SIZE (1024 * 1024 * 8)

extern bool verbose;
extern size_t test_mm_size;
extern size_t test_size;
extern int random_seed;
/********************************************************************/



/********************************************************************/
/*  THE FOLLOWING MACROS ARE NORMALLY NOT MODIFIED BY USER           */
/********************************************************************/


/* Use the single BTE stream version of AMI streams; in the current
   option this is the only option */
#define AMI_IMP_SINGLE


/* enable/disable virtual interface; normally disabled */
#ifndef AMI_VIRTUAL_BASE
#define AMI_VIRTUAL_BASE 0
#endif
#ifndef BTE_VIRTUAL_BASE 
#define BTE_VIRTUAL_BASE 0
#endif



/********************************************************************/
/*                            logging;                              */
/*              this should NOT be modified by user!!!              */
/*       in order to enable/disable library/application logging,    */
/*     run tpie configure script with appropriate options           */
/********************************************************************/
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
/********************************************************************/


#endif


