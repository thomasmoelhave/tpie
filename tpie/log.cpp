//
// File: tpie_log.cpp
// Authors: Darren Erik Vengroff <dev@cs.duke.edu>
//          Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 5/12/94
//

// We are logging
#define TPL_LOGGING	1

#include <cstdlib>
#include <time.h>
#include <tpie/tempname.h>
#include <tpie/tpie_log.h>

#define TPLOGPFX "tpielog"

// Local initialization function. Create a permanent repository for the log
// file name. Should be called only once, by theLogName() below.
static std::string& __tpie_log_name() 
{
	static std::string tln;
	TPIE_OS_SRANDOM(static_cast<unsigned int>(TPIE_OS_TIME(NULL)));
	tln = tpie_tempnam(TPLOGPFX, TPLOGDIR, "txt");
	return tln;
}

std::string& tpie_log_name() {
  static std::string& tln = __tpie_log_name();
  return tln;
}


logstream &tpie_log() {
  static logstream log(tpie_log_name(), TPIE_LOG_DEBUG, TPIE_LOG_DEBUG);
  return log;
}

void tpie_log_init(TPIE_LOG_LEVEL level) {
  TP_LOG_SET_THRESHOLD(level);
}
