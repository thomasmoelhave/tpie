// Copyright (c) 1994 Darren Erik Vengroff
//
// File: tpie_log.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//
// $Id: tpie_log.h,v 1.13 2000-11-05 19:53:23 tavi Exp $
//
#ifndef _TPIE_LOG_H
#define _TPIE_LOG_H

// Logging levels, from higest priority to lowest.
enum {
  TP_LOG_FATAL = 0,	// Fatal errors are always logged no matter what;
  TP_LOG_WARNING,	// Warning about some internal condition;
  TP_LOG_APP_DEBUG,     // Debugging info for the application only;
  TP_LOG_DEBUG_INFO,	// Debugging info.
};

#if TPL_LOGGING		

// We are logging.

#include <logstream.h>


// The main tpie log to which all logging information goes.
extern logstream *tpl;

void init_tpie_logs(void);

// Macros to simplify logging.  The argument to the macro can be any type
// that log streams have an output operator for.

#define LOG_FLUSH_LOG (tpl->ofstream::flush())

// eg: LOG_FATAL(LOG_ID_MSG)
#define LOG_ID_MSG __FILE__ << " line " << __LINE__ << ": "

#define LOG_FATAL(msg) (*tpl << setpriority(TP_LOG_FATAL) << msg)
#define LOG_WARNING(msg)  (*tpl << setpriority(TP_LOG_WARNING) << msg)
#define LOG_APP_DEBUG(msg)  (*tpl << setpriority(TP_LOG_APP_DEBUG)  << msg)
#define LOG_DEBUG_INFO(msg)  (*tpl << setpriority(TP_LOG_DEBUG_INFO)  << msg)

#define LOG_FATAL_ID(msg)  \
  (LOG_FATAL(LOG_ID_MSG << msg << "\n"), LOG_FLUSH_LOG)
#define LOG_WARNING_ID(msg)  \
  (LOG_WARNING(LOG_ID_MSG << msg << "\n"), LOG_FLUSH_LOG)
#define LOG_APP_DEBUG_ID(msg) \
  (LOG_APP_DEBUG(LOG_ID_MSG << msg << "\n"), LOG_FLUSH_LOG)
#define LOG_DEBUG_ID(msg)  \
  (LOG_DEBUG_INFO(LOG_ID_MSG << msg << "\n"), LOG_FLUSH_LOG)

#define LOG_SET_THRESHOLD(level) (*tpl << setthreshold(level))

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
#define LOG_WARNING(msg) 
#define LOG_APP_DEBUG(msg)
#define LOG_DEBUG_INFO(msg) 

#define LOG_FATAL_ID(msg)
#define LOG_WARNING_ID(msg)
#define LOG_APP_DEBUG_ID(msg)
#define LOG_DEBUG_ID(msg)

#define LOG_SET_THRESHOLD(level)
#define LOG_FLUSH_LOG {}

#endif // TPL_LOGGING

#endif // _TPIE_LOG_H 
