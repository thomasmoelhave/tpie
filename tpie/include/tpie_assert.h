// Copyright (c) 1994 Darren Erik Vengroff
//
// File: tpie_assert.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//
// $Id: tpie_assert.h,v 1.9 2004-08-12 12:35:32 jan Exp $
//

#ifndef _TPIE_ASSERT_H
#define _TPIE_ASSERT_H

// Get definitions for working with Unix and Windows
#include <portability.h>

#include <tpie_log.h>
#include <assert.h>
#include <iostream>

#if DEBUG_ASSERTIONS

#define tp_assert(condition,message) { \
  if (!(condition)) { \
   TP_LOG_FATAL_ID("Assertion failed:"); \
   TP_LOG_FATAL_ID(message); \
    cerr << "Assertion failed: " << message << "\n"; \
    assert(0); \
  } \
}

#else
#define tp_assert(condition,message)
#endif

#endif // _TPIE_ASSERT_H

