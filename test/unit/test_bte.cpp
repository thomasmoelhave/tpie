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

#include <tpie/bte/stream_stdio.h>
#include <tpie/bte/stream_ufs.h>
#include <tpie/bte/stream_mmap.h>
#include <tpie/bte/stream_cache.h>
#include <tpie/stream.h>
#include <tpie/bte/err.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <stdint.h>
using namespace tpie::bte;
using namespace std;

#define ERR(x) {cerr << x << endl; return 1;}
const size_t size = 1024*1024*10;

template <class T> 
int test_bte(T & bte, char * test) {

	if(!strcmp(test,"basic")) {
		srand(42);
		for(size_t i= 0; i < size; ++i) 
			if(bte.write_item(rand()) != NO_ERROR) ERR("Write failed");
		if(bte.stream_len() != size) ERR("Stream size wrong");
		if(bte.tell() != size) ERR("Tell failed");
		if(bte.seek(0) != NO_ERROR) ERR("Seek failed");
		if(bte.tell() != 0) ERR("Tell failed");
		srand(42);
		for(size_t i= 0; i < size; ++i) {
			int * x;
			if(bte.read_item(&x) != NO_ERROR) ERR("Read failed");
			if(*x != rand()) ERR("Wrong value returned");
		}
		return 0;
	} else if(!strcmp(test,"randomread")) {
		int * buf = new int[size];
		srand(42);
		if(bte.seek(0) != NO_ERROR) ERR("Seek failed (0)");
		for(size_t i =0; i < size; ++i) {
			int r = rand();
			buf[i] = r;
			if(bte.write_item(r)) ERR("Write failed");
			if(i % (size/200) == 1) {
				r = rand();
				int * x;
				if(bte.seek(r%(i+1)) != NO_ERROR) ERR("Seek failed (1)");
				if(bte.read_item(&x) != NO_ERROR) ERR("Read failed");
				if(*x != buf[r%(i+1)]) ERR("Wrong value returned");
				if(bte.seek(i+1) != NO_ERROR) ERR("Seek failed (2)");
			}
		}
		delete[] buf;
		return 0;
	} else if(!strcmp(test,"array")) {
		srand(42);
		int buf[1024];
		for(size_t i= 0; i < size; i += 1024) {
			for(size_t j=0; j < 1024; ++j) buf[j] = rand();
			if(bte.write_array(buf, 1024) != NO_ERROR) ERR("Write failed");
		}
		if(bte.stream_len() != size) ERR("Stream size wrong");
		if(bte.tell() != size) ERR("Tell failed");
		if(bte.seek(0) != NO_ERROR) ERR("Seek failed");
		if(bte.tell() != 0) ERR("Tell failed");
		srand(42);
		for(size_t i= 0; i < size; i += 1024) {
			size_t x=1024;
			if(bte.read_array(buf, x) != NO_ERROR || x != 1024) ERR("Write failed");
			for(size_t j=0; j < 1024; ++j) 	if(buf[j] != rand()) ERR("Wrong value returned");
		}
		return 0;
	}
	return 1;
}

int main(int argc, char **argv) {
	if(argc != 3) return 1;
	unlink("/tmp/stream");
	if(!strcmp(argv[1],"stdio")) {
		stream_stdio<int> stream("/tmp/stream", WRITE_STREAM);
		return test_bte(stream, argv[2]);
	} else if(!strcmp(argv[1],"cache")) {
		stream_cache<int> stream("/tmp/stream", WRITE_STREAM, size*sizeof(int));
		return test_bte(stream, argv[2]);
	} else if(!strcmp(argv[1],"ami_stream")) {
		tpie::ami::stream<int> stream("/tmp/stream", tpie::ami::WRITE_STREAM);
		return test_bte(stream, argv[2]);
#ifndef WIN32
	} else if(!strcmp(argv[1],"mmap")) {
		stream_mmap<int> stream("/tmp/stream", WRITE_STREAM);
		return test_bte(stream, argv[2]);
	} else if(!strcmp(argv[1],"ufs")) {
		stream_ufs<int> stream("/tmp/stream", WRITE_STREAM);
		return test_bte(stream, argv[2]);
#endif WIN32
	}
	return 1;
}
