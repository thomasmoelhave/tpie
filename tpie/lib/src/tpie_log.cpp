//
// File: tpie_log.cpp
// Authors: Darren Erik Vengroff <dev@cs.duke.edu>
//          Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 5/12/94
//

#include <versions.h>
VERSION(tpie_log_cpp,"$Id: tpie_log.cpp,v 1.12 2003-04-23 07:48:34 tavi Exp $");

// We are logging
#define TPL_LOGGING	1

#include <stdlib.h>
#include <time.h>
#include <tpie_tempnam.h>
#include <tpie_log.h>

#define TPLOGPFX "tpielog"

// Local initialization function. Create a permanent repository for the log
// file name. Should be called only once, by theLogName() below.
static char *__theLogName() {
  static char tln[128];
  TPIE_OS_SRANDOM(time(NULL));
  strncpy(tln, tpie_tempnam(TPLOGPFX, TPLOGDIR), 128);
  return tln;
}

char *theLogName() {
  static char *tln = __theLogName();
  return tln;
}

logstream &theLog() {
  static logstream log(theLogName(),
                       TP_LOG_DEBUG, TP_LOG_DEBUG);
  return log;
}
