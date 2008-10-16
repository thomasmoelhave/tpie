#ifndef _PQSEQUENCE3_H_
#define _PQSEQUENCE3_H_

#include <ami.h>
#include "OPQHeap.h"

using namespace std;

/////////////////////////////////////////////////////////
///
///  \class PQSequence3
///  \author Lars Hvam Petersen
///
///  Inspiration: Sanders - Fast priority queues for cached memory
///
/////////////////////////////////////////////////////////
template<typename T, typename Comparator = std::less<T>, typename OPQType = OPQHeap<T, Comparator> >
class PQSequence3 {
 public:
  /////////////////////////////////////////////////////////
  ///
  /// Constructor
  ///
  /// \param f Factor of memory that the priority queue is 
  /// allowed to use.
  ///
  /////////////////////////////////////////////////////////
  PQSequence3(double f=1.0);

  /////////////////////////////////////////////////////////
  ///
  /// Destructor
  ///
  /////////////////////////////////////////////////////////
  ~PQSequence3();

  /////////////////////////////////////////////////////////
  ///
  /// Insert an element into the priority queue
  ///
  /// \param x The item
  ///
  /////////////////////////////////////////////////////////
  void push(const T& x);

  /////////////////////////////////////////////////////////
  ///
  /// Remove the top element from the priority queue
  ///
  /////////////////////////////////////////////////////////
  void pop();

  /////////////////////////////////////////////////////////
  ///
  /// See whats on the top of the priority queue
  ///
  /// \return Top element
  ///
  /////////////////////////////////////////////////////////
  const T& top();

  /////////////////////////////////////////////////////////
  ///
  /// Returns the size of the queue
  ///
  /// \return Queue size
  ///
  /////////////////////////////////////////////////////////
  const TPIE_OS_OFFSET size();

  /////////////////////////////////////////////////////////
  ///
  /// Return true if queue is empty otherwise false
  ///
  /// \return Boolean - empty or not
  ///
  /////////////////////////////////////////////////////////
  const bool empty();

  /////////////////////////////////////////////////////////
  ///
  /// fixme 
  ///
  /// \param f fixme
  ///
  /// \return fixme
  ///
  /////////////////////////////////////////////////////////
  template <typename F> F pop_equals(F f);

  /////////////////////////////////////////////////////////
  ///
  /// Dumps the current contents of the queue to stdout 
  ///
  /////////////////////////////////////////////////////////
  void dump();

 private:
  Comparator comp_;
  T dummy;

  T min;
  bool min_in_buffer;

  OPQType* opq;
  T* buffer; // deletion buffer
  T* gbuffer0; // group buffer 0
  T* mergebuffer; // merge buffer
  TPIE_OS_OFFSET* slot_state;
  TPIE_OS_OFFSET* group_state;
  
  TPIE_OS_OFFSET setting_k;
  TPIE_OS_OFFSET current_r;
  TPIE_OS_OFFSET setting_m;
  TPIE_OS_OFFSET setting_mmark;

  TPIE_OS_OFFSET slot_data_id;

  TPIE_OS_OFFSET m_size;
  TPIE_OS_OFFSET buffer_size;
  TPIE_OS_OFFSET buffer_start;

//////////////////
// TPIE wrappers
  AMI_err err;

  const inline void seek_offset(AMI_STREAM<T>* data, TPIE_OS_OFFSET offset) {
    if((err = data->seek(offset))!= AMI_ERROR_NO_ERROR) {
      cout << "AMI_ERROR " << err << " while seeking node" << endl;
      exit(-1);
    }
  }

  inline T* read_item(AMI_STREAM<T>* data) { 
    T* read_ptr;
    if((err = data->read_item(&read_ptr)) != AMI_ERROR_NO_ERROR) {
      cout << "AMI error while reading item, code: " << err << endl;
      exit(-1);
    }
    return read_ptr;
  }
  
  inline void write_item(AMI_STREAM<T>* data, T write) { 
    if((err = data->write_item(write)) != AMI_ERROR_NO_ERROR) {
      cout << "AMI error while reading item, code: " << err << endl;
      exit(-1);
    }
  }
// end TPIE wrappers
/////////////////////

  inline void slot_start_set(TPIE_OS_OFFSET slot, TPIE_OS_OFFSET n) {
    slot_state[slot*3] = n;
  }

  const inline TPIE_OS_OFFSET slot_start(TPIE_OS_OFFSET slot) {
    return slot_state[slot*3];
  }

  inline void slot_size_set(TPIE_OS_OFFSET slot, TPIE_OS_OFFSET n) {
//cout << "change slot " << slot << " size" << endl;
    assert(slot<setting_k*setting_k);
    slot_state[slot*3+1] = n;
  }

  const inline TPIE_OS_OFFSET slot_size(TPIE_OS_OFFSET slot) {
    return slot_state[slot*3+1];
  }

  inline void group_start_set(TPIE_OS_OFFSET group, TPIE_OS_OFFSET n) {
    group_state[group*2] = n;
  }

  const inline TPIE_OS_OFFSET group_start(TPIE_OS_OFFSET group) {
    return group_state[group*2];
  }

  inline void group_size_set(TPIE_OS_OFFSET group, TPIE_OS_OFFSET n) {
    assert(group<setting_k);
    group_state[group*2+1] = n;
  }
  
  const inline TPIE_OS_OFFSET group_size(TPIE_OS_OFFSET group) {
    return group_state[group*2+1];
  }

  char filename[BTE_STREAM_PATH_NAME_LEN];
  char datafiles[BTE_STREAM_PATH_NAME_LEN];

  const inline char* datafile(TPIE_OS_OFFSET id) {
    sprintf(filename, "%s%i", datafiles, (int)id); // todo, not int really
    return filename;
  }

  const inline char* datafile_group(TPIE_OS_OFFSET id) {
    sprintf(filename, "%sg%i", datafiles, (int)id); // todo, not int really
    return filename;
  }

  const inline char* slot_data(TPIE_OS_OFFSET slotid) {
    return datafile(slot_state[slotid*3+2]);
  }

  inline void slot_data_set(TPIE_OS_OFFSET slotid, TPIE_OS_OFFSET n) {
    slot_state[slotid*3+2] = n;
  }

  const inline char* group_data(TPIE_OS_OFFSET groupid) {
    return datafile_group(groupid);
  }

  inline TPIE_OS_OFFSET slot_max_size(TPIE_OS_OFFSET slotid) {
    return setting_m*(TPIE_OS_OFFSET)(pow((long double)setting_k,(long double)(slotid/setting_k))); // todo, too many casts
  }

  void write_slot(TPIE_OS_OFFSET slotid, T* arr, TPIE_OS_OFFSET len) {
    assert(len > 0);
//cout << "write slot " << slotid << " " << len << endl;
//cout << "write slot " << slot_data(slotid) << endl;
    AMI_STREAM<T>* data = new AMI_STREAM<T>(slot_data(slotid));
//cout << "write slot new done" << endl;
    if((err = data->write_array(arr, len)) != AMI_ERROR_NO_ERROR) {
      cout << "AMI_ERROR " << err << " during write_slot()" << endl;
      exit(1);
    }
    delete data;
    slot_start_set(slotid, 0);
    slot_size_set(slotid, len);
    if(current_r == 0 && slotid < setting_k) {
      current_r = 1;
    }
//cout << "write slot done" << endl;
  }

  TPIE_OS_OFFSET free_slot(TPIE_OS_OFFSET group);
  void empty_group(TPIE_OS_OFFSET group);
  void fill_buffer();
  void fill_group_buffer(TPIE_OS_OFFSET group);
  void compact(TPIE_OS_OFFSET slot);
  void validate();
  void remove_group_buffer(TPIE_OS_OFFSET group);

};

#ifndef CPPPQSEQUENCE3
#include "PQSequence3.cpp"
#endif

#endif
