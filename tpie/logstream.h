// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, The TPIE development team
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
/// logstream class used by definitions in \ref tpie_log.h.
///////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_LOGSTREAM_H__
#define __TPIE_LOGSTREAM_H__

#include <tpie/config.h>
#include <tpie/loglevel.h>
#include <streambuf>
#include <ostream>

namespace tpie {

struct log_target {
	virtual void log(log_level level, const char * message, size_t message_size) = 0;
	virtual ~log_target() { }
};

class log_stream_buf: public std::basic_streambuf<char, std::char_traits<char> >  {
private:
	const static size_t buff_size = 2048;
	const static size_t max_targets = 8;

	char m_buff[buff_size];
	log_target * m_log_targets[max_targets];
	size_t m_log_target_count;
	log_level m_level;
	bool m_enabled;

public:
	log_stream_buf(log_level level);
	virtual ~log_stream_buf();
	void flush();	
	virtual int overflow(int c = traits_type::eof());
	virtual int sync();
	void set_level(log_level level);
	void add_target(log_target * t);
	void remove_target(log_target * t);
	inline void enable(bool e) {flush(); m_enabled=e;}
	inline bool enabled() {return m_enabled;}
};


///////////////////////////////////////////////////////////////////////////
/// A log is like a regular output stream, but it also supports messages
/// at different priorities, see \ref log_level. 
///////////////////////////////////////////////////////////////////////////
class logstream: public std::ostream {
private:
	log_stream_buf m_buff;
public:
	///////////////////////////////////////////////////////////////////////////
	/// Constructor.
	///////////////////////////////////////////////////////////////////////////
	inline logstream(log_level level=LOG_INFORMATIONAL): std::ostream(&m_buff), m_buff(level) {}

	///////////////////////////////////////////////////////////////////////////
	/// Add a target for the log messages
	///////////////////////////////////////////////////////////////////////////
	inline void add_target(log_target * t) {m_buff.add_target(t);}

	///////////////////////////////////////////////////////////////////////////
	/// Remove a target for the log messages
	///////////////////////////////////////////////////////////////////////////
	inline void remove_target(log_target * t) {m_buff.remove_target(t);}
	
	///////////////////////////////////////////////////////////////////////////
	/// Set the current level of logging
///////////////////////////////////////////////////////////////////////////	
	inline void set_level(log_level level) {m_buff.set_level(level);}

	inline void disable(bool d=false) {m_buff.enable(!d);}
	inline void enable(bool e=true) {m_buff.enable(e);}
	inline bool enabled() {return m_buff.enabled();}
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
	// logmanip(const logmanip<TP>& other) : _f(), _a() {
	// 	*this = other;
	// }
	
	///////////////////////////////////////////////////////////////////////////
	/// Assigment operator.
	///////////////////////////////////////////////////////////////////////////
	// logmanip<TP>& operator=(const logmanip<TP>& other) {
	// 	if (this != &other) {
	// 		_f = other._f;
	// 		_a = other._a;
	// 	}
	// 	return *this;
	// }
};

logstream& manip_level(logstream& tpl, log_level p);
logmanip<log_level> setlevel(log_level p);
    
}  //  tpie namespace


#endif //__TPIE_LOGSTREAM_H__

#endif // _LOGSTREAM_H 
