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
#include <tpie/util.h>

using namespace tpie;
using namespace std;
using namespace tpie::stream;
int main(int argc, char ** argv) {
  TPIE_CONCEPT_ASSERT((block_transfer_engine<posix_block_transfer_engine>));
//   BOOST_CONCEPT_ASSERT((pushable< tpie::streaming::sort<test_sink> >));
//   BOOST_CONCEPT_ASSERT((pushable< tpie::streaming::pull_sort<int> >));
//   BOOST_CONCEPT_ASSERT((pullable< tpie::streaming::pull_sort<int> >));
//   BOOST_CONCEPT_ASSERT((pushable< tpie::streaming::buffer<test_sink> >));
//   BOOST_CONCEPT_ASSERT((pushable< tpie::streaming::pull_buffer<int> >));
//   BOOST_CONCEPT_ASSERT((pullable< tpie::streaming::pull_buffer<int> >));

	
// 	if (argc != 2) return 1;
// 	remove("/tmp/stream");
  
// 	if (!strcmp(argv[1], "source")) {
// 		stream<int> s;
// 		for(int i=0; test[i]; ++i)
// 			s.write_item(test[i]);
// 		s.seek(0);
		
// 		test_sink sink;
// 		sink.ok();
// 		stream_source<stream<int>, test_sink> ss(&s, sink);
// 		ss.run();
// 		sink.final();
// 	} else if (!strcmp(argv[1], "sink")) {
	  
// 		stream<int> s;
// 		stream_sink<stream<int> > sink(&s);
	  
// 		sink.begin();
// 		for(int i=0; test[i]; ++i)
// 			sink.push(test[i]);
// 		sink.end();
		
// 		s.seek(0);
		
// 		int * item;
// 		int i=0;
// 		while(s.read_item(&item) != tpie::ami::END_OF_STREAM) 
// 			if (*item != test[i++] ) ERR("sink");
// 		if(test[i] != 0) ERR("sink");
// 	} else if (!strcmp(argv[1], "sort")) {
// 		vector<int> t2;
// 		for(int i=0; test[i]; ++i)
// 			t2.push_back(test[i]);
// 		std::random_shuffle(t2.begin(), t2.end());
		
// 		test_sink sink;
// 		tpie::streaming::sort<test_sink> sort(sink);
// 		TPIE_OS_SIZE_T mem = 50*1024*1024;
// 		sort.setMemoryIn(mem);
// 		sort.setMemoryOut(mem);
		
// 		sort.begin();
// 		for(int i=0; test[i]; ++i)
// 			sort.push(t2[i]);
// 		sink.ok();
// 		sort.end();
// 		sink.final();
		
// 	} else if (!strcmp(argv[1], "pull_sort")) {
// 		vector<int> t2;
// 		for(int i=0; test[i]; ++i)
// 			t2.push_back(test[i]);
// 		std::random_shuffle(t2.begin(), t2.end());
		
// 		tpie::streaming::pull_sort<int> sort;
// 		TPIE_OS_SIZE_T mem = 50*1024*1024;
// 		sort.setMemoryIn(mem);
// 		sort.setMemoryOut(mem);
		
// 		sort.begin();
// 		for (int i=0; test[i]; ++i)
// 			sort.push(t2[i]);
// 		sort.end();
		
// 		sort.beginPull();
// 		for (int i=0; test[i]; ++i) {
// 			if (sort.atEnd() ) ERR("atEnd()");
// 			if (sort.pull() != test[i]) ERR("pull()");
// 		}
// 		if (!sort.atEnd() ) ERR("atEnd()");
// 		sort.endPull();
		
// 	} else if (!strcmp(argv[1], "buffer")) {
// 		test_sink sink;
// 		tpie::streaming::buffer<test_sink> buffer(sink);
// 		TPIE_OS_SIZE_T mem = 50*1024*1024;
// 		buffer.setMemoryIn(mem);
// 		buffer.setMemoryOut(mem);
		
// 		buffer.begin();
// 		for(int i=0; test[i]; ++i)
// 			buffer.push(test[i]);
// 		sink.ok();
// 		buffer.end();
// 		sink.final();
		
// 	} else if (!strcmp(argv[1], "pull_buffer")) {
// 		tpie::streaming::pull_buffer<int> buffer;
// 		TPIE_OS_SIZE_T mem = 50*1024*1024;
// 		buffer.setMemoryIn(mem);
// 		buffer.setMemoryOut(mem);
		
// 		buffer.begin();
// 		for (int i=0; test[i]; ++i)
// 			buffer.push(test[i]);
// 		buffer.end();
		
// 		buffer.beginPull();
// 		for (int i=0; test[i]; ++i) {
// 			if (buffer.atEnd() ) ERR("atEnd()");
// 			if (buffer.pull() != test[i]) ERR("pull()");
// 		}
// 		if (!buffer.atEnd() ) ERR("atEnd()");
// 		buffer.endPull();
// 	} else {
// 		return 1;
// 	}


//   return 0;

}

