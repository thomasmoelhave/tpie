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

#include <tpie/cpu_timer.h>
#include <boost/date_time.hpp>

#ifdef _WIN32
#define TPIE_OS_SET_CLOCK_TICK				\
		clock_tick_ = CLOCKS_PER_SEC			
#else
#define TPIE_OS_SET_CLOCK_TICK clock_tick_ = sysconf(_SC_CLK_TCK); elapsed_.tms_utime = 0; elapsed_.tms_stime = 0; elapsed_.tms_cutime = 0; elapsed_.tms_cstime = 0;
#endif

#ifdef _WIN32
#define TPIE_OS_UNIX_ONLY_SET_ELAPSED_TIME(current)
#else
#define TPIE_OS_UNIX_ONLY_SET_ELAPSED_TIME(current) elapsed_.tms_utime += (current).tms_utime - last_sync_.tms_utime; elapsed_.tms_stime += (current).tms_stime - last_sync_.tms_stime; elapsed_.tms_cutime += (current).tms_cutime - last_sync_.tms_cutime; elapsed_.tms_cstime += (current).tms_cstime - last_sync_.tms_cstime;
#endif

#ifdef _WIN32
#define TPIE_OS_LAST_SYNC_REAL_DECLARATION last_sync_real_ = clock();
#else
#define TPIE_OS_LAST_SYNC_REAL_DECLARATION last_sync_real_ = times(&last_sync_);	
#endif

#ifdef _WIN32
#define TPIE_OS_USER_TIME_BODY return double(elapsed_real()) / double(clock_tick())
#else
#define TPIE_OS_USER_TIME_BODY return double(elapsed().tms_utime) / double(clock_tick())
#endif

#ifdef _WIN32
#define TPIE_OS_SYSTEM_TIME_BODY return double(elapsed_real()) / double(clock_tick())
#else
#define TPIE_OS_SYSTEM_TIME_BODY return double(elapsed().tms_stime) / double(clock_tick())
#endif


#ifdef _WIN32	
#define TPIE_OS_OPERATOR_OVERLOAD \
    return s << double(wt.elapsed_real()) / double(wt.clock_tick()); 
#else
#define TPIE_OS_OPERATOR_OVERLOAD return s << double(wt.elapsed().tms_utime) / double(wt.clock_tick()) << "u " << double(wt.elapsed().tms_stime) / double(wt.clock_tick()) << "s " << double(wt.elapsed_real()) / double(wt.clock_tick());	
#endif

namespace tpie {

cpu_timer::cpu_timer() :
    clock_tick_(0), last_sync_(), elapsed_(), last_sync_real_(0), elapsed_real_(0),
    running_(false) {
    TPIE_OS_SET_CLOCK_TICK;
}

void cpu_timer::sync() {
    tms current_;
#ifdef _WIN32
    current_ = boost::posix_time::second_clock::local_time();
    clock_t current_real_ = clock();
#else
    clock_t current_real_ = times(&current_);
#endif
    TPIE_OS_UNIX_ONLY_SET_ELAPSED_TIME(current_);
    
    elapsed_real_ += current_real_ - last_sync_real_;
    
    last_sync_ = current_;
    last_sync_real_ = current_real_;
}


void cpu_timer::start() {

    if (!running_) {
	TPIE_OS_LAST_SYNC_REAL_DECLARATION;
	running_ = true;
    }
}

void cpu_timer::stop() {
    if (running_) {
        sync();
        running_ = false;
    }
}

void cpu_timer::reset() {
    if (running_) {		
	TPIE_OS_LAST_SYNC_REAL_DECLARATION;
    }
    
    TPIE_OS_SET_CLOCK_TICK;	
    elapsed_real_ = 0;
}

double cpu_timer::user_time() {
    if (running_) sync();
    TPIE_OS_USER_TIME_BODY;
}

double cpu_timer::system_time() {
    if (running_) sync();
    TPIE_OS_SYSTEM_TIME_BODY;
}

double cpu_timer::wall_time() {
    if (running_) sync();
    return double(elapsed_real_) / double(clock_tick_);
}

std::ostream& operator<<(std::ostream &s, cpu_timer &wt) {
    if (wt.running()) {
        wt.sync();
    }
    
    TPIE_OS_OPERATOR_OVERLOAD;
}

}
