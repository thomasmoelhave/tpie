// Copyright (c) 1994 Darren Erik Vengroff
//
// File: tpie_log.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//
// $Id: tpie_log.h,v 1.18 2003-04-17 20:03:53 jan Exp $
//

#ifndef _TPIE_LOG_H
#define _TPIE_LOG_H

// Logging levels, from higest priority to lowest.
enum {
  TP_LOG_FATAL = 0,	// Fatal errors are always logged no matter what;
  TP_LOG_WARNING,	// Warning about some internal condition;
  TP_LOG_APP_DEBUG,     // Debugging info for the application only;
  TP_LOG_DEBUG,		// Debugging info.
};

// We are logging.

// Get definitions for working with Unix and Windows
#include <portability.h>

#include <logstream.h>

#if TPL_LOGGING		

// Serves double duty: initilizes the log on the first call, and
// returns the only logstream object. Defined in tpie_log.cpp
logstream& theLog();

// Macros to simplify logging.  The argument to the macro can be any type
// that log streams have an output operator for.

#define LOG_FLUSH_LOG (!logstream::log_initialized || theLog().ofstream::flush())

// eg: LOG_FATAL(LOG_ID_MSG)
#define LOG_ID_MSG __FILE__ << " line " << __LINE__ << ": "

#define LOG_FATAL(msg) \
  (!logstream::log_initialized || theLog() << setpriority(TP_LOG_FATAL) << msg)
#define LOG_WARNING(msg) \
  (!logstream::log_initialized || theLog() << setpriority(TP_LOG_WARNING) << msg)
#define LOG_APP_DEBUG(msg) \
  (!logstream::log_initialized || theLog() << setpriority(TP_LOG_APP_DEBUG)  << msg)
#define LOG_DEBUG(msg) \
  (!logstream::log_initialized || theLog() << setpriority(TP_LOG_DEBUG)  << msg)

#define LOG_FATAL_ID(msg)  \
  (LOG_FATAL(LOG_ID_MSG << msg << "\n"), LOG_FLUSH_LOG)
#define LOG_WARNING_ID(msg)  \
  (LOG_WARNING(LOG_ID_MSG << msg << "\n"), LOG_FLUSH_LOG)
#define LOG_APP_DEBUG_ID(msg) \
  (LOG_APP_DEBUG(LOG_ID_MSG << msg << "\n"), LOG_FLUSH_LOG)
#define LOG_DEBUG_ID(msg)  \
  (LOG_DEBUG(LOG_ID_MSG << msg << "\n"), LOG_FLUSH_LOG)

#define LOG_SET_THRESHOLD(level) (theLog() << setthreshold(level))

#else // !TPL_LOGGING

// We are not compiling logging.

#define LOG_FATAL(msg) 
#define LOG_WARNING(msg) 
#define LOG_APP_DEBUG(msg)
#define LOG_DEBUG(msg) 

#define LOG_FATAL_ID(msg)
#define LOG_WARNING_ID(msg)
#define LOG_APP_DEBUG_ID(msg)
#define LOG_DEBUG_ID(msg)

#define LOG_SET_THRESHOLD(level)
#define LOG_FLUSH_LOG {}

#endif // TPL_LOGGING

#endif // _TPIE_LOG_H 
