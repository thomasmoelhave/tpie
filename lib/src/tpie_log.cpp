// Copyright (c) 1994 Darren Erik Vengroff
//
// File: tpie_log.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//

#include <versions.h>
VERSION(tpie_log_cpp,"$Id: tpie_log.cpp,v 1.7 2001-02-19 20:52:24 tavi Exp $");

// We are logging
#define TPL_LOGGING	1

#include <tpie_log.h>
#include <string.h>
#include <stdlib.h>

extern "C" char *mktemp(char *);

#define TP_LOG_NAME	"/tmp/TPLOG_XXXXXX"

// tavi: 02/19/2001 deleted all of Darren's stuff b/c was giving seg faults.
logstream &theLog() {
  static char tpl_name[] = TP_LOG_NAME;
  static logstream log(mktemp(tpl_name),TP_LOG_DEBUG_INFO, TP_LOG_DEBUG_INFO);
  return log;
}
