// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, The TPIE development team
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
#include "../app_config.h"

#define REV 10000

#if(REV < 1876)
#include <tpie/stream.h>
#else
#include <tpie/stream/fd_file.h>
#endif
#include <iostream>
#include "testtime.h"

#if(REV < 1876)
using namespace tpie::ami;

#else
using namespace tpie::stream;
#endif
using namespace tpie::test;

const size_t size=4*1024*1024/sizeof(uint64_t);

int main() {
	test_realtime_t start;
	test_realtime_t end;
	
	//The porpous of this test is to test the speed of the io calls, not the file system
	getTestRealtime(start);
	{
#if(REV < 1876)
		stream<uint64_t> s("tmp", WRITE_STREAM);
		uint64_t x=42;
		for(size_t i=0; i < size*1024; ++i) s.write_item(x);
#else
		fd_file<uint64_t, false, true> f;
		f.open("tmp");
		{
			fd_file<uint64_t, false, true>::stream s(f, 0);
			uint64_t x=42;
			for(size_t i=0; i < size*1024; ++i) s.write(x);
		}
#endif
	}
	getTestRealtime(end);
	std::cout << " " << testRealtimeDiff(start,end);
	std::cout.flush();
	
	getTestRealtime(start);
	{
#if(REV < 1876)
		stream<uint64_t> s("tmp", READ_STREAM);
		uint64_t * x;
		for(size_t i=0; i < size*1024; ++i) s.read_item(&x);
#else
		fd_file<uint64_t, true, false> f;
		f.open("tmp");
		{
			fd_file<uint64_t, true, false>::stream s(f, 0);
			uint64_t x;
			for(size_t i=0; i < size*1024; ++i) x = s.read();
		}
#endif
	}
	getTestRealtime(end);
	std::cout << " " << testRealtimeDiff(start,end);
	std::cout.flush();
	
	getTestRealtime(start);
	{
		uint64_t x[1024];
		for(int i=0; i < 1024; ++i) x[i]=42;
#if(REV < 1876)
		stream<uint64_t> s("tmp", WRITE_STREAM);
		for(size_t i=0; i < size; ++i) s.write_array(x,1024);
#else
		fd_file<uint64_t, false, true> f;
		f.open("tmp");
		{
			fd_file<uint64_t, false, true>::stream s(f, 0);
			for(size_t i=0; i < size; ++i) s.write((uint64_t*)x, x+1024);
		}
#endif
	}
	getTestRealtime(end);
	std::cout << " " << testRealtimeDiff(start,end);
	std::cout.flush();
	
	getTestRealtime(start);
	{
		uint64_t x[1024];
		for(uint64_t i=0; i < 1024; ++i) x[i]=42;
		
#if(REV < 1876)
		stream<uint64_t> s("tmp", READ_STREAM);
		for(size_t i=0; i < size; ++i) {TPIE_OS_OFFSET y=1024; s.read_array(x,&y);}
#else
		fd_file<uint64_t, true, false> f;
		f.open("tmp");
		{
			fd_file<uint64_t, true, false>::stream s(f, 0);
			for(size_t i=0; i < size; ++i) s.read((uint64_t*)x, x+1024);
		}
#endif
	}
	getTestRealtime(end);
	std::cout << " " << testRealtimeDiff(start,end) << std::endl;
}
