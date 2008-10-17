//
// File: cpu_timer.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/11/95
//
// $Id: cpu_timer.h,v 1.8 2004-08-17 16:48:11 jan Exp $
//
// A timer measuring user time, system time and wall clock time.  The
// timer can be start()'ed, stop()'ed, and queried. Querying can be
// done without stopping the timer, to report intermediate values.
//
#ifndef _TPIE_CPU_TIMER_H
#define _TPIE_CPU_TIMER_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <iostream>
#include <tpie/timer.h>

namespace tpie {
    
    class cpu_timer : public timer {

    private:
	long        clock_tick_;
	
	TPIE_OS_TMS last_sync_;
	TPIE_OS_TMS elapsed_;
	
	clock_t     last_sync_real_;
	clock_t     elapsed_real_;

	bool        running_;

    public:
	cpu_timer();
	virtual ~cpu_timer();
	
	void start();
	void stop();
	void sync();
	void reset();
	
	double user_time();
	double system_time();
	double wall_time();

	bool running() const { 
	    return running_;
	}

	long clock_tick() const {
	    return clock_tick_;
	}

	TPIE_OS_TMS last_sync() const {
	    return last_sync_;
	}
	
	TPIE_OS_TMS elapsed() const {
	    return elapsed_;
	}

	clock_t last_sync_real() const {
	    return last_sync_real_;
	}
	
	clock_t elapsed_real() const {
	    return elapsed_real_;
	}
	
    };
    
	std::ostream &operator<<(std::ostream &s, tpie::cpu_timer &ct);
}
 

#endif // _TPIE_CPU_TIMER_H 
