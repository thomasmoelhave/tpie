#define CPPPQSEQUENCE3

#include <iostream>
#include <fstream>
#include "MergeHeap.h"

using namespace std;

/////////////////////////////
// Public
/////////////////////////////

template <typename T, typename Comparator, typename OPQType>
PQSequence3<T, Comparator, OPQType>::PQSequence3(double f) { // constructor 

  TPIE_OS_SIZE_T mm_avail = MM_manager.memory_limit();
  cout << "PQSequence3: Memory limit: " << mm_avail/1024/1024 << "mb("<< mm_avail << "bytes)"<< endl;
  mm_avail = (TPIE_OS_SIZE_T)((double)mm_avail*f);
  cout << "m_for_queue: " << mm_avail << endl;
  cout << "memory before alloc: " << MM_manager.memory_available() << "b" << endl;

  if(mm_avail<50000) {
    cout << "No calculation of settings!! Disabling memory manager" << endl;
    MM_manager.ignore_memory_limit();
    setting_k = 3; // maximal fanout
    setting_m = 4; // in elements
    setting_mmark = 2; // in elements
  } else {
    TPIE_OS_OFFSET max_k = 500;
    AMI_STREAM<T> tmp;
    TPIE_OS_SIZE_T usage;
    tmp.main_memory_usage(&usage, MM_STREAM_USAGE_MAXIMUM);

    mm_avail = mm_avail 
      - max_k*max_k*3*sizeof(TPIE_OS_OFFSET) // slot states
      - max_k*2*sizeof(TPIE_OS_OFFSET) // group states
      - 4*usage // aux streams
      - 1000;
    if(mm_avail < 0) {
      cout << "Priority Queue: Increase allowed memory" << endl;
      exit(-1);
    }
    
  setting_m = mm_avail/sizeof(T);

//  1/8m+ 4m = total
// m = total/(1/8+4)

    setting_m = setting_m/(4+(1/8)); // opq, gbuffer0, and mergebuffer(2*m)
    setting_mmark = (setting_m/8)/sizeof(T);
// one block from each stream plus one element from each: use merge buffer ram
    setting_k = std::min((unsigned int)max_k,(int)((setting_m*2/sizeof(T))-sizeof(T)*max_k)/((usage/sizeof(T))+sizeof(T)));
    cout << "max calc k:" << (int)((setting_m*2/sizeof(T))-sizeof(T)*max_k)/((usage/sizeof(T))+sizeof(T)) << endl;
    if(setting_k <= 2) {
      cout << "Priority Queue: Fanout too small" << endl;
      exit(-1);
    }
    cout << "stream usage(bytes): " << usage << endl;
  }

  current_r = 0;
  m_size = 0; // total size of priority queue
  buffer_size = 0;
  buffer_start = 0;

  cout << "PQSequence3" << endl 
    << "\tsetting_k: " << setting_k << endl 
    << "\tsetting_mmark: " << setting_mmark << endl 
    << "\tsetting_m: " << setting_m << endl;

  assert(setting_k > 0);
  assert(current_r == 0);
  assert(setting_m > 0);
  assert(setting_mmark > 0);
  assert(setting_m > setting_mmark);
  if(setting_m < setting_mmark) {
    cout << "wrong settings" << endl;
    exit(-1);
  }

  opq = new OPQType(setting_m);
  assert(OPQType::sorted_factor == 1);
  if(opq == NULL) throw std::bad_alloc();
// state arrays contain: start + size
  slot_state = new TPIE_OS_OFFSET[setting_k*setting_k*3];
  if(slot_state == NULL) throw std::bad_alloc();
  group_state = new TPIE_OS_OFFSET[setting_k*2]; // R merge can max be k splitout
  if(group_state == NULL) throw std::bad_alloc();
  buffer = new T[setting_mmark];
  if(buffer == NULL) throw std::bad_alloc();
  gbuffer0 = new T[setting_m];
  if(gbuffer0 == NULL) throw std::bad_alloc();
  mergebuffer = new T[setting_m*2];
  if(mergebuffer == NULL) throw std::bad_alloc();

// clear memory
  for(TPIE_OS_OFFSET i = 0; i<setting_k*setting_k; i++) {
    slot_state[i*3] = 0;
    slot_state[i*3+1] = 0;
    slot_state[i*3+2] = i;
  }
  slot_data_id = setting_k*setting_k+1;

  for(TPIE_OS_OFFSET i = 0; i< setting_k*2; i++) {
    group_state[i] = 0;
  }
  
  // calculate maximum size of queue
  if(setting_k < 5) { // the maxsize variable easily overflows...
    TPIE_OS_OFFSET maxsize = 0;
    for(TPIE_OS_OFFSET i = 0; i< setting_k; i++) {
      maxsize = maxsize + setting_k*slot_max_size(setting_k*i);
    }
  
    cout << "\tQueue max size(only external): " << maxsize << endl;
  }

  sprintf(datafiles, "%s", tpie_tempnam("AMI_PQ_DATA"));
cout << "memory after alloc: " << MM_manager.memory_available() << "b" << endl;
}

template <typename T, typename Comparator, typename OPQType>
PQSequence3<T, Comparator, OPQType>::~PQSequence3() { // destructor
  for(TPIE_OS_OFFSET i = 0; i < setting_k*setting_k; i++) { // unlink slots
    TPIE_OS_UNLINK(slot_data(i));
  }
  for(TPIE_OS_OFFSET i = 0; i < setting_k; i++) { // unlink groups 
    TPIE_OS_UNLINK(group_data(i));
  }

  delete opq;
  delete slot_state;
  delete group_state;
  delete buffer;
  delete gbuffer0;
  delete mergebuffer;
}

template <typename T, typename Comparator, typename OPQType>
void PQSequence3<T, Comparator, OPQType>::push(const T& x) {
//cout << "push start" << endl;
  if(opq->full()) {
//cout << "----calling free" << endl;
    TPIE_OS_OFFSET slot = free_slot(0);
//cout << "done calling free, free found: " << slot << endl;
    assert(opq->sorted_size() == setting_m);
    T* arr = opq->sorted_array();
    if(buffer_size > 0) { // maintain heap invariant for buffer
      memcpy(&mergebuffer[0], &arr[0], sizeof(T)*opq->sorted_size());
      memcpy(&mergebuffer[opq->sorted_size()], &buffer[buffer_start], sizeof(T)*buffer_size);
      std::sort(mergebuffer, mergebuffer+(buffer_size+opq->sorted_size()), comp_);
      memcpy(&buffer[buffer_start], &mergebuffer[0], sizeof(T)*buffer_size);
      memcpy(&arr[0], &mergebuffer[buffer_size], sizeof(T)*opq->sorted_size());
    }
    if(group_size(0)> 0) { // maintain heap invariant for gbuffer0
      assert(group_size(0)+opq->sorted_size() <= setting_m*2);
      TPIE_OS_OFFSET j = 0;
      for(TPIE_OS_OFFSET i = group_start(0); i < group_start(0)+group_size(0); i++) {
        mergebuffer[j] = gbuffer0[i%setting_m];
        ++j;
      }
      memcpy(&mergebuffer[j], &arr[0], sizeof(T)*opq->sorted_size());
      std::sort(mergebuffer, mergebuffer+(group_size(0)+opq->sorted_size()), comp_);
      memcpy(&gbuffer0[0], &mergebuffer[0], sizeof(T)*group_size(0));
      group_start_set(0,0);
      memcpy(&arr[0], &mergebuffer[group_size(0)], sizeof(T)*opq->sorted_size());
    }
    write_slot(slot, arr, opq->sorted_size());
    opq->sorted_pop();

  }
  opq->push(x);
  m_size++;
#ifndef NDEBUG
  validate();
#endif
}

template <typename T, typename Comparator, typename OPQType>
void PQSequence3<T, Comparator, OPQType>::pop() {
//cout << "pop" << endl;
  if(m_size == 0) {
    cout << "Error, queue is empty, pop" << endl;
    exit(-1);
  }
  top();
//cout << "return to pop from top" << endl;
  if(min_in_buffer) {
    buffer_size--;
    buffer_start++;
    if(buffer_size == 0) {
      buffer_start = 0;
    }
  } else {
    opq->pop();
  }
  m_size--;
#ifndef NDEBUG
  validate();
#endif
//cout << "end pop" << endl;
}

template <typename T, typename Comparator, typename OPQType>
const T& PQSequence3<T, Comparator, OPQType>::top() {
//cout << "top of top" << endl;
//dump();
  if(buffer_size == 0 && opq->size() != m_size) {
//cout << "fill min buffer" << endl;
    fill_buffer();
//cout << "done fill min buffer" << endl;
  }
//cout << "buffer filled" << endl;
  if(buffer_size == 0 && opq->size() == 0) {
//    dump();
    cout << "Error, queue is empty, top" << endl;
    exit(-1);
  } else if(opq->size() == 0) {
    min=buffer[buffer_start];
    min_in_buffer = true;
  } else if(buffer_size == 0) {
    min=opq->top();
    min_in_buffer = false;
  } else if(comp_(buffer[buffer_start], opq->top())) { // compare
    min=buffer[buffer_start];
    min_in_buffer = true;
  } else {
    min=opq->top();
    min_in_buffer = false;
  }
//cout << "min in buffer: " << min_in_buffer << endl;
//cout << "end of top" << endl;
#ifndef NDEBUG
  validate();
#endif
  return min;
}

template <typename T, typename Comparator, typename OPQType>
const TPIE_OS_OFFSET PQSequence3<T, Comparator, OPQType>::size() {
  return m_size;
}

template <typename T, typename Comparator, typename OPQType>
const bool PQSequence3<T, Comparator, OPQType>::empty() {
  return m_size == 0;
}

template <typename T, typename Comparator, typename OPQType> template <typename F>
F PQSequence3<T, Comparator, OPQType>::pop_equals(F f) {
  T a = top();
  f(a);
  pop();
  if(size() == 0) return f;
  T b = top();
  while(!(comp_(a, b))) { // compare
    f(b);
    pop();
    if(size() == 0) return f;
    b = top();
  }
  return f;
}

template <typename T, typename Comparator, typename OPQType>
void PQSequence3<T, Comparator, OPQType>::dump() {
  cout << "--------------------------------------------------------------" << endl
    << "DUMP:\tTotal size: " << m_size << ", OPQ size: " << opq->size() 
    << ", OPQ top: ";
  if(opq->size()>0) {
    cout << opq->top();
  } else {
    cout << "empty";
  }
  cout << ", current_r: " << current_r << endl
    << "\tBuffer size: " << buffer_size << ", buffer start: " << buffer_start 
    << endl << "\t";

  // output main buffer
  for(TPIE_OS_OFFSET i = 0; i<setting_mmark; i++) {
    cout << (i<buffer_start || buffer_start+buffer_size <=i ?"(":"") 
      << buffer[i] 
      << (i<buffer_start || buffer_start+buffer_size <=i ?")":"") 
      << " ";
  }
  cout << endl;

  // output groups
  for(TPIE_OS_OFFSET i =0; i<current_r; i++) {
    cout << "GROUP " << i << " ------------------------------------------------------" << endl;
    cout << "\tGroup Buffer, size: " << group_size(i) << ", start: " 
      << group_start(i) << endl << "\t\tBuffer(no ('s): ";
   
    if(i == 0) { // group buffer 0 is special
      cout << "internal: ";
      TPIE_OS_OFFSET k = 0;
	    for(k = 0; k < setting_m; k++) {
      	cout << gbuffer0[k] << " ";
     	} 
    	cout << endl;
    } else {  
      // output group buffer contents
      AMI_STREAM<T>* stream = new AMI_STREAM<T>(group_data(i));
      TPIE_OS_OFFSET k = 0;
      if(group_size(i) > 0) {
        for(k = 0; k < setting_m; k++) {
          cout << *read_item(stream) << " ";
        } 
      }
      delete stream;
      for(TPIE_OS_OFFSET l = k; l < setting_m; l++) {
          cout << "() ";
      }
      cout << endl;
    }

    // output slots
    for(TPIE_OS_OFFSET j = i*setting_k; j<i*setting_k+setting_k; j++) {
      cout << "\t\tSlot " << j << "(size: " << slot_size(j) 
        << " start: " << slot_start(j) << "):";

      AMI_STREAM<T>* stream = new AMI_STREAM<T>(slot_data(j));
      TPIE_OS_OFFSET k;
      for(k = 0; k < slot_start(j)+slot_size(j); k++) {
        cout << (k>=slot_start(j)?"":"(") << *read_item(stream) 
          << (k>=slot_start(j)?"":")") << " ";
      }
      delete stream;
      for(TPIE_OS_OFFSET l = k; l < slot_max_size(j); l++) {
        cout << "() ";
      }

      cout << endl;
    } 
  }
  cout << "--------------------------------------------------------------" << endl;
}

/////////////////////////////
// Private
/////////////////////////////

template <typename T, typename Comparator, typename OPQType>
TPIE_OS_OFFSET PQSequence3<T, Comparator, OPQType>::free_slot(TPIE_OS_OFFSET group) {
//cout << "free slot group " << group << "?" << endl;
  TPIE_OS_OFFSET i;
  if(group>=setting_k) {
    cout << "Error, queue is full no free slots in invalid group " << group << ". Increase k." << endl;
    exit(-1);
  }
  for(i = group*setting_k; i < group*setting_k+setting_k; i++) {
    if(slot_size(i) == 0) {
//cout << "Slot " << i << " is good" << endl;
      break;
    }
  }

  // it all goes to next level, wee
  if(i == group*setting_k+setting_k) {
//cout << "empty group from free slot" << endl;
    empty_group(group);
    if(slot_size(group*setting_k) != 0) {
//cout << "again" << endl; 
      return free_slot(group); // some group buffers might have been moved
    }
    return group*setting_k;
  }
  return i;
}

template <typename T, typename Comparator, typename OPQType>
void PQSequence3<T, Comparator, OPQType>::fill_buffer() {
//cout << "fill buffer" << endl;
//dump();
  if(buffer_size !=0) {
    return;
  }
  if(current_r == 0) { // todo: check that this is ok
    return;
  }

  // refill group buffers, if needed
//cout << "refill group buffers" << endl;
  for(TPIE_OS_OFFSET i=0;i<current_r;i++) {
    if(group_size(i)<setting_mmark) {
//cout << "fill group buffer " << i << endl;
      fill_group_buffer(i);
//cout << "fill group buffer " << i << " done" << endl;
//dump();
    }
    if(group_size(i) == 0 && i==current_r-1) {
      current_r--;
    }
  }

//cout << "done filling groups" << endl;
  // merge to buffer
//cout << "current_r: " << current_r << endl;
  delete mergebuffer;

  MergeHeap<T, Comparator> heap(current_r);
  AMI_STREAM<T>* data[current_r];
  for(TPIE_OS_OFFSET i = 0; i<current_r; i++) {
    if(i == 0 && group_size(i)>0) {
      heap.push(gbuffer0[group_start(0)], 0);
    } else if(group_size(i)>0) {
      data[i] = new AMI_STREAM<T>(group_data(i));
//      assert(slot_size(group*setting_k+i>0));
      seek_offset(data[i], group_start(i));
      heap.push(*read_item(data[i]), i);
    } else if(i > 0) {
// dummy, well :o/
//cout << "create dummy " << i << endl;
      data[i] = new AMI_STREAM<T>();
      data[i]->persist(PERSIST_DELETE);
    }
  }
//cout << "init done" << endl;

  while(!heap.empty() && buffer_size!=setting_mmark) {
    TPIE_OS_OFFSET current_group = heap.top_run();
    if(current_group!= 0 && data[current_group]->tell() == setting_m) {
//cout << "fill group seeking to 0" << endl;
      seek_offset(data[current_group], 0);
    }
    buffer[(buffer_size+buffer_start)%setting_m] = heap.top();
    buffer_size++;
    
    assert(group_size(current_group)-1 >= 0);
    group_size_set(current_group, group_size(current_group)-1);
    group_start_set(current_group, (group_start(current_group)+1)%setting_m);
    if(group_size(current_group) == 0) {
      heap.pop();
    } else {
      if(current_group == 0) {
//cout << "0 push: " << endl;
//cout << gbuffer0[group_start(0)] << endl;
        heap.pop_and_push(gbuffer0[group_start(0)], 0);
      } else {
        heap.pop_and_push(*read_item(data[current_group]), current_group);
      }
    }
  }
//cout << "while done" << endl;
  mergebuffer = new T[setting_m*2];
  if(mergebuffer == NULL) throw std::bad_alloc();

  for(TPIE_OS_OFFSET i = 1; i<current_r; i++) {
    delete data[i];
  }
//cout << "end fill buffer" << endl;
}

template <typename T, typename Comparator, typename OPQType>
void PQSequence3<T, Comparator, OPQType>::fill_group_buffer(TPIE_OS_OFFSET group) {
  assert(group_size(group) < setting_mmark);
  // max k + 1 open streams
  // 1 merge heap
  // opq still in action

  // merge
{
  AMI_STREAM<T> out(group_data(group));
//cout << "seek to " << (group_start(group)+group_size(group))%setting_m << endl;
  if(group > 0) {
    if((err = out.seek((group_start(group)+group_size(group))%setting_m))!= AMI_ERROR_NO_ERROR) {
      cout << "AMI_ERROR " << err << " while seeking node" << endl;
      exit(-1);
    }
  }
//  seek_offset(out, (group_start(group)+group_size(group))%setting_m);

  delete mergebuffer;

  MergeHeap<T, Comparator> heap(setting_k);
  AMI_STREAM<T>* data[setting_k];
  for(TPIE_OS_OFFSET i = 0; i<setting_k; i++) {
    if(slot_size(group*setting_k+i)>0) {
      data[i] = new AMI_STREAM<T>(slot_data(group*setting_k+i));
//      assert(slot_size(group*setting_k+i>0));
      seek_offset(data[i], slot_start(group*setting_k+i));
      heap.push(*read_item(data[i]), group*setting_k+i);
    } else {
// dummy, well :o/
      data[i] = new AMI_STREAM<T>();
      data[i]->persist(PERSIST_DELETE);
    }
  }

  while(!heap.empty() && group_size(group)!=setting_m) {
    TPIE_OS_OFFSET current_slot = heap.top_run();
if(group == 0) {
  gbuffer0[(group_start(0)+group_size(0))%setting_m] = heap.top();
} else {
    if(out.tell() == setting_m) {
//cout << "fill group seeking to 0" << endl;
//      seek_offset(out, 0);
      out.seek(0);
    }
//    write_item(out, heap.top());
    if((err = out.write_item(heap.top())) != AMI_ERROR_NO_ERROR) {
      cout << "AMI error while reading item, code: " << err << endl;
      exit(-1);
    }
}
    group_size_set(group, group_size(group) + 1);
    slot_start_set(current_slot, slot_start(current_slot)+1);
    slot_size_set(current_slot, slot_size(current_slot)-1);
    if(slot_size(current_slot) == 0) {
      heap.pop();
    } else {
      heap.pop_and_push(*read_item(data[current_slot-group*setting_k]), current_slot);
    }
  }

  for(TPIE_OS_OFFSET i = 0; i<setting_k; i++) {
    delete data[i];
  }
//  delete out;
}

  mergebuffer = new T[setting_m*2];
  if(mergebuffer == NULL) throw std::bad_alloc();

  // compact if needed
/*  for(TPIE_OS_OFFSET i=group*setting_k;i<group*setting_k+setting_k; i++) {
    if(slot_size(i) <= slot_max_size(i)/2 && slot_size(i) >  0) {
//cout << "compact from fill group buffer slot: " << i << endl;
      compact(i);
    }
  }
*/
}


template <typename T, typename Comparator, typename OPQType>
void PQSequence3<T, Comparator, OPQType>::compact(TPIE_OS_OFFSET slot1) {
//  cout << "compact slot " << slot1 << endl;
  assert(slot_size(slot1) > 0);

  for(TPIE_OS_OFFSET i = (slot1/setting_k)*setting_k; i < (slot1/setting_k)*setting_k + setting_k; i++) {
    if(i != slot1) {
      if(slot_size(i) > 0 && slot_size(i) + slot_size(slot1) <= slot_max_size(slot1)) {
        TPIE_OS_OFFSET slot2 = i;
//cout << "compacting slot " << slot1 << " with " << slot2 << endl;

        AMI_STREAM<T>* stream1 = new AMI_STREAM<T>(slot_data(slot1));
        seek_offset(stream1, slot_start(slot1));
        T e1 = *read_item(stream1);
        TPIE_OS_OFFSET used1 = 0;
        AMI_STREAM<T>* stream2 = new AMI_STREAM<T>(slot_data(slot2));
        seek_offset(stream2, slot_start(slot2));
        T e2 = *read_item(stream2);
        TPIE_OS_OFFSET used2 = 0;

        TPIE_OS_OFFSET new_data_id = slot_data_id++;
        AMI_STREAM<T>* out = new AMI_STREAM<T>(datafile(new_data_id));

        while(used1 + used2 < slot_size(slot1) + slot_size(slot2)) {
          if(used1 == slot_size(slot1)) { // rest from slot2
            write_item(out, e2);
            used2++;
            if(used2 < slot_size(slot2)) e2 = *read_item(stream2);
          } else if(used2 == slot_size(slot2)) { // rest from slot1
            write_item(out, e1);
            used1++;
            if(used1 < slot_size(slot1)) e1 = *read_item(stream1);
          } else if(comp_(e1, e2)) { // compare - 10/1-07
            write_item(out, e1);
            used1++;
            if(used1 < slot_size(slot1)) e1 = *read_item(stream1);
          } else {
            write_item(out, e2);
            used2++;
            if(used2 < slot_size(slot2)) e2 = *read_item(stream2);
          }
        }
        
        delete stream1;
        delete stream2;

        slot_start_set(slot1, 0);
        TPIE_OS_UNLINK(slot_data(slot1));
        slot_data_set(slot1, new_data_id);
        slot_size_set(slot1, slot_size(slot1) + slot_size(slot2));
        slot_size_set(slot2, 0);
        slot_start_set(slot2, 0);

        delete out;
        return;
      }
    }
  }
}

template <typename T, typename Comparator, typename OPQType>
void PQSequence3<T, Comparator, OPQType>::empty_group(TPIE_OS_OFFSET group) {
//cout << "Empty group " << group << endl;
  if(group > setting_k) {
    cout << "Error: Priority queue is full" << endl;
    exit(-1);
  }

  TPIE_OS_OFFSET newslot = free_slot(group+1);
 
  assert(slot_size(newslot) == 0);
  slot_start_set(newslot, 0);
  if(current_r < newslot/setting_k+1) {
//cout << "increasing current_r, empty_group" << endl; 
    current_r = newslot/setting_k+1;
  }

  bool ret = false;

  delete mergebuffer;

  AMI_STREAM<T>* newstream = new AMI_STREAM<T>(slot_data(newslot));
  MergeHeap<T, Comparator> heap(setting_k);

  AMI_STREAM<T>* data[setting_k];
  for(TPIE_OS_OFFSET i = 0; i<setting_k; i++) {
    data[i] = new AMI_STREAM<T>(slot_data(group*setting_k+i));
    if(slot_size(group*setting_k+i) == 0) {
//      cout << "no need to emtpy group "<<group<<", slot: " << group*setting_k+i << " is empty" << endl;
      ret = true;
      break;
    }
    assert(slot_size(group*setting_k+i)>0);
    seek_offset(data[i], slot_start(group*setting_k+i));
    heap.push(*read_item(data[i]), group*setting_k+i);
  }
//cout << "init done" << endl;

  while(!heap.empty() && !ret) {
    TPIE_OS_OFFSET current_slot = heap.top_run();
    write_item(newstream, heap.top());
    slot_size_set(newslot,slot_size(newslot)+1);
//cout << heap.top() << " from slot " << current_slot << endl;
    slot_start_set(current_slot, slot_start(current_slot)+1);
    slot_size_set(current_slot, slot_size(current_slot)-1);
    if(slot_size(current_slot) == 0) {
      heap.pop();
    } else {
      heap.pop_and_push(*read_item(data[current_slot-group*setting_k]), current_slot);
    }
  }

//cout << "start delete" << endl;
  for(TPIE_OS_OFFSET i = 0; i<setting_k; i++) {
    if(ret == false) {
      delete data[i];
    } else if(ret == true && slot_size(group*setting_k+i) >0 ) {
      delete data[i];
    }
  }
//cout << "end delete" << endl;

  delete newstream;

  mergebuffer = new T[setting_m*2];
  if(mergebuffer == NULL) throw std::bad_alloc();

  if(group_size(group+1) > 0 && !ret) {
    remove_group_buffer(group+1); // todo, this might recurse?
  }
}

template <typename T, typename Comparator, typename OPQType>
void PQSequence3<T, Comparator, OPQType>::validate() {
#ifndef NDEBUG
//cout << "validate start" << endl;
  // validate size
  TPIE_OS_OFFSET size = 0;
  size = size + opq->size();
  size = size + buffer_size;
  for(TPIE_OS_OFFSET i = 0; i<setting_k;i++) {
    size = size + group_size(i);
  }
  for(TPIE_OS_OFFSET i = 0; i<setting_k*setting_k;i++) {
    size = size + slot_size(i);
  }
  if(m_size != size) {
    cout << "Error: Validate: Size not ok" << endl;
    exit(-1);
  }

  // validate internal order in "nodes"
  if(buffer_size > 0) { // buffer
    T last = buffer[buffer_start];
    for(TPIE_OS_OFFSET i = buffer_start; i<buffer_start+buffer_size;i++) {
      if(comp_(buffer[i],last)) {
        cout << "Error: Buffer ordered validation failed" << endl;
      }
      last = buffer[i];
    }
  }
// todo: validate gbuffer0
  for(TPIE_OS_OFFSET i = 1; i < setting_k; i++) { // groups, nb: cyclic
    if(group_size(i) > 0) {
      AMI_STREAM<T>* stream = new AMI_STREAM<T>(group_data(i));
      seek_offset(stream, group_start(i));
      if(stream->tell() == setting_m) {
        seek_offset(stream, 0);
      }
      T last = *read_item(stream);
      for(TPIE_OS_OFFSET j = 1; j < group_size(i); j++) {
        if(stream->tell() == setting_m) {
          seek_offset(stream, 0);
        }
        T read = *read_item(stream);
        if(comp_(read, last)) { // compare
dump();
          cout << "Error: Group buffer " << i << " order invalid (last: " 
            << last << ", read: " << read << ")" << endl;
          exit(-1);
        } 
      }
      delete stream;
    }
  }
  for(TPIE_OS_OFFSET i = 0; i < setting_k*setting_k; i++) { // slots
    if(slot_size(i) > 0){
      AMI_STREAM<T>* stream = new AMI_STREAM<T>(slot_data(i));
      seek_offset(stream, slot_start(i));
      T last = *read_item(stream);
      for(TPIE_OS_OFFSET j = 1; j < slot_size(i); j++) {
        T read = *read_item(stream);
        if(comp_(read, last)) { // compare
          cout << "Error: Slot " << i << " order invalid (last: "
            << last << ", read: " << read << ")" << endl;
          exit(-1);
        }
      }
      delete stream;
    }
  }

  // validate heap properties
  if(buffer_size > 0) { // buffer --> group buffers
    T buf_max = buffer[buffer_start+buffer_size-1];
    for(TPIE_OS_OFFSET i = 1; i < setting_k; i++) { // todo: gbuffer0
      if(group_size(i) > 0) {
        AMI_STREAM<T>* stream = new AMI_STREAM<T>(group_data(i));
        seek_offset(stream, group_start(i));
        if(stream->tell() == setting_m) {
          seek_offset(stream, 0);
        }
        T first = *read_item(stream);
        if(comp_(first, buf_max)) { // compare
dump();
          cout << "Error: Heap property invalid, buffer -> group buffer " << i 
            << "(buffer: " << buf_max << ", first: " << first << ")"<< endl;
          exit(-1);
        }
        delete stream;
      }
    }
  }
  
  // todo: gbuffer0
  for(TPIE_OS_OFFSET i = 1; i < setting_k; i++) { // group buffers --> slots
    if(group_size(i) > 0) {
      AMI_STREAM<T>* stream = new AMI_STREAM<T>(group_data(i));
      seek_offset(stream, (group_start(i)+group_size(i)-1)%setting_m);
      T item_group = *read_item(stream);
//cout << "item_group: " << item_group << endl;
      delete stream;

      for(TPIE_OS_OFFSET j = i*setting_k; j<i*setting_k+setting_k;j++) {
        if(slot_size(j) > 0) {
          AMI_STREAM<T>* stream = new AMI_STREAM<T>(slot_data(j));
          seek_offset(stream, slot_start(j));
          T item_slot = *read_item(stream);
          delete stream;

          if(comp_(item_slot, item_group)) { // compare
dump();
            cout << "Error: Heap property invalid, group buffer " << i 
              << " -> slot " << j << "(group: " << item_group 
              << ", slot: " << item_slot << ")" << endl;
            exit(-1);
          }
        }
      }
    }
  }
//cout << "validate end" << endl;
#endif
}

template <typename T, typename Comparator, typename OPQType>
void PQSequence3<T, Comparator, OPQType>::remove_group_buffer(TPIE_OS_OFFSET group) {
//cout << "remove group buffer " << group << endl;
#ifndef NDEBUG
  if(group == 0) {
    cout << "remove group buffer 0, fy!" << endl;
    exit(-1);
  }
#endif

  // this is the easiest thing to do
  TPIE_OS_OFFSET slot = free_slot(0);
  if(group_size(group) == 0) return;

  assert(group < setting_k);
  T* arr = new T[group_size(group)];
  AMI_STREAM<T>* data = new AMI_STREAM<T>(group_data(group));
  seek_offset(data, group_start(group));
  TPIE_OS_OFFSET size = group_size(group);
  if(group_start(group) + group_size(group) <= setting_m) {
    if((err = data->read_array(arr, &size)) != AMI_ERROR_NO_ERROR) {
      cout << "AMI_ERROR " << err << " during read_array()" << endl;
      exit(1);
    }
  } else {
// two reads
    TPIE_OS_OFFSET first_read = setting_m - data->tell();
    TPIE_OS_OFFSET second_read = size - first_read;
//cout << "read array " << first_read << " " << second_read << endl;
    if((err = data->read_array(arr, &first_read)) != AMI_ERROR_NO_ERROR) {
      cout << "AMI_ERROR " << err << " during read_array()" << endl;
      exit(1);
    }
    seek_offset(data,0);
    if((err = data->read_array(arr+first_read, &second_read)) != AMI_ERROR_NO_ERROR) {
      cout << "AMI_ERROR " << err << " during read_array()" << endl;
      exit(1);
    }
  }
  delete data;
  assert(group_size(group) > 0);
/* 
for(TPIE_OS_OFFSET i = 0; i < group_size(group); i++) {
  cout << "arr[" << i << "]: " << arr[i] << endl;
}
for(TPIE_OS_OFFSET i = group_start(0); i < group_start(0)+group_size(0); i++) {
  cout << "gbuffer0[" << (i%setting_m) << "]: " << gbuffer0[i%setting_m] << endl;
}
*/
  // make sure that the new slot in group 0 is heap ordered with gbuffer0
  if(group > 0 && group_size(0) != 0) {
// this code is also used in PQFishspear
      TPIE_OS_OFFSET j = 0;
      for(TPIE_OS_OFFSET i = group_start(0); i < group_start(0)+group_size(0); i++) {
        mergebuffer[j] = gbuffer0[i%setting_m];
        ++j;
      }
      memcpy(&mergebuffer[j], &arr[0], sizeof(T)*group_size(group));
      std::sort(mergebuffer, mergebuffer+(group_size(0)+group_size(group)), comp_);
      memcpy(&gbuffer0[0], &mergebuffer[0], sizeof(T)*group_size(0));
      group_start_set(0,0);
      memcpy(&arr[0], &mergebuffer[group_size(0)], sizeof(T)*group_size(group));
  }

/*
for(TPIE_OS_OFFSET i = 0; i < group_size(group); i++) {
  cout << "arr[" << i << "]: " << arr[i] << endl;
}
for(TPIE_OS_OFFSET i = group_start(0); i < group_start(0)+group_size(0); i++) {
  cout << "gbuffer0[" << (i%setting_m) << "]: " << gbuffer0[i%setting_m] << endl;
}
*/

  write_slot(slot, arr, group_size(group));
  group_start_set(group, 0);
  group_size_set(group, 0);
//cout << "compact from remove_group_buffer" << endl;
//  compact(slot);
  delete arr;
//cout << "this dump" << endl;
//dump();
//  cout << "remove grp buffer done" << endl;
}
