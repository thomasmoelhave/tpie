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

using namespace tpie;

cpu_timer::cpu_timer() :
    clock_tick_(0), last_sync_(), elapsed_(), last_sync_real_(0), elapsed_real_(0),
    running_(false) {
    TPIE_OS_SET_CLOCK_TICK;
}

cpu_timer::~cpu_timer() {
    // No code in this destructor.
}

void cpu_timer::sync() {

    clock_t current_real_;
    
    TPIE_OS_TMS current_;
    TPIE_OS_SET_CURRENT_TIME(current_);
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

std::ostream& tpie::operator<<(std::ostream &s, cpu_timer &wt) {
    if (wt.running()) {
        wt.sync();
    }
    
    TPIE_OS_OPERATOR_OVERLOAD;
}


