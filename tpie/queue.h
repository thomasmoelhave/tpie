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

#ifndef _TPIE_AMI_QUEUE_H
#define _TPIE_AMI_QUEUE_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
#include <tpie/stream.h>
#include <tpie/stack.h> 


namespace tpie {

    namespace ami {
///////////////////////////////////////////////////////////////////
/// 
///  Basic Implementation of I/O Efficient FIFO queue;
///  uses two stacks
/// 
///  \author The TPIE Project
/// 
///////////////////////////////////////////////////////////////////

	template<class T>
	class queue {

	public:

	    ////////////////////////////////////////////////////////////////////
	    ///
	    /// Constructor for Temporary Queue
	    ///
	    ////////////////////////////////////////////////////////////////////

	    queue(); 

	    ////////////////////////////////////////////////////////////////////
	    ///
	    /// Constructor for Queue with filename
	    ///
	    ////////////////////////////////////////////////////////////////////

	    queue(const std::string& basename);

	    ////////////////////////////////////////////////////////////////////
	    ///  
	    ///  Destructor, closes the underlying stream
	    ///
	    ////////////////////////////////////////////////////////////////////

	    ~queue(void);

	    ////////////////////////////////////////////////////////////////////
	    ///  
	    ///  Returns whether the stack is empty or not.
	    ///
	    ////////////////////////////////////////////////////////////////////

	    bool is_empty();

	    ////////////////////////////////////////////////////////////////////
	    ///  
	    ///  Returns the number of items currently on the queue.
	    ///
	    ////////////////////////////////////////////////////////////////////

	    stream_offset_type size() { 
		return m_Qsize; 
	    }

	    ////////////////////////////////////////////////////////////////////
	    ///  
	    ///  Enqueue an item
	    ///
	    ///  \param  t    The item to be enqueued
	    ///
	    ////////////////////////////////////////////////////////////////////

	    err enqueue(const T &t);

	    ////////////////////////////////////////////////////////////////////
	    ///  
	    ///  Dequeues an item
	    ///
	    ///  \param  t    A pointer to a pointer that will point to the 
	    ///               front item.
	    ///
	    ////////////////////////////////////////////////////////////////////

	    err dequeue(T **t);

	    ////////////////////////////////////////////////////////////////////
	    ///
	    ///  Peeks at the frontmost item in the queue
	    ///
	    ///  \param  t    A pointer to a pointer that will point to the 
	    ///               front item.
	    ///
	    ////////////////////////////////////////////////////////////////////

	    err peek(T **t);

	    ////////////////////////////////////////////////////////////////////
	    ///  
	    ///  Set the persistence status of the (stacks underlying the) queue.
	    ///
	    ///  \param  p    A persistence status.
	    ///
	    ////////////////////////////////////////////////////////////////////

	    void persist(persistence p);

	    ////////////////////////////////////////////////////////////////////
	    ///  
	    ///  Truncates the underlying stream to the exact size (rounded up
	    ///  to the next block) of items.
	    ///
	    ////////////////////////////////////////////////////////////////////

	    err trim();

	    ////////////////////////////////////////////////////////////////////
	    ///  
	    ///  Compute the memory used by the queue
	    ///
	    ///  \param  usage       Where the usage will be stored.
	    ///  \param  usage_type  The type of usage_type inquired from 
	    ///                      the stream.
	    ///
	    ////////////////////////////////////////////////////////////////////

	    err main_memory_usage(memory_size_type *usage,
				  mem::stream_usage usage_type) const;
    
	private:
	    err refill();
	    stack<T>* m_enQstack;
	    stack<T>* m_deQstack;
	    stream_offset_type m_Qsize;
	};

/////////////////////////////////////////////////////////////////////////

template<class T>
queue<T>::queue() {
    m_enQstack = new stack<T>();
    m_deQstack = new stack<T>();
    m_enQstack->persist(PERSIST_DELETE);
    m_deQstack->persist(PERSIST_DELETE);
    m_Qsize=0;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
queue<T>::queue(const std::string& basename) {
    std::string fname = basename + ".nq";
    m_enQstack = new stack<T>(fname);
    fname = basename + ".dq";
    m_deQstack = new stack<T>(fname);
    m_enQstack->persist(PERSIST_PERSISTENT);
    m_deQstack->persist(PERSIST_PERSISTENT);
    m_Qsize=m_enQstack->size()+m_deQstack->size();
}

/////////////////////////////////////////////////////////////////////////

template<class T>
queue<T>::~queue(void) {
    delete m_enQstack;
    delete m_deQstack;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
bool queue<T>::is_empty() {
    return m_Qsize == 0;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
void queue<T>::persist(persistence p) {
    m_enQstack->persist(p);
    m_deQstack->persist(p);
}

/////////////////////////////////////////////////////////////////////////

template<class T>
err queue<T>::enqueue(const T &t) {
    // Elements are pushed onto an Enqueue stack
    err ae = m_enQstack->push(t);
    if(ae == NO_ERROR){
	m_Qsize++;
    }
    return ae;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
err queue<T>::dequeue(T **t) {
    err ae;
    // Elements popped from Dequeue stack
    if(m_deQstack->size()>0){
	ae = m_deQstack->pop(t);
	if(ae == NO_ERROR){
	    m_Qsize--;
	}
	return ae;
    } else if(m_Qsize == 0) {
	return END_OF_STREAM;
    } else {
	ae = refill();
	if(ae == NO_ERROR) {
	    ae=m_deQstack->pop(t);
	    if(ae == NO_ERROR) { 
		m_Qsize--;
	    }
	}
	return ae;
    }
}

/////////////////////////////////////////////////////////////////////////

template<class T>
err queue<T>::peek(T **t) {
    err ae;
    if(m_Qsize == 0) {
	return END_OF_STREAM;
    }

    if(m_deQstack->size()>0) {
	return m_deQstack->peek(t);
    }

    ae = refill();
    if(ae == NO_ERROR) {
	return m_deQstack->peek(t);
    }
    return ae;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
err queue<T>::trim() {
    err ae;
    ae = m_enQstack->trim();
    if(ae != NO_ERROR) {
	return ae;
    }
    return m_deQstack->trim(); 
}

/////////////////////////////////////////////////////////////////////////

//move elements from Enqueue stack to Dequeue stack
template<class T>
err queue<T>::refill() {
    T* tmp;
    err ae;
    
    while((ae=m_enQstack->pop(&tmp)) == NO_ERROR){
	ae=m_deQstack->push(*tmp);
	if(ae != NO_ERROR) { 
	    return ae; 
	}
    }
    if(ae != END_OF_STREAM) {
	return ae;
    }
    return NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
err queue<T>::main_memory_usage(memory_size_type *usage,
				mem::stream_usage usage_type) const {
    
    memory_size_type usage2;

    //  Get the usage for the underlying stacks
    if(m_enQstack->main_memory_usage(usage, usage_type) != NO_ERROR) {
	TP_LOG_WARNING_ID("bte error");
	return BTE_ERROR;
    }

    if(m_deQstack->main_memory_usage(&usage2, usage_type) != NO_ERROR) {
	TP_LOG_WARNING_ID("bte error");
	return BTE_ERROR;
    }

    *usage += usage2;

    switch (usage_type) {
    case mem::STREAM_USAGE_OVERHEAD:
    case mem::STREAM_USAGE_CURRENT:
    case mem::STREAM_USAGE_MAXIMUM:
    case mem::STREAM_USAGE_SUBSTREAM:
    case mem::STREAM_USAGE_BUFFER:
	*usage += sizeof(*this);            //  Attributes.

	break;

    default:
	tp_assert(0, "Unknown mem::stream_usage type added.");
    }

    return NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////

    }  //  ami namespace

}  //  tpie namespace

#endif // _TPIE_AMI_QUEUE_H 
