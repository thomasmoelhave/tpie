// Copyright (c) 1994 Darren Erik Vengroff
//
// File: tpie_assert.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//
// $Id: tpie_assert.h,v 1.3 1994-08-31 19:28:03 darrenv Exp $
//
// This code is based on 
// 	myassert.h
//	Darren Erik Vengroff [dev@cs.brown.edu]
//


#ifndef _TPIE_ASSERT_H
#define _TPIE_ASSERT_H

#include <tpie_log.h>

#if DEBUG_CERR
#include <iostream.h>
#define CERR_OUT(s) {cerr << s; cerr.flush();}
#else
#define CERR_OUT(s)
#endif

#define QUOTE(x) # x

#define QUOTEMACRORESULT(x) QUOTE(x)

#if DEBUG_ASSERTIONS

#ifndef DEBUG_STR
#define DEBUG_STR	1
#endif

#define tp_assert(c, msg)						\
{									\
    if (!(c)) {								\
	LOG_ASSERT(__FILE__ ":" QUOTEMACRORESULT(__LINE__)		\
                   ": Assertion failed: ");				\
        LOG_ASSERT(msg);						\
    	LOG_ASSERT('\n');						\
	LOG_FLUSH_LOG;							\
	CERR_OUT(__FILE__ ":" QUOTEMACRORESULT(__LINE__)		\
                 ": Assertion failed: ");				\
        CERR_OUT(msg);							\
        CERR_OUT('\n');							\
    }									\
}
#else
#define tp_assert(c,msg)
#endif

#if DEBUG_STR
#define TP_DEBUG_OUT(s) LOG_DEBUG_INFO(s); LOG_FLUSH_LOG; CERR_OUT(s)
#else
#define TP_DEBUG_OUT(s)
#endif
	

#endif // _TPIE_ASSERT_H

