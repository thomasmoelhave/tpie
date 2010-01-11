// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
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

#include "app_config.h"
#include <tpie/portability.h>
#include <tpie/priority_queue.h>
#include <iostream>
#include <tpie/progress_indicator_arrow.h>
#include <queue>
#include <cstdlib>

using namespace std;

using namespace tpie;

const double PI = acos(-1.0);

void pq_large_instance(bool crash_test){
  MM_manager.set_memory_limit(50*1024*1024);
  tpie_log_init(LOG_DEBUG);
  stream_offset_type cnt=0;
  double mem_frac = crash_test ? 1.0 : 0.1;
  ami::priority_queue<long long, std::greater<long long> > pq(mem_frac);
  std::priority_queue<long long, vector<long long>,std::less<long long> > pq2;
  double cycle = crash_test ? 20000000000.0 : 50000000.0;
  const stream_offset_type iterations=500000000;
  progress_indicator_arrow progress("Running test","Running test:",0,iterations,1);
  for(stream_offset_type j=0; j<iterations; j++){
	progress.step();
    double i = static_cast<double>(j);
    double th = (cos(i*2.0*PI/cycle)+1.0)*(RAND_MAX/2);
    if(!crash_test && !pq.empty()){
      if(pq.top()!=pq2.top()){
	std::cerr << "Priority queues differ, got " << pq.top() << " but expected " 
	     << pq2.top() << "\n";
	assert(0);
      }
    }else{
      if(!crash_test)
	assert(cnt==0);
    }
    if(rand()<th){
      //std::cout << "Insert\n";
      cnt++;
      long long r = rand();
      pq.push(r);
      if(!crash_test)
	pq2.push(r);
    }else{
      //std::cout << "Delete\n";
      if(pq.empty())
	continue;
      cnt--;
      pq.pop();
      if(!crash_test)
	pq2.pop();
    }
    assert(pq.size()==cnt);
    if(j%5000000==0) {
      std::cout << "\nElements in pq: " << cnt << " (" << cnt/1024/1024 << " mebielements).\n";
      std::cout << "\nMemory available: " << MM_manager.memory_available() << " (" << MM_manager.memory_available()/1024/1024 << " mebibytes).\n";
	}
  }
  progress.done("Done");
}

void pq_internal_instance(){
  int size=100000;
  ami::pq_internal_heap<int, std::greater<int> > pq(size);
  std::priority_queue<int, vector<int>,std::less<int> > pq2;
  for(int i=0;i<size;i++){
    int r = rand();
    pq.insert(r);
    pq2.push(r);
  }
  while(!pq.empty()){
    if(pq.peekmin()!=pq2.top()){
      std::cerr << "Internal memory heap failed.\n";
      assert(0);
    }
    pq.delmin();
    pq2.pop();
  }
}

void pq_small_instance(){
  //MM_manager.set_memory_limit(10*1024*1024);
  //std::cout << "LOGGING: " << logstream::log_initialized << "\n";
  //tpie_log_init(TPIE_LOG_WARNING);
  //std::cout << "LOGGING: " << logstream::log_initialized << "\n";


  std::cout << "tpie::priority_queue Debug - M test" << std::endl;
    stream_offset_type iterations = 10000;
    MM_manager.set_memory_limit(16*1024*1024);
  progress_indicator_arrow progress("Running test","Running test:",1100,iterations,1);
    for(stream_offset_type it = 1100; it < iterations; it++)  {
	  progress.step();
      ami::priority_queue<int, std::greater<int> > pq(0.75);
      std::priority_queue<int, vector<int>,std::less<int> > pq2;

      stream_offset_type elements = 71;
      seed_random(static_cast<unsigned int>(it));
      for(stream_offset_type i=0;i<elements;i++) {
		  int src_int = tpie::random()%220;
        pq.push(src_int);
        pq2.push(src_int);
//        std::cout << "push " << src_int << std::endl;
      }
//std::cout << "all push done" << std::endl;
          pq.pop();
          pq2.pop();
          pq.pop();
          pq2.pop();
          pq.pop();
          pq2.pop();
//          pq.pop();
//          pq2.pop();
//pq2.dump("internal.dot");
      stream_offset_type pop = 61; 
      for(stream_offset_type i=0;i<pop;i++) {
        if(!pq.empty()) {
//          std::cout << "pop " << pq.top() << " " << pq2.top() << std::endl;
          if(pq.top() != pq2.top()) {
            std::cout << "main, run21 error1, " << i << " got: " << pq.top() << " expected " << pq2.top() << std::endl;
            //pq.dump();
            exit(-1);
          }
//          pq.dump();
          pq.pop();
          pq2.pop();
        }
      }
      for(stream_offset_type i=0;i<elements;i++) {
		  int src_int = tpie::random()%220;
        pq.push(src_int);
        pq2.push(src_int);
//        std::cout << "push " << src_int << std::endl;
      }
//      std::cout << "Pop remaining" << std::endl;
//pq.dump();
      while(!pq.empty()) {
        if(pq.top() != pq2.top()) {
          std::cout << "main, run21 error2, got: " << pq.top() << " expected " << pq2.top() << std::endl;
          //pq.dump();
          exit(-1);
        }
	//        std::cout << "pop " << pq.top() << std::endl;
        pq.pop();
        pq2.pop();
      }
    }
	progress.done("Done");
  }


int main(int argc,char** argv){
  if(argc<2){
    std::cout << "Arguments are test_priority_queue <type>\nWhere type is either \"small\" , \"large\" or \"internal\"\n";
  }
  if(strcmp(argv[1],"small")==0)
    pq_small_instance();
  else if(strcmp(argv[1],"large")==0)
    pq_large_instance(strcmp(argv[2],"crash")==0);
  else if(strcmp(argv[1],"internal")==0)
    pq_internal_instance();
}
