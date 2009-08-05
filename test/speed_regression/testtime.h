// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2005-2009, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>


#ifndef __TPIE_TESTTIME_H
#define __TPIE_TESTTIME_H
#include <sys/time.h>
#include <sys/resource.h>
#include <stdint.h>
#include <time.h>

///////////////////////////////////////////////////////////////////////////
/// \file testtime.h 
/// Time managment for tests
///////////////////////////////////////////////////////////////////////////

namespace tpie {
	namespace test {

		///////////////////////////////////////////////////////////////////
		/// Type used for vaiable holding time information
		///////////////////////////////////////////////////////////////////
		typedef struct rusage test_time_t;

		///////////////////////////////////////////////////////////////////
		/// Type used for vaiable holding real time information
		///////////////////////////////////////////////////////////////////
		typedef uint_fast64_t test_realtime_t;
		
		///////////////////////////////////////////////////////////////////
		/// Sample the time and store it
		///////////////////////////////////////////////////////////////////
		inline void getTestTime(test_time_t &a) {
			getrusage(RUSAGE_SELF, &a);
		}

		///////////////////////////////////////////////////////////////////
		/// Sample the real time and store it
		///////////////////////////////////////////////////////////////////
		inline void getTestRealtime(test_realtime_t& a) {
			struct timeval tv;
			gettimeofday(&tv, NULL);
			a = tv.tv_sec;
			a *= 1000*1000;
			a += tv.tv_usec;
		}

		///////////////////////////////////////////////////////////////////
		/// Calculate time difference in micro seconds
		///////////////////////////////////////////////////////////////////
		inline uint_fast64_t testTimeDiff(const test_time_t& a, const test_time_t& b) {
			uint_fast64_t time = b.ru_utime.tv_sec - a.ru_utime.tv_sec;
			time *= 1000*1000;
			time += b.ru_utime.tv_usec - a.ru_utime.tv_usec;
			return time;
		}

		///////////////////////////////////////////////////////////////////
		/// Calculate real time difference in micro seconds
		///////////////////////////////////////////////////////////////////
		inline uint_fast64_t testRealtimeDiff(const test_realtime_t a, const test_realtime_t b) {
			return b - a;
		}
 
		///////////////////////////////////////////////////////////////////
		/// Calculate page fault difference
		///////////////////////////////////////////////////////////////////
		inline uint_fast64_t testPagefaultDiff(const test_time_t& a, const test_time_t& b) {
			return b.ru_majflt - a.ru_majflt;
		}

		///////////////////////////////////////////////////////////////////
		/// Calculate io usage in blocks
		///////////////////////////////////////////////////////////////////
		inline uint_fast64_t testIODiff(const test_time_t& a, const test_time_t& b) {
			return b.ru_inblock - a.ru_inblock + b.ru_oublock - a.ru_oublock;
		}
	}
}
#endif //__TPIE_TESTTIME_H
