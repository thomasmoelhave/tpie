// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
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
    /// Returns the file name of the log stream.
    ///////////////////////////////////////////////////////////////////////////
    std::string& tpie_log_name();
    
    ///////////////////////////////////////////////////////////////////////////
    /// Returns the only logstream object. 
    ///////////////////////////////////////////////////////////////////////////
    logstream& tpie_log();
    
	void set_log_target(log_target * r);
	log_target * get_log_target();


    ///////////////////////////////////////////////////////////////////////////
    /// Initializes the log.
    ///////////////////////////////////////////////////////////////////////////
    void tpie_log_init(log_level level = LOG_WARNING);    
#if TPL_LOGGING		
    
// Macros to simplify logging.  The argument to the macro can be any type
// that log streams have an output operator for.
    
#define TP_LOG_FLUSH_LOG (!tpie::logstream::log_initialized || tpie::tpie_log().flush())
    
// eg: LOG_FATAL(tpie::LOG_ID_MSG)
#define TP_LOG_ID_MSG __FILE__ << " line " << __LINE__ << ": "

/** Macro to simplify \ref logging. \sa log_lecel. */
#ifndef TP_LOG_FATAL
#define TP_LOG_FATAL(msg)						\
    (!tpie::logstream::log_initialized || tpie::tpie_log() << /*tpie::setpriority(tpie::LOG_FATAL) <<*/ msg)
#endif
/** Macro to simplify \ref logging. \sa log_lecel. */
#ifndef TP_LOG_WARNING
#define TP_LOG_WARNING(msg)						\
    (!tpie::logstream::log_initialized || tpie::tpie_log() << /*tpie::setpriority(tpie::LOG_WARNING) <<*/ msg)
#endif
/** Macro to simplify \ref logging. \sa log_lecel. */
#define TP_LOG_APP_DEBUG(msg)						\
    (!tpie::logstream::log_initialized || tpie::tpie_log() << tpie::setpriority(tpie::LOG_APP_DEBUG)  << msg)
/** Macro to simplify \ref logging. \sa log_lecel. */
#ifndef TP_LOG_DEBUG
#define TP_LOG_DEBUG(msg)						\
    (!tpie::logstream::log_initialized || tpie::tpie_log() << tpie::setpriority(tpie::LOG_DEBUG)  << msg)
#endif
/** Macro to simplify \ref logging. \sa log_lecel. */
// #ifndef TP_LOG_MEM_DEBUG
// #define TP_LOG_MEM_DEBUG(msg)						\
//     (!tpie::logstream::log_initialized || tpie::tpie_log() << tpie::setpriority(tpie::LOG_MEM_DEBUG)  << msg)
// #endif
    
/** Macro to simplify \ref logging. \sa log_lecel. */
#define TP_LOG_FATAL_ID(msg)						\
    (TP_LOG_FATAL(TP_LOG_ID_MSG << msg << "\n"), TP_LOG_FLUSH_LOG)
/** Macro to simplify \ref logging. \sa log_lecel. */
#define TP_LOG_WARNING_ID(msg)						\
    (TP_LOG_WARNING(TP_LOG_ID_MSG << msg << "\n"), TP_LOG_FLUSH_LOG)
/** Macro to simplify \ref logging. \sa log_lecel. */
#define TP_LOG_APP_DEBUG_ID(msg)					\
    (TP_LOG_APP_DEBUG(TP_LOG_ID_MSG << msg << "\n"), TP_LOG_FLUSH_LOG)
/** Macro to simplify \ref logging. \sa log_lecel. */
#define TP_LOG_DEBUG_ID(msg)						\
    (TP_LOG_DEBUG(TP_LOG_ID_MSG << msg << "\n"), TP_LOG_FLUSH_LOG)
/** Macro to simplify \ref logging. \sa log_lecel. */
#define TP_LOG_MEM_DEBUG_ID(msg)					\
    (TP_LOG_MEM_DEBUG(TP_LOG_ID_MSG << msg << "\n"), TP_LOG_FLUSH_LOG)
    
/** Set the current \ref log_level threshold for \ref logging in TPIE. */
#define TP_LOG_SET_THRESHOLD(level) (tpie_log() << setthreshold(level))
    
#else // !TPL_LOGGING
    
// We are not compiling logging.
    
#ifndef TP_LOG_FATAL
#define TP_LOG_FATAL(msg) 
#endif

#ifndef TP_LOG_WARNING
#define TP_LOG_WARNING(msg) 
#endif

#ifndef TP_LOG_APP_DEBUG
#define TP_LOG_APP_DEBUG(msg)
#endif

#ifndef TP_LOG_DEBUG
#define TP_LOG_DEBUG(msg) 
#endif

#ifndef TP_LOG_MEM_DEBUG
#define TP_LOG_MEM_DEBUG(msg)
#endif

    
#define TP_LOG_FATAL_ID(msg)
#define TP_LOG_WARNING_ID(msg)
#define TP_LOG_APP_DEBUG_ID(msg)
#define TP_LOG_DEBUG_ID(msg)
#define TP_LOG_MEM_DEBUG_ID(msg)
    
#define TP_LOG_SET_THRESHOLD(level)
#define TP_LOG_FLUSH_LOG {}
    
#endif // TPL_LOGGING

}  //  tpie namespace

#endif // _TPIE_LOG_H 
