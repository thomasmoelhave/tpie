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

#ifndef _TPIE_LOGSTREAM_H
#define _TPIE_LOGSTREAM_H
///////////////////////////////////////////////////////////////////////////
/// \file logstream.h 
/// Provides stream definitions specifically for logging purposes in TPIE.
/// \anchor logging \par logging in TPIE
/// When logging is turned on, TPIE creates a log file TPLOG_XXXXXX, where 
/// XXXXXX is a unique system dependent identifier.
/// TPIE writes into this file using a logstream class, 
/// which is derived from ofstream and has the additional functionality of
/// setting a priority and a threshold for logging. If the priority of a message
/// is below the threshold, the message is not logged. There are four priority
/// levels defined in TPIE, see \ref log_level. 
/// By default, the threshold of the log is set to the lowest level, 
/// TP_LOG_WARNING. To change the threshold level, use LOG_SET_THRESHOLD().
/// The threshold level can be reset as many times as needed in a program. 
/// This enables the developer to focus the debugging eFFort on a certain part
/// of the program. 
///
/// The following compile-time macros are provided for writing into the log:
///
/// TP_LOG_FATAL(msg), TP_LOG_FATAL_ID(msg) 
///
/// TP_LOG_WARNING(msg), TP_LOG_WARNING_ID(msg) 
///
/// TP_LOG_APP_DEBUG(msg), TP_LOG_APP_DEBUG_ID(msg) 
///
/// TP_LOG_DEBUG(msg), TP_LOG_DEBUG_ID(msg) 
///
/// ,where \p msg is the information to be logged; \p msg can be any type that is
/// supported by the C++ fstream class. Each of these macros sets the
/// corresponding priority and sends \p msg to the log stream.
/// The macros ending in _ID record the source code filename and line number
/// in the log, while the corresponding macros without the _ID suffix do not.
///
/// \internal \todo make it happen (ticket 33):
/// Note that logging can be toggled on and off for both the TPIE library
/// as well as for TPIE apps by using the switches in the CMAKE
/// interface for building TPIE. 
///////////////////////////////////////////////////////////////////////////


#include <tpie/config.h>

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <sstream>

namespace tpie {


///////////////////////////////////////////////////////////////////////////
/// TPIE \ref logging levels, from higest priority to lowest.
///////////////////////////////////////////////////////////////////////////
	enum log_level {
		/** LOG_FATAL is the highest error level and is used for all kinds of errors
		 *  that would normally impair subsequent computations; LOG_FATAL errors are
		 *  always logged */
		LOG_FATAL = 0,	
		/** LOG_WARNING is the next lowest and is used for warnings. */
		LOG_WARNING,	
		/** LOG_APP_DEBUG can be used by applications built on top of TPIE, for 
		 * logging debugging information. */ 
		LOG_APP_DEBUG,     
		/** LOG_DEBUG is the lowest level and is used by the TPIE library for 
		 * logging debugging information. */ 
		LOG_DEBUG,		
		/** Logging level for warnings concerning memory allocation and deallocation. */
		LOG_MEM_DEBUG
    };
}

namespace tpie {


	struct log_target {
		virtual void operator()(log_level level, const char * message) = 0;
	};

/** A macro for declaring output operators for log streams. */
#define _DECLARE_LOGSTREAM_OUTPUT_OPERATOR(T) logstream& operator<<(T)
    
    ///////////////////////////////////////////////////////////////////////////
    /// A log is like a regular output stream, but it also supports messages
    /// at different priorities, see \ref log_level.  If a message's priority is at least as high
    /// as the current priority threshold, then it appears in the log.  
    /// Otherwise, it does not.  Lower numbers have higher priority; 0 is
    /// the highest.
    /// \internal \todo document members
    ///////////////////////////////////////////////////////////////////////////
    class logstream { //: private std::ofstream {
    public:
		///////////////////////////////////////////////////////////////////////////
		/// Flag signaling whether the log is initialized. 
		///////////////////////////////////////////////////////////////////////////
		static bool log_initialized;
		
		///////////////////////////////////////////////////////////////////////////
		/// Current priority, i.e. \ref log_level. 
		///////////////////////////////////////////////////////////////////////////
		log_level priority;
		
		///////////////////////////////////////////////////////////////////////////
		/// The current threshold level for \ref logging.
		///////////////////////////////////////////////////////////////////////////
		log_level threshold;
		
		///////////////////////////////////////////////////////////////////////////
		/// The target where log messages should go
		///////////////////////////////////////////////////////////////////////////
		log_target * m_target;

		std::stringstream m_sstream;
		bool disable;

		///////////////////////////////////////////////////////////////////////////
		/// Constructor.
		///////////////////////////////////////////////////////////////////////////
		logstream(log_target * target, log_level p = LOG_FATAL, log_level tp = LOG_FATAL);

		///////////////////////////////////////////////////////////////////////////
		/// Destructor.
		///////////////////////////////////////////////////////////////////////////
		~logstream();

		inline log_target * get_target() {return m_target;}
		
		inline void set_target(log_target * t) {m_target=t;}

		inline operator const bool() const {return true;}
		inline bool flush();
		
		// Output operators
		_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const char *);
		_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const std::string &);
		_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const char);
		_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const int);
		_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const unsigned int);
		_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const long int);
		_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const long unsigned int);
		_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const float);
		_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const double);
#ifdef _WIN64
		_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const size_t);
#endif
		
		//  Unix "long long", Win32 "LONGLONG".
		TPIE_OS_DECLARE_LOGSTREAM_LONGLONG
    };
    
    
	///////////////////////////////////////////////////////////////////////////
	/// The logmanip template is based on the omanip template from iomanip.h 
	/// in the libg++ sources.
	///////////////////////////////////////////////////////////////////////////
	template <class TP> class logmanip {
		logstream& (*_f)(logstream&, TP);
		TP _a;
    public:
		///////////////////////////////////////////////////////////////////////////
		/// Constructor.
		///////////////////////////////////////////////////////////////////////////
		logmanip(logstream& (*f)(logstream&, TP), TP a) : _f(f), _a(a) {}
	
		///////////////////////////////////////////////////////////////////////////
		/// Extracts a message from the logmanip object and inserting it into 
		/// the logstream \p o.
		///////////////////////////////////////////////////////////////////////////
		friend logstream& operator<< (logstream& o, const logmanip<TP>& m) {
			(*m._f)(o, m._a); 
			return o;
		}
	
		///////////////////////////////////////////////////////////////////////////
		/// Copy constructor.
		///////////////////////////////////////////////////////////////////////////
		logmanip(const logmanip<TP>& other) : _f(), _a() {
			*this = other;
		}
	
		///////////////////////////////////////////////////////////////////////////
		/// Assigment operator.
		///////////////////////////////////////////////////////////////////////////
		logmanip<TP>& operator=(const logmanip<TP>& other) {
			if (this != &other) {
				_f = other._f;
				_a = other._a;
			}
			return *this;
		}
    };
    
    logstream& manip_priority(logstream& tpl, log_level p);

    logmanip<log_level> setthreshold(log_level p);

    logstream& manip_threshold(logstream& tpl, log_level p);

    logmanip<log_level> setpriority(log_level p);
    
}  //  tpie namespace

#endif // _LOGSTREAM_H 
