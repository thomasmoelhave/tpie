// Copyright (c) 1994 Darren Erik Vengroff
//
// File: tpie_log.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//
// $Id: tpie_log.h,v 1.5 1994-06-03 13:25:36 dev Exp $
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

#define LOG_FLUSH_LOG (tpl->ofstream::flush())

#define LOG_FATAL(msg) (*tpl << setpriority(TP_LOG_FATAL) << msg)
#define LOG_ERROR(msg) (*tpl << setpriority(TP_LOG_ERROR) << msg)
#define LOG_WARNING(msg)  (*tpl << setpriority(TP_LOG_WARNING) << msg)
#define LOG_ASSERT(msg)  (*tpl << setpriority(TP_LOG_ASSERT)  << msg)
#define LOG_DEBUG_INFO(msg)  (*tpl << setpriority(TP_LOG_DEBUG_INFO)  << msg)
#define LOG_OS_ERROR(msg)  (*tpl << setpriority(TP_LOG_OS_ERROR) << msg)
#define LOG_DATA_ERROR(msg)  (*tpl << setpriority(TP_LOG_DATA_ERROR) << msg)
#define LOG_INFO(msg)  (*tpl << setpriority(TP_LOG_INFO) << msg)

// We want to make sure that our logs get constructed before they can
// possibly be used.  In order to do this, we use the trick Scott
// Meyers gives in Item 47 of his book, and define a class whose sole
// purpose is to ensure that the logs get created exactly once.

class log_init {
private:
    // The number of log_init objects that exist.
    static unsigned int count;

public:
    log_init(void);
    ~log_init(void);
};

// Now define a static object of type log_init.  Every .cpp file that
// includes this header file will get its own static object, but only
// the first that is initialized will actually cause the logs to be
// created.

static log_init source_file_log_init;
    
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

#define LOG_FLUSH_LOG {}


#endif // TPL_LOGGING

#endif // _TPIE_LOG_H 
