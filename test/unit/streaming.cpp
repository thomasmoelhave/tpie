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
#include <cstring>
#include <tpie/bte/err.h>
#include <tpie/portability.h>
#include <tpie/streaming.h>

using namespace tpie;
using namespace std;
using namespace tpie::streaming;
//namespace sc = tpie::streaming::concepts;

int test[] = {
     2,    3,    5,    7,   11,   13,   17,   19,   23,   29, 
    31,   37,   41,   43,   47,   53,   59,   61,   67,   71, 
    73,   79,   83,   89,   97,  101,  103,  107,  109,  113, 
   127,  131,  137,  139,  149,  151,  157,  163,  167,  173, 
   179,  181,  191,  193,  197,  199,  211,  223,  227,  229, 
   233,  239,  241,  251,  257,  263,  269,  271,  277,  281, 
   283,  293,  307,  311,  313,  317,  331,  337,  347,  349, 
   353,  359,  367,  373,  379,  383,  389,  397,  401,  409, 
   419,  421,  431,  433,  439,  443,  449,  457,  461,  463, 
   467,  479,  487,  491,  499,  503,  509,  521,  523,  541, 
   547,  557,  563,  569,  571,  577,  587,  593,  599,  601, 
   607,  613,  617,  619,  631,  641,  643,  647,  653,  659, 
   661,  673,  677,  683,  691,  701,  709,  719,  727,  733, 
   739,  743,  751,  757,  761,  769,  773,  787,  797,  809, 
   811,  821,  823,  827,  829,  839,  853,  857,  859,  863, 
   877,  881,  883,  887,  907,  911,  919,  929,  937,  941, 
   947,  953,  967,  971,  977,  983,  991,  997, 1009, 1013, 
  1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 
  1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 
  1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 
  1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 
  1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 
  1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 0};


#define ERR(x) {cerr << x << endl; exit(1);}

struct test_sink: public memory_single {
	typedef int item_type;
	typedef empty_type begin_data_type;
	typedef empty_type end_data_type;
	bool b;
	bool e;
	int c;
	bool o;
	test_sink(): b(false), e(false), c(0), o(false) {}
	
	void begin(stream_size_type items=max_items, empty_type * data=0) {
		unused(items);
		unused(data);
		if (e || b) ERR("begin()");
		b=true;
	}
	
	void ok() {o=true;}
	
	void end(empty_type * data=0) {
		unused(data);
		if (!b || e) ERR("end()");
		e=true;
	}
	
	void push(const int & x) {
		if (!o) ERR("Push to early");
		if (test[c++] != x) ERR("push()");
	}

	void final() {
		if (!e || test[c] != 0) ERR("final()");
	};
};

int main(int argc, char ** argv) {
//  BOOST_CONCEPT_ASSERT((sc::pushable< tpie::streaming::stream_sink<stream<int> > >));
// 	BOOST_CONCEPT_ASSERT((sc::pushable< tpie::streaming::sort<test_sink> >));
// 	BOOST_CONCEPT_ASSERT((sc::pushable< tpie::streaming::pull_sort<int> >));
// 	BOOST_CONCEPT_ASSERT((sc::pullable< tpie::streaming::pull_sort<int> >));
// 	BOOST_CONCEPT_ASSERT((sc::pushable< tpie::streaming::buffer<test_sink> >));
// 	BOOST_CONCEPT_ASSERT((sc::pushable< tpie::streaming::pull_buffer<int> >));
// 	BOOST_CONCEPT_ASSERT((sc::pullable< tpie::streaming::pull_buffer<int> >));
// 	BOOST_CONCEPT_ASSERT((sc::memory_managable< tpie::streaming::stream_sink<stream<int> > >));
// 	BOOST_CONCEPT_ASSERT((sc::memory_managable< tpie::streaming::sort<test_sink> >));
// 	BOOST_CONCEPT_ASSERT((sc::memory_managable< tpie::streaming::pull_sort<int> >));
// 	BOOST_CONCEPT_ASSERT((sc::memory_managable< tpie::streaming::buffer<test_sink> >));
// 	BOOST_CONCEPT_ASSERT((sc::memory_managable< tpie::streaming::pull_buffer<int> >));
	
	double blockFactor=file_base::calculate_block_factor(32*sizeof(int));
			
	if (argc != 2) return 1;
	remove("/tmp/stream");
  
	if (!strcmp(argv[1], "source")) {
		file_stream<int> s(blockFactor);
		s.open();
		for(memory_size_type i=0; test[i]; ++i)
			s.write(test[i]);
		s.seek(0);
		
		test_sink sink;
		sink.ok();
		stream_source<test_sink> ss(s, sink);
		ss.process();
		sink.final();
	} else if (!strcmp(argv[1], "sink")) {
		file_stream<int> s(blockFactor);
		s.open();
		stream_sink<int> sink(s);
	  
		sink.begin();
		for(memory_size_type i=0; test[i]; ++i)
			sink.push(test[i]);
		sink.end();
		
		s.seek(0);
		
		memory_size_type i=0;
		while(s.can_read()) 
			if (s.read() != test[i++] ) ERR("sink");
		if(test[i] != 0) ERR("sink");
	} /*else if (!strcmp(argv[1], "sort")) {
		vector<int> t2;
		for(int i=0; test[i]; ++i)
			t2.push_back(test[i]);
		std::random_shuffle(t2.begin(), t2.end());
		
		test_sink sink;
		tpie::streaming::sort<test_sink> sort(sink);
		TPIE_OS_SIZE_T mem = 50*1024*1024;
		sort.setMemoryIn(mem);
		sort.setMemoryOut(mem);
		
		sort.begin();
		for(int i=0; test[i]; ++i)
			sort.push(t2[i]);
		sink.ok();
		sort.end();
		sink.final();
		
		} else if (!strcmp(argv[1], "pull_sort")) {
		vector<int> t2;
		for(int i=0; test[i]; ++i)
			t2.push_back(test[i]);
		std::random_shuffle(t2.begin(), t2.end());
		
		tpie::streaming::pull_sort<int> sort;
		TPIE_OS_SIZE_T mem = 50*1024*1024;
		sort.setMemoryIn(mem);
		sort.setMemoryOut(mem);
		
		sort.begin();
		for (int i=0; test[i]; ++i)
			sort.push(t2[i]);
		sort.end();
		
		sort.beginPull();
		for (int i=0; test[i]; ++i) {
			if (sort.atEnd() ) ERR("atEnd()");
			if (sort.pull() != test[i]) ERR("pull()");
		}
		if (!sort.atEnd() ) ERR("atEnd()");
		sort.endPull();
		
		} */ 
	else if (!strcmp(argv[1], "buffer")) {
		test_sink sink;
		tpie::streaming::buffer<test_sink> buffer(sink);
		memory_size_type mem = 50*1024*1024;
		buffer.set_memory_in(mem);
		buffer.set_memory_out(mem);
		
		buffer.begin();
		for(int i=0; test[i]; ++i)
			buffer.push(test[i]);
		sink.ok();
		buffer.end();
		sink.final();
		
	} else if (!strcmp(argv[1], "pull_buffer")) {
		tpie::streaming::pull_buffer<int> buffer;
		memory_size_type mem = 50*1024*1024;
		buffer.set_memory_in(mem);
		buffer.set_memory_out(mem);
		
		buffer.begin();
		for (int i=0; test[i]; ++i)
			buffer.push(test[i]);
		buffer.end();
		
		buffer.pull_begin();
		for (int i=0; test[i]; ++i) {
			if (!buffer.can_pull() ) ERR("can_pull");
			if (buffer.pull() != test[i]) ERR("pull()");
		}
		if (buffer.can_pull() ) ERR("can_pull 2");
		buffer.pull_end();
	} else {
		return 1;
	}
	return 0;
}



