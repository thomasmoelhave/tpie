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
#include "tpie_log.h"
#include <cassert>
#include "pq_overflow_heap.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cmath>
#include <string>
#include <cstring> // for memcpy
#include <sstream>
#include "pq_merge_heap.h"
#include <tpie/err.h>
#include <tpie/stream.h>
#include <tpie/array.h>

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
///  Inspiration: Sanders - Fast priority queues for cached memory (1999)
///  Refer to Section 2 and Figure 1 for an overview of the algorithm
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
	/// \param b Block factor
    ///
    /////////////////////////////////////////////////////////
    priority_queue(double f=1.0, double b=1.0);

	/////////////////////////////////////////////////////////
    ///
    /// Constructor
    ///
    /// \param mmavail Number of bytes the priority queue is
    /// allowed to use.
	/// \param b Block factor
    ///
    /////////////////////////////////////////////////////////
    priority_queue(TPIE_OS_SIZE_T mm_avail, double b=1.0);


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
    TPIE_OS_OFFSET size() const;

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

	tpie::auto_ptr<OPQType> opq; // insert heap
	tpie::array<T> buffer; // deletion buffer
	tpie::array<T> gbuffer0; // group buffer 0
	tpie::array<T> mergebuffer; // merge buffer for merging deletion buffer and group buffer 0
	tpie::array<TPIE_OS_OFFSET> slot_state;
	tpie::array<TPIE_OS_OFFSET> group_state;

    TPIE_OS_SIZE_T setting_k;
    TPIE_OS_SIZE_T current_r;
    TPIE_OS_SIZE_T setting_m;
    TPIE_OS_SIZE_T setting_mmark;

    TPIE_OS_OFFSET slot_data_id;

    TPIE_OS_OFFSET m_size;
    TPIE_OS_SIZE_T buffer_size;
    TPIE_OS_SIZE_T buffer_start;

	double block_factor;

	void init(TPIE_OS_SIZE_T mm_avail);

    void slot_start_set(TPIE_OS_SIZE_T slot, TPIE_OS_OFFSET n); 
    TPIE_OS_OFFSET slot_start(TPIE_OS_SIZE_T slot) const; 
    void slot_size_set(TPIE_OS_SIZE_T slot, TPIE_OS_OFFSET n); 
    TPIE_OS_OFFSET slot_size(TPIE_OS_SIZE_T slot) const; 
    void group_start_set(TPIE_OS_SIZE_T group, TPIE_OS_OFFSET n); 
    TPIE_OS_OFFSET group_start(TPIE_OS_SIZE_T group) const; 
    void group_size_set(TPIE_OS_SIZE_T group, TPIE_OS_OFFSET n); 
    TPIE_OS_OFFSET group_size(TPIE_OS_SIZE_T group) const; 
    std::string filename;
    std::string datafiles;
    const std::string& datafile(TPIE_OS_OFFSET id); 
    const std::string& datafile_group(TPIE_OS_OFFSET id); 
    const std::string& slot_data(TPIE_OS_SIZE_T slotid); 
    void slot_data_set(TPIE_OS_SIZE_T slotid, TPIE_OS_OFFSET n); 
    const std::string& group_data(TPIE_OS_SIZE_T groupid); 
    TPIE_OS_OFFSET slot_max_size(TPIE_OS_SIZE_T slotid); 
    void write_slot(TPIE_OS_SIZE_T slotid, T* arr, TPIE_OS_OFFSET len); 
    TPIE_OS_SIZE_T free_slot(TPIE_OS_SIZE_T group);
    void empty_group(TPIE_OS_SIZE_T group);
    void fill_buffer();
    void fill_group_buffer(TPIE_OS_SIZE_T group);
    void compact(TPIE_OS_SIZE_T slot);
    void validate();
    void remove_group_buffer(TPIE_OS_SIZE_T group);
    void dump();
};

#include "priority_queue.inl"

    }  //  ami namespace

}  //  tpie namespace

#endif
