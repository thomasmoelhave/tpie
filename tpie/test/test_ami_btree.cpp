// Copyright (C) 2001 Octavian Procopiuc
//
// File:    test_ami_btree.cpp
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// Test file for class AMI_btree.
//

#include <versions.h>
VERSION(test_ami_btree_cpp, "$Id: test_ami_btree.cpp,v 1.6 2002-01-14 17:38:31 tavi Exp $");

#include <fstream>

#include "app_config.h"
#include <cpu_timer.h>
#include <ami_btree.h>

#define SIZE_OF_STRUCTURE 148
#define TOTAL_INSERTS     50000
#define TOTAL_SEARCHES    5000

typedef long key_type;

struct structure {
  key_type key_;
  char data_[SIZE_OF_STRUCTURE - sizeof(key_type)];
  structure(key_type k): key_(k) {}
  structure() {}
};

struct key_from_structure {
  key_type operator()(const structure& v) const { return v.key_; }
};

#define AMI_BTREE_NODE_INT  AMI_btree_node<key_type, structure, less<key_type>, key_from_structure>
#define AMI_BTREE_LEAF_INT  AMI_btree_leaf<key_type, structure, less<key_type>, key_from_structure>
#define AMI_BTREE_INT  AMI_btree<key_type, structure, less<key_type>, key_from_structure>

// This is 2**31-1, the max value returned by random().
#define MAX_RANDOM ((double)0x7fffffff)
// This is the max value that we want.
#define MAX_RANDOM_VALUE 1000000000
// The number of deletions to perform.
#define DELETE_COUNT 100

size_t bulk_load_count = 1000000;
size_t insert_count = 1000;
long range_search_lo = 0;
long range_search_hi = 10000000;

int main(int argc, char **argv) {

  int i;
  structure s[DELETE_COUNT], ss;
  cpu_timer wt;

  LOG_SET_THRESHOLD(TP_LOG_APP_DEBUG);
  MM_manager.set_memory_limit(32*1024*1024);
  MM_manager.enforce_memory_limit();

  AMI_BTREE_INT *btree;
  if (argc > 1) {
    bulk_load_count = atol(argv[1]);
  } else {
    cerr << "Usage: " << argv[0] << " <point_count>\n";
    exit(1);
  }

  AMI_btree_params params;
  params.node_block_factor = 2;
  params.leaf_block_factor = 2;
  params.node_cache_size = 80;

  btree = new AMI_BTREE_INT(params);

  if (btree->status() == AMI_BTREE_STATUS_INVALID) {
    cerr << argv[0] << ": Error initializing AMI_btree. Aborting.\n";
    delete btree;
    exit(1);
  }

  cout << "\n";
  cout << "Element size: " << sizeof(structure) << " bytes. "
       << "Key size: " << sizeof(key_type) << " bytes.\n";
  srandom(time(NULL));

  // Timing stream write.
  cout << "BEGIN Stream write\n";
  AMI_STREAM<structure>* is = new AMI_STREAM<structure>;
  cout << "\tCreating stream with " << bulk_load_count << " random elements.\n";
  wt.start();
  for (size_t j = 0; j < bulk_load_count; j++) {
    is->write_item(structure(long((random()/MAX_RANDOM) * MAX_RANDOM_VALUE)));
  }
  wt.stop();
  cout << "END Stream write " << wt << "\n";
  wt.reset();

  // Testing Bulk loading.
  if (btree->size() == 0) {
    cout << "BEGIN Load\n";
    cout << "\tBulk loading from the stream created.\n";
    wt.start();
    AMI_STREAM<structure> *os = new AMI_STREAM<structure>;
    if (btree->sort(is, os) != AMI_ERROR_NO_ERROR)
      cerr << argv[0] << ": Error during sort.\n";
    else {
      cout << "\tSorted " << wt << "\n";
      if (btree->load_sorted(os) != AMI_ERROR_NO_ERROR)
	cerr << argv[0] << ": Error during bulk loading.\n";
    }
    os->persist(PERSIST_DELETE);
    delete os;
    wt.stop();
    cout << "END Load " << wt << "\n";
  }

  delete is;

  cout << "Tree size: " << btree->size() << " elements. Tree height: " 
       << btree->height() << ".\n";
  wt.reset();

  // Testing insertion.
  cout << "BEGIN Insert\n";
  cout << "\tInserting " << insert_count << " elements.\n";
  cout << "\t" << flush;
  wt.start();
  for (i = 1; i <= insert_count; i++) {
    if (i <= DELETE_COUNT)
      s[i-1] = ss = structure(i+100000);
    else
      ss = structure(long((random()/MAX_RANDOM) * MAX_RANDOM_VALUE));
    btree->insert(ss);
    if (i % (insert_count/10) == 0)
      cout << i << " " << flush;
  }
  cout << "\n";
  wt.stop();
  cout << "END Insert " << wt << "\n";
  
  cout << "Tree size: " << btree->size() << " elements. Tree height: " 
       << btree->height() << ".\n";
  wt.reset();

  // Testing range query.
  cout << "BEGIN Search\n";
  cout << "\tSearching with range [" << range_search_lo << ", " 
       << range_search_hi << "]\n";

  AMI_STREAM<structure>* os = new AMI_STREAM<structure>;
  wt.start();
  btree->range_query(range_search_lo, range_search_hi, os);
  wt.stop();
  cout << "\tFound " << os->stream_len() << " elements.\n";
  delete os;
  cout << "END Search " << wt << "\n";

  // Testing erase.
  cout << "BEGIN Delete\n";
  cout << "\tDeleting " << key_from_structure()(s[0]) << " through " 
       <<  key_from_structure()(s[DELETE_COUNT-1]) << ": \n";
  int j = 0;
  for (i = 0; i < DELETE_COUNT ; i++) {
    if (btree->erase(key_from_structure()(s[i])))
      j++;
  }

  cout << "\t\tfound " << j << " keys. ";
  if (j == DELETE_COUNT)
    cout << "(OK)\n";
  else
    cout << "(Potential problem!)\n";
  
  cout << "\tDeleting " << (long)-1 << flush;
  if (btree->erase((long)-1))
    cout << ": found. (Potential problem!)\n";
  else
    cout << ": not found. (OK)\n";

  cout << "\tDeleting " <<  key_from_structure()(s[0]) << flush;
  if (btree->erase(key_from_structure()(s[0])))
    cout << ": found. (Potential problem!)\n";
  else
    cout << ": not found. (OK)\n";
  cout << "END Delete\n";
  

  cout << "Tree size: " << btree->size() << " elements. Tree height: " 
       << btree->height() << ".\n";

  cout << "Block collection statistics:\n"
       << "\tREAD:    " 
       << btree->stats().get(LEAF_READ)+btree->stats().get(NODE_READ) << endl
       << "\tCREATE:  " 
       << btree->stats().get(LEAF_CREATE)+btree->stats().get(NODE_CREATE) << endl
       << "\tFETCH:   " 
       << btree->stats().get(LEAF_FETCH)+btree->stats().get(NODE_FETCH) << endl
       << "\tWRITE:   " 
       << btree->stats().get(LEAF_WRITE)+btree->stats().get(NODE_WRITE) << endl
       << "\tDELETE:  " 
       << btree->stats().get(LEAF_DELETE)+btree->stats().get(NODE_DELETE) << endl
       << "\tRELEASE: " 
       << btree->stats().get(LEAF_RELEASE)+btree->stats().get(NODE_RELEASE) << endl;

  cout << "Stream statistics:\n"
       << "\tITEM READ:  "
       << AMI_STREAM<structure>::gstats().get(ITEM_READ) << endl
       << "\tITEM WRITE: "
       << AMI_STREAM<structure>::gstats().get(ITEM_WRITE) << endl
       << "\tITEM_SEEK:  "
       << AMI_STREAM<structure>::gstats().get(ITEM_SEEK) << endl
       << "\tBLOCK READ: "
       << AMI_STREAM<structure>::gstats().get(BLOCK_READ) << endl
       << "\tBLOCK WRITE "
       << AMI_STREAM<structure>::gstats().get(BLOCK_WRITE) << endl;

  btree->persist(PERSIST_DELETE);
  delete btree;
  return 0;
}
