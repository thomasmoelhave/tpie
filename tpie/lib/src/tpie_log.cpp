//
// File: tpie_log.cpp
// Authors: Darren Erik Vengroff <dev@cs.duke.edu>
//          Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 5/12/94
//

#include <versions.h>
VERSION(tpie_log_cpp,"$Id: tpie_log.cpp,v 1.11 2003-04-17 21:03:36 jan Exp $");

// We are logging
#define TPL_LOGGING	1

#include <stdlib.h>
#include <time.h>
#include <tpie_tempnam.h>
#include <tpie_log.h>

#define TPLOGPFX "tpielog"

logstream &theLog() {
	static logstream log((TPIE_OS_SRANDOM(time(NULL)), tpie_tempnam(TPLOGPFX, TPLOGDIR)),
                       TP_LOG_DEBUG, TP_LOG_DEBUG);
  return log;
}
