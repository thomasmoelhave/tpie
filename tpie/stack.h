// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2008, 2011, The TPIE development team
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

#ifndef _TPIE_AMI_STACK_H
#define _TPIE_AMI_STACK_H

#include <tpie/array.h>
#include <tpie/portability.h>
#include <tpie/stream.h>
#include <tpie/tpie_assert.h>

namespace tpie {

///////////////////////////////////////////////////////////////////
///
///  An implementation of an external-memory stack.
///
///////////////////////////////////////////////////////////////////

template<class T> 
class stack {


public:
   
    ////////////////////////////////////////////////////////////////////
    ///
    ///   Initialize temporary stack
    ///
    ////////////////////////////////////////////////////////////////////

    stack(); 

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Initializes the stack by (re-)opening the file given.
    ///
    ///  \param  path    The path to a file used for storing the items.
    ///  \param  block_factor  The block factor to use
    ///
    ////////////////////////////////////////////////////////////////////

    stack(const std::string& path, double block_factor=1.0);

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Closes the underlying stream and truncates it to the logical
    ///  end of the stack. TODO verify this behavior
    ///
    ////////////////////////////////////////////////////////////////////

    ~stack();

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Pushes one item onto the stack. Returns ERROR_* as 
    ///  given by the underlying stream.
    ///
    ///  \param  t    The item to push onto the stack.
    ///
    ////////////////////////////////////////////////////////////////////

    inline void push(const T & t) throw(stream_exception);

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Pops one item from the stack.
    ///
    ////////////////////////////////////////////////////////////////////

    const T & pop() throw(stream_exception); 

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Peeks at the topmost item on the stack.
    ///
    ////////////////////////////////////////////////////////////////////

    const T & top() throw(stream_exception);

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Returns the number of items currently on the stack.
    ///
    ////////////////////////////////////////////////////////////////////

    stream_size_type size() const {
		return m_file_stream.offset();
    }

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Returns whether the stack is empty or not.
    ///
    ////////////////////////////////////////////////////////////////////

    bool empty() const {
		return (m_size == 0);
    }
	TPIE_DEPRECATED(bool is_empty() const);

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Set the persistence status of the (stream underlying the) stack.
    ///
    ///  \param  p    A persistence status.
    ///
    ////////////////////////////////////////////////////////////////////

    TPIE_DEPRECATED(void persist(persistence p));

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  \deprecated Does nothing.
    ///
    ////////////////////////////////////////////////////////////////////

    TPIE_DEPRECATED(void trim());

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Compute the memory used by a stack.
    ///
    ////////////////////////////////////////////////////////////////////

	static memory_size_type memory_usage(float blockFactor=1.0) {
		return sizeof(stack<T>)
			+ file_stream<T>::memory_usage(blockFactor);
			//+ file<T>::memory_usage() - sizeof(file<T>)
			//+ 2*file<T>::stream::memory_usage(blockFactor) - 2*sizeof(file<T>::stream);
	}


    ////////////////////////////////////////////////////////////////////
    /// \deprecated This should go as soon as all old code has been migrated.
    ////////////////////////////////////////////////////////////////////
    //TPIE_OS_OFFSET stream_len() const {
	//	std::cerr << "Using AMI_stack<T>::stream_len() is deprecated." << std::endl;
	//	return m_size;
    //}

protected:

	/** The file_stream used to store the items. */
	file_stream<T> m_file_stream;

    /**  The current size of the stack (in items).  */
    TPIE_OS_OFFSET m_size;

private:
	temp_file m_temp;

};

/////////////////////////////////////////////////////////////////////////

template<class T>
stack<T>::stack() : 
	m_file_stream(),
    m_size(0) {

	m_file_stream.open(m_temp.path());

}

/////////////////////////////////////////////////////////////////////////

template<class T>
stack<T>::stack(const std::string& path, double block_factor) :
	m_file_stream(block_factor),
	m_size(0) {

	m_file_stream.open(path);
	
	m_file_stream.seek(0, file_base::end);

}

/////////////////////////////////////////////////////////////////////////

template<class T>
stack<T>::~stack() {
	m_file_stream.truncate(this->size());
}

template<class T>
bool stack<T>::is_empty() const {
	return empty();
}

/////////////////////////////////////////////////////////////////////////

template<class T>
void stack<T>::push(const T & t) throw(stream_exception) {
	m_file_stream.write(t);
}

/////////////////////////////////////////////////////////////////////////

template<class T>
const T & stack<T>::pop() throw(stream_exception) {

	const T & item = m_file_stream.read_back();

	return item;

}

/////////////////////////////////////////////////////////////////////////

template<class T>
const T & stack<T>::top() throw(stream_exception) {

	T item = m_file_stream.read_back();

	m_file_stream.read();

	return item;

}

/////////////////////////////////////////////////////////////////////////

template<class T>
void stack<T>::persist(persistence p) {
	m_temp.set_persistent(p != PERSIST_DELETE);
}

template<class T>
void stack<T>::trim() {
	// Do nothing.
}

namespace ami {

///////////////////////////////////////////////////////////////////
///
///  An implementation of an external-memory stack.
///
///////////////////////////////////////////////////////////////////

template<class T> 
class stack {


public:
   
    ////////////////////////////////////////////////////////////////////
    ///
    ///  Initializes the stack.
    ///
    ////////////////////////////////////////////////////////////////////

    stack() :
		m_ulate() {
		// Empty ctor.
	}

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Initializes the stack by (re-)opening the file given.
    ///
    ///  \param  path    The path to a file used for storing the items.
    ///  \param  type    An stream_type that indicates the 
    ///                  read/write mode of the file.
    ///
    ////////////////////////////////////////////////////////////////////

    stack(const std::string& path, 
		  stream_type type = READ_WRITE_STREAM) :
		m_ulate(path) {
		// Empty ctor.
		unused(type);
	}

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Closes the underlying stream and truncates it to the logical
    ///  end of the stack. TODO verify this behavior
    ///
    ////////////////////////////////////////////////////////////////////

    ~stack() {
		// Empty dtor.
	}

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Pushes one item onto the stack. Returns ERROR_* as 
    ///  given by the underlying stream.
    ///
    ///  \param  t    The item to be pushed onto the stack.
    ///
    ////////////////////////////////////////////////////////////////////

    err push(const T &t);

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Pops one item from the stack. Returns ERROR_* as 
    ///  given by the underlying stream or END_OF_STREAM
    ///  if the stack is empty.
    ///
    ///  \param  t    A pointer to a pointer that will point to the 
    ///               topmost item.
    ///
    ////////////////////////////////////////////////////////////////////

    err pop(const T **t); 

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Peeks at the topmost item on the stack. Returns ERROR_* as 
    ///  given by the underlying stream or END_OF_STREAM
    ///  if the stack is empty.
    ///
    ///  \param  t    A pointer to a pointer that will point to the 
    ///               topmost item.
    ///
    ////////////////////////////////////////////////////////////////////

    err peek(const T **t); 

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Returns the number of items currently on the stack.
    ///
    ////////////////////////////////////////////////////////////////////

    TPIE_OS_OFFSET size() const {
		return m_ulate.size();
    }

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Returns whether the stack is empty or not.
    ///
    ////////////////////////////////////////////////////////////////////

    bool is_empty() const {
		return m_ulate.empty();
    }

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Set the persistence status of the (stream underlying the) stack.
    ///
    ///  \param  p    A persistence status.
    ///
    ////////////////////////////////////////////////////////////////////

	void persist(persistence p) {
		m_persistence = p;
		return m_ulate.persist(p);
    }

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Returns the persistence status of the (stream underlying the) 
    ///  stack.
    ///
    ////////////////////////////////////////////////////////////////////

	persistence persist() const { 
		return m_persistence;
    }

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Truncates the underlying stream to the exact size (rounded up
    ///  to the next block) of items. In the current implementation,
	///  this does nothing.
    ///
    ////////////////////////////////////////////////////////////////////

    err trim() {
		m_ulate.trim();
		return NO_ERROR;
    }

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Compute the memory used by the stack and the aggregated stream.
    ///
    ///  \param  usage       Where the usage will be stored.
    ///  \param  usage_type  The type of usage_type inquired from 
    ///                      the stream.
    ///
    ////////////////////////////////////////////////////////////////////

	err main_memory_usage(TPIE_OS_SIZE_T *usage,
						  stream_usage usage_type) const;


    ////////////////////////////////////////////////////////////////////
    /// \deprecated This should go as soon as all old code has been migrated.
    ////////////////////////////////////////////////////////////////////
    TPIE_OS_OFFSET stream_len() const {
		std::cerr << "Using AMI_stack<T>::stream_len() is deprecated." << std::endl;
		return size();
    }

private:

	tpie::stack<T> m_ulate;

	persistence m_persistence;

};

/////////////////////////////////////////////////////////////////////////

template<class T>
err stack<T>::push(const T &t) {

    err retval = NO_ERROR;

	try {
		m_ulate.push(t);
	} catch (end_of_stream_exception & e) {
		retval = END_OF_STREAM;
	} catch (stream_exception & e) {
		retval = IO_ERROR;
	}

	return retval;

}

/////////////////////////////////////////////////////////////////////////

template<class T>
err stack<T>::pop(const T **t) {

    err retval = NO_ERROR;

	try {

		const T & res = m_ulate.pop();
		*t = &res;

	} catch (end_of_stream_exception & e) {
		retval = END_OF_STREAM;
	} catch (stream_exception & e) {
		retval = IO_ERROR;
	}

	return retval;

}

/////////////////////////////////////////////////////////////////////////

template<class T>
err stack<T>::peek(const T **t) {

    err retval = NO_ERROR;
	
	try {

		const T & res = m_ulate.top();
		*t = &res;

	} catch (end_of_stream_exception & e) {
		retval = END_OF_STREAM;
	} catch (stream_exception & e) {
		retval = IO_ERROR;
	}

	return retval;

}

/////////////////////////////////////////////////////////////////////////

template<class T>
err stack<T>::main_memory_usage(TPIE_OS_SIZE_T *usage,
								stream_usage usage_type) const {
    
    switch (usage_type) {

		//  All these types are o.k.
    case STREAM_USAGE_OVERHEAD:
    case STREAM_USAGE_CURRENT:
    case STREAM_USAGE_MAXIMUM:
    case STREAM_USAGE_SUBSTREAM:
    case STREAM_USAGE_BUFFER:
		*usage = tpie::stack<T>::memory_usage();
		*usage += sizeof(*this);
		break;

    default:
		tp_assert(0, "Unknown mem::stream_usage type added.");	
    }
    
    return NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////

} // ami namespace

}  //  tpie namespace

#endif // _TPIE_AMI_STACK_H 
