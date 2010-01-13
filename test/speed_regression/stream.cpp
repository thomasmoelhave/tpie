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

#define REV 100000

#if(REV < 5)
#include <tpie/stream.h>
#else
#include <tpie/file_stream.h>
#endif
#include <iostream>
#include "testtime.h"

#if(REV < 5)
using namespace tpie::ami;
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
		file_stream<uint64_t> s;
		s.open("tmp", file_base::write);
		uint64_t x=42;
		for(size_t i=0; i < size*1024; ++i) s.write(x);
#endif
	}
	getTestRealtime(end);
	std::cout << " " << testRealtimeDiff(start,end);
	std::cout.flush();
	
	getTestRealtime(start);
	{
		uint64_t y=0;
#if(REV < 5)
		stream<uint64_t> s("tmp", READ_STREAM);
		for(size_t i=0; i < size*1024; ++i) {
			uint64_t * x;
			s.read_item(&x);
			y ^= x;
		}
#else
		file_stream<uint64_t> s;
		s.open("tmp", file_base::read);
		for(size_t i=0; i < size*1024; ++i) y ^= s.read();
#endif
		if (y==777) std::cout << " " << std::endl;
	}
	getTestRealtime(end);
	std::cout << " " << testRealtimeDiff(start,end);
	std::cout.flush();
	
	getTestRealtime(start);
	{
		uint64_t x[1024];
		for(int i=0; i < 1024; ++i) x[i]=42;
#if(REV < 5)
		stream<uint64_t> s("tmp", WRITE_STREAM);
		for(size_t i=0; i < size; ++i) s.write_array(x,1024);
#else
		file_stream<uint64_t> s;
		s.open("tmp", file_base::write);
		for(size_t i=0; i < size; ++i) s.write((uint64_t*)x, x+1024);
#endif
	}
	getTestRealtime(end);
	std::cout << " " << testRealtimeDiff(start,end);
	std::cout.flush();
	
	getTestRealtime(start);
	{
		uint64_t x[1024];
		uint64_t y=0;
		for(uint64_t i=0; i < 1024; ++i) x[i]=42;
#if(REV < 5)
		stream<uint64_t> s("tmp", READ_STREAM);
        for(size_t i=0; i < size; ++i) {
			stream_offset_type z=1024; 
			s.read_array(x,&z);
			for(uint64_t j=0; j < y; ++j) y ^= x[j];
		}
#else
		file_stream<uint64_t> s;
		s.open("tmp", file_base::read);
		for(size_t i=0; i < size; ++i) {
			s.read((uint64_t*)x, x+1024);
			for(uint64_t j=0; j < y; ++j) y ^= x[j];
		}			
#endif
		if (y==777) std::cout << " " << std::endl;
	}
	getTestRealtime(end);
	std::cout << " " << testRealtimeDiff(start,end) << std::endl;
}
