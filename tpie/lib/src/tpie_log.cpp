// Copyright (c) 1994 Darren Erik Vengroff
//
// File: tpie_log.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//

#include <versions.h>
VERSION(tpie_log_cpp,"$Id: tpie_log.cpp,v 1.5 2000-01-10 22:06:14 hutchins Exp $");

// We are logging
#define TPL_LOGGING	1

#include <tpie_log.h>

extern "C" char *mktemp(char *);

#define TP_LOG_NAME	"/tmp/TPLOG_XXXXXX"

// The names of the TPIE logs
static char tpl_name[] = TP_LOG_NAME;

// The main TPIE log. 
logstream *tpl;

// The counter of log_init instances.  It is implicity set to 0.
unsigned int log_init::count;

// The constructor and destructor that ensure that the log files are
// created exactly once, and destroyed when appropriate.
log_init::log_init(void)
{
    if (count++ == 0) {
        tpl = new logstream(mktemp(tpl_name), 
			    TP_LOG_DEBUG_INFO, TP_LOG_DEBUG_INFO);
    }
}


log_init::~log_init(void)
{
    if (--count == 0) {
        delete tpl;
    }
}
