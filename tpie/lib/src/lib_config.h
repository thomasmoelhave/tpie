// Copyright (c) 1994 Darren Vengroff
//
// File: lib_config.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 10/31/94
//
// $Id: lib_config.h,v 1.2 1994-11-03 16:25:48 dev Exp $
//
#ifndef _LIB_CONFIG_H
#define _LIB_CONFIG_H

#include <config.h>

// Use logs if requested.
#if TP_LOG_LIB
#define TPL_LOGGING 1
#endif
#include <tpie_log.h>

// Enable assertions if requested.
#if TP_ASSERT_LIB
#define DEBUG_ASSERTIONS 1
#define DEBUG_CERR 1 
#define DEBUG_STR 1
#endif
#include <tpie_assert.h>


#endif // _LIB_CONFIG_H 
