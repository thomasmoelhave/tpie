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
#include <stdexcept>
#include <cmath>
#include <string>
#include <sstream>
#include "pq_merge_heap.h"

namespace tpie {

    namespace ami {

	struct priority_queue_error : public std::logic_error {
		priority_queue_error(const std::string& what) : std::logic_error(what)
		{ }
	};
	
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
    /// Constructor
    ///
    /// \param mmavail Number of bytes the priority queue is
    /// allowed to use.
    ///
    /////////////////////////////////////////////////////////
    priority_queue(memory_size_type mm_avail);


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
    stream_offset_type size() const;

    /////////////////////////////////////////////////////////
    ///
    /// Return true if queue is empty otherwise false
    ///
    /// \return Boolean - empty or not
    ///
    /////////////////////////////////////////////////////////
    bool empty() const;

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
    stream_offset_type* slot_state;
    stream_offset_type* group_state;

    memory_size_type setting_k;
    memory_size_type current_r;
    memory_size_type setting_m;
    memory_size_type setting_mmark;

    stream_offset_type slot_data_id;

    stream_offset_type m_size;
    memory_size_type buffer_size;
    memory_size_type buffer_start;

    //////////////////
    // TPIE wrappers
    ami::err err;

	void init(memory_size_type mm_avail);

    void seek_offset(stream<T>* data, stream_offset_type offset);

    T* read_item(stream<T>* data); 

    void write_item(stream<T>* data, T write); 
    // end TPIE wrappers
    /////////////////////

    void slot_start_set(memory_size_type slot, stream_offset_type n); 
    stream_offset_type slot_start(memory_size_type slot) const; 
    void slot_size_set(memory_size_type slot, stream_offset_type n); 
    stream_offset_type slot_size(memory_size_type slot) const; 
    void group_start_set(memory_size_type group, stream_offset_type n); 
    stream_offset_type group_start(memory_size_type group) const; 
    void group_size_set(memory_size_type group, stream_offset_type n); 
    stream_offset_type group_size(memory_size_type group) const; 
    std::string filename;
    std::string datafiles;
    const std::string& datafile(stream_offset_type id); 
    const std::string& datafile_group(stream_offset_type id); 
    const std::string& slot_data(memory_size_type slotid); 
    void slot_data_set(memory_size_type slotid, stream_offset_type n); 
    const std::string& group_data(memory_size_type groupid); 
    stream_offset_type slot_max_size(memory_size_type slotid); 
    void write_slot(memory_size_type slotid, T* arr, stream_offset_type len); 
    memory_size_type free_slot(memory_size_type group);
    void empty_group(memory_size_type group);
    void fill_buffer();
    void fill_group_buffer(memory_size_type group);
    void compact(memory_size_type slot);
    void validate();
    void remove_group_buffer(memory_size_type group);
    void dump();
};

#include "priority_queue.inl"

    }  //  ami namespace

}  //  tpie namespace

#endif
