#ifndef _TPIE_PRIORITY_QUEUE_H_
#define _TPIE_PRIORITY_QUEUE_H_

#include <tpie/config.h>
#include "portability.h"
#include "ami.h"
#include "tpie_log.h"
#include <cassert>
#include "pq_overflow_heap.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "pq_merge_heap.h"

namespace tpie {

    namespace ami {
	
/////////////////////////////////////////////////////////
///
///  \class priority_queue
///  \author Lars Hvam Petersen
///
///  Inspiration: Sanders - Fast priority queues for cached memory
///
/////////////////////////////////////////////////////////
template<typename T, typename Comparator = std::less<T>, typename OPQType = pq_overflow_heap<T, Comparator> >
class priority_queue {
public:
    /////////////////////////////////////////////////////////
    ///
    /// Constructor
    ///
    /// \param f Factor of memory that the priority queue is 
    /// allowed to use.
    ///
    /////////////////////////////////////////////////////////
    priority_queue(double f=1.0);

    /////////////////////////////////////////////////////////
    ///
    /// Destructor
    ///
    /////////////////////////////////////////////////////////
    ~priority_queue();

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
    /// See what's on the top of the priority queue
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
    /// Pop all elements with priority equal to that of the
    /// top element, and process each by invoking f's call
    /// operator on the element.
    ///
    /// \param f - assumed to have a call operator with parameter of type T.
    ///
    /// \return The argument f
    ///
    /////////////////////////////////////////////////////////
    template <typename F> F pop_equals(F f);

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
    ami::err err;

    const  void seek_offset(stream<T>* data, TPIE_OS_OFFSET offset);

    T* read_item(stream<T>* data); 

    void write_item(stream<T>* data, T write); 
    // end TPIE wrappers
    /////////////////////

    void slot_start_set(TPIE_OS_OFFSET slot, TPIE_OS_OFFSET n); 
    const TPIE_OS_OFFSET slot_start(TPIE_OS_OFFSET slot); 
    void slot_size_set(TPIE_OS_OFFSET slot, TPIE_OS_OFFSET n); 
    const TPIE_OS_OFFSET slot_size(TPIE_OS_OFFSET slot); 
    void group_start_set(TPIE_OS_OFFSET group, TPIE_OS_OFFSET n); 
    const TPIE_OS_OFFSET group_start(TPIE_OS_OFFSET group); 
    void group_size_set(TPIE_OS_OFFSET group, TPIE_OS_OFFSET n); 
    const TPIE_OS_OFFSET group_size(TPIE_OS_OFFSET group); 
    std::string filename;
    std::string datafiles;
    const char* datafile(TPIE_OS_OFFSET id); 
    const char* datafile_group(TPIE_OS_OFFSET id); 
    const char* slot_data(TPIE_OS_OFFSET slotid); 
    void slot_data_set(TPIE_OS_OFFSET slotid, TPIE_OS_OFFSET n); 
    const char* group_data(TPIE_OS_OFFSET groupid); 
    TPIE_OS_OFFSET slot_max_size(TPIE_OS_OFFSET slotid); 
    void write_slot(TPIE_OS_OFFSET slotid, T* arr, TPIE_OS_OFFSET len); 
    TPIE_OS_OFFSET free_slot(TPIE_OS_OFFSET group);
    void empty_group(TPIE_OS_OFFSET group);
    void fill_buffer();
    void fill_group_buffer(TPIE_OS_OFFSET group);
    void compact(TPIE_OS_OFFSET slot);
    void validate();
    void remove_group_buffer(TPIE_OS_OFFSET group);
    void dump();
};

#include "priority_queue.inl"

    }  //  ami namespace

}  //  tpie namespace

#endif
