//
// File: cpu_timer.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/11/95
//
// $Id: cpu_timer.h,v 1.4 2003-04-17 18:52:29 jan Exp $
//
// A timer measuring user time, system time and wall clock time.  The
// timer can be start()'ed, stop()'ed, and queried. Querying can be
// done without stopping the timer, to report intermediate values.
//
#ifndef _CPU_TIMER_H
#define _CPU_TIMER_H

// Get definitions for working with Unix and Windows
#include <portability.h>

#include <ostream.h>
#include <timer.h>

class cpu_timer : public timer {
private:
  long clock_tick;

  TPIE_OS_TIME last_sync;
  TPIE_OS_TIME elapsed;

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

  double user_time();
  double system_time();
  double wall_time();
  
  friend ostream &operator<<(ostream &s, cpu_timer &ct);
};

ostream &operator<<(ostream &s, cpu_timer &ct);

#endif // _CPU_TIMER_H 
