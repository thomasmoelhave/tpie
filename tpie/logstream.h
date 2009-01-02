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



// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <fstream>

namespace tpie {
    
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
    class logstream : public std::ofstream {
	
    public:

  ///////////////////////////////////////////////////////////////////////////
  /// Flag signaling whether the log is initialized. 
  ///////////////////////////////////////////////////////////////////////////
	static bool log_initialized;

  ///////////////////////////////////////////////////////////////////////////
  /// Current priority, i.e. \ref log_level. 
  ///////////////////////////////////////////////////////////////////////////
	unsigned int priority;

  ///////////////////////////////////////////////////////////////////////////
  /// The current threshold level for \ref logging.
  ///////////////////////////////////////////////////////////////////////////
	unsigned int threshold;

  ///////////////////////////////////////////////////////////////////////////
  /// Constructor.
  ///////////////////////////////////////////////////////////////////////////
	logstream(const std::string& fname, unsigned int p = 0, unsigned int tp = 0);

  ///////////////////////////////////////////////////////////////////////////
  /// Destructor.
  ///////////////////////////////////////////////////////////////////////////
	~logstream();
	
	// Output operators
	
	_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const char *);
	_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const char);
	_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const int);
	_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const unsigned int);
	_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const long int);
	_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const long unsigned int);
	_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const float);
	_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const double);
	_DECLARE_LOGSTREAM_OUTPUT_OPERATOR(const size_t);
	
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
    
    logstream& manip_priority(logstream& tpl, unsigned long p);

    logmanip<unsigned long> setthreshold(unsigned long p);

    logstream& manip_threshold(logstream& tpl, unsigned long p);

    logmanip<unsigned long> setpriority(unsigned long p);
    
}  //  tpie namespace

#endif // _LOGSTREAM_H 
