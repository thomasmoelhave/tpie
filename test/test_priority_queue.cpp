//#define TP_LOG_APPS

#include "app_config.h"
#include <portability.h>
#include <priority_queue.h>
#include <iostream>
#include <queue>
#include <cstdlib>

using namespace std;

const double PI = acos(-1.0);

void pq_large_instance(){
  MM_manager.set_memory_limit(10*1024*1024);
  int cnt=0;
  tpie::priority_queue<int, std::greater<int> > pq;
  std::priority_queue<int, vector<int>,std::less<int> > pq2;
  for(int i=0;;i++){
    double th = (cos(i*2.0*PI/500000.0)+1.0)*(RAND_MAX/2);
    if(!pq.empty()){
      if(pq.top()!=pq2.top()){
	cerr << "Priority queues differ, got " << pq.top() << " but expected " 
	     << pq2.top() << "\n";
	assert(0);
      }
    }else{
      assert(cnt==0);
    }
    if(rand()<th){
      //cout << "Insert\n";
      cnt++;
      int r = rand();
      pq.push(r);
      pq2.push(r);
    }else{
      //cout << "Delete\n";
      if(pq.empty())
	continue;
      cnt--;
      pq.pop();
      pq2.pop();
    }
    cout << "Size: " << cnt << "\r";
  }
}

void pq_internal_instance(){
  int size=100000;
  tpie::pq_internal_heap<int, std::greater<int> > pq(size);
  std::priority_queue<int, vector<int>,std::less<int> > pq2;
  for(int i=0;i<size;i++){
    int r = rand();
    pq.insert(r);
    pq2.push(r);
  }
  while(!pq.empty()){
    if(pq.peekmin()!=pq2.top()){
      cerr << "Internal memory heap failed.\n";
      assert(0);
    }
    pq.delmin();
    pq2.pop();
  }
}

void pq_small_instance(){
  MM_manager.set_memory_limit(10*1024*1024);
  //cout << "LOGGING: " << logstream::log_initialized << "\n";
  //tpie_log_init(TPIE_LOG_WARNING);
  //cout << "LOGGING: " << logstream::log_initialized << "\n";


  cout << "tpie::priority_queue Debug - M test" << endl;
    TPIE_OS_OFFSET iterations = 2500;
    //    MM_manager.set_memory_limit(600);
    for(TPIE_OS_OFFSET it = 1100; it < iterations; it++)  {
      cout << "Iteration: " << it << " avail: " << MM_manager.memory_available() << "\n";
      tpie::priority_queue<int, std::greater<int> > pq;
      std::priority_queue<int, vector<int>,std::less<int> > pq2;

      TPIE_OS_OFFSET elements = 71;
      TPIE_OS_SRANDOM(it);
      for(TPIE_OS_OFFSET i=0;i<elements;i++) {
        int src_int = TPIE_OS_RANDOM()%220;
        pq.push(src_int);
        pq2.push(src_int);
//        cout << "push " << src_int << endl;
      }
//cout << "all push done" << endl;
          pq.pop();
          pq2.pop();
          pq.pop();
          pq2.pop();
          pq.pop();
          pq2.pop();
//          pq.pop();
//          pq2.pop();
//pq2.dump("internal.dot");
      TPIE_OS_OFFSET pop = 61; 
      for(TPIE_OS_OFFSET i=0;i<pop;i++) {
        if(!pq.empty()) {
//          cout << "pop " << pq.top() << " " << pq2.top() << endl;
          if(pq.top() != pq2.top()) {
            cout << "main, run21 error1, " << i << " got: " << pq.top() << " expected " << pq2.top() << endl;
            //pq.dump();
            exit(-1);
          }
//          pq.dump();
          pq.pop();
          pq2.pop();
        }
      }
      for(TPIE_OS_OFFSET i=0;i<elements;i++) {
        int src_int = TPIE_OS_RANDOM()%220;
        pq.push(src_int);
        pq2.push(src_int);
//        cout << "push " << src_int << endl;
      }
//      cout << "Pop remaining" << endl;
//pq.dump();
      while(!pq.empty()) {
        if(pq.top() != pq2.top()) {
          cout << "main, run21 error2, got: " << pq.top() << " expected " << pq2.top() << endl;
          //pq.dump();
          exit(-1);
        }
	//        cout << "pop " << pq.top() << endl;
        pq.pop();
        pq2.pop();
      }
      cout << endl;
    }
  }


int main(int argc,char** argv){
  if(argc!=2){
    cout << "Arguments are test_priority_queue <type>\nWhere type is either \"small\" , \"large\" or \"internal\"\n";
  }
  if(strcmp(argv[1],"small")==0)
    pq_small_instance();
  else if(strcmp(argv[1],"large")==0)
    pq_large_instance();
  else if(strcmp(argv[1],"internal")==0)
    pq_internal_instance();
}
