//
// File: tpie_log.cpp
// Authors: Darren Erik Vengroff <dev@cs.duke.edu>
//          Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 5/12/94
//

#include <versions.h>
VERSION(tpie_log_cpp,"$Id: tpie_log.cpp,v 1.10 2002-07-20 21:28:44 tavi Exp $");

// We are logging
#define TPL_LOGGING	1

#include <stdlib.h>
#include <time.h>
#include <tpie_tempnam.h>
#include <tpie_log.h>

#define TPLOGDIR "/tmp"
#define TPLOGPFX "tpielog"

logstream &theLog() {
  static logstream log((srandom(time(NULL)), tpie_tempnam(TPLOGPFX, TPLOGDIR)),
                       TP_LOG_DEBUG, TP_LOG_DEBUG);
  return log;
}
