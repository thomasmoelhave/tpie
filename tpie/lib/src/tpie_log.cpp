// Copyright (c) 1994 Darren Erik Vengroff
//
// File: tpie_log.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//


static char tpie_log_id[] = "$Id: tpie_log.cpp,v 1.3 1994-06-03 13:39:12 dev Exp $";

// We are logging
#define TPL_LOGGING	1

#include <tpie_log.h>

extern "C" char *mktemp(char *);

#define TP_LOG_NAME	"/tmp/TPLOG_XXXXXX"
#define TP_PLOG_NAME	"/tmp/TPPRF_XXXXXX"

// The names of the TPIE logs
static char tpl_name[] = TP_LOG_NAME;
static char tp_perfl_name[] = TP_PLOG_NAME;

// The main TPIE log. 
logstream *tpl;

// The performance log
logstream *tp_perfl;

// The counter of log_init instances.  It is implicity set to 0.
unsigned int log_init::count;

// The constructor and destructor that ensure that the log files are
// created exactly once, and destroyed when appropriate.
log_init::log_init(void)
{
    if (count++ == 0) {
        tpl = new logstream(mktemp(tpl_name), TP_LOG_INFO, TP_LOG_INFO);

        // Why does this cause seg faults?
        // LOG_INFO("Automatically created tpl.\n");
        // LOG_FLUSH_LOG;

        tp_perfl = new logstream(mktemp(tp_perfl_name), TP_LOG_INFO,
                                 TP_LOG_INFO);
    }
}


log_init::~log_init(void)
{
    if (--count == 0) {
        delete tpl;
        delete tp_perfl;
    }
}


