// Copyright (c) 1994 Darren Erik Vengroff
//
// File: logstream.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//
// $Id: logstream.h,v 1.2 1994-05-12 14:46:00 dev Exp $
//
#ifndef _LOGSTREAM_H
#define _LOGSTREAM_H

#include <fstream.h>
#include <iomanip.h>

// A log is like a regular output stream, but it also supports messages
// at different priorities.  If a message's priority is at least as high
// as the current priority threshold, then it appears in the log.  
// Otherwise, it does not.  Lower numbers have higher priority; 0 is
// the highest.  1 is the default if not 

class logstream : ofstream {
  private:
    unsigned int priority;
    unsigned int thresh_p;

  public:
    logstream(const char *fname, unsigned int p = 0, unsigned int tp = 0);

    void set_priority(unsigned int dp);
    unsigned int get_default_priority(void);
    void threshold_priority(unsigned int tp);
    unsigned int get_threshold_priority(void);    
}

// Setting priority on the fly with manipulators.

logstream& manip_priority(logstream& tpl, unsigned int p)
{
    tpl.priority = p;
    return tpl;
}




#endif // _LOGSTREAM_H 
