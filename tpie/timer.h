// Copyright (c) 1995 Darren Vengroff
//
// File: timer.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/11/95
//
// $Id: timer.h,v 1.2 2003-04-17 19:58:26 jan Exp $
//
// General definition of a virtual timer class.
//
#ifndef _TPIE_TIMER_H
#define _TPIE_TIMER_H

// Get definitions for working with Unix and Windows
#include <portability.h>

namespace tpie {
    
    class timer {
	
    public:
	virtual void start(void) = 0;
	virtual void stop(void) = 0;
	virtual void reset(void) = 0;
	virtual ~timer() {} ;
    };

}

#endif // _TPIE_TIMER_H 
