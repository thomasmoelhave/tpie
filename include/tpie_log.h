// Copyright (c) 1994 Darren Erik Vengroff
//
// File: tpie_log.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//
// $Id: tpie_log.h,v 1.2 1994-05-18 18:50:16 dev Exp $
//
#ifndef _TPIE_LOG_H
#define _TPIE_LOG_H

#if TPL_LOGGING		

// We are logging.

#include <logstream.h>

// Logging levels, from higest priority to lowest.
enum {
    TP_LOG_FATAL = 0,	// Fatal problems are always logged no matter what.
    TP_LOG_ERROR,	// An internal error
    TP_LOG_WARNING,	// Warning about some internal condition	
    TP_LOG_ASSERT,	// Assertion failure information
    TP_LOG_DEBUG_INFO,	// Debugging information
    TP_LOG_OS_ERROR,	// An error came back from the OS.
    TP_LOG_DATA_ERROR,	// Inconsistent data in a stream.
    TP_LOG_INFO		// General information
};

// The main tpie log to which all logging information goes.
extern logstream *tpl;

// The performance log, if there is one.  This can be set up by
// the application to moniter performance.
extern logstream *tp_perfl;

void init_tpie_logs(void);

// Macros to simplify logging.  The argument to the macro can be any type
// that log streams have an output operator for.

#define LOG_FATAL(msg) (*tpl << setpriority(TP_LOG_FATAL) << msg)
#define LOG_ERROR(msg) (*tpl << setpriority(TP_LOG_ERROR) << msg)
#define LOG_WARNING(msg)  (*tpl << setpriority(TP_LOG_WARNING) << msg)
#define LOG_ASSERT(msg)  (*tpl << setpriority(TP_LOG_ASSERT)  << msg)
#define LOG_DEBUG_INFO(msg)  (*tpl << setpriority(TP_LOG_DEBUG)  << msg)
#define LOG_OS_ERROR(msg)  (*tpl << setpriority(TP_LOG_OS_ERROR) << msg)
#define LOG_DATA_ERROR(msg)  (*tpl << setpriority(TP_LOG_DATA_ERROR) << msg)
#define LOG_INFO(msg)  (*tpl << setpriority(TP_LOG_INFO) << msg)

#define LOG_FLUSH_LOG (tpl->ofstream::flush())

#else // !TPL_LOGGING

// We are not compiling logging in(msg)
// the logging macros.

#define LOG_FATAL(msg) 
#define LOG_ERROR(msg) 
#define LOG_WARNING(msg) 
#define LOG_ASSERT(msg) 
#define LOG_DEBUG_INFO(msg) 
#define LOG_OS_ERROR(msg) 
#define LOG_DATA_ERROR(msg) 
#define LOG_INFO(msg) 

#endif // TPL_LOGGING

#endif // _TPIE_LOG_H 
