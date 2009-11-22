 // -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, The TPIE development team

// This file is part of TPIE.

// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.

// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>
#include "../app_config.h"
#include <tpie/portability.h>
#include <cstring>
#include <tpie/stream/posix_bte.h>
#include <tpie/stream/concepts.h>
#include <tpie/stream/exception.h>
#include <tpie/stream/fd_file.h>
#include <tpie/util.h>

using namespace tpie;
using namespace std;
using namespace tpie::stream;

#define ERR(x) {cerr << x << endl; exit(1);}

template <typename T> 
void test_bte() {
	remove("tmp");
	{
		int d=42;
		T x(false, true, sizeof(int), 2345);
		x.open("tmp");
		if (x.size() != 0) ERR("New stream has wrong size");
		if (x.path() != "tmp") ERR("Wrong path");
		
		x.write(&d, 0, 1);
		x.write(&d, 1, 1);
		
		try {
			x.read(&d, 0, 1);
			ERR("Read should faild");
		} catch(io_exception &) {
			//Do nothing
		}
		
		if (x.size() != 2) ERR("Wrong size");
		x.close();
	}

	try {
		T x(true, false, sizeof(int)+1, 2345);
		x.open("tmp");
		ERR("Opened file with wrong item size");
	} catch(invalid_file_exception&) {
		//Do nothing
	}

	try {
		T x(true, false, sizeof(int), 2344);
		x.open("tmp");
		ERR("Opened file with wrong type magic");
	} catch(invalid_file_exception&) {
		//Do nothing
	}

	{
		int d;
		T x(true , true, sizeof(int), 2345);
		x.open("tmp");
		if (x.read(&d, 1, 1) != 1 || d != 42) ERR("Read failed");
		d=12;
		x.write(&d, 1, 1);
		x.write(&d, 2, 1);
		if (x.read(&d, 0, 1) != 1 || d != 42) ERR("Read failed");
		if (x.read(&d, 1, 1) != 1 || d != 12) ERR("Read failed");
		if (x.read(&d, 2, 1) != 1 || d != 12) ERR("Read failed");
		if (x.size() != 3) ERR("Wrong size");
		x.close();
	}
	
	{
		T x(true, false, sizeof(int), 2345);
		x.open("tmp");
		try {
			int d=44;
			x.write(&d, 0, 1);
			ERR("Write should faild");
		} catch(io_exception &) {
			//Do nothing
		}
		int d;
		if (x.read(&d, 0, 1) != 1 || d != 42) ERR("Read failed");
		if (x.read(&d, 1, 1) != 1 || d != 12) ERR("Read failed");
		if (x.read(&d, 2, 1) != 1 || d != 12) ERR("Read failed");
		x.close();
	}
	remove("tmp");
}

int main(int argc, char ** argv) {
	TPIE_CONCEPT_ASSERT((block_transfer_engine<posix_block_transfer_engine>));
	TPIE_CONCEPT_ASSERT((file< fd_file<int, true, false> >));

	if (argc == 2 && !strcmp(argv[1], "posix_bte")) {
		test_bte<posix_block_transfer_engine>();
		remove("tmp");
	} else if (argc == 2 && !strcmp(argv[1], "fd_file")) {
		{
			//First a simple test
			remove("tmp");
			fd_file<int, false, true, 8> file;
			file.open("tmp");
			{
				fd_file<int, false, true, 8>::stream stream(file, 0);
				if (file.size() != 0) ERR("size failed(1)");
				for(int i=0; i < 40; ++i)
					stream.write((i*8209)%8273);
			}
			if (file.size() != 40) ERR("size failed(2)");
			file.close();
		}

		{
			fd_file<int, true, false, 13> file;
			file.open("tmp");
			if (file.size() != 40) ERR("size failed(3)");
			{
				fd_file<int, true, false, 13>::stream stream(file, 0);
				for(int i=0; i< 40; ++i) {
					if (stream.has_more() == false) ERR("has_more failed");
					if (stream.read() != (i*8209)%8273) ERR("read failed");
				}
				if (stream.has_more() == true) ERR("has_more failed (2)");
				try {
					int r =stream.read();
					unused(r);
					ERR("read did not fail as expected");
				} catch(end_of_stream_exception &) {
					//Do nothing
				}			
			}
			file.close();

		}

		{
			fd_file<int, true, true, 16> file;
			file.open("tmp");
			srandom(1234);
			const int cnt=4;
			const int size=128;
			fd_file<int, true, true, 16>::stream ** streams = new fd_file<int, true, true, 16>::stream*[cnt];
			for(int i=0; i < cnt; ++i) 
				streams[i] = new fd_file<int, true, true, 16>::stream(file, 0);
			
			int content[size];
			for(int i=0; i < size; ++i) {
				content[i] = random();
				streams[0]->write(content[i]);
			}
			
			for(int i=0; i < 200000; ++i ) {
				int l=random()%size;
				int s=random()%cnt;
				streams[s]->seek(l);
				if (random() % 2 == 0) {
					if (streams[s]->read() != content[l]) ERR("read failed(2)");
				} else {
					content[l] = random();
					streams[s]->write(content[l]);
				}
			}
			for(int i=0; i < cnt; ++i)
				delete streams[i];
			delete[] streams;
			
			file.close();
		}
		
	} else {
		return 1;
	}
	return 0;
}
