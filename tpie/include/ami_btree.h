// Copyright (C) 2001 Octavian Procopiuc
//
// File:    ami_btree.h
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: ami_btree.h,v 1.6 2001-06-26 14:56:48 adanner Exp $
//
// AMI_btree declaration and implementation.
//

#ifndef _AMI_BTREE_H
#define _AMI_BTREE_H

// STL files.
#include <algorithm>
#include <map>
#include <stack>
#include <vector>
#include <functional>
#include <fstream>

// TPIE streams.
#include <ami.h>
// The triple class.
#include <triple.h>
// AMI_COLLECTION.
#include <ami_coll.h>
// The Block class.
#include <ami_block.h>
// The cache manager.
#include <ami_cache.h>
// Statistics.
#include <tree_stats.h>

enum AMI_btree_status {
  AMI_BTREE_STATUS_VALID,
  AMI_BTREE_STATUS_INVALID,
};


// Parameters for the AMI_btree. Passed to the AMI_btree constructor.
class AMI_btree_params {
public:

  // Min number of Value's in a leaf. 0 means use default B-tree behavior.
  size_t leaf_size_min;
  // Min number of Key's in a node. 0 means use default B-tree behavior.
  size_t node_size_min;
  // Max number of Value's in a leaf. 0 means use all available capacity.
  size_t leaf_size_max;
  // Max number of Key's in a node. 0 means use all available capacity.
  size_t node_size_max;
  // How much bigger is the leaf logical block than the system block.
  size_t leaf_block_factor;
  // How much bigger is the node logical block than the system block.
  size_t node_block_factor;
  // The max number of leaves cached.
  size_t leaf_cache_size;
  // The max number of nodes cached.
  size_t node_cache_size;

  // The default parameter values.
  AMI_btree_params(): 
    leaf_size_min(0), node_size_min(0), 
    leaf_size_max(0), node_size_max(0),
    leaf_block_factor(1), node_block_factor(1), 
    leaf_cache_size(8), node_cache_size(8) {}
};

// A global object storing the default parameter values.
const AMI_btree_params btree_params_default = AMI_btree_params();

// Forward references.
template<class Key, class Value, class Compare, class KeyOfValue> class AMI_btree_leaf;
template<class Key, class Value, class Compare, class KeyOfValue> class AMI_btree_node;

// The AMI_btree class.
template <class Key, class Value, class Compare, class KeyOfValue>
class AMI_btree {
public:

  // Construct an empty B-tree.
  AMI_btree(const AMI_btree_params &params = btree_params_default);

  // Constructor. Initialize a AMI_btree from the given leaves and nodes.
  AMI_btree(const char *base_file_name,
	AMI_collection_type type = AMI_WRITE_COLLECTION,
	const AMI_btree_params &params = btree_params_default);

  // Sort in_stream and place the result in out_stream. If out_stream
  // is NULL, a new temporary stream is created.
  AMI_err sort(AMI_STREAM<Value>* in_stream, AMI_STREAM<Value>* &out_stream);

  // Bulk load from sorted stream.
  AMI_err load_sorted(AMI_STREAM<Value>* stream_s, float leaf_fill = .7, float node_fill = .5);

  // Bulk load from given stream. Calls sort and then load_sorted.
  AMI_err load(AMI_STREAM<Value>* s, float leaf_fill = .7, float node_fill = .5);

  // Write all elements stored in the tree to the given stream.
  AMI_err unload(AMI_STREAM<Value>* s);

  // Bulk load from another AMI_btree.
  AMI_err load(AMI_btree<Key, Value, Compare, KeyOfValue>* bt,
	       float leaf_fill = .7, float node_fill = .5);

  // Insert an element.
  bool insert(const Value& v);

  // Erase an element based on key.
  bool erase(const Key& k);

  // Find an element based on key. If found, return true and store the element in v.
  bool find(const Key& k, Value& v);

  // Find a predecessor based on key.
  bool pred(const Key& k, Value& v);

  // Find a successor based on key.
  bool succ(const Key& k, Value& v);
    
  // Report all values in the range determined by keys k1 and k2.
  size_t range_query(const Key& k1, const Key& k2, AMI_STREAM<Value>* s);

  size_t window_query(const Key& k1, const Key& k2, AMI_STREAM<Value>* s) 
  { return range_query(k1, k2, s); }

  // Return the number of Value elements stored.
  size_t size() const { return header_.size; }

  size_t leaf_count() const { return pcoll_leaves_->size(); }
  size_t node_count() const { return pcoll_nodes_->size(); }

  size_t os_block_count() const { 
    return pcoll_leaves_->size() * params_.leaf_block_factor + 
           pcoll_nodes_->size() * params_.node_block_factor; 
  }

  // Return the bid of the root. Make this protected or remove it.
  AMI_bid root_bid() const { return header_.root_bid; }

  // Return the height.
  size_t height() const { return header_.height; }

  // Set the persistence. It passes per along to the two block collections.
  void persist(persistence per);

  // Inquire the (real) parameters.
  const AMI_btree_params& params() const { return params_; }

  // Inquire the status.
  AMI_btree_status status() { return status_; }

  // Flush the caches and return a const reference to a tree_stats object.
  const tree_stats &stats();

  // Destructor. Flush the caches and close the collections.
  ~AMI_btree();

protected:

  // Function object for the node cache write out.
  class remove_node {
  public:
    void operator()(AMI_btree_node<Key, Value, Compare, KeyOfValue>* p) { delete p; }
  };
  // Function object for the leaf cache write out.
  class remove_leaf { 
  public:
    void operator()(AMI_btree_leaf<Key, Value, Compare, KeyOfValue>* p) { delete p; }
  };

  class header_type {
  public:
    AMI_bid root_bid;
    size_t height;
    size_t size;

    header_type(): root_bid(0), height(0), size(0) {}
  };

  // Critical information: root bid, height, size (will be stored into
  // the header of the nodes collection).
  header_type header_;

  // The node cache.
  AMI_CACHE_MANAGER<AMI_btree_node<Key, Value, Compare, KeyOfValue>*, remove_node>* node_cache_;
  // The leaf cache.
  AMI_CACHE_MANAGER<AMI_btree_leaf<Key, Value, Compare, KeyOfValue>*, remove_leaf>* leaf_cache_;

  // Run-time parameters.
  AMI_btree_params params_;

  // The collection storing the leaves.
  AMI_COLLECTION * pcoll_leaves_;

  // The collection storing the internal nodes (could be the same).
  AMI_COLLECTION * pcoll_nodes_;

  // Comparison object.
  Compare comp_;

  class comp_for_sort {
    Compare comp_;
  public:
    int compare(const Value& v1, const Value& v2) {
      return (comp_(KeyOfValue()(v1), KeyOfValue()(v2)) ? -1: 
	      (comp_(KeyOfValue()(v2), KeyOfValue()(v1)) ? 1: 0));
    }
  }; 

  // The status. Set during construction.
  AMI_btree_status status_;

  // Stack to store the path to a leaf.
  stack<pair<AMI_bid, size_t> > path_stack_;

  // Statistics.
  tree_stats stats_;

  // Use this to obtain keys from Value elements, rather than
  // KeyOfValue().
  KeyOfValue kov_;

  // Insert helpers.
  bool insert_split(const Value& v, 
		    AMI_btree_leaf<Key, Value, Compare, KeyOfValue>* p, 
		    AMI_bid& leaf_id, bool loading = false);
  bool insert_empty(const Value& v);
  bool insert_load(const Value& v,   
		   AMI_btree_leaf<Key, Value, Compare, KeyOfValue>* &lcl);

  // Intialization routine shared by all constructors.
  void shared_init(const char* base_file_name, AMI_collection_type type);

  // Empty the path stack.
  void empty_stack() { while (!path_stack_.empty()) path_stack_.pop(); }

  // Find the leaf where an element with key k might be.  Return the
  // bid of that leaf. The stack contains the path to that leaf (but
  // not the leaf itself). Each item in the stack is a pair of a bid
  // of a node and the position (in this node) of the link to the son
  // that is next on the path to the leaf.
  AMI_bid find_leaf(const Key& k);

  // Return the leaf with the minimum key element. Nothing is pushed
  // on the stack.
  AMI_bid find_min_leaf();

  // Return true if leaf p is underflow.
  bool underflow_leaf(AMI_btree_leaf<Key, Value, Compare, KeyOfValue> *p) const;

  // Return true if node p is underflow.
  bool underflow_node(AMI_btree_node<Key, Value, Compare, KeyOfValue> *p) const;

  // Return the underflow size of a leaf. Moved this function from the
  // leaf class here for saving the space of the minimum fanout, a.
  size_t cutoff_leaf(AMI_btree_leaf<Key, Value, Compare, KeyOfValue> *p) const;

  // Return the underflow size of a node.  Moved this function from the
  // node class here for saving the space of the minimum fanout, a.
  size_t cutoff_node(AMI_btree_node<Key, Value, Compare, KeyOfValue> *p) const;

  // Return true if leaf p is full.
  bool full_leaf(const AMI_btree_leaf<Key, Value, Compare, KeyOfValue> *p) const;

  // Return true if node p is full.
  bool full_node(const AMI_btree_node<Key, Value, Compare, KeyOfValue> *p) const;

  // Try to balance p (when underflow) by borrowing one element from a sibling.
  // f is the father of p and pos is the position of the link to p in f.
  // Return false if unsuccessful.
  bool balance_leaf(AMI_btree_node<Key, Value, Compare, KeyOfValue> *f, 
		    AMI_btree_leaf<Key, Value, Compare, KeyOfValue> *p, size_t pos);

  // Same as above, but p is a node.
  bool balance_node(AMI_btree_node<Key, Value, Compare, KeyOfValue> *f, 
		    AMI_btree_node<Key, Value, Compare, KeyOfValue> *p, size_t pos);

  // (When balancing fails,) merge p with a sibling.  f is the father
  // of p and pos is the position of the link to p in f.
  void merge_leaf(AMI_btree_node<Key, Value, Compare, KeyOfValue> *f, 
		  AMI_btree_leaf<Key, Value, Compare, KeyOfValue>* &p, size_t pos);

  // Same as above, but p is a node.
  void merge_node(AMI_btree_node<Key, Value, Compare, KeyOfValue> *f, 
		  AMI_btree_node<Key, Value, Compare, KeyOfValue>* &p, size_t pos);

  AMI_btree_node<Key, Value, Compare, KeyOfValue>* fetch_node(AMI_bid bid = 0);
  AMI_btree_leaf<Key, Value, Compare, KeyOfValue>* fetch_leaf(AMI_bid bid = 0);

  void release_leaf(AMI_btree_leaf<Key, Value, Compare, KeyOfValue>* p);
  void release_node(AMI_btree_node<Key, Value, Compare, KeyOfValue>* p);
};

// Shortcuts.
#define AMI_BTREE_NODE AMI_btree_node<Key, Value, Compare, KeyOfValue>
#define AMI_BTREE_LEAF AMI_btree_leaf<Key, Value, Compare, KeyOfValue>
#define AMI_BTREE      AMI_btree<Key, Value, Compare, KeyOfValue>

// Determines how elements are stored in a leaf. If set to 1, elements
// are stored in a sorted list, which results in slower insertions. If
// set to 0, elements are stored in the order in which they are
// inserted, which results in slower queries.
#define LEAF_ELEMENTS_SORTED 0

// Determines whether "previous" pointers are maintained for leaves. If
// set to 0, these pointers are not meaningful and should not be used
// (although space for them is allocated in the leaf). TODO: remove
// them completely? They are harder to maintain then "next" pointers and
// are not used anywhere.
#define LEAF_PREV_POINTER 0

// The AMI_btree_leaf class.
// Stores size() elements of type Value.
template<class Key, class Value, class Compare, class KeyOfValue>
class AMI_btree_leaf: public AMI_block<Value, triple<size_t, AMI_bid, AMI_bid> > {

  Compare comp_;

  // This is a hack. It allows comparison between 
  // Values and Keys for STL's lower_bound().
  struct Compare_value_key { 
    bool operator()(const Value& v, const Key& k) const { 
      return Compare()(KeyOfValue()(v), k); 
    }
  };
  struct Compare_value_value { 
    bool operator()(const Value& v1, const Value& v2) const { 
      return Compare()(KeyOfValue()(v1), KeyOfValue()(v2)); 
    }
  };
  Compare_value_key comp_value_key_;
  Compare_value_value comp_value_value_;
  
public:
  // Compute the capacity of the el vector STATICALLY (but you have to
  // give it the correct logical block size!).
  static size_t el_capacity(size_t block_size);

  // Find and return the position of key k 
  // (ie, the lowest position where it would be inserted).
  size_t find(const Key& k);

  // Predeccessor of k
  size_t pred(const Key& k);
    
  // Successor of k
  size_t succ(const Key& k);

  // Constructor.
  AMI_btree_leaf(AMI_COLLECTION* pcoll, AMI_bid bid = 0);

  // Number of elements stored in this leaf.
  size_t& size() { return info()->first; }
  const size_t& size() const { return info()->first; }

  // Maximum number of elements that can be stored in this leaf.
  size_t capacity() const { return el.capacity(); }

  AMI_bid& prev() { return info()->second; }
  const AMI_bid& prev() const { return info()->second; }

  AMI_bid& next() { return info()->third; }
  const AMI_bid& next() const { return info()->third; }

  bool full() const { return size() == capacity(); }

  bool empty() const { return size() == 0; }

  // Split into two leaves containing the same number of elements.
  // Return the median key (ie, the key of the last elem. stored 
  // in this leaf, after split).
  Key split(AMI_BTREE_LEAF &right);

  // Merge this leaf with another leaf.
  void merge(const AMI_BTREE_LEAF &right);

  // Insert a data element. The leaf should NOT be full.
  // Return false if the key is already in the tree.
  bool insert(const Value& v);

  // Insert element into position pos.
  void insert_pos(const Value& v, size_t pos);

  // Delete an element given by its key. The leaf should NOT be empty.
  // Return false if the key is not found in the tree.
  bool erase(const Key& k);

  // Erase element from position pos.
  void erase_pos(size_t pos);

  // Sort elements.
  void sort();

  // Destructor.
  ~AMI_btree_leaf();
};

// The AMI_btree_node class.
// An internal node of the AMI_btree.
// It stores size() keys and size()+1 links representing 
// the following pattern: Link0 Key0 Link1 Key1 ... LinkS KeyS Link(S+1)
template<class Key, class Value, class Compare, class KeyOfValue>
class AMI_btree_node: public AMI_block<Key, size_t> {

  Compare comp_;
  
public:

  // Compute the capacity of the lk vector STATICALLY (but you have to
  // give it the correct logical block size!).
  static size_t lk_capacity(size_t block_size);
  // Compute the capacity of the el vector STATICALLY.
  static size_t el_capacity(size_t block_size);

  // Find and return the position of key k 
  // (ie, the lowest position in the array of keys where it would be inserted).
  size_t find(const Key& k);

  // Constructor. Calls the block constructor with the 
  // appropriate number of links.
  AMI_btree_node(AMI_COLLECTION* pcoll, AMI_bid bid = 0);

  // Number of keys stored in this node.
  size_t& size() { return (size_t&) (*info()); }
  const size_t& size() const { return (size_t&) (*info()); }

  // Maximum number of keys that can be stored in this node.
  size_t capacity() const { return el.capacity(); }

  bool full() const { return size() == capacity(); }

  bool empty() const { return size() == 0; }

  // Split into two leaves containing the same number of elements.
  // Return the median key, to be stored in the father node.
  Key split(AMI_BTREE_NODE &right);

  // Merge this node with another node.
  void merge(const AMI_BTREE_NODE &right, const Key& k);

  // Insert a key and link into a non-full node in a given position.
  // No validity checks.
  void insert_pos(const Key& k, AMI_bid l, size_t k_pos, size_t l_pos);

  // Insert a key and link into a non-full node 
  // (uses the key k to find the right position).
  void insert(const Key& k, AMI_bid l);

  // Delete an element given by its key.
  void erase_pos(size_t k_pos, size_t l_pos);

  ~AMI_btree_node();
};


//////////////////////////////////////////////////////////
///////////////// ***Implementation*** ///////////////////
//////////////////////////////////////////////////////////


////////////////////////////////////
//////// **AMI_btree_leaf** ////////
////////////////////////////////////

template<class Key, class Value, class Compare, class KeyOfValue>
size_t AMI_BTREE_LEAF::el_capacity(size_t block_size) {
  return AMI_block<Value, triple<size_t, AMI_bid, AMI_bid> >::el_capacity(block_size, 0);
}

//// *AMI_btree_leaf::AMI_btree_leaf* ////
template<class Key, class Value, class Compare, class KeyOfValue>
AMI_BTREE_LEAF::AMI_btree_leaf(AMI_COLLECTION* pcoll, AMI_bid lbid)
              : AMI_block<Value, triple<size_t, AMI_bid, AMI_bid> >(pcoll, 0, lbid) {
  if (lbid == 0) {
    size() = 0;
    next() = 0;
#if LEAF_PREV_POINTER
    prev() = 0;
#endif
  }
}

//// *AMI_btree_leaf::split* ////
template<class Key, class Value, class Compare, class KeyOfValue>
Key  AMI_BTREE_LEAF::split(AMI_BTREE_LEAF &right) {

#if (!LEAF_ELEMENTS_SORTED)
  sort();
#endif

  // save the original size of this leaf.
  size_t original_size = size();

  // The new leaf will have half of this leaf's elements.
  // If the original size is odd, the new leaf will have fewer elements.
  right.size() = original_size / 2;

  // Update this leaf's size.
  size() = original_size - right.size();

  // Copy the elements of the new leaf from the end 
  // of this leaf's array of elements.
  right.el.copy(0, right.size(), el, size());

  dirty() = 1;
  right.dirty() = 1;
  
  // Return the key of the last element from this leaf.
  return KeyOfValue()(el[size() - 1]);
}

//// *AMI_btree_leaf::merge* ////
template<class Key, class Value, class Compare, class KeyOfValue>
void AMI_BTREE_LEAF::merge(const AMI_BTREE_LEAF &right) {

  // Make sure there's enough place.
  assert(size() + right.size() <= capacity());

#if LEAF_ELEMENTS_SORTED
  assert(comp_(KeyOfValue()(el[size() - 1]), KeyOfValue()(right.el[0])));
#endif

  // save the original size of this leaf.
  size_t original_size = size();
   
  // Update this leaf's size.
  size() = original_size + right.size();

  // Copy the elements of the right leaf to the end 
  // of this leaf's array of elements.
  el.copy(original_size, right.size(), right.el, 0);

  next() = right.next();
  dirty() = 1;  
}

//// *AMI_btree_leaf::insert_pos* ////
template<class Key, class Value, class Compare, class KeyOfValue>
void AMI_BTREE_LEAF::insert_pos(const Value& v, size_t pos) {

  // Sanity check.
  assert(!full());

  // Insert mechanics.
  if (pos == size())
    el[pos] = v;
  else
    el.insert(v, pos);

  // Increase size by one and update the dirty bit.
  size()++;
  dirty() = 1;
}

//// *AMI_btree_leaf::insert* ////
template<class Key, class Value, class Compare, class KeyOfValue>
bool AMI_BTREE_LEAF::insert(const Value& v) {

#if LEAF_ELEMENTS_SORTED
  // Find the position where v should be.
  size_t pos;
  if (size() == 0)
    pos = 0;
  else if (comp_(KeyOfValue()(el[size()-1]), KeyOfValue()(v)))
    pos = size();
  else
    pos = find(KeyOfValue()(v));

  if (pos < size()) {
    // Check for duplicate key.
    if (!comp_(KeyOfValue()(v), KeyOfValue()(el[pos])) && 
	!comp_(KeyOfValue()(el[pos]), KeyOfValue()(v))) {
      LOG_WARNING_ID("Attempting to insert duplicate key.");
      return false;
    }
  }
  insert_pos(v, pos);
#else
  // Insert it on the last position. No duplicate check!
  insert_pos(v, size());
#endif

  return true;
}

//// *AMI_btree_leaf::find* ////
template<class Key, class Value, class Compare, class KeyOfValue>
size_t AMI_BTREE_LEAF::find(const Key& k) {
#if LEAF_ELEMENTS_SORTED
  // Sanity check.
  assert(size() >= 2 ? comp_(KeyOfValue()(el[0]), KeyOfValue()(el[size()-1])): true);
  return ::lower_bound(&el[0], &el[size()-1] + 1, k, comp_value_key_) - &el[0];
#else
  size_t i;
  for (i = 0; i < size(); i++)
    if (!comp_(KeyOfValue()(el[i]), k) && !comp_(k, KeyOfValue()(el[i])))
      return i;
  return size();
#endif
}

//// *AMI_btree_leaf::pred* ////
template<class Key, class Value, class Compare, class KeyOfValue>
size_t AMI_BTREE_LEAF::pred(const Key& k) {

size_t pred_idx;
#if LEAF_ELEMENTS_SORTED
  // Sanity check.
  assert(size() >= 2 ? comp_(KeyOfValue()(el[0]), KeyOfValue()(el[size()-1])): true);
  pred_idx = ::lower_bound(&el[0], &el[size()-1] + 1, k,comp_value_key_) - &el[0];
  // lower_bound pos is off by one for pred
  // Final pred_idx cannot be matching key
  if (pred_idx != 0)
    pred_idx--;
  return pred_idx;
#else
  size_t i=0;
  size_t j;
  // Find candidate
  while (i < size() && !comp_(KeyOfValue()(el[i]), k) )
    i++;
  pred_idx = i;
  // Check for closer candidates
  for (j = i+1; j < size(); j++)
    if (comp_(KeyOfValue()(el[j]), k) && comp_(KeyOfValue()(el[i]), KeyOfValue()(el[j])))
      pred_idx = j;
  if (i != size())
    return pred_idx;
  else
    return 0;
#endif
}

//// *AMI_btree_leaf::succ* ////
template<class Key, class Value, class Compare, class KeyOfValue>
size_t AMI_BTREE_LEAF::succ(const Key& k) {

size_t succ_idx;
#if LEAF_ELEMENTS_SORTED
  // Sanity check.
  assert(size() >= 2 ? comp_(KeyOfValue()(el[0]), KeyOfValue()(el[size()-1])): true);
  succ_idx = ::lower_bound(&el[0], &el[size()-1] + 1, k, comp_value_key_) - &el[0];
  // Bump up one spot if keys match
  if (succ_idx != size() &&
      !comp_(k,KeyOfValue()(el[succ_idx])) && !comp_(KeyOfValue()(el[succ_idx]),k) )
    succ_idx++;
  return succ_idx;
#else
  size_t i=0;
  size_t j;

  // Find candidate
  while (i < size() && !comp_(k, KeyOfValue()(el[i])) )
    i++;
  succ_idx = i;
  // Check for closer candidates
  for (j = i+1; j < size(); j++)
    if (comp_(k, KeyOfValue()(el[j])) && comp_(KeyOfValue()(el[j]), KeyOfValue()(el[i])))
      succ_idx = j;
  
  if (i != size())
    return succ_idx;
  else
    return 0;
#endif
}

//// *AMI_btree_leaf::erase* ////
template<class Key, class Value, class Compare, class KeyOfValue>
bool AMI_BTREE_LEAF::erase(const Key& k) {

  // Sanity check.
  assert(!empty());

  // Find the position where k should be.
  size_t pos = find(k);

  // Make sure we found an exact match.
  if (pos == size())
    return false;
  // TODO: make sure this is right.
  if (comp_(KeyOfValue()(el[pos]), k) || comp_(k, KeyOfValue()(el[pos])))
    return false;
  
  erase_pos(pos);

  return true;
}

//// *AMI_btree_leaf::erase_pos* ////
template<class Key, class Value, class Compare, class KeyOfValue>
void AMI_BTREE_LEAF::erase_pos(size_t pos) {
 
  // Erase mechanics.
  el.erase(pos);
  
  // Decrease size by one and update dirty bit.
  size()--;
  dirty() = 1;
}

//// *AMI_btree_leaf::sort* ////
template<class Key, class Value, class Compare, class KeyOfValue>
void AMI_BTREE_LEAF::sort() {
  ::sort(&el[0], &el[size()-1] + 1, comp_value_value_);
}

//// *AMI_btree_leaf::~AMI_btree_leaf* ////
template<class Key, class Value, class Compare, class KeyOfValue>
AMI_BTREE_LEAF::~AMI_btree_leaf() {
  // TODO: is there anything to do here?
}


////////////////////////////////
//////// **AMI_btree_node** ////////
////////////////////////////////

template<class Key, class Value, class Compare, class KeyOfValue>
size_t AMI_BTREE_NODE::lk_capacity(size_t block_size) {
  return (size_t) ((block_size - sizeof(size_t) - sizeof(AMI_bid)) /
		   (sizeof(Key) + sizeof(AMI_bid)) + 1);
}

template<class Key, class Value, class Compare, class KeyOfValue>
size_t AMI_BTREE_NODE::el_capacity(size_t block_size) {
  assert((AMI_block<Key, size_t>::el_capacity(block_size, lk_capacity(block_size))) == (size_t) (lk_capacity(block_size) - 1));
  return (size_t) (lk_capacity(block_size) - 1);
}

//// *AMI_btree_node::AMI_btree_node* ////
template<class Key, class Value, class Compare, class KeyOfValue>
AMI_BTREE_NODE::AMI_btree_node(AMI_COLLECTION* pcoll, AMI_bid nbid): 
   AMI_block<Key, size_t>(pcoll, lk_capacity(pcoll->block_size()), nbid) {
  if (nbid == 0)
    size() = 0;
}


//// *AMI_btree_node::split* ////
template<class Key, class Value, class Compare, class KeyOfValue>
Key AMI_BTREE_NODE::split(AMI_BTREE_NODE &right) {

  //TODO: Is this needed? I want to be left with at least one key in each node.
  assert(size() >= 3);

  // save the original size of this node.
  size_t original_size = size();

  // The new node will have half of this node's keys and half of its links.
  right.size() = original_size / 2;

  // Update this node's size (subtract one to account for the key 
  // that is going up the tree).
  size() = original_size - right.size() - 1;

  // Copy the keys of the new node from the end of this node's array of keys.
  //memcpy(right.elem(0), elem(size()+1), right.size() * sizeof(Key));
  right.el.copy(0, right.size(), el, size() + 1);

  // Copy the links of the new node from the end of this node's array of links.
  //memcpy(right.link(0), link(size()+1), (right.size()+1) * sizeof(AMI_bid));
  right.lk.copy(0, right.size() + 1, lk, size() + 1);

  dirty() = 1;
  right.dirty() = 1;

  // Return a copy of the key past the last key (no longer stored here).
  return el[size()];
}

//// *AMI_btree_node::insert_pos* ////
template<class Key, class Value, class Compare, class KeyOfValue>
void AMI_BTREE_NODE::insert_pos(const Key& k, AMI_bid l, size_t k_pos, size_t l_pos) {

  assert(!full());

  // Insert mechanics.
  if (k_pos == size())
    el[k_pos] = k;
  else
    el.insert(k, k_pos);
  if (l_pos == size() + 1)
    lk[l_pos] = l;
  else
    lk.insert(l, l_pos);

  // Update size and dirty bit.
  size()++;
  dirty() = 1;
}


//// *AMI_btree_node::insert* ////
template<class Key, class Value, class Compare, class KeyOfValue>
void AMI_BTREE_NODE::insert(const Key& k, AMI_bid l) {

  // Find the position using STL's binary search.
  size_t pos = ::lower_bound(&el[0], &el[size()-1] + 1, k, comp_) - &el[0];

  // Insert.
  insert_pos(k, l, pos, pos + 1);
}

//// *AMI_btree_node::erase_pos* ////
template<class Key, class Value, class Compare, class KeyOfValue>
void AMI_BTREE_NODE::erase_pos(size_t k_pos, size_t l_pos) {

  assert(!empty());

  // Erase mechanics.
  el.erase(k_pos);
  lk.erase(l_pos);

  // Update the size and dirty bit.
  size()--;
  dirty() = 1;
}


//// *AMI_btree_node::merge* ////
template<class Key, class Value, class Compare, class KeyOfValue>
void AMI_BTREE_NODE::merge(const AMI_BTREE_NODE &right, const Key& k) {

  // Make sure there's enough place.
  assert(size() + right.size() + 1 <= capacity());

  // save the original size of this leaf.
  size_t original_size = size();
   
  // Update this leaf's size. We add one to account for the key 
  // that's added in-between.
  size() = original_size + right.size() + 1;

  // Copy the elements of the right leaf to the end of this leaf's
  // array of elements.
  el[original_size] = k;
  el.copy(original_size + 1, right.size(), right.el, 0);

  // Copy the links also.
  lk.copy(original_size + 1, right.size() + 1, right.lk, 0);

  dirty() = 1;
}

//// *AMI_btree_node::find* ////
template<class Key, class Value, class Compare, class KeyOfValue>
size_t AMI_BTREE_NODE::find(const Key& k) {
  return ::lower_bound(&el[0], &el[size()-1] + 1, k, comp_) - &el[0];
}

//// *AMI_btree_node::~AMI_btree_node* ////
template<class Key, class Value, class Compare, class KeyOfValue>
AMI_BTREE_NODE::~AMI_btree_node() {
  // TODO: is there anything to do here?
}

///////////////////////////
//////// **AMI_btree** ////////
///////////////////////////


//// *AMI_btree::AMI_btree* ////
template <class Key, class Value, class Compare, class KeyOfValue>
AMI_BTREE::AMI_btree(const AMI_btree_params &params): header_(), params_(params), 
  status_(AMI_BTREE_STATUS_VALID) {

  char *base_name = ami_single_temp_name("AMI_BTREE");

  shared_init(base_name, AMI_WRITE_COLLECTION);

  if (status_ == AMI_BTREE_STATUS_VALID) {
    persist(PERSIST_DELETE);
  }
}

//// *AMI_btree::AMI_btree* ////
template <class Key, class Value, class Compare, class KeyOfValue>
AMI_BTREE::AMI_btree(const char *base_file_name, AMI_collection_type type, 
	     const AMI_btree_params &params):
  header_(), params_(params), stats_(), kov_(), status_(AMI_BTREE_STATUS_VALID) {

  shared_init(base_file_name, type);

  if (status_ == AMI_BTREE_STATUS_VALID) {
    if (pcoll_leaves_->size() > 0) {
      // Read root bid, height and size from header.
      header_ = *((header_type *) pcoll_nodes_->user_data());
      // TODO: sanity checks.
    }
    persist(PERSIST_PERSISTENT);
  }
}


//// *AMI_btree::shared_init* ////
template <class Key, class Value, class Compare, class KeyOfValue>
void AMI_BTREE::shared_init(const char* base_file_name, AMI_collection_type type) {

  if (base_file_name == NULL) {
    status_ = AMI_BTREE_STATUS_INVALID;
    LOG_WARNING_ID("AMI_btree::AMI_btree: NULL file name.");
    return;
  }

#define PATH_NAME_LENGTH 128

  char lcollname[PATH_NAME_LENGTH];
  char ncollname[PATH_NAME_LENGTH];
  strncpy(lcollname, base_file_name, PATH_NAME_LENGTH - 2);
  strncpy(ncollname, base_file_name, PATH_NAME_LENGTH - 2);
  strcat(lcollname, ".l");
  strcat(ncollname, ".n");

  pcoll_leaves_ = new AMI_COLLECTION(lcollname, type, params_.leaf_block_factor);
  pcoll_nodes_ = new AMI_COLLECTION(ncollname, type, params_.node_block_factor);

  if (pcoll_leaves_->status() != AMI_COLLECTION_STATUS_VALID) {
    status_ = AMI_BTREE_STATUS_INVALID;
    LOG_WARNING_ID("AMI_btree::AMI_btree: Could not open leaves collection.");
    return;
  }
  if (pcoll_nodes_->status() != AMI_COLLECTION_STATUS_VALID) {
    status_ = AMI_BTREE_STATUS_INVALID;
    LOG_WARNING_ID("AMI_btree::AMI_btree: Could not open nodes collection.");
    return;
  }    

  // Initialize the caches.
  node_cache_ = new AMI_CACHE_MANAGER<AMI_BTREE_NODE*, remove_node>(params_.node_cache_size, 8);
  leaf_cache_ = new AMI_CACHE_MANAGER<AMI_BTREE_LEAF*, remove_leaf>(params_.leaf_cache_size, 8);

  // Give meaningful values to parameters, if necessary.
  AMI_BTREE_LEAF *dummy_leaf = fetch_leaf(0);
  if (params_.leaf_size_max == 0 || params_.leaf_size_max > dummy_leaf->capacity())
    params_.leaf_size_max = dummy_leaf->capacity();
  dummy_leaf->persist(PERSIST_DELETE);
  release_leaf(dummy_leaf);

  if (params_.leaf_size_min == 0)
    params_.leaf_size_min = params_.leaf_size_max / 2;

  AMI_BTREE_NODE* dummy_node = fetch_node(0);
  if (params_.node_size_max == 0 || params_.node_size_max > dummy_node->capacity())
    params_.node_size_max = dummy_node->capacity();
  dummy_node->persist(PERSIST_DELETE);
  release_node(dummy_node);

  if (params_.node_size_min == 0)
    params_.node_size_min = params_.node_size_max / 2;

  // Set the right block factor parameters for the case of an existing tree.
  params_.leaf_block_factor = pcoll_leaves_->block_factor();
  params_.node_block_factor = pcoll_nodes_->block_factor();
}


//// *AMI_btree::sort* ////
template <class Key, class Value, class Compare, class KeyOfValue>
AMI_err AMI_BTREE::sort(AMI_STREAM<Value>* in_stream, AMI_STREAM<Value>* &out_stream) {

  if (status_ != AMI_BTREE_STATUS_VALID) {
    LOG_WARNING_ID("sort: tree is invalid.");
    return AMI_ERROR_GENERIC_ERROR;
  }
  if (in_stream == NULL) {
    LOG_WARNING_ID("sort: attempting to sort a NULL stream pointer.");
    return AMI_ERROR_GENERIC_ERROR;
  }  
  if (in_stream->stream_len() == 0) {
    LOG_WARNING_ID("sort: attempting to sort an empty stream.");
    return AMI_ERROR_GENERIC_ERROR;
  }

  AMI_err err;
  comp_for_sort cmp;
  
  if (out_stream == NULL) {
    out_stream = new AMI_STREAM<Value>;
    out_stream->persist(PERSIST_DELETE);
  }
  
  err = AMI_sort(in_stream, out_stream, &cmp);

  if (err != AMI_ERROR_NO_ERROR)
    LOG_WARNING_ID("sort: sorting returned error.");
  else  if (in_stream->stream_len() != out_stream->stream_len()) {
    LOG_WARNING_ID("sort: sorted stream has different length than unsorted stream.");
    err = AMI_ERROR_GENERIC_ERROR;
  }

  return err;
}

//// *AMI_btree::load_sorted* ////
template <class Key, class Value, class Compare, class KeyOfValue>
AMI_err AMI_BTREE::load_sorted(AMI_STREAM<Value>* s, float leaf_fill, float node_fill) {

  if (status_ != AMI_BTREE_STATUS_VALID) {
    LOG_WARNING_ID("load: tree is invalid.");
    return AMI_ERROR_GENERIC_ERROR;
  }
  if (s == NULL) {
    LOG_WARNING_ID("load: attempting to load with NULL stream pointer.");
    return AMI_ERROR_GENERIC_ERROR;
  }

  Value* pv;
  AMI_err err = AMI_ERROR_NO_ERROR;
  AMI_btree_params params_saved = params_;
  params_.leaf_size_max = ::min(params_.leaf_size_max, size_t(leaf_fill*params_.leaf_size_max));
  params_.node_size_max = ::min(params_.leaf_size_max, size_t(node_fill*params_.node_size_max));

  AMI_BTREE_LEAF* lcl = NULL; // locally cached leaf.

  s->seek(0);
  while ((err = s->read_item(&pv)) == AMI_ERROR_NO_ERROR) {
    insert_load(*pv, lcl);
  }

  if (err != AMI_ERROR_END_OF_STREAM)
    LOG_WARNING_ID("load: error occured while reading the input stream.");
  else
    err = AMI_ERROR_NO_ERROR;

  if (lcl != NULL)
    release_leaf(lcl);
  params_ = params_saved;

  return err;
}

//// *AMI_btree::load* ////
template <class Key, class Value, class Compare, class KeyOfValue>
AMI_err AMI_BTREE::load(AMI_STREAM<Value>* s, float leaf_fill, float node_fill) {

  AMI_err err;
  AMI_STREAM<Value>* stream_s = new AMI_STREAM<Value>;

  err = sort(s, stream_s);

  if (err != AMI_ERROR_NO_ERROR)
    return err;

  err = load_sorted(stream_s, leaf_fill, node_fill);

  delete stream_s;
  return err;
}

//// *AMI_btree::unload* ////
template <class Key, class Value, class Compare, class KeyOfValue>
AMI_err AMI_BTREE::unload(AMI_STREAM<Value>* s) {

  if (status_ != AMI_BTREE_STATUS_VALID) {
    LOG_WARNING_ID("unload: tree is invalid. unload aborted.");
    return AMI_ERROR_GENERIC_ERROR;
  }
  if (s == NULL) {
    LOG_WARNING_ID("unload: NULL stream pointer. unload aborted.");
    return AMI_ERROR_GENERIC_ERROR;
  }

  AMI_bid lbid = find_min_leaf();
  AMI_BTREE_LEAF* l;
  AMI_err err = AMI_ERROR_NO_ERROR;
  size_t i;

  tp_assert(lbid != 0, "");
  
  while (lbid != 0) {
    l = fetch_leaf(lbid);
    for (i = 0; i < l->size(); i++)
      s->write_item(l->el[i]);
    lbid = l->next();
    release_leaf(l);
  }
}

//// *AMI_btree::load* ////
template <class Key, class Value, class Compare, class KeyOfValue>
AMI_err AMI_BTREE::load(AMI_BTREE* bt, float leaf_fill, float node_fill) {

  if (status_ != AMI_BTREE_STATUS_VALID) {
    LOG_WARNING_ID("load: tree is invalid.");
    return AMI_ERROR_GENERIC_ERROR;
  }
  if (bt == NULL) {
    LOG_WARNING_ID("load: NULL btree pointer.");
    return AMI_ERROR_GENERIC_ERROR;
  }
  if (bt->status() != AMI_BTREE_STATUS_VALID) {
    LOG_WARNING_ID("load: provided tree is invalid.");
    return AMI_ERROR_GENERIC_ERROR;
  }

  AMI_btree_params params_saved = params_;
  AMI_err err = AMI_ERROR_NO_ERROR;
  params_.leaf_size_max = ::min(params_.leaf_size_max, size_t(leaf_fill*params_.leaf_size_max));
  params_.node_size_max = ::min(params_.leaf_size_max, size_t(node_fill*params_.node_size_max));
  AMI_BTREE_LEAF* lcl = NULL; // locally cached leaf.

  // Get the bid of the min leaf in bt.
  AMI_bid lbid = bt->find_min_leaf();
  // Pointer to a leaf in bt.
  AMI_BTREE_LEAF* btl;
  size_t i;

  tp_assert(lbid != 0, "");

  // Iterate over all leaves of bt.
  while (lbid != 0) {
    btl = bt->fetch_leaf(lbid);

    for (i = 0; i < btl->size(); i++) {
      insert_load(btl->el[i], lcl);
    }

    // Get next leaf in bt.
    lbid = btl->next();

    bt->release_leaf(btl);
  }

  if (lcl != NULL)
    release_leaf(lcl);
  params_ = params_saved;
  return err;
}

//// *AMI_btree::find* ////
template <class Key, class Value, class Compare, class KeyOfValue>
bool AMI_BTREE::find(const Key& k, Value& v) {

  bool ans;
  size_t idx;

  // Find the leaf that might contain the key and fetch it.
  AMI_bid bid = find_leaf(k);
  AMI_BTREE_LEAF *p = fetch_leaf(bid);

  // Check whether we have a match.
  idx = p->find(k);

  if (idx < p->size() && 
      !comp_(kov_(p->el[idx]), k) && 
      !comp_(k, kov_(p->el[idx]))) {
    v = p->el[idx]; // using Value's assignment operator.
    ans = true;
  } else
    ans = false;

  // Write back the leaf and empty the stack.
  release_leaf(p);
  empty_stack();

  return ans;
}

//// *AMI_btree::pred* ////
template <class Key, class Value, class Compare, class KeyOfValue>
bool AMI_BTREE::pred(const Key& k, Value& v) {

  bool ans = false;
  size_t idx;
  AMI_BTREE_NODE * pn;
  AMI_BTREE_LEAF * pl;
  AMI_bid bid;
  size_t pos;
  size_t level, levelup;
  pair<AMI_bid, size_t> tos;

  assert(header_.height >= 1);
  assert(path_stack_.empty());

  // Get a close candidate and path_stack
  bid = find_leaf(k);
  pl = fetch_leaf(bid);
  idx = pl->pred(k);
  
  // Check whether we have a match.
  if (comp_(kov_(pl->el[idx]), k)){
    v = pl->el[idx]; 
    ans = true;
  }
  
  release_leaf(pl);
  levelup=0;
  // while no predecessor, wiggle around tree
  // find adjacent leaves. 
  while ((!path_stack_.empty()) && (ans == false) ){
    tos = path_stack_.top();
    path_stack_.pop();
    levelup++;
    pn = fetch_node(tos.first);
    if (tos.second != 0 ){
      levelup--;
      pos=tos.second-1;
      path_stack_.push(pair<AMI_bid, size_t>(tos.first,pos ));
      bid=pn->lk[pos];
      for (level = levelup; level >0; level-- ){
        release_node(pn);
        pn = fetch_node(bid);
        path_stack_.push(pair<AMI_bid, size_t>(bid, pn->size()));
        bid=pn->lk[pn->size()]; 
      }
      pl = fetch_leaf(bid);
      idx = pl->pred(k);
      // check again
      if (comp_(kov_(pl->el[idx]), k)){
        v = pl->el[idx]; 
        ans = true;
      }
      release_leaf(pl);
      levelup=0;
    }
    release_node(pn);
  } // while empty/false

  // Write back the leaf and empty the stack.
  empty_stack();

  return ans;
}

//// *AMI_btree::succ* ////
template <class Key, class Value, class Compare, class KeyOfValue>
bool AMI_BTREE::succ(const Key& k, Value& v) {

  bool ans = false;
  size_t idx;
  AMI_BTREE_NODE * pn;
  AMI_BTREE_LEAF * pl;
  AMI_bid bid;
  size_t pos;
  size_t level, levelup;
  pair<AMI_bid, size_t> tos;

  assert(header_.height >= 1);
  assert(path_stack_.empty());

  // Get a close candidate and path_stack
  bid = find_leaf(k);
  pl = fetch_leaf(bid);
  idx = pl->succ(k);
  
  // Check whether we have a match.
  if (comp_(k,kov_(pl->el[idx]))){
    v = pl->el[idx]; 
    ans = true;
  }
  
  release_leaf(pl);
  levelup=0;
  // while no successor, wiggle around tree
  // find adjacent leaves. 
  while ((!path_stack_.empty()) && (ans == false) ){
    tos = path_stack_.top();
    path_stack_.pop();
    levelup++;
    pn = fetch_node(tos.first);
    if (tos.second != pn->size() ){
      levelup--;
      pos=tos.second+1;
      path_stack_.push(pair<AMI_bid, size_t>(tos.first,pos ));
      bid=pn->lk[pos];
      for (level = levelup; level >0; level-- ){
        release_node(pn);
        pn = fetch_node(bid);
        path_stack_.push(pair<AMI_bid, size_t>(bid, 0));
        bid=pn->lk[0]; 
      }
      pl = fetch_leaf(bid);
      idx = pl->succ(k);
      // check again
      if (comp_(k, kov_(pl->el[idx]))){
        v = pl->el[idx]; 
        ans = true;
      }
      release_leaf(pl);
      levelup=0;
    }
    release_node(pn);
  } // while empty/false

  // Write back the leaf and empty the stack.
  empty_stack();

  return ans;
}

//// *AMI_btree::range_query* ////
template <class Key, class Value, class Compare, class KeyOfValue>
size_t AMI_BTREE::range_query(const Key& k1, const Key& k2, AMI_STREAM<Value>* s) {

  Key kmin = comp_(k1, k2) ? k1: k2;
  Key kmax = comp_(k1, k2) ? k2: k1;

  // Find the leaf that might contain kmin.
  AMI_bid bid = find_leaf(kmin);
  AMI_BTREE_LEAF *p = fetch_leaf(bid);
  bool done = false;
  size_t result = 0;

#if  LEAF_ELEMENTS_SORTED

  size_t j;
  j = p->find(kmin);
  while (bid != 0 && !done) {
    while (j < p->size() && !done) {
      if (comp_(kov_(p->el[j]), kmax) || 
	  (!comp_(kov_(p->el[j]), kmax) && !comp_(kmax, kov_(p->el[j])))) {
	if (s != NULL)
	  s->write_item(p->el[j]);
	result++;
      } else
	done = true;
      j++;
    }
    bid = p->next();
    release_leaf(p);
    if (bid != 0 && !done)
      p = fetch_leaf(bid);
    j = 0;
  }

#else

  size_t i;
  // Check elements of p.
  for (i = 0; i < p->size(); i++) {
    if (comp_(kov_(p->el[i]), kmax) && comp_(kmin, kov_(p->el[i])) ||
	!comp_(kov_(p->el[i]), kmax) && !comp_(kmax, kov_(p->el[i])) ||
	!comp_(kov_(p->el[i]), kmin) && !comp_(kmin, kov_(p->el[i]))) {
      if (s != NULL)
	s->write_item(p->el[i]);
      result++;
    }
  }
  bid = p->next();
  release_leaf(p);

  if (bid != 0) {
    p = fetch_leaf(bid);
    AMI_bid pnbid = p->next();
    AMI_BTREE_LEAF* pn;

    while (pnbid != 0 && !done) {
      pn = fetch_leaf(pnbid);
      if (comp_(kov_(pn->el[0]), kmax)) {
	// Write all elements from p to stream s.
	for (i = 0; i < p->size(); i++) {
	  if (s!= NULL)
	    s->write_item(p->el[i]);
	  result++;
	}
      } else 
	done = true;

      release_leaf(p);
      p = pn;
      pnbid = p->next();
    }

    // Check elements of p.
    for (i = 0; i < p->size(); i++) {
      if (comp_(kov_(p->el[i]), kmax) ||
	  (!comp_(kov_(p->el[i]), kmax) && !comp_(kmax, kov_(p->el[i])))) {
	if (s!= NULL)
	  s->write_item(p->el[i]);
	result++;
      }
    }
    release_leaf(p);
  }
#endif

  empty_stack();
  return result;
}

//// *AMI_btree::insert* ////
template <class Key, class Value, class Compare, class KeyOfValue>
bool AMI_BTREE::insert(const Value& v) {

  bool ans;

  // Check for empty tree.
  if (header_.height == 0) {
    return insert_empty(v);
  }

  // Find the leaf where v should be inserted and fetch it.
  AMI_bid bid = find_leaf(kov_(v));
  AMI_BTREE_LEAF *p = fetch_leaf(bid);

  // If the leaf is not full, insert v into it.
  if (!full_leaf(p)) {
    ans = p->insert(v);
    release_leaf(p);
  } else {
    ans = insert_split(v, p, bid);
  }

  empty_stack();

  // Update the size and return.
  header_.size += ans ? 1: 0;
  return ans;
}


//// *AMI_btree::insert_load* ////
template <class Key, class Value, class Compare, class KeyOfValue>
bool AMI_BTREE::insert_load(const Value& v, AMI_BTREE_LEAF* &lcl) {

  AMI_BTREE_LEAF *p;
  bool ans = false;
  AMI_bid bid;

  // Check for empty tree.
  if (header_.height == 0) {
    ans =  insert_empty(v);
    lcl = fetch_leaf(header_.root_bid);
    return ans;
  }

  p = lcl;
  // Verify sorting.
  assert(!comp_(kov_(v), kov_(lcl->el[lcl->size()-1])));
  
  if (!comp_(kov_(p->el[p->size()-1]), kov_(v)))
    ans = false;
  else {
    // If the leaf is not full, insert v into it.
    if (!full_leaf(p)) {
      ans = p->insert(v);
    } else {
      //AMI_bid pbid = p->bid();////
      release_leaf(p);
      
      // Do the whole routine.
      bid = find_leaf(kov_(v));
      //assert(bid == pbid);////
      // Should be in cache.
      p = fetch_leaf(bid);
      // bid will store the id of the leaf containing v after insert.
      ans = insert_split(v, p, bid, true);
      lcl = fetch_leaf(bid);
    }
  }

  empty_stack();

  // Update the size and return.
  header_.size += ans ? 1: 0;
  return ans;
}


//// *AMI_btree::insert_empty* ////
template <class Key, class Value, class Compare, class KeyOfValue>
bool AMI_BTREE::insert_empty(const Value& v) {
  bool ans;
  assert(header_.size == 0);

  // Create new root (as leaf).
  AMI_BTREE_LEAF* lroot = fetch_leaf();
  
  // Store its bid.
  header_.root_bid = lroot->bid();
  
  lroot->next() = 0;
#if LEAF_PREV_POINTER
  lroot->prev() = 0;
#endif

  // Insert v into the root.
  ans = lroot->insert(v);
  assert(ans);
  
  // Don't want the root object around.
  release_leaf(lroot);
  
  // Height and size are now 1.
  header_.height = 1;
  header_.size = 1;
  
  status_ = AMI_BTREE_STATUS_VALID;
  return ans;
}

//// *AMI_btree::insert_split* ////
template <class Key, class Value, class Compare, class KeyOfValue>
bool AMI_BTREE::insert_split(const Value& v, AMI_BTREE_LEAF* p, AMI_bid& leaf_id, bool loading) {

  AMI_BTREE_LEAF *q, *r;
  pair<AMI_bid, size_t> top;
  AMI_bid bid;
  bool ans;

  // Split the leaf.
  q = fetch_leaf();
  Key mid_key;
  if (loading) {
    mid_key = kov_(p->el[p->size()-1]);
  } else
    mid_key = p->split(*q);

  // Update the next pointers.
  q->next() = p->next();
  p->next() = q->bid();

#if LEAF_PREV_POINTER
  // Update the prev pointers.
  q->prev() = p->bid();  
  if (q->next() != 0) {
    r = fetch_leaf(q->next());
    r->prev() = q->bid();
    release_leaf(r);
  }
#endif

  bid = q->bid();
  
  // Insert in the appropriate leaf.
  if (!comp_(mid_key, kov_(v)) && !comp_(kov_(v), mid_key)) {
    ans = false;
    LOG_WARNING_ID("Attempting to insert duplicate key");
    // TODO: during loading, this is not enough. q may remain empty!
  } else {
    ans = (comp_(mid_key, kov_(v)) ? q: p)->insert(v);
    leaf_id = (comp_(mid_key, kov_(v)) ? q: p)->bid();
    assert(!loading || q->size() == 1);
  }

  release_leaf(p);
  release_leaf(q);
  
  Key fmid_key;  
  AMI_BTREE_NODE *qq, *fq;
  
  // Go up the tree.
  while (bid != 0 && !path_stack_.empty()) {
    
    // Pop the stack to find q's father.
    top = path_stack_.top();
    path_stack_.pop();
    
    // Read the father of q.
    fq = fetch_node(top.first);
    
    // Check whether we need to go further up the tree.
    if (!full_node(fq)) {
      
      // Insert the key and link into position.
      fq->insert_pos(mid_key, bid, top.second, top.second + 1);

      // Exit the loop.
      bid = 0;
      
    } else { // Need to split further.
      
      // Split fq.
      qq = fetch_node();
      fmid_key = fq->split(*qq);
	
      // Insert in the appropriate node.
      (comp_(fmid_key, mid_key) ? qq: fq)->insert(mid_key, bid);
      
      // Prepare for next iteration.
      mid_key = fmid_key;
      bid = qq->bid();
      release_node(qq);
    }

    release_node(fq);
    
  } // End of while.
  
  // Check whether the root was split.
  if (bid != 0) {
    
    assert(path_stack_.empty());
    
    // Create a new root node with the 2 links.
    AMI_BTREE_NODE* nroot = fetch_node();
    // Not very nice...
    nroot->lk[0] = header_.root_bid;
    nroot->insert_pos(mid_key, bid, 0, 1);
    
    // Update the root id.
    header_.root_bid = nroot->bid();
    
    release_node(nroot);
    
    // Update the height.
    header_.height++;
    
  } 
  return ans;
}


//// *AMI_btree::find_leaf* ////
template <class Key, class Value, class Compare, class KeyOfValue>
AMI_bid AMI_BTREE::find_leaf(const Key& k) {

  AMI_BTREE_NODE * p;
  AMI_bid bid = header_.root_bid;
  size_t pos;
  int level;

  assert(header_.height >= 1);
  assert(path_stack_.empty());

  // Go down the tree.
  for (level = header_.height - 1; level > 0; level--) {
    // Fetch the node.
    p = fetch_node(bid);
    // Find the position of the link to the child node.
    pos = p->find(k);
    // Push the current node and position on the path stack.
    path_stack_.push(pair<AMI_bid, size_t>(bid, pos));
    // Find the actual block id of the child node.
    bid = p->lk[pos];
    // Release the node.
    release_node(p);
  }

  // This should be the id of a leaf.
  return bid;
}

//// *AMI_btree::find_min_leaf* ////
template <class Key, class Value, class Compare, class KeyOfValue>
AMI_bid AMI_BTREE::find_min_leaf() {
  AMI_BTREE_NODE* p;
  AMI_bid bid = header_.root_bid;
  int level;

  assert(header_.height >= 1);
  
  for (level = header_.height - 1; level > 0; level--) {    
    p = fetch_node(bid);
    bid = p->lk[0];
    release_node(p);
  }

  return bid;
}

//// *AMI_btree::underflow_leaf* ////
template <class Key, class Value, class Compare, class KeyOfValue>
bool AMI_BTREE::underflow_leaf(AMI_BTREE_LEAF *p) const {
  return p->size() <= cutoff_leaf(p);
}

//// *AMI_btree::underflow_node* ////
template <class Key, class Value, class Compare, class KeyOfValue>
bool AMI_BTREE::underflow_node(AMI_BTREE_NODE *p) const {
  return p->size() <= cutoff_node(p);
}

//// *AMI_btree::cutoff_leaf* ////
template <class Key, class Value, class Compare, class KeyOfValue>
size_t AMI_BTREE::cutoff_leaf(AMI_BTREE_LEAF *p) const {
  // Be careful how you test for the root (thanks, Andrew).
  return (p->bid() == header_.root_bid && header_.height == 1) ? 0 
    : params_.leaf_size_min - 1;
}

//// *AMI_btree::cutoff_node* ////
template <class Key, class Value, class Compare, class KeyOfValue>
size_t AMI_BTREE::cutoff_node(AMI_BTREE_NODE *p) const {
  return (p->bid() == header_.root_bid) ? 0 : params_.node_size_min - 1;
}

//// *AMI_btree::full_leaf* ////
template <class Key, class Value, class Compare, class KeyOfValue>
bool AMI_BTREE::full_leaf(const AMI_BTREE_LEAF *p) const {
  return (p->size() == params_.leaf_size_max); 
}

//// *AMI_btree::full_node* ////
template <class Key, class Value, class Compare, class KeyOfValue>
bool AMI_BTREE::full_node(const AMI_BTREE_NODE *p) const {
  return (p->size() == params_.node_size_max);
}

//// *AMI_btree::balance_node* ////
template <class Key, class Value, class Compare, class KeyOfValue>
bool AMI_BTREE::balance_node(AMI_BTREE_NODE *f, AMI_BTREE_NODE *p, size_t pos) {

  bool ans = false;
  AMI_BTREE_NODE *sib;

  assert(p->bid() == f->lk[pos]);

  // First try to borrow from the right sibling.
  if (pos < f->size()) {

    sib = fetch_node(f->lk[pos + 1]);
    if (sib->size() >= cutoff_node(sib) + 2) {

      // Rotate left. Insert the key from the father (f) and the link
      // from the sibling (sib) to the end of p.
      p->insert_pos(f->el[pos], sib->lk[0], p->size(), p->size() + 1);
      // Move the key from sib up to the father (f).
      f->el[pos] = sib->el[0];
      // Remove the first key and link of sib.
      sib->erase_pos(0, 0);
      ans = true;

    } 
    release_node(sib);
  }

  if (pos > 0 && !ans) {

    sib = fetch_node(f->lk[pos - 1]);
    if (sib->size() >= cutoff_node(sib) + 2) {
      // Rotate right.
      p->insert_pos(f->el[pos - 1], sib->lk[sib->size()], 0, 0);
      f->el[pos - 1] = sib->el[sib->size() - 1];
      sib->erase_pos(sib->size() - 1, sib->size());
      ans = true;

    } 
    release_node(sib);
  }

  // Return.
  return ans;
}

//// *AMI_btree::balance_leaf* ////
template <class Key, class Value, class Compare, class KeyOfValue>
bool AMI_BTREE::balance_leaf(AMI_BTREE_NODE *f, AMI_BTREE_LEAF *p, size_t pos) {

  bool ans = false;
  AMI_BTREE_LEAF *sib;

  // First try to borrow from the right sibling.
  if (pos < f->size()) {
    sib = fetch_leaf(f->lk[pos + 1]);
    if (sib->size() >= cutoff_leaf(sib) + 2) {

#if (!LEAF_ELEMENTS_SORTED)
      sib->sort();
#endif
      // Rotate left.
      // Insert the first element from sib to the end of p.
      p->insert(sib->el[0]);
      // Update the key in the father (f).
      f->el[pos] = kov_(sib->el[0]);
      // Delete the first element from sib.
      sib->erase_pos(0);
      ans = true;

    }
    release_leaf(sib);
  }

  if (pos > 0 && !ans) {

    sib = fetch_leaf(f->lk[pos - 1]);
    if (sib->size() >= cutoff_leaf(sib) + 2) {

#if (!LEAF_ELEMENTS_SORTED)
      sib->sort();
#endif
      // Rotate right.
      // Insert the last element of sib to the beginning of p.
      p->insert(sib->el[sib->size() - 1]);
      // Update the key in the father.
      f->el[pos - 1] = kov_(sib->el[sib->size() - 2]);
      // Delete the last element from sib.
      sib->erase_pos(sib->size() - 1); 
      ans = true;

    }
    release_leaf(sib);
  }

  return ans;
}

//// *AMI_btree::merge_leaf* ////
template <class Key, class Value, class Compare, class KeyOfValue>
void AMI_BTREE::merge_leaf(AMI_BTREE_NODE* f, AMI_BTREE_LEAF* &p, size_t pos) {

  AMI_BTREE_LEAF * sib, *r;

  // f will be the father of both p and sib.

  if (pos < f->size()) {

    // Merge with right sibling.
    // Fetch the sibling.
    sib = fetch_leaf(f->lk[pos + 1]);
    // Update the next pointer.
    p->next() = sib->next();
#if LEAF_PREV_POINTER
    // Update the prev pointer.
    if (p->next() != 0) {
      r = fetch_leaf(p->next());
      r->prev() = p->bid();
      release_leaf(r);
    }
#endif
    // Do the merge.
    p->merge(*sib);
    // Delete the sibling.
    sib->persist(PERSIST_DELETE);
    release_leaf(sib);
    // Delete the entry for the sibling from the father.
    f->erase_pos(pos, pos + 1);

  } else {

    // Merge with left sibling.
    sib = fetch_leaf(f->lk[pos - 1]);
    sib->next() = p->next();
#if LEAF_PREV_POINTER
    if (sib->next() != 0) {
      r = fetch_leaf(sib->next());
      r->prev() = sib->bid();
      release_leaf(r);
    }
#endif
    sib->merge(*p);
    p->persist(PERSIST_DELETE);
    release_leaf(p);
    f->erase_pos(pos - 1, pos);
    p = sib;
  }
  
}

//// *AMI_btree::merge_node* ////
template <class Key, class Value, class Compare, class KeyOfValue>
void AMI_BTREE::merge_node(AMI_BTREE_NODE* f, AMI_BTREE_NODE* &p, size_t pos) {

  AMI_BTREE_NODE * sib;

  // f will be the father of both p and sib.

  if (pos < f->size()) {
    sib = fetch_node(f->lk[pos + 1]);
    p->merge(*sib, f->el[pos]);
    // Delete the sibling.
    sib->persist(PERSIST_DELETE);
    release_node(sib);
    f->erase_pos(pos, pos + 1);
  } else {
    sib = fetch_node(f->lk[pos - 1]);
    sib->merge(*p, f->el[pos - 1]);
    p->persist(PERSIST_DELETE);
    release_node(p);
    f->erase_pos(pos - 1, pos);
    p = sib;
  }

}

//// *AMI_btree::erase* ////
template <class Key, class Value, class Compare, class KeyOfValue>
bool AMI_BTREE::erase(const Key& k) {

  bool ans;

  if (header_.height == 0) 
    return false;

  // Find the leaf where the data might be and fetch it.
  AMI_bid bid = find_leaf(k);
  AMI_BTREE_LEAF *p = fetch_leaf(bid);

  // Check for exact match and delete.
  ans = p->erase(k);

  // Sanity check.
  assert(ans || !underflow_leaf(p));

  // Update the size.
  header_.size -= ans ? 1: 0;

  if (!underflow_leaf(p)) { 
    // No underflow. Cleanup and exit.
    release_leaf(p);
    empty_stack();
    return ans;
  }

  AMI_BTREE_NODE * q;
  pair<AMI_bid, size_t> top;

  // Underflow. Balance or merge up the tree.
  // Treat the first iteration separately since it deals with leaves.
  if (!path_stack_.empty()) {

    // Pop the father of p from the stack.
    top = path_stack_.top();
    path_stack_.pop();

    // Load the father of p;
    q = fetch_node(top.first);

    // Can we borrow an element from a sibling?
    if (balance_leaf(q, p, top.second)) {
      bid = 0; // Done.
    } else {

      // Merge p with a sibling.
      merge_leaf(q, p, top.second);

      // Check for underflow in the father.
      bid = (underflow_node(q) ? q->bid() : 0);
    }

    // Prepare for next iteration (or exit).
    release_leaf(p);
  }

  AMI_BTREE_NODE * pp = q;

  // The rest of the iterations up the tree.
  while (!path_stack_.empty() && bid != 0) {
      
    // Find the father of p.
    top = path_stack_.top();
    path_stack_.pop();

    // Load the father of p;
    q = fetch_node(top.first);

    // Try to balance p by borrowing from sibling(s).
    if (balance_node(q, pp, top.second)) {

      bid = 0;

    } else {
      
      // Merge p with right sibling.
      merge_node(q, pp, top.second);

      // Check for underflow in the father.
      bid = (underflow_node(q) ? q->bid() : 0);
    }

    // Prepare for next iteration (or exit).
    release_node(pp);
    pp = q;

  } // end of while..

  // Check for root underflow.
  if (bid != 0) {
    
    assert(path_stack_.empty());
    assert(pp->bid() == header_.root_bid);

    // New root.
    header_.root_bid = pp->lk[0];

    // Remove old root from collection.
    pp->persist(PERSIST_DELETE);

    header_.height--;
  }

  release_node(pp);

  // Empty the path stack and return.
  empty_stack();
  return ans;
}

template <class Key, class Value, class Compare, class KeyOfValue>
void AMI_BTREE::persist(persistence per) {
  pcoll_leaves_->persist(per);
  pcoll_nodes_->persist(per);
}

template <class Key, class Value, class Compare, class KeyOfValue>
AMI_BTREE::~AMI_btree() {
  if (status_ == AMI_BTREE_STATUS_VALID) {
    // Write initialization info into the pcoll_nodes_ header.
    *((header_type *) pcoll_nodes_->user_data()) = header_;
  }
  delete node_cache_;
  delete leaf_cache_;

  // Delete the two collections.
  delete pcoll_leaves_;
  delete pcoll_nodes_;
}

template <class Key, class Value, class Compare, class KeyOfValue>
AMI_BTREE_NODE* AMI_BTREE::fetch_node(AMI_bid bid) {
  AMI_BTREE_NODE* q;
  stats_.record(TREE_NODE_FETCH);
  // Warning: using short-circuit evaluation. Order is important.
  if ((bid == 0) || !node_cache_->read(bid, q)) {
    q = new AMI_BTREE_NODE(pcoll_nodes_, bid);
  }
  return q;
}

template <class Key, class Value, class Compare, class KeyOfValue>
AMI_BTREE_LEAF* AMI_BTREE::fetch_leaf(AMI_bid bid) {
  AMI_BTREE_LEAF* q;
  stats_.record(TREE_LEAF_FETCH);
  // Warning: using short-circuit evaluation. Order is important.
  if ((bid == 0) || !leaf_cache_->read(bid, q)) {
    q = new AMI_BTREE_LEAF(pcoll_leaves_, bid);
  }
  return q;
}

template <class Key, class Value, class Compare, class KeyOfValue>
void AMI_BTREE::release_node(AMI_BTREE_NODE *p) {
  stats_.record(TREE_NODE_RELEASE);
  if (p->persist() == PERSIST_DELETE)
    delete p;
  else
    node_cache_->write(p->bid(), p);
}

template <class Key, class Value, class Compare, class KeyOfValue>
void AMI_BTREE::release_leaf(AMI_BTREE_LEAF *p) {
  stats_.record(TREE_LEAF_RELEASE);
  if (p->persist() == PERSIST_DELETE)
    delete p;
  else
    leaf_cache_->write(p->bid(), p);
}

template <class Key, class Value, class Compare, class KeyOfValue>
const tree_stats &AMI_BTREE::stats() {
  node_cache_->flush();
  leaf_cache_->flush();
  stats_.set(TREE_LEAF_READ, pcoll_leaves_->stats().get(BC_GET));
  stats_.set(TREE_LEAF_WRITE, pcoll_leaves_->stats().get(BC_PUT));
  stats_.set(TREE_LEAF_CREATE, pcoll_leaves_->stats().get(BC_NEW));
  stats_.set(TREE_LEAF_DELETE, pcoll_leaves_->stats().get(BC_DELETE));
  stats_.set(TREE_LEAF_COUNT, pcoll_leaves_->size());
  stats_.set(TREE_NODE_READ, pcoll_nodes_->stats().get(BC_GET));
  stats_.set(TREE_NODE_WRITE, pcoll_nodes_->stats().get(BC_PUT));
  stats_.set(TREE_NODE_CREATE, pcoll_nodes_->stats().get(BC_NEW));
  stats_.set(TREE_NODE_DELETE, pcoll_nodes_->stats().get(BC_DELETE));
  stats_.set(TREE_NODE_COUNT, pcoll_nodes_->size());
  return stats_;
}

#endif // _AMI_BTREE_H
