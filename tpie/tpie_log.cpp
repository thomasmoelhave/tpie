// Copyright 2008, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

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
