// Copyright (c) 1994 Darren Erik Vengroff
//
// File: tpie_assert.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//
// $Id: tpie_assert.h,v 1.1 1994-05-18 18:50:44 dev Exp $
//
// This code is based on 
// 	myassert.h
//	Darren Erik Vengroff [dev@cs.brown.edu]
//


#ifndef _TPIE_ASSERT_H
#define _TPIE_ASSERT_H

#include <tpie_log.h>


#define QUOTE(x) # x

#define QUOTEMACRORESULT(x) QUOTE(x)

#if DEBUG_ASSERTIONS

#ifndef DEBUG_STR
#define DEBUG_STR	1
#endif

#define tp_assert(c, msg)						\
{									\
    if (!(c)) {								\
	LOG_ASSERT("Assertion failed (line "				\
		   QUOTEMACRORESULT(__LINE__)				\
		   " of " __FILE__ "): " msg "\n");			\
	LOG_FLUSH_LOG;							\
    }									\
}
#else
#define tp_assert(c,msg)
#endif

#if DEBUG_STR
#define TP_DEBUGSTR(s) LOG_DEBUG_INFO(s)
#else
#define TP_DEBUGSTR(s)
#endif
	

#endif // _TPIE_ASSERT_H
