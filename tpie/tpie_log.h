// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2011, The TPIE development team
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

#ifndef _TPIE_LOG_H
#define _TPIE_LOG_H
///////////////////////////////////////////////////////////////////////////
/// \file tpie_log.h
/// Provides \ref logging functionalities and log_level codes for different priorities of log messages.
///////////////////////////////////////////////////////////////////////////

#include <tpie/config.h>
#include <tpie/logstream.h>

namespace tpie {

///////////////////////////////////////////////////////////////////////////
/// Returns the file name of the log stream
/// This assumes that init_default_log has been called
///////////////////////////////////////////////////////////////////////////
const std::string& log_name();

void init_default_log();
void finish_default_log();

///////////////////////////////////////////////////////////////////////////
/// Returns the only logstream object. 
///////////////////////////////////////////////////////////////////////////
logstream& get_log();

inline logstream & log_fatal() {return get_log() << setlevel(LOG_FATAL);}
inline logstream & log_error() {return get_log() << setlevel(LOG_ERROR);}
inline logstream & log_info() {return get_log() << setlevel(LOG_INFORMATIONAL);}
inline logstream & log_warning() {return get_log() << setlevel(LOG_WARNING);}
inline logstream & log_app_debug() {return get_log() << setlevel(LOG_APP_DEBUG);}
inline logstream & log_debug() {return get_log() << setlevel(LOG_DEBUG);}
inline logstream & log_mem_debug() {return get_log() << setlevel(LOG_MEM_DEBUG);}

#if TPL_LOGGING		
// Macros to simplify logging.  The argument to the macro can be any type
// that log streams have an output operator for.

#define TP_LOG_FLUSH_LOG tpie::get_log().flush()
    
#define TP_LOG_FATAL(msg) tpie::log_fatal() << msg
#define TP_LOG_WARNING(msg)	tpie::log_warning() << msg
#define TP_LOG_APP_DEBUG(msg) tpie::log_app_debug() << msg
#define TP_LOG_DEBUG(msg) tpie::log_debug() << msg
#define TP_LOG_MEM_DEBUG(msg) tpie::log_mem_debug() << msg

// eg: LOG_FATAL(tpie::LOG_ID_MSG)
#define TP_LOG_ID_MSG __FILE__ << " line " << __LINE__ << ": "

/** Macro to simplify \ref logging. \sa log_lecel. */
#define TP_LOG_FATAL_ID(msg) TP_LOG_FATAL(TP_LOG_ID_MSG << msg << std::endl)

/** Macro to simplify \ref logging. \sa log_lecel. */
#define TP_LOG_WARNING_ID(msg) TP_LOG_WARNING(TP_LOG_ID_MSG << msg << std::endl)

/** Macro to simplify \ref logging. \sa log_lecel. */
#define TP_LOG_APP_DEBUG_ID(msg) TP_LOG_APP_DEBUG(TP_LOG_ID_MSG << msg << std::endl)

/** Macro to simplify \ref logging. \sa log_lecel. */
#define TP_LOG_DEBUG_ID(msg) TP_LOG_DEBUG(TP_LOG_ID_MSG << msg << std::endl)

/** Macro to simplify \ref logging. \sa log_lecel. */
#define TP_LOG_MEM_DEBUG_ID(msg) TP_LOG_MEM_DEBUG(TP_LOG_ID_MSG << msg << std::endl)
    
#else // !TPL_LOGGING
    
// We are not compiling logging.
#define TP_LOG_FATAL(msg) 
#define TP_LOG_WARNING(msg) 
#define TP_LOG_APP_DEBUG(msg)
#define TP_LOG_DEBUG(msg) 
#define TP_LOG_MEM_DEBUG(msg)
    
#define TP_LOG_FATAL_ID(msg)
#define TP_LOG_WARNING_ID(msg)
#define TP_LOG_APP_DEBUG_ID(msg)
#define TP_LOG_DEBUG_ID(msg)
#define TP_LOG_MEM_DEBUG_ID(msg)
    
#define TP_LOG_FLUSH_LOG {}
    
#endif // TPL_LOGGING

}  //  tpie namespace

#endif // _TPIE_LOG_H 
