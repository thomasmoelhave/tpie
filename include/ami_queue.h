// Copyright (c) 2005 Andrew Danner
//
// File: ami_queue.h
// Author: Andrew Danner <adanner@cs.duke.edu>
// Created: 2/22/05
//
// $Id: ami_queue.h,v 1.3 2006-05-10 11:39:19 aveng Exp $
//
#ifndef _AMI_QUEUE_H
#define _AMI_QUEUE_H

// Get definitions for working with Unix and Windows
#include <portability.h>
// Get the AMI_STREAM definition.
#include <ami_stream.h>
#include <ami_stack.h> 


///////////////////////////////////////////////////////////////////
/// 
///  \class AMI_queue<T>
/// 
///  Basic Implementation of I/O Efficient FIFO queue
///  Uses two stacks
/// 
///  \author The TPIE Project
/// 
///////////////////////////////////////////////////////////////////

template<class T>
class AMI_queue {

  public:

    ////////////////////////////////////////////////////////////////////
    ///
    /// Constructor for Temporary Queue
    ///
    ////////////////////////////////////////////////////////////////////

    AMI_queue(); 

    ////////////////////////////////////////////////////////////////////
    ///
    /// Constructor for Queue with filename
    ///
    ////////////////////////////////////////////////////////////////////

    AMI_queue(const char* basename);

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Destructor, closes the underlying stream
    ///
    ////////////////////////////////////////////////////////////////////

    ~AMI_queue(void);

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

    TPIE_OS_OFFSET size() { return m_Qsize; }

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Enqueue an item
    ///
    ///  \param  t    The item to be enqueued
    ///
    ////////////////////////////////////////////////////////////////////

    AMI_err enqueue(const T &t);

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Dequeues an item
    ///
    ///  \param  t    A pointer to a pointer that will point to the 
    ///               front item.
    ///
    ////////////////////////////////////////////////////////////////////

    AMI_err dequeue(T **t);

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Peeks at the frontmost item in the queue
    ///
    ///  \param  t    A pointer to a pointer that will point to the 
    ///               front item.
    ///
    ////////////////////////////////////////////////////////////////////

    AMI_err peek(T **t);

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

    AMI_err trim();

    ////////////////////////////////////////////////////////////////////
    ///  
    ///  Compute the memory used by the queue
    ///
    ///  \param  usage       Where the usage will be stored.
    ///  \param  usage_type  The type of usage_type inquired from 
    ///                      the stream.
    ///
    ////////////////////////////////////////////////////////////////////

    AMI_err main_memory_usage(TPIE_OS_SIZE_T *usage,
            MM_stream_usage usage_type) const;

  private:
    AMI_err refill();
    AMI_stack<T>* m_enQstack;
    AMI_stack<T>* m_deQstack;
    TPIE_OS_OFFSET m_Qsize;
};

/////////////////////////////////////////////////////////////////////////

template<class T>
AMI_queue<T>::AMI_queue() {
  m_enQstack = new AMI_stack<T>();
  m_deQstack = new AMI_stack<T>();
  m_enQstack->persist(PERSIST_DELETE);
  m_deQstack->persist(PERSIST_DELETE);
  m_Qsize=0;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
AMI_queue<T>::AMI_queue(const char* basename)
{
  char fname[BTE_STREAM_PATH_NAME_LEN];
  strncpy(fname, basename, BTE_STREAM_PATH_NAME_LEN-4);
  strcat(fname,".nq"); 
  m_enQstack = new AMI_stack<T>(fname);
  strncpy(fname, basename, BTE_STREAM_PATH_NAME_LEN-4);
  strcat(fname,".dq"); 
  m_deQstack = new AMI_stack<T>(fname);
  m_enQstack->persist(PERSIST_PERSISTENT);
  m_deQstack->persist(PERSIST_PERSISTENT);
  m_Qsize=m_enQstack->size()+m_deQstack->size();
}

/////////////////////////////////////////////////////////////////////////

template<class T>
AMI_queue<T>::~AMI_queue(void)
{
  delete m_enQstack;
  delete m_deQstack;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
bool AMI_queue<T>::is_empty() {
  return m_Qsize == 0;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
void AMI_queue<T>::persist(persistence p) {
  m_enQstack->persist(p);
  m_deQstack->persist(p);
}

/////////////////////////////////////////////////////////////////////////

template<class T>
AMI_err AMI_queue<T>::enqueue(const T &t)
{
  //Elements are pushed onto an Enqueue stack
  AMI_err ae = m_enQstack->push(t);
  if(ae == AMI_ERROR_NO_ERROR){
    m_Qsize++;
  }
  return ae;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
AMI_err AMI_queue<T>::dequeue(T **t)
{
  AMI_err ae;
  //Elements popped from Dequeue stack
  if(m_deQstack->size()>0){
    ae = m_deQstack->pop(t);
    if(ae == AMI_ERROR_NO_ERROR){
      m_Qsize--;
    }
    return ae;
  } else if(m_Qsize == 0) {
    return AMI_ERROR_END_OF_STREAM;
  } else {
    ae = refill();
    if(ae == AMI_ERROR_NO_ERROR) {
      ae=m_deQstack->pop(t);
      if(ae == AMI_ERROR_NO_ERROR) { 
        m_Qsize--;
      }
    }
    return ae;
  }
}

/////////////////////////////////////////////////////////////////////////

template<class T>
AMI_err AMI_queue<T>::peek(T **t) {
  AMI_err ae;
  if(m_Qsize == 0) {
    return AMI_ERROR_END_OF_STREAM;
  }

  if(m_deQstack->size()>0) {
    return m_deQstack->peek(t);
  }

  ae = refill();
  if(ae == AMI_ERROR_NO_ERROR) {
    return m_deQstack->peek(t);
  }
  return ae;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
AMI_err AMI_queue<T>::trim() {
  m_enQstack->trim();
  m_deQstack->trim(); 
}

/////////////////////////////////////////////////////////////////////////

//move elements from Enqueue stack to Dequeue stack
template<class T>
AMI_err AMI_queue<T>::refill() {
  T* tmp;
  AMI_err ae;

  while((ae=m_enQstack->pop(&tmp)) == AMI_ERROR_NO_ERROR){
    ae=m_deQstack->push(*tmp);
    if(ae != AMI_ERROR_NO_ERROR) { 
      return ae; 
    }
  }
  if(ae != AMI_ERROR_END_OF_STREAM) {
    return ae;
  }
  return AMI_ERROR_NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////

template<class T>
AMI_err AMI_queue<T>::main_memory_usage(TPIE_OS_SIZE_T *usage,
          MM_stream_usage usage_type) const {

  TPIE_OS_SIZE_T usage2;

  //  Get the usage for the underlying stacks
  if(m_enQstack->main_memory_usage(usage, usage_type) != AMI_ERROR_NO_ERROR) {
    TP_LOG_WARNING_ID("bte error");
    return AMI_ERROR_BTE_ERROR;
  }

  if(m_deQstack->main_memory_usage(usage2, usage_type) != AMI_ERROR_NO_ERROR) {
    TP_LOG_WARNING_ID("bte error");
    return AMI_ERROR_BTE_ERROR;
  }

  *usage += usage2;

  switch (usage_type) {
    case MM_STREAM_USAGE_OVERHEAD:
    case MM_STREAM_USAGE_CURRENT:
    case MM_STREAM_USAGE_MAXIMUM:
    case MM_STREAM_USAGE_SUBSTREAM:
    case MM_STREAM_USAGE_BUFFER:
      *usage += sizeof(*this);            //  Attributes.
      break;
    default:
      tp_assert(0, "Unknown MM_stream_usage type added.");
  }

  return AMI_ERROR_NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////

#endif // _AMI_QUEUE_H 
