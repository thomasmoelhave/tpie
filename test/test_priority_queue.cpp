#include "priority_queue.h"
#include <iostream>
#include <queue>

using namespace std;


void pq_small_instance(){
    cout << "tpie::priority_queue Debug - M test" << endl;
    TPIE_OS_OFFSET iterations = 1000000;
    MM_manager.set_memory_limit(600);
    for(TPIE_OS_OFFSET it = 1100; it < iterations; it++)  {
      cout << "Iteration: " << it << endl;
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


int main(){

  tpie::priority_queue<int> pq;
  pq.push(5);
  pq.push(2);
  pq.push(11);
  cout << pq.top() << "\n";
  pq.pop();
  cout << pq.top() << "\n";
  pq.pop();
  cout << pq.top() << "\n";
  pq.pop();  

  pq_small_instance();
}
