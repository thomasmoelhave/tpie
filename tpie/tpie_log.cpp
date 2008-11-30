// We are logging
#define TPL_LOGGING	1

#include <cstdlib>
#include <time.h>
#include <tpie/tempname.h>
#include <tpie/tpie_log.h>

using namespace tpie;

///////////////////////////////////////////////////////////////////////////
/// Local initialization function. Create a permanent repository for the log
/// file name. Should be called only once, by theLogName() below.
///////////////////////////////////////////////////////////////////////////
static std::string& __tpie_log_name() 
{
	static std::string tln;
	tln = tempname::tpie_name("log", "" , "txt");
	return tln;
}

std::string& tpie::tpie_log_name() {
  static std::string& tln = __tpie_log_name();
  return tln;
}


logstream& tpie::tpie_log() {
  static logstream log(tpie_log_name(), LOG_DEBUG, LOG_DEBUG);
  return log;
}

void tpie::tpie_log_init(log_level level) {
  TP_LOG_SET_THRESHOLD(level);
}
