//
// File: tpie_log.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//

#include <versions.h>
VERSION(tpie_log_cpp,"$Id: tpie_log.cpp,v 1.9 2002-02-04 06:20:20 tavi Exp $");

// We are logging
#define TPL_LOGGING	1

// For tempnam().
#include <stdio.h>

#include <tpie_log.h>

#define TP_LOG_DIR "/tmp"
#define TP_LOG_PFX "TPLOG"

logstream &theLog() {
  static logstream log(tempnam(TP_LOG_DIR, TP_LOG_PFX), 
		       TP_LOG_DEBUG, TP_LOG_DEBUG);
  return log;
}
