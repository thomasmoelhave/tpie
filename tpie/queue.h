// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2010, The TPIE development team
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

#ifndef __TPIE_QUEUE_H__
#define __TPIE_QUEUE_H__

////////////////////////////////////////////////////////////////////////////////
/// \file queue.h
/// \brief I/O efficient queue
////////////////////////////////////////////////////////////////////////////////
#include <tpie/portability.h>
#include <tpie/deprecated.h>
#include <tpie/err.h>
#include <tpie/tempname.h>
#include <tpie/file_stream.h>
#include <limits>
#include <tpie/persist.h>
namespace tpie {

///////////////////////////////////////////////////////////////////
/// \brief Basic Implementation of I/O Efficient FIFO queue
/// \author The TPIE Project
///////////////////////////////////////////////////////////////////
template<class T>
class queue {
public:
	////////////////////////////////////////////////////////////////////
	/// \brief Constructor for Temporary Queue
	////////////////////////////////////////////////////////////////////
	queue(stream_size_type /*elements*/=std::numeric_limits<stream_size_type>::max(),
		  double blockFactor=1.0): m_size(0), m_push(blockFactor), m_pop(blockFactor) {
		m_push.open();
		m_pop.open();
	}

	////////////////////////////////////////////////////////////////////
	/// \brief Check if the queue is empty
	/// \return true if the queue is empty otherwize false
	////////////////////////////////////////////////////////////////////
	inline bool empty() {return m_size == 0;}

	////////////////////////////////////////////////////////////////////
	/// \brief Returns the number of items currently on the queue.
	/// \return Number of itmes in the queue
	////////////////////////////////////////////////////////////////////
	inline stream_size_type size() {return m_size;}
	
	////////////////////////////////////////////////////////////////////
	/// \brief Enqueue an item
	/// \param t The item to be enqueued
	////////////////////////////////////////////////////////////////////
	inline void push(const T & t) {
		m_push.write(t);
		++m_size;
	}
	
	////////////////////////////////////////////////////////////////////
	/// \brief Dequeues an item
	/// \return The dequeued item
	////////////////////////////////////////////////////////////////////
private:
	void check_empty_pop() {
		if(!m_pop.can_read()) {
			m_push.swap(m_pop);
			m_push.truncate(0);
			m_pop.seek(0);
		}
	}
public:
	const T & pop() {
		check_empty_pop();
		--m_size;
		return m_pop.read();
	}

	////////////////////////////////////////////////////////////////////
	/// \brief Returns at the frontmost item in the queue
	/// \return The front most item in the queue
	////////////////////////////////////////////////////////////////////
	const T & front() {
		check_empty_pop();
		return m_pop.peek();
	}

	////////////////////////////////////////////////////////////////////
	/// \brief Compute the memory used by the queue
	////////////////////////////////////////////////////////////////////
	static memory_size_type memory_usage(double blockFactor=1.0) {
		return sizeof(queue<T>)
			+ 2*file_stream<T>::memory_usage(blockFactor) - 2*sizeof(file_stream<T>);
	}

private:
	stream_size_type m_size;
	file_stream<T> m_push;
	file_stream<T> m_pop;
};

namespace ami {
	TPIE_DEPRECATED_CLASS_A(
		template <typename T> 
		class TPIE_DEPRECATED_CLASS_B queue: public tpie::queue<T> {
		public:
			queue() {}
			queue(const std::string& basename): tpie::queue<T>(basename) {}

			////////////////////////////////////////////////////////////////////
			/// \brief Check if the queue is empty
			/// \return true if the queue is empty otherwise false
			////////////////////////////////////////////////////////////////////
			bool is_empty() {
				return this->empty();
			}

			////////////////////////////////////////////////////////////////////
			/// \brief Enqueue an item
			/// \param t The item to be enqueued
			////////////////////////////////////////////////////////////////////
			err enqueue(const T &t) {
				this->push(t);
				return NO_ERROR;
			}

			////////////////////////////////////////////////////////////////////
			/// \brief Dequeue an item and make *t point to the items
			/// \return NO_ERROR if the element was popped successfully
			////////////////////////////////////////////////////////////////////
			err dequeue(const T **t) {
				*t = &this->pop();
				return NO_ERROR;
			}

			///////////////////////////////////////////////////////////////////////////////
			/// \brief Makes *t point to the first element of the queue
			/// \return NO_ERROR if the element was pushed successfully
			///////////////////////////////////////////////////////////////////////////////
			err peek(const T **t) {
				*t = &this->front();
				return NO_ERROR;
			}

			////////////////////////////////////////////////////////////////////
			/// \brief Deprecated, does nothing
			////////////////////////////////////////////////////////////////////
			err trim() {

			}

			////////////////////////////////////////////////////////////////////
			/// Deprecated, does nothing
			/// \param p A persistence status.
			////////////////////////////////////////////////////////////////////
			void persist(persistence) {

			}
		};
	);
}
}  //  tpie namespace
#endif // __TPIE_QUEUE_H__
