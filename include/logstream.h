// Copyright (c) 1994 Darren Erik Vengroff
//
// File: tp_log.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//
// $Id: logstream.h,v 1.1 1994-05-12 14:05:23 dev Exp $
//
#ifndef _TP_LOG_H
#define _TP_LOG_H

#include <iostream.h>

// A log is like a regular output stream, but it also supports messages
// at different priorities.  If a message's priority is at least as high
// as the current priority threshold, then it appears in the log.  
// Otherwise, it does not.

class tp_log : ostream {
private:
  

}


#endif // _TP_LOG_H 
