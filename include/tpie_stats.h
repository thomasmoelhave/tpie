// Copyright (C) 2001 Octavian Procopiuc
//
// File:    tpie_stats.h
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: tpie_stats.h,v 1.2 2001-12-29 05:18:23 tavi Exp $
//
// The tpie_stats class for recording statistics.
//
#ifndef _TPIE_STATS_H
#define _TPIE_STATS_H

template<int C>
class tpie_stats {
private:

  // The array storing the C statistics.
  unsigned long stats_[C];

public:

  // Reset all counts to 0.
  void reset() {
    for (int i = 0; i < C; i++)
      stats_[i] = 0;
  }
  // Default constructor. Set all counts to 0.
  tpie_stats() {
    reset();
  }
  // Copy constructor.
  tpie_stats(const tpie_stats<C>& ts) {
    for (int i = 0; i < C; i++)
      stats_[i] = ts.stats_[i];
  }
  // Record ONE event of type t.
  void record(int t) {
    stats_[t]++;
  }
  // Record k events of type t.
  void record(int t, unsigned long k) {
    stats_[t] += k;
  }
  // Record the events stored in s.
  void record(const tpie_stats<C>& s) {
    for (int i = 0; i < C; i++)
      stats_[i] += s.stats_[i];
  }
  // Set the number of type t events to k.
  void set(int t, unsigned long k) {
    stats_[t] = k;
  }
  // Inquire the number of type t events.
  unsigned long get(int t) const {
    return stats_[t];
  }
  
  //  ostream &output(ostream &s) const;

  ~tpie_stats() {}
};

#endif //_TPIE_STATS_H
