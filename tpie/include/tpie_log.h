// Copyright (c) 1994 Darren Erik Vengroff
//
// File: tpie_log.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//
// $Id: tpie_log.h,v 1.1 1994-05-12 21:00:30 dev Exp $
//
#ifndef _TPIE_LOG_H
#define _TPIE_LOG_H

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

#endif // _TPIE_LOG_H 
