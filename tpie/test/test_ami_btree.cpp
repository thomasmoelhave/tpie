//
// File:    test_ami_btree.cpp
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
// 
// Test file for class AMI_btree.
//

#include <versions.h>
VERSION(test_ami_btree_cpp, "$Id: test_ami_btree.cpp,v 1.8 2002-01-25 23:24:10 tavi Exp $");

#include <fstream>
#include <functional>
#include "app_config.h"
#include <cpu_timer.h>
#include <ami_btree.h>

#define SIZE_OF_STRUCTURE 148

// Key type.
typedef long bkey_t;

// Element type for the btree.
struct el_t {
  bkey_t key_;
  char data_[SIZE_OF_STRUCTURE - sizeof(bkey_t)];
  el_t(bkey_t k): key_(k) {}
  el_t() {}
};

struct key_from_el {
  bkey_t operator()(const el_t& v) const { return v.key_; }
};

typedef AMI_btree_node<bkey_t,el_t,less<bkey_t>,key_from_el> btree_node_t;
typedef AMI_btree_leaf<bkey_t,el_t,less<bkey_t>,key_from_el> btree_leaf_t;
typedef AMI_btree<bkey_t,el_t,less<bkey_t>,key_from_el> btree_t;
typedef AMI_STREAM<el_t> stream_t;

// This is 2**31-1, the max value returned by random().
#define MAX_RANDOM ((double)0x7fffffff)
// This is the max value that we want.
#define MAX_VALUE 1000000000
// The number of deletions to perform.
#define DELETE_COUNT 500

// Global variables.
size_t bulk_load_count;
size_t insert_count = 5000;
long range_search_lo = 0;
long range_search_hi = 10000000;

int main(int argc, char **argv) {

  int i;
  el_t s[DELETE_COUNT], ss;
  cpu_timer wt;
  btree_t *btree;
  char *base_file = NULL;

  LOG_SET_THRESHOLD(TP_LOG_APP_DEBUG);
  MM_manager.set_memory_limit(32*1024*1024);
  MM_manager.enforce_memory_limit();

  if (argc > 1) {
    bulk_load_count = atol(argv[1]);
    if (argc > 2)
      base_file = argv[2];
  } else {
    cerr << "Usage: " << argv[0] << " <point_count> [base_file_name]\n";
    exit(1);
  }

  AMI_btree_params params;
  params.node_block_factor = 2;
  params.leaf_block_factor = 2;

  btree = (base_file == NULL) ? new btree_t(params): 
    new btree_t(base_file, AMI_WRITE_COLLECTION, params);
  
  if (btree->status() == AMI_BTREE_STATUS_INVALID) {
    cerr << argv[0] << ": Error initializing AMI_btree. Aborting.\n";
    delete btree;
    exit(1);
  }

  cout << "\n";
  cout << "Element size: " << sizeof(el_t) << " bytes. "
       << "Key size: " << sizeof(bkey_t) << " bytes.\n";
  srandom(time(NULL));

  // Timing stream write.
  cout << "BEGIN Stream write\n";
  stream_t* is = new stream_t;
  cout << "\tCreating stream with " << bulk_load_count << " random elements.\n";
  wt.start();
  for (size_t j = 0; j < bulk_load_count; j++) {
    is->write_item(el_t(long((random()/MAX_RANDOM) * MAX_VALUE)));
  }
  wt.stop();
  cout << "END Stream write " << wt << "\n";
  wt.reset();

  // Testing Bulk loading.
  if (btree->size() == 0) {
    cout << "BEGIN Bulk Load\n";
    //    cout << "\tBulk loading from the stream created.\n";
    cout << "\tSorting... " << flush;
    wt.start();
    stream_t *os = new stream_t;
    if (btree->sort(is, os) != AMI_ERROR_NO_ERROR)
      cerr << argv[0] << ": Error during sort.\n";
    else {
      cout << "Done. " << wt << endl;
      cout << "\tLoading... " << flush;
      if (btree->load_sorted(os) != AMI_ERROR_NO_ERROR)
	cerr << argv[0] << ": Error during bulk loading.\n";
      else
	cout << "Done. " << endl;
    }
    os->persist(PERSIST_DELETE);
    delete os;
    wt.stop();
    cout << "END Bulk Load " << wt << "\n";
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
      s[i-1] = ss = el_t(i+100000);
    else
      ss = el_t(long((random()/MAX_RANDOM) * MAX_VALUE));
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

  stream_t* os = new stream_t;
  wt.start();
  btree->range_query(range_search_lo, range_search_hi, os);
  wt.stop();
  cout << "\tFound " << os->stream_len() << " elements.\n";
  delete os;
  cout << "END Search " << wt << "\n";

  // Testing erase.
  cout << "BEGIN Delete\n";
  cout << "\tDeleting " << key_from_el()(s[0]) << " through " 
       <<  key_from_el()(s[DELETE_COUNT-1]) << ": \n";
  int j = 0;
  for (i = 0; i < DELETE_COUNT ; i++) {
    if (btree->erase(key_from_el()(s[i])))
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

  cout << "\tDeleting " <<  key_from_el()(s[0]) << flush;
  if (btree->erase(key_from_el()(s[0])))
    cout << ": found. (Potential problem!)\n";
  else
    cout << ": not found. (OK)\n";
  cout << "END Delete\n";
  

  cout << "Tree size: " << btree->size() << " elements. Tree height: " 
       << btree->height() << ".\n";

  tpie_stats_tree bts = btree->stats();
  delete btree;
  
  cout << "Block collection statistics (global):\n"
       << "\tGET BLOCK:    "
       << AMI_COLLECTION::gstats().get(BLOCK_GET) << endl
       << "\tPUT BLOCK:    "
       << AMI_COLLECTION::gstats().get(BLOCK_PUT) << endl
       << "\tNEW BLOCK     "
       << AMI_COLLECTION::gstats().get(BLOCK_NEW) << endl
       << "\tDELETE BLOCK: "
       << AMI_COLLECTION::gstats().get(BLOCK_DELETE) << endl
    ;
  cout << "Tree statistics:\n"
       << "\tREAD (LEAF+NODE):    " 
       << bts.get(LEAF_READ) + bts.get(NODE_READ) << endl
       << "\tCREATE (LEAF+NODE):  " 
       << bts.get(LEAF_CREATE) + bts.get(NODE_CREATE) << endl
       << "\tFETCH (LEAF+NODE):   " 
       << bts.get(LEAF_FETCH) + bts.get(NODE_FETCH) << endl
       << "\tWRITE (LEAF+NODE):   " 
       << bts.get(LEAF_WRITE) + bts.get(NODE_WRITE) << endl
       << "\tDELETE (LEAF+NODE):  " 
       << bts.get(LEAF_DELETE) + bts.get(NODE_DELETE) << endl
       << "\tRELEASE (LEAF+NODE): " 
       << bts.get(LEAF_RELEASE) + bts.get(NODE_RELEASE) << endl
    ;
  cout << "Stream statistics (global):\n"
       << "\tREAD ITEM:    "
       << stream_t::gstats().get(ITEM_READ) << endl
       << "\tWRITE ITEM:   "
       << stream_t::gstats().get(ITEM_WRITE) << endl
       << "\tSEEK ITEM:    "
       << stream_t::gstats().get(ITEM_SEEK) << endl
       << "\tREAD BLOCK:   "
       << stream_t::gstats().get(BLOCK_READ) << endl
       << "\tWRITE BLOCK:  "
       << stream_t::gstats().get(BLOCK_WRITE) << endl
    ;

  return 0;
}
