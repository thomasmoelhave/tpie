// Copyright (c) 1995 Darren Vengroff
//
// File: cpu_timer.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/11/95
//
// $Id: cpu_timer.h,v 1.2 1995-03-25 14:08:33 darrenv Exp $
//
#ifndef _CPU_TIMER_H
#define _CPU_TIMER_H

#include <sys/times.h>

#include <ostream.h>

#include <timer.h>

class cpu_timer : public timer
{
private:
    long clock_tick;
    tms last_sync;
    tms elapsed;
    clock_t last_sync_real;
    clock_t elapsed_real;
    bool running;
public:
    cpu_timer();
    virtual ~cpu_timer();
    
    void start();
    void stop();
    void sync();
    void reset();

    friend ostream &operator<<(ostream &s, cpu_timer &ct);
};

ostream &operator<<(ostream &s, cpu_timer &ct);

#endif // _CPU_TIMER_H 
