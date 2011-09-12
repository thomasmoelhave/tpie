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
/// \brief Implements an I/O efficient queue
////////////////////////////////////////////////////////////////////////////////
#include <tpie/portability.h>
#include <tpie/file.h>
#include <tpie/err.h>
#include <tpie/tempname.h>
#include <tpie/file.h>
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
	queue(stream_size_type elements=std::numeric_limits<stream_size_type>::max(), 
		  double block_factor=1.0): m_size(0), m_file(block_factor), m_back(m_file), m_front(m_file) {
		m_file.open(m_temp.path(), file_base::read_write, sizeof(stream_size_type) );
		unused(elements);
	}

	////////////////////////////////////////////////////////////////////
	/// \brief Does nothing.
	///
	/// This method is included so the interface is compatible with internal_queue
	////////////////////////////////////////////////////////////////////
	void resize(stream_size_type) {}
	
	////////////////////////////////////////////////////////////////////
	/// \brief Load or create a named queue
	/// \param basename The base name of the queue to load/create
	////////////////////////////////////////////////////////////////////
	queue(const std::string& basename, double block_factor=1.0);

	////////////////////////////////////////////////////////////////////
	/// \brief Destructor, closes the underlying stream
	////////////////////////////////////////////////////////////////////
	~queue() {m_file.write_user_data(m_back.offset());}

	////////////////////////////////////////////////////////////////////
	/// \brief Check if the queue is empty
	/// \return true if the queue is empty otherwize false
	////////////////////////////////////////////////////////////////////
	inline bool empty() {return m_size == 0;}
	TPIE_DEPRECATED(bool is_empty());

	////////////////////////////////////////////////////////////////////
	/// \brief Returns the number of items currently on the queue.
	/// \return Number of itmes in the queue
	////////////////////////////////////////////////////////////////////
	inline stream_size_type size() {return m_size;}
	
	////////////////////////////////////////////////////////////////////
	/// \brief Enqueue an item
	/// \param t The item to be enqueued
	////////////////////////////////////////////////////////////////////
	inline void push(const T & t) {m_back.write(t);}
	TPIE_DEPRECATED(ami::err enqueue(const T &t));
	
	////////////////////////////////////////////////////////////////////
	/// \brief Dequeues an item
	/// \return The dequeued item
	////////////////////////////////////////////////////////////////////
	const T & pop() {return m_front.read();}
	TPIE_DEPRECATED(ami::err dequeue(const T **t));

	////////////////////////////////////////////////////////////////////
	/// \brief Returns at the frontmost item in the queue
	/// \return The front most item in the queue
	////////////////////////////////////////////////////////////////////
	const T & front() {m_front.peek();}
	TPIE_DEPRECATED(ami::err peek(const T **t));

	////////////////////////////////////////////////////////////////////
	/// \brief Deprecated, does nothing
	////////////////////////////////////////////////////////////////////
	TPIE_DEPRECATED(ami::err trim());

	////////////////////////////////////////////////////////////////////
	/// \brief Compute the memory used by the queue
	////////////////////////////////////////////////////////////////////
	static memory_size_type memory_usage(double blockFactor=1.0) {
		return sizeof(queue<T>)
			+ file<T>::memory_usage() - sizeof(file<T>)
			+ 2*file<T>::stream::memory_usage(blockFactor) - 2*sizeof(file<T>::stream);
	}

	////////////////////////////////////////////////////////////////////
	/// Set the persistence status of the (stacks underlying the) queue.
	/// \param p A persistence status.
	////////////////////////////////////////////////////////////////////
	TPIE_DEPRECATED(void persist(persistence p));
private:
	temp_file m_temp;
	stream_size_type m_size;
	file<T> m_file;
	typename file<T>::stream m_back;
	typename file<T>::stream m_front;
};


template<class T>
void queue<T>::persist(persistence p) {
	m_temp.set_persistent(p != PERSIST_DELETE);
}

template<class T>
queue<T>::queue(const std::string& basename, double blockFactor): 
	m_temp(basename), m_size(0), m_file(blockFactor), m_front(m_file), m_back(m_file) {
	m_temp.set_persistent(true);
	m_file.open(basename, file_base::read_write, sizeof(stream_size_type) );
	if (m_file.size() != 0) {
		stream_size_type t;
		m_file.read_user_data(t);
		m_front.seek(t);
	}
	m_back.seek(0, file_base::end);
	m_size = m_back.offset() - m_front.offset();
}


/////////////////////////////////////////////////////////////////////////

template<class T>
bool queue<T>::is_empty() {return empty();}

/////////////////////////////////////////////////////////////////////////

template<class T>
ami::err queue<T>::enqueue(const T &t) {
	push(t);
	return ami::NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
ami::err queue<T>::dequeue(const T **t) {
	*t = &pop();
	return ami::NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
ami::err queue<T>::peek(const T **t) {
	return front();
}

namespace ami {
	TPIE_DEPRECATED_CLASS_A(
		template <typename T> 
		class TPIE_DEPRECATED_CLASS_B queue: public tpie::queue<T> {
		public:
			queue() {}
			queue(const std::string& basename): tpie::queue<T>(basename) {}
		};
	);
}
}  //  tpie namespace
#endif // __TPIE_QUEUE_H__
