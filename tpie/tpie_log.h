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
/// Logging functionality and log_level codes for different priorities of log messages.
///////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <memory>
#include <ostream>
#include <sstream>
#include <stack>
#include <streambuf>
#include <string_view>
#include <tpie/config.h>
#include <tpie/loglevel.h>
#include <tpie/logstream.h>
#include <tpie/tpie_export.h>
#include <vector>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief TPIE logging levels, from higest priority to lowest.
///////////////////////////////////////////////////////////////////////////////
enum log_level {
	/** LOG_FATAL is the highest error level and is used for all kinds of errors
	 *  that would normally impair subsequent computations; LOG_FATAL errors are
	 *  always logged */
	LOG_FATAL = 0,

	/** LOG_ERROR is used for none fatal errors. */
	LOG_ERROR,

	/** LOG_WARNING  is used for warnings. */
	LOG_WARNING,

	/** LOG_INFORMATIONAL is used for informational messagse. */
	LOG_INFORMATIONAL,

	/** LOG_APP_DEBUG can be used by applications built on top of TPIE, for
	 * logging debugging information. */
	LOG_APP_DEBUG,

	/** LOG_DEBUG is the lowest level and is used by the TPIE library for
	 * logging debugging information. */
	LOG_DEBUG,

	/** Logging level for warnings concerning memory allocation and deallocation. */
	LOG_MEM_DEBUG,

	LOG_PIPE_DEBUG,

	/** Logging levels to be further defined by user applications. */
	LOG_USER1,
	LOG_USER2,
	LOG_USER3
};

struct log_target {
	virtual void log(log_level level, std::string_view message) = 0;
	virtual ~log_target() { }
	virtual void begin_group(std::string_view) {};
	virtual void end_group() {};
};

TPIE_EXPORT void add_log_target(log_target * t);
TPIE_EXPORT void remove_log_target(log_target * t);

TPIE_EXPORT void begin_log_group(std::string_view name);
TPIE_EXPORT void end_log_group();

TPIE_EXPORT void log_to_targets(log_level level, std::string_view message);
TPIE_EXPORT bool get_log_enabled();
TPIE_EXPORT void set_log_enabled(bool enabled);


/** A simple logger that writes messages to a tpie temporary file */
class TPIE_EXPORT file_log_target: public log_target {
private:
	std::stack<std::string> groups;
public:
	std::ofstream m_out;
	std::string m_path;
	log_level m_threshold;

	/** Construct a new file logger
	 * \param threshold record messages at or above this severity threshold
	 * */
	file_log_target(log_level threshold);

	/** Implement \ref log_target virtual method to record message
	 * \param level severity of message
	 * \param message content of message
	 * */
	void log(log_level level, std::string_view message);

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Creates a new logging group. All console output that occurs after
	/// this will appear in the same visual group.
	///////////////////////////////////////////////////////////////////////////////
	void begin_group(std::string_view name);

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Closes the most recently created logging group.
	///////////////////////////////////////////////////////////////////////////////
	void end_group();
private:
	std::string build_prefix(size_t length);
};

/** A simple logger that writes messages to stderr */
class TPIE_EXPORT stderr_log_target: public log_target {
private:
	std::stack<std::string> groups;
public:
	log_level m_threshold;

    /** Construct a new stderr logger
	 * \param threshold record messages at or above this severity threshold
	 * */
	stderr_log_target(log_level threshold);

	/** Implement \ref log_target virtual method to record message
	 * \param level severity of message
	 * \param message content of message
	 * \param size lenght of message array
	 * */
	void log(log_level level, std::string_view message);

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Creates a new logging group. All console output that occurs after
	/// this will appear in the same visual group.
	///////////////////////////////////////////////////////////////////////////////
	void begin_group(std::string_view name);

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Closes the most recently created logging group.
	///////////////////////////////////////////////////////////////////////////////
	void end_group();
private:
	std::string build_prefix(size_t length);
};

///////////////////////////////////////////////////////////////////////////
/// \brief Returns the file name of the log stream.
/// This assumes that init_default_log has been called.
///////////////////////////////////////////////////////////////////////////
const std::string& log_name();

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_init to initialize the log subsystem.
///////////////////////////////////////////////////////////////////////////////
void init_default_log();

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_finish to deinitialize the log subsystem.
///////////////////////////////////////////////////////////////////////////////
void finish_default_log();

class LogObj: public std::ostream {
private:
	std::stringbuf buff;
	log_level level;

	void do_flush() {
		log_to_targets(level, buff.str());
		buff.str("");
	}
public:
	LogObj(log_level level=LOG_INFORMATIONAL): std::ostream(&buff), level(level) {}
	LogObj(LogObj&& o): std::ostream(&buff), buff(std::move(o.buff)), level(o.level) {}
	~LogObj() {do_flush();}

	LogObj & operator=(LogObj &&) = delete;
	LogObj(const LogObj&) = delete;
	LogObj & operator=(const LogObj &) = delete;


	[[deprecated]] void flush() {do_flush();}
	[[deprecated]] void set_level(log_level level) {
		do_flush();
		this->level = level;
	}
	[[deprecated]] void add_target(log_target * t) { add_log_target(t); }
	[[deprecated]] void remove_target(log_target * t) { remove_log_target(t); }
};

inline LogObj get_log_by_level(log_level level) {return LogObj(level);}

/// \brief Return logstream for writing fatal log messages.
inline LogObj log_fatal() {return LogObj(LOG_FATAL);}

/// \brief Return logstream for writing error log messages.
inline LogObj log_error() {return LogObj(LOG_ERROR);}

/// \brief Return logstream for writing info log messages.
inline LogObj log_info() {return LogObj(LOG_INFORMATIONAL);}

/// \brief Return logstream for writing warning log messages.
inline LogObj log_warning() {return LogObj(LOG_WARNING);}

/// \brief Return logstream for writing app_debug log messages.
inline LogObj log_app_debug() {return LogObj(LOG_APP_DEBUG);}

/// \brief Return logstream for writing debug log messages.
inline LogObj log_debug() noexcept {return LogObj(LOG_DEBUG);}

/// \brief Return logstream for writing mem_debug log messages.
inline LogObj log_mem_debug() {return LogObj(LOG_MEM_DEBUG);}

/// \brief Return logstream for writing pipe_debug log messages.
inline LogObj log_pipe_debug() {return LogObj(LOG_PIPE_DEBUG);}

class scoped_log_enabler {
private:
	bool m_orig;
public:
	bool get_orig() const noexcept {return m_orig;}
	scoped_log_enabler(bool e) noexcept {
		m_orig = get_log_enabled();
		set_log_enabled(e);
	}
	~scoped_log_enabler() noexcept {
		set_log_enabled(m_orig);
	}
};


///////////////////////////////////////////////////////////////////////////
/// \brief Returns the only logstream object.
///////////////////////////////////////////////////////////////////////////
[[deprecated]] inline LogObj get_log() {return LogObj(LOG_INFORMATIONAL);}

#if TPL_LOGGING	
/// \def TP_LOG_FLUSH_LOG  \deprecated Use \ref get_log().flush() instead.
#define TP_LOG_FLUSH_LOG tpie::get_log().flush()

/// \def TP_LOG_FATAL \deprecated Use \ref log_fatal() instead.
#define TP_LOG_FATAL(msg) tpie::log_fatal() << msg
/// \def TP_LOG_WARNING \deprecated Use \ref log_warning() instead.
#define TP_LOG_WARNING(msg)	tpie::log_warning() << msg
/// \def TP_LOG_APP_DEBUG \deprecated Use \ref log_app_debug() instead.
#define TP_LOG_APP_DEBUG(msg) tpie::log_app_debug() << msg
/// \def TP_LOG_DEBUG \deprecated Use \ref log_debug() instead.
#define TP_LOG_DEBUG(msg) tpie::log_debug() << msg
/// \def TP_LOG_MEM_DEBUG \deprecated Use \ref log_mem_debug() instead.
#define TP_LOG_MEM_DEBUG(msg) tpie::log_mem_debug() << msg

#define TP_LOG_ID_MSG __FILE__ << " line " << __LINE__ << ": "

/** \def TP_LOG_FATAL_ID Macro to simplify \ref logging. \sa log_level. */
#define TP_LOG_FATAL_ID(msg) TP_LOG_FATAL(TP_LOG_ID_MSG << msg << std::endl)

/** \def TP_LOG_WARNING_ID Macro to simplify \ref logging. \sa log_level. */
#define TP_LOG_WARNING_ID(msg) TP_LOG_WARNING(TP_LOG_ID_MSG << msg << std::endl)

/** \def TP_LOG_APP_DEBUG_ID Macro to simplify \ref logging. \sa log_level. */
#define TP_LOG_APP_DEBUG_ID(msg) TP_LOG_APP_DEBUG(TP_LOG_ID_MSG << msg << std::endl)

/** \def TP_LOG_DEBUG_ID Macro to simplify \ref logging. \sa log_level. */
#define TP_LOG_DEBUG_ID(msg) TP_LOG_DEBUG(TP_LOG_ID_MSG << msg << std::endl)

/** \def TP_LOG_MEM_DEBUG_ID Macro to simplify \ref logging. \sa log_level. */
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
