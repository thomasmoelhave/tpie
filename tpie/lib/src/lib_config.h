//
// File: lib_config.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 10/31/94
//
// $Id: lib_config.h,v 1.3 2002-01-14 17:29:03 tavi Exp $
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
#endif
#include <tpie_assert.h>


#endif // _LIB_CONFIG_H 
