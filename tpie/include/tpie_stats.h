// Copyright (C) 2001 Octavian Procopiuc
//
// File:    statistics.h
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: tpie_stats.h,v 1.1 2001-12-29 04:09:00 tavi Exp $
//
// Class statistics.
//
#ifndef _STATISTICS_H
#define _STATISTICS_H

template<int C>
class statistics {
private:

  unsigned long stats_[C];

public:

  // Reset all counts to 0.
  void reset() {
    for (int i = 0; i < C; i++)
      stats_[i] = 0;
  }
  // Constructor. Set all counts to 0.
  statistics() {
    reset();
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
  void record(const statistics<C>& s) {
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

  ~statistics() {}
};

#endif //_STATISTICS_H
