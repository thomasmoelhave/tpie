// Copyright (c) 1994 Darren Erik Vengroff
//
// File: tpie_log.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//


static char tpie_log_id[] = "$Id: tpie_log.cpp,v 1.2 1994-05-17 18:18:29 dev Exp $";

// We are logging
#define TPL_LOGGING	1

#include <tpie_log.h>

extern "C" char *mktemp(char *);

#define TP_LOG_NAME	"/tmp/TPLOG_XXXXXX"

// The name of the TPIE log
static char tpl_name[] = TP_LOG_NAME;

// The main TPIE log.  Unless changed, everything is logged.
logstream *tpl;

// The performance log
logstream *tp_perfl;


void init_tpie_logs(void)
{
    tpl = new logstream(mktemp(tpl_name), TP_LOG_INFO, TP_LOG_INFO);
}
