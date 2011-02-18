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
#include <tpie/logstream.h>
#include <cstdio>
namespace tpie {

log_stream_buf::log_stream_buf(log_level level): m_level(level) {
	setp(m_buff, m_buff+buff_size-2);
}

log_stream_buf::~log_stream_buf() {flush();}

void log_stream_buf::flush() {
	*pptr() = 0;
	if (m_log_target_count == 0)
		//As a special service if noone is listening and
		fwrite(m_buff, 1, pptr() - m_buff, stderr);
	else
		for(size_t i=0; i < m_log_target_count; ++i)
			m_log_targets[i]->log(m_level, m_buff, pptr() - m_buff);
	setp(m_buff, m_buff+buff_size-2);
}

int log_stream_buf::overflow(int c) {
	flush();
	*pptr() = c;
	pbump(1);
	flush();
	return c;
}

int log_stream_buf::sync() {
	//Do not display the messages before there is a target
	if (m_log_target_count == 0) return 0; 
	flush();
	return 0;
}

void log_stream_buf::set_level(log_level level) {
	if (m_level==level) return;
	sync();
	m_level=level;
}

void log_stream_buf::add_target(log_target * t) {
	m_log_targets[m_log_target_count] = t;
	++m_log_target_count;
}

void log_stream_buf::remove_target(log_target * t) {
	size_t l=m_log_target_count;
	for(size_t i=0; i < m_log_target_count; ++i) 
		if (m_log_targets[i] == t) 
			l=i;
	if (l == m_log_target_count) return;
	--m_log_target_count;
	std::swap(m_log_targets[l], m_log_targets[m_log_target_count]);
	m_log_targets[m_log_target_count]=0;
}


// Setting priority and threshold on the fly with manipulators.
logstream& manip_level(logstream& tpl, log_level p) {
	tpl.set_level(p);
    return tpl;
}

logmanip<log_level> setlevel(log_level p) {
   return logmanip<log_level>(&manip_level, p);
} 

} //namespace tpie


