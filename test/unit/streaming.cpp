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
#include <tpie/bte/err.h>
#include <tpie/portability.h>
#include <cstring>
#include <tpie/streaming.h>

using namespace tpie::bte;
using namespace tpie::ami;
using namespace tpie;
using namespace std;
using namespace tpie::streaming;

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
	bool b;
	bool e;
	int c;
	bool o;
	test_sink(): b(false), e(false), c(0), o(false) {}
	
	void begin(size_t count=0) {
		if (e || b) ERR("begin()");
		b=true;
	}
	
	void ok() {o=true;}
	
	void end() {
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
	if (argc != 2) return 1;
	remove("/tmp/stream");
  
	if (!strcmp(argv[1], "source")) {
		stream<int> s;
		for(int i=0; test[i]; ++i)
			s.write_item(test[i]);
		s.seek(0);
		
		test_sink sink;
		sink.ok();
		stream_source<stream<int>, test_sink> ss(&s, sink);
		ss.run();
		sink.final();
	} else if (!strcmp(argv[1], "sink")) {
	  
		stream<int> s;
		stream_sink<stream<int> > sink(&s);
	  
		sink.begin();
		for(int i=0; test[i]; ++i)
			sink.push(test[i]);
		sink.end();
		
		s.seek(0);
		
		int * item;
		int i=0;
		while(s.read_item(&item) != tpie::ami::END_OF_STREAM) 
			if (*item != test[i++] ) ERR("sink");
		if(test[i] != 0) ERR("sink");
	} else if (!strcmp(argv[1], "sort")) {
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
		
	} else if (!strcmp(argv[1], "buffer")) {
		test_sink sink;
		tpie::streaming::buffer<test_sink> buffer(sink);
		TPIE_OS_SIZE_T mem = 50*1024*1024;
		buffer.setMemoryIn(mem);
		buffer.setMemoryOut(mem);
		
		buffer.begin();
		for(int i=0; test[i]; ++i)
			buffer.push(test[i]);
		sink.ok();
		buffer.end();
		sink.final();
		
	} else if (!strcmp(argv[1], "pull_buffer")) {
		tpie::streaming::pull_buffer<int> buffer;
		TPIE_OS_SIZE_T mem = 50*1024*1024;
		buffer.setMemoryIn(mem);
		buffer.setMemoryOut(mem);
		
		buffer.begin();
		for (int i=0; test[i]; ++i)
			buffer.push(test[i]);
		buffer.end();
		
		buffer.beginPull();
		for (int i=0; test[i]; ++i) {
			if (buffer.atEnd() ) ERR("atEnd()");
			if (buffer.pull() != test[i]) ERR("pull()");
		}
		if (!buffer.atEnd() ) ERR("atEnd()");
		buffer.endPull();
	} else {
		return 1;
	}


  return 0;

}



