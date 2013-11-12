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
#include <tpie/internal_queue.h>
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
		  double blockFactor=1.0)
		: m_size(0)
		, m_queueA(blockFactor)
		, m_queueB(blockFactor)
		, m_centerQueue(blockFactor*get_block_size()/sizeof(T))
		, m_currentQueue(true)
	{
		m_queueA.open();
		m_queueB.open();
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

private:
	file_stream<T> & push_queue() {
		if(m_currentQueue)
			return m_queueA;
		return m_queueB;
	}

	file_stream<T> & pop_queue() {
		if(m_currentQueue)
			return m_queueB;
		return m_queueA;
	}
public:
	////////////////////////////////////////////////////////////////////
	/// \brief Enqueue an item
	/// \param t The item to be enqueued
	////////////////////////////////////////////////////////////////////
	inline void push(const T & t) {
		if(push_queue().size() == 0 && !m_centerQueue.full())
			m_centerQueue.push(t);
		else
			push_queue().write(t);
	}

	////////////////////////////////////////////////////////////////////
	/// \brief Dequeues an item
	/// \return The dequeued item
	////////////////////////////////////////////////////////////////////
private:
	void swap_file_streams() {
		m_currentQueue = !m_currentQueue;
		pop_queue().seek(0);
		push_queue().truncate(0);
	}
public:
	const T & pop() {
		if(pop_queue().can_read())
			return pop_queue().read();
		else if(!m_centerQueue.empty()) {
			const T & i = m_centerQueue.front();
			m_centerQueue.pop();
			return i;
		}
		else {
			swap_file_streams();
			return pop_queue().read();
		}
	}

	////////////////////////////////////////////////////////////////////
	/// \brief Returns at the frontmost item in the queue
	/// \return The front most item in the queue
	////////////////////////////////////////////////////////////////////
	const T & front() {
		if(pop_queue().can_read())
			return pop_queue().peek();
		else if(!m_centerQueue.empty())
			return m_centerQueue.front();
		else {
			swap_file_streams();
			return pop_queue().peek();
		}
	}

	////////////////////////////////////////////////////////////////////
	/// \brief Compute the memory used by the queue
	////////////////////////////////////////////////////////////////////
	static memory_size_type memory_usage(double blockFactor=1.0) {
		return sizeof(queue<T>)
			+ 3*file_stream<T>::memory_usage(blockFactor) - 2*sizeof(file_stream<T>);
	}

private:
	stream_size_type m_size;
	file_stream<T> m_queueA;
	file_stream<T> m_queueB;
	internal_queue<T> m_centerQueue;
	bool m_currentQueue;
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
				try {
					*t = &this->pop();
				} catch (end_of_stream_exception e) {
					return ami::END_OF_STREAM;
				}
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
				return NO_ERROR;
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
