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

#ifndef _TPIE_AMI_STACK_H
#define _TPIE_AMI_STACK_H

#include <tpie/portability.h>
#include <tpie/stream.h>

namespace tpie {

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
    ///  Initializes the stack and creates an AMI_STREAM<T> for
    ///  storing the items.
    ///
    ////////////////////////////////////////////////////////////////////

    stack(); 

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
	  stream_type type = READ_WRITE_STREAM);

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Closes the underlying stream and truncates it to the logical
    ///  end of the stack.
    ///
    ////////////////////////////////////////////////////////////////////

    ~stack();

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

    err pop(T **t); 

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

    err peek(T **t); 

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Returns the number of items currently on the stack.
    ///
    ////////////////////////////////////////////////////////////////////

    stream_offset_type size() const {
	return m_size;
    }

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Returns whether the stack is empty or not.
    ///
    ////////////////////////////////////////////////////////////////////

    bool is_empty() const {
	return (m_size == 0);
    }

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Set the persistence status of the (stream underlying the) stack.
    ///
    ///  \param  p    A persistence status.
    ///
    ////////////////////////////////////////////////////////////////////

    void persist(persistence p) {
	m_amiStream->persist(p);
    }

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Returns the persistence status of the (stream underlying the) 
    ///  stack.
    ///
    ////////////////////////////////////////////////////////////////////

    persistence persist() const { 
	return m_amiStream->persist(); 
    }

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Truncates the underlying stream to the exact size (rounded up
    ///  to the next block) of items.
    ///
    ////////////////////////////////////////////////////////////////////

    err trim() {
	//Truncate to allow room for all elements (in mem + on disk)
	err retval = m_amiStream->truncate(m_size);
	if(retval != NO_ERROR){
	    return retval;
	}
	//Move file pointer to end of last element on disk
	return m_amiStream->seek(m_size-m_itemsInMemory);
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

    err main_memory_usage(memory_size_type *usage,
			  mem::stream_usage usage_type) const;


    ////////////////////////////////////////////////////////////////////
    /// \deprecated This should go as soon as all old code has been migrated.
    ////////////////////////////////////////////////////////////////////
    stream_offset_type stream_len() const {
	std::cerr << "Using AMI_stack<T>::stream_len() is deprecated." << std::endl;
	return m_size;
    }

protected:

    /**  The stream used for storing the items.  */
    stream<T>* m_amiStream;

    /**  The current size of the stack (in items).  */
    stream_offset_type m_size;

    /**  The logical block size of the underlying stream (in items).  */
    memory_size_type m_logicalBlockSize; 

    /**  The number of items currently present in memory.  */
    memory_size_type m_itemsInMemory;

    /**  Pointers to the at most two blocks of items kept in memory.  */
    T* m_block[2];

private:

    /**  How many items should be read. (To avoid local variables.)  */
    stream_offset_type toBeRead;

};

/////////////////////////////////////////////////////////////////////////

template<class T>
stack<T>::stack() : 
    m_amiStream(NULL), 
    m_size(0),
    m_logicalBlockSize(0),
    m_itemsInMemory(0),
    toBeRead(0) {

    //  No error checking done for the time being.
    m_amiStream = new stream<T>();

    m_logicalBlockSize = m_amiStream->chunk_size();

    //  Create two dummy blocks.
    m_block[0] = new T[m_logicalBlockSize];
    m_block[1] = new T[m_logicalBlockSize];

    //  Zero the contents of the both blocks.
    //  This may be commented out to increase performance
    //  but for the time being, we'll keep it to aid 
    //  debugging.
    memset(m_block[0], 0, m_logicalBlockSize * sizeof(T));
    memset(m_block[1], 0, m_logicalBlockSize * sizeof(T));

}

/////////////////////////////////////////////////////////////////////////

template<class T>
stack<T>::stack(const std::string& path, stream_type type) :
    m_amiStream(NULL), 
    m_size(0),
    m_logicalBlockSize(0),
    m_itemsInMemory(0),
    toBeRead(0) {

    //  No error checking done for the time being.
    m_amiStream = new stream<T>(path, type);

    //  Set the size of the stack to be the number of items present 
    //  in the underlying stream file. 
    m_size = m_amiStream->stream_len();

    m_logicalBlockSize = m_amiStream->chunk_size();

    //  Create two dummy blocks.
    m_block[0] = new T[m_logicalBlockSize];
    m_block[1] = new T[m_logicalBlockSize];

    //  Zero the contents of the both blocks.
    memset(m_block[0], 0, m_logicalBlockSize * sizeof(T));
    memset(m_block[1], 0, m_logicalBlockSize * sizeof(T));

    stream_offset_type numberOfFullBlocks = m_size / m_logicalBlockSize;
    
    //  Read the remainder.
    toBeRead = m_size - (numberOfFullBlocks * m_logicalBlockSize);

    //  No error checking done for the time being.
    m_amiStream->seek(numberOfFullBlocks * m_logicalBlockSize);
    m_amiStream->read_array(m_block[0], &toBeRead);

    // Put file pointer at end of last full block
    m_amiStream->seek(m_size-toBeRead);

    m_itemsInMemory = static_cast<memory_size_type>(toBeRead);
}

/////////////////////////////////////////////////////////////////////////

template<class T>
stack<T>::~stack() {

    //  Unload all in-memory data to disk.
    if (m_itemsInMemory < m_logicalBlockSize) {
	m_amiStream->write_array(m_block[0], m_itemsInMemory);
    }
    else { 
	m_amiStream->write_array(m_block[0], m_logicalBlockSize);
	m_amiStream->write_array(m_block[1], 
				 m_itemsInMemory -  m_logicalBlockSize);  
    }

    delete[] m_block[0];
    delete[] m_block[1];

    //explicitly needed for correct operation of trim
    m_itemsInMemory=0;
    
    //  Make sure there are no left-overs.
    trim();

    delete m_amiStream;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
err stack<T>::push(const T &t) {

    err retval = NO_ERROR;

    //  Do we need to flush items to disk?
    if (m_itemsInMemory == 2*m_logicalBlockSize) {

	//  Write the first block to disk.
	retval = m_amiStream->write_array(m_block[0], m_logicalBlockSize);

	if (retval != NO_ERROR) {
	    return retval;
	}

	//  "Move" the second block to the place where the
	//  first block used to be.
	T* dummy      = m_block[0];
	m_block[0] = m_block[1];
	m_block[1] = dummy;

	//  Zero the contents of the second block.
	//  This may be commented out to increase performance
	//  but for the time being, we'll keep it to aid 
	//  debugging.
	memset(m_block[1], 0, m_logicalBlockSize * sizeof(T));

	//  Decrease the number of items in main memory.
	m_itemsInMemory -= m_logicalBlockSize;
    }

    //  Check to which block to write the new element to.
    if (m_itemsInMemory < m_logicalBlockSize) {

	//  First block.
	(m_block[0])[m_itemsInMemory] = t;
    }
    else {

	//  Second block.
	(m_block[1])[m_itemsInMemory-m_logicalBlockSize] = t;
    }

    //  There is one more item on the stack...
    m_size++;

    //  ...which is also kept in main memory.
    m_itemsInMemory++;

    return retval;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
err stack<T>::pop(T **t) {

    err retval = NO_ERROR;

    //  Do we have items to work on?
    if (m_size) {

	if (!m_itemsInMemory) {

	    //  No items in memory, so try to fetch some from disk.
	    if (m_size < stream_offset_type(m_logicalBlockSize)) {
		// Can't do anything. This should not happen!
		return GENERIC_ERROR;
	    }
	    else {

		toBeRead = m_logicalBlockSize;

		//  Seek back one full block.
		retval = m_amiStream->seek(m_size - m_logicalBlockSize);

		if (retval != NO_ERROR) {
		    return retval;
		}
        
		// Read one full block from end
		retval = m_amiStream->read_array(m_block[0],
						 &toBeRead);

		if (retval != NO_ERROR) {
		    return retval;
		}
        
		//  Seek back one full block.
		//  Rewind stream one block again, so new blocks go at 
		//  end of stack
		retval = m_amiStream->seek(m_size - m_logicalBlockSize);

		if (retval != NO_ERROR) {
		    return retval;
		}

		m_itemsInMemory += m_logicalBlockSize;

		//  Zero the contents of the second block.
		//  This may be commented out to increase performance
		//  but for the time being, we'll keep it to aid 
		//  debugging.
		memset(m_block[1], 0, m_logicalBlockSize * sizeof(T));

	    }
	}

	//  It is important to decrease m_itemsInMemory
	//  at _this_ point.
	m_itemsInMemory--;

	//  Check from which block to read the topmost element.
	if (m_itemsInMemory < m_logicalBlockSize) {

	    //  First block.
	    *t = &((m_block[0])[m_itemsInMemory]);
	}
	else {

	    //  Second block.
	    *t = &((m_block[1])[m_itemsInMemory-m_logicalBlockSize]);
	}

	//  One item less on the stack...
	m_size--;
    }
    else {

	//  Not in a physical but in a logical way.
	return END_OF_STREAM;
    }

    return retval;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
err stack<T>::peek(T **t) {

    err retval = pop(t);

    if (retval != NO_ERROR) {
	return retval;
    }

    return push(**t);

}

/////////////////////////////////////////////////////////////////////////

template<class T>
err stack<T>::main_memory_usage(memory_size_type *usage,
				mem::stream_usage usage_type) const {
    
    //  Get the usage for the underlying stream.
    if (m_amiStream->main_memory_usage(usage, usage_type) 
	!= NO_ERROR) {

	TP_LOG_WARNING_ID("bte error");		
	return BTE_ERROR;

    }
    
    switch (usage_type) {

	//  All these types are o.k.
    case mem::STREAM_USAGE_OVERHEAD:
    case mem::STREAM_USAGE_CURRENT:
    case mem::STREAM_USAGE_MAXIMUM:
    case mem::STREAM_USAGE_SUBSTREAM:
    case mem::STREAM_USAGE_BUFFER: 
	*usage += sizeof(*this);            //  Attributes.
	*usage += 2 * m_logicalBlockSize * sizeof(T);   //  Two blocks.
	break;

    default:
	tp_assert(0, "Unknown mem::stream_usage type added.");	
    }
    
    return NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////

    }  //  ami namespace

}  //  tpie namespace

#endif // _TPIE_AMI_STACK_H 
