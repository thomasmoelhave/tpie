// Copyright (C) 2001 Octavian Procopiuc
//
// File:   d_vector.h
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: d_vector.h,v 1.2 2003-09-17 03:07:21 tavi Exp $
//

#include <vector>
#include "app_config.h"
#include <ami.h>

//
// Class d_vector. It is an in-memory vector which can be read from a
// file during construction, and written to a file during
// destruction. It has all the methods required by Logmethod for the
// Tree0 template parameter. Insertion is done at the end of the
// vector and window queries are answered by scanning the entire
// vector.
//
template<class Key, class Value, class KeyOfValue>
class d_vector {
public:
  typedef Key key_t;
  typedef Value record_t;
  typedef AMI_STREAM<Value> stream_t;

private:
  vector<Value> *v_;
  persistence per_;
  stream_t* str_;
  size_t max_block_count_;
  size_t os_block_size_;

public:
  d_vector(const char* base_file_name, AMI_collection_type t, size_t params = 0): 
    max_block_count_(params) {
    AMI_err err;
    Value* pp;
    os_block_size_ = TPIE_OS_BLOCKSIZE();
    v_ = new vector<Value>(0);
    v_->reserve(max_block_count_ * (os_block_size_/sizeof(Value) + 1));
    str_ = new stream_t(base_file_name);
    if (str_->stream_len() > 0) {
      while ((err = str_->read_item(&pp)) == AMI_ERROR_NO_ERROR) {
	v_->push_back(*pp);
      } 
    }
    str_->truncate(0);
  }

  size_t size() { return v_->size(); }

  bool erase(const Value& p) { return true; }

  bool find() { return true; }

  size_t window_query(const Key& lop, const Key& hip, stream_t* s) { 
    size_t result = 0;
    for (int i = 0; i < v_->size(); i++) {
      if (lop < KeyOfValue()((*v_)[i]) && KeyOfValue()((*v_)[i]) < hip) {
	if (s != NULL)
	  s->write_item((*v_)[i]);
	result++;
      }
    }
    return result; 
  }

  void persist(persistence per) { per_ = per; }

  void unload(stream_t* s) {
    assert(s != NULL);
    for (int i = 0; i < v_->size(); i++) 
      s->write_item((*v_)[i]);
  }

  size_t os_block_count() {
    return v_->size()/(os_block_size_/sizeof(Value)) + 1;
  }

  void insert(const Value& p) {
    v_->push_back(p);
  }

  size_t params() { return 0; }

  ~d_vector() {
    assert(str_ != NULL);
    if (per_ == PERSIST_PERSISTENT) {
      for (int i = 0; i < v_->size(); i++) {
	str_->write_item((*v_)[i]);
      }
    }
    delete v_;
    str_->persist(per_);
    delete str_;
  }

};
