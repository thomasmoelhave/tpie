// Copyright (c) 1994 Darren Vengroff
//
// File: ami_stack.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/15/94
//
// $Id: ami_stack.h,v 1.14 2006-04-13 22:30:43 adanner Exp $
//
#ifndef _AMI_STACK_H
#define _AMI_STACK_H

#include <portability.h>
#include <ami_stream.h>

///////////////////////////////////////////////////////////////////
///
///  \class AMI_stack<T>
///
///  An implementation of an external-memory stack.
///
///  \author The TPIE Project
///
///////////////////////////////////////////////////////////////////

template<class T> 
class AMI_stack {

public:
   
    ////////////////////////////////////////////////////////////////////
    ///
    ///  Initializes the stack and creates an AMI_STREAM<T> for
    ///  storing the items.
    ///
    ////////////////////////////////////////////////////////////////////

    AMI_stack(); 

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Initializes the stack by (re-)opening the file given.
    ///
    ///  \param  path    The path to a file used for storing the items.
    ///  \param  type    An AMI_STREAM_TYPE that indicates the 
    ///                  read/write mode of the file.
    ///
    ////////////////////////////////////////////////////////////////////

    AMI_stack(const char* path, 
	      AMI_stream_type type = AMI_READ_WRITE_STREAM);

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Closes the underlying stream and truncates it to the logical
    ///  end of the stack.
    ///
    ////////////////////////////////////////////////////////////////////

    ~AMI_stack();

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Pushes one item onto the stack. Returns AMI_ERROR_* as 
    ///  given by the underlying stream.
    ///
    ///  \param  t    The item to be pushed onto the stack.
    ///
    ////////////////////////////////////////////////////////////////////

    AMI_err push(const T &t);

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Pops one item from the stack. Returns AMI_ERROR_* as 
    ///  given by the underlying stream or AMI_ERROR_END_OF_STREAM
    ///  if the stack is empty.
    ///
    ///  \param  t    A pointer to a pointer that will point to the 
    ///               topmost item.
    ///
    ////////////////////////////////////////////////////////////////////

    AMI_err pop(T **t); 

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Peeks at the topmost item on the stack. Returns AMI_ERROR_* as 
    ///  given by the underlying stream or AMI_ERROR_END_OF_STREAM
    ///  if the stack is empty.
    ///
    ///  \param  t    A pointer to a pointer that will point to the 
    ///               topmost item.
    ///
    ////////////////////////////////////////////////////////////////////

    AMI_err peek(T **t); 

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Returns the number of items currently on the stack.
    ///
    ////////////////////////////////////////////////////////////////////

    TPIE_OS_OFFSET size() const {
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

    AMI_err trim() {
	return m_amiStream->truncate(m_size); 
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

    AMI_err main_memory_usage(TPIE_OS_SIZE_T *usage,
			      MM_stream_usage usage_type) const;


    //  This should go as soon as all old code has been migrated.
    TPIE_OS_OFFSET stream_len() const {
	cerr << "Using AMI_stack<T>::stream_len() is deprecated." << endl;
	return m_size;
    }

protected:

    /**  The stream used for storing the items.  */
    AMI_STREAM<T>* m_amiStream;

    /**  The current size of the stack (in items).  */
    TPIE_OS_OFFSET m_size;

    /**  The logical block size of the underlying stream (in items).  */
    TPIE_OS_OFFSET m_logicalBlockSize; 

    /**  The number of items currently present in memory.  */
    TPIE_OS_OFFSET m_itemsInMemory;

    /**  Pointers to the at most two blocks of items kept in memory.  */
    T* m_block[2];

private:

    /**  How many items should be read. (To avoid local variables.)  */
    TPIE_OS_OFFSET toBeRead;

};

/////////////////////////////////////////////////////////////////////////

template<class T>
AMI_stack<T>::AMI_stack() : 
    m_amiStream(NULL), 
    m_size(0),
    m_logicalBlockSize(0),
    m_itemsInMemory(0),
    toBeRead(0) {

    //  No error checking done for the time being.
    m_amiStream = new AMI_STREAM<T>();

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
AMI_stack<T>::AMI_stack(const char* path, AMI_stream_type type) :
    m_amiStream(NULL), 
    m_size(0),
    m_logicalBlockSize(0),
    m_itemsInMemory(0),
    toBeRead(0) {

    //  No error checking done for the time being.
    m_amiStream = new AMI_STREAM<T>(path, type);

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

    TPIE_OS_OFFSET numberOfFullBlocks = m_size / m_logicalBlockSize;
    
    //  Read the remainder.
    toBeRead = m_size - (numberOfFullBlocks * m_logicalBlockSize);

    //  No error checking done for the time being.
    m_amiStream->seek(numberOfFullBlocks * m_logicalBlockSize);
    m_amiStream->read_array(m_block[0], &toBeRead);

    m_itemsInMemory = toBeRead;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
AMI_stack<T>::~AMI_stack() {

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
    
    //  Make sure there are no left-overs.
    trim();

    delete m_amiStream;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
AMI_err AMI_stack<T>::push(const T &t) {

    AMI_err ae = AMI_ERROR_NO_ERROR;

    //  Do we need to flush items to disk?
    if (m_itemsInMemory == 2*m_logicalBlockSize) {

      //  Write the first block to disk.
      ae = m_amiStream->write_array(m_block[0], m_logicalBlockSize);

      if (ae != AMI_ERROR_NO_ERROR) {
        return ae;
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

    return ae;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
AMI_err AMI_stack<T>::pop(T **t) {

  AMI_err ae = AMI_ERROR_NO_ERROR;

  //  Do we have items to work on?
  if (m_size) {

    if (!m_itemsInMemory) {

      //  No items in memory, so try to fetch some from disk.
      if (m_size < m_logicalBlockSize) {
        // Can't do anything. This should not happen!
        return AMI_ERROR_GENERIC_ERROR;
      }
      else {

        toBeRead = m_logicalBlockSize;

        //  Seek back one full block.
        ae = m_amiStream->seek(m_size - m_logicalBlockSize);

        if (ae != AMI_ERROR_NO_ERROR) {
          return ae;
        }
        
        // Read one full block from end
        ae = m_amiStream->read_array(m_block[0],
            &toBeRead);

        if (ae != AMI_ERROR_NO_ERROR) {
          return ae;
        }
        
        //  Seek back one full block.
        //  Rewind stream one block again, so new blocks go at 
        //  end of stack
        ae = m_amiStream->seek(m_size - m_logicalBlockSize);

        if (ae != AMI_ERROR_NO_ERROR) {
          return ae;
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
    return AMI_ERROR_END_OF_STREAM;
  }

  return ae;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
AMI_err AMI_stack<T>::peek(T **t) {

    AMI_err ae = pop(t);

    if (ae != AMI_ERROR_NO_ERROR) {
	return ae;
    }

    return push(**t);

}

/////////////////////////////////////////////////////////////////////////

template<class T>
AMI_err AMI_stack<T>::main_memory_usage(TPIE_OS_SIZE_T *usage,
					MM_stream_usage usage_type) const {
    
    //  Get the usage for the underlying stream.
    if (m_amiStream->main_memory_usage(usage, usage_type) 
	!= AMI_ERROR_NO_ERROR) {

	TP_LOG_WARNING_ID("bte error");		
	return AMI_ERROR_BTE_ERROR;

    }
    
    switch (usage_type) {

	//  All these types are o.k.
    case MM_STREAM_USAGE_OVERHEAD:
    case MM_STREAM_USAGE_CURRENT:
    case MM_STREAM_USAGE_MAXIMUM:
    case MM_STREAM_USAGE_SUBSTREAM:
    case MM_STREAM_USAGE_BUFFER: 
	*usage += sizeof(*this);            //  Attributes.
	*usage += 2 * m_logicalBlockSize;   //  Two blocks.
	break;

    default:
	tp_assert(0, "Unknown MM_stream_usage type added.");	
    }
    
    return AMI_ERROR_NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////

#endif // _AMI_STACK_H 
