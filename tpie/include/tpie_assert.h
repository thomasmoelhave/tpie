// Copyright (c) 1994 Darren Erik Vengroff
//
// File: tpie_assert.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//
// $Id: tpie_assert.h,v 1.5 2001-05-08 21:13:20 tavi Exp $
//

#ifndef _TPIE_ASSERT_H
#define _TPIE_ASSERT_H

#include <tpie_log.h>
#include <assert.h>
#include <iostream.h>

#if DEBUG_ASSERTIONS

#define tp_assert(condition,message) { \
  if (!(condition)) { \
    LOG_FATAL_ID("Assertion failed:"); \
    LOG_FATAL_ID(message); \
    cerr << "Assertion failed: " << message << endl; \
    assert(0); \
  } \
}

#else
#define tp_assert(condition,message)
#endif

#endif // _TPIE_ASSERT_H

