// Copyright (c) 1995 Darren Vengroff
//
// File: cpu_timer.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/11/95
//

#include <versions.h>
VERSION(cpu_timer_cpp,"$Id: cpu_timer.cpp,v 1.4 2002-01-03 20:21:54 tavi Exp $");

#include <unistd.h>
#include <sys/times.h>

#include <cpu_timer.h>


cpu_timer::cpu_timer() :
        running(false)
{
    clock_tick = sysconf(_SC_CLK_TCK);
    
    elapsed.tms_utime = 0;
    elapsed.tms_stime = 0;
    elapsed.tms_cutime = 0;
    elapsed.tms_cstime = 0;

    elapsed_real = 0;
}

cpu_timer::~cpu_timer()
{
}

void cpu_timer::sync()
{
    tms current;
    clock_t current_real;
    
    current_real = times(&current);

    elapsed.tms_utime += current.tms_utime - last_sync.tms_utime;
    elapsed.tms_stime += current.tms_stime - last_sync.tms_stime;
    elapsed.tms_cutime += current.tms_cutime - last_sync.tms_cutime;
    elapsed.tms_cstime += current.tms_cstime - last_sync.tms_cstime;

    elapsed_real += current_real - last_sync_real;

    last_sync = current;
    last_sync_real = current_real;
}


void cpu_timer::start()
{
    if (!running) {
        last_sync_real = times(&last_sync);
        running = true;
    }
}

void cpu_timer::stop()
{
    if (running) {
        sync();
        running = false;
    }
}

void cpu_timer::reset()
{
    if (running) {
        last_sync_real = times(&last_sync);
    }
    
    elapsed.tms_utime = 0;
    elapsed.tms_stime = 0;
    elapsed.tms_cutime = 0;
    elapsed.tms_cstime = 0;

    elapsed_real = 0;
}

double cpu_timer::user_time() {
  if (running) sync();
  return double(elapsed.tms_utime) / double(clock_tick);
}

double cpu_timer::system_time() {
  if (running) sync();
  return double(elapsed.tms_stime) / double(clock_tick);
}

double cpu_timer::wall_time() {
  if (running) sync();
  return double(elapsed_real) / double(clock_tick);
}

ostream &operator<<(ostream &s, cpu_timer &wt)
{
    if (wt.running) {
        wt.sync();
    }

    return s << double(wt.elapsed.tms_utime) / double(wt.clock_tick) << "u " <<
        double(wt.elapsed.tms_stime) / double(wt.clock_tick) << "s " <<
        double(wt.elapsed_real) / double(wt.clock_tick);
}


