//
// File:    test_ami_btree.cpp
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
// 
// Test file for class AMI_btree.
//

#include "app_config.h"
#include <portability.h>

#include <versions.h>
VERSION(test_ami_btree_cpp, "$Id: test_ami_btree.cpp,v 1.19 2003-09-14 21:46:25 tavi Exp $");
#include <app_params.h>

#include <cpu_timer.h>
#include <ami_btree.h>

#define SIZE_OF_STRUCTURE 128

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

typedef AMI_btree< bkey_t,el_t,less<bkey_t>,key_from_el,BTE_COLLECTION_UFS > u_btree_t;
//typedef AMI_btree< bkey_t,el_t,less<bkey_t>,key_from_el,BTE_COLLECTION_MMAP > m_btree_t;
typedef AMI_STREAM< el_t > stream_t;

// Template instantiations (to get meaningful output from gprof)
//template class AMI_btree_node<bkey_t,el_t,less<bkey_t>,key_from_el>;
//template class AMI_btree_leaf<bkey_t,el_t,less<bkey_t>,key_from_el>;
//template class AMI_btree< bkey_t,el_t,less<bkey_t>,key_from_el,BTE_COLLECTION_UFS >;
//template class AMI_btree< bkey_t,el_t,less<bkey_t>,key_from_el,BTE_COLLECTION_MMAP >;
//template class AMI_STREAM< el_t >;
//template class AMI_collection_single< BTE_COLLECTION_UFS >;
//template class AMI_collection_single< BTE_COLLECTION_MMAP >;


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

  print_configuration(cerr);

  int i;
  size_t j;
  el_t s[DELETE_COUNT], ss;
  cpu_timer wt;
  char *base_file = NULL;

  // Log debugging info from the application, but not from the library. 
  tpie_log_init(TPIE_LOG_APP_DEBUG); 
 
  MM_manager.set_memory_limit(64*1024*1024);
  MM_manager.enforce_memory_limit();

  if (argc > 1) {
    bulk_load_count = atol(argv[1]);
    if (argc > 2)
      base_file = argv[2];
  } else {
    cerr << "Usage: " << argv[0] << " <point_count> [base_file_name]\n";
    exit(1);
  }

  cout << "\n";
  cout << "Element size: " << sizeof(el_t) << " bytes. "
       << "Key size: " << sizeof(bkey_t) << " bytes.\n";
  TPIE_OS_SRANDOM(time(NULL));

  // Timing stream write.
  cout << "BEGIN Stream write\n";
  stream_t* is = (base_file == NULL) ? new stream_t:
    new stream_t(base_file);
  cout << "\tCreating stream with " << bulk_load_count << " random elements.\n";
  wt.start();
  for (j = 0; j < bulk_load_count; j++) {
    is->write_item(el_t(long((TPIE_OS_RANDOM()/MAX_RANDOM) * MAX_VALUE)));
  }
  wt.stop();
  cout << "END Stream write " << wt << "\n";
  wt.reset();


  //////  Testing Bulk loading. ///////

  u_btree_t *u_btree;
  AMI_btree_params params;
  params.node_block_factor = 1;
  params.leaf_block_factor = 1;
  params.leaf_cache_size = 32;
  params.node_cache_size = 64;

  u_btree = (base_file == NULL) ? new u_btree_t(params): 
    new u_btree_t(base_file, AMI_WRITE_COLLECTION, params);
  
  if (!u_btree->is_valid()) {
    cerr << argv[0] << ": Error initializing btree. Aborting.\n";
    delete u_btree;
    exit(1);
  }
  
  if (u_btree->size() == 0) {
    cout << "BEGIN Bulk Load\n";
    //    cout << "\tBulk loading from the stream created.\n";
    cout << "\tSorting... " << flush;
    wt.start();
    stream_t *os = new stream_t;
    if (u_btree->sort(is, os) != AMI_ERROR_NO_ERROR)
      cerr << argv[0] << ": Error during sort.\n";
    else {
      wt.stop();
      cout << "Done. " << wt << "\n";
      wt.reset();
      wt.start();
      cout << "\tLoading... " << flush;
      if (u_btree->load_sorted(os) != AMI_ERROR_NO_ERROR)
	cerr << argv[0] << ": Error during bulk loading.\n";
      else
	cout << "Done. " << wt << "\n";
      wt.stop();
    }
    os->persist(PERSIST_DELETE);
    delete os;
    ///    wt.stop();
    cout << "END Bulk Load " /*<< wt*/ << "\n";
  }

  delete is;

  cout << "Tree size: " << u_btree->size() << " elements. Tree height: " 
       << u_btree->height() << ".\n";
  wt.reset();

  delete u_btree;

  //////  Testing insertion.  //////

  //  u_btree_t *u_btree;
  params.leaf_cache_size = 16;
  params.node_cache_size = 64;

  u_btree = (base_file == NULL) ? new u_btree_t(params): 
    new u_btree_t(base_file, AMI_WRITE_COLLECTION, params);

  if (!u_btree->is_valid()) {
    cerr << argv[0] << ": Error reinitializing btree. Aborting.\n";
    delete u_btree;
    exit(1);
  }

  cout << "BEGIN Insert\n";
  cout << "\tInserting " << insert_count << " elements.\n";
  cout << "\t" << flush;
  wt.start();
  for (i = 1; i <= insert_count; i++) {
    if (i <= DELETE_COUNT)
      s[i-1] = ss = el_t(i+100000);
    else
      ss = el_t(long((TPIE_OS_RANDOM()/MAX_RANDOM) * MAX_VALUE));
    u_btree->insert(ss);
    if (i % (insert_count/10) == 0)
      cout << i << " " << flush;
  }
  cout << "\n";
  wt.stop();
  cout << "END Insert " << wt << "\n";
  
  cout << "Tree size: " << u_btree->size() << " elements. Tree height: " 
       << u_btree->height() << ".\n";
  wt.reset();


  //////  Testing range query.  ///////

  cout << "BEGIN Search\n";
  cout << "\tSearching with range [" << range_search_lo << ", " 
       << range_search_hi << "]\n";

  stream_t* os = new stream_t;
  wt.start();
  u_btree->range_query(range_search_lo, range_search_hi, os);
  wt.stop();
  cout << "\tFound " << os->stream_len() << " elements.\n";
  delete os;
  cout << "END Search " << wt << "\n";


  ///////  Testing erase.  ///////

  cout << "BEGIN Delete\n";
  cout << "\tDeleting " << key_from_el()(s[0]) << " through " 
       <<  key_from_el()(s[DELETE_COUNT-1]) << ": \n";
  j = 0;
  for (i = 0; i < DELETE_COUNT ; i++) {
    if (u_btree->erase(key_from_el()(s[i])))
      j++;
  }

  cout << "\t\tfound " << j << " keys. ";
  if (j == DELETE_COUNT)
    cout << "(OK)\n";
  else
    cout << "(Potential problem!)\n";
  
  cout << "\tDeleting " << (long)-1 << flush;
  if (u_btree->erase((long)-1))
    cout << ": found. (Potential problem!)\n";
  else
    cout << ": not found. (OK)\n";

  cout << "\tDeleting " <<  key_from_el()(s[0]) << flush;
  if (u_btree->erase(key_from_el()(s[0])))
    cout << ": found. (Potential problem!)\n";
  else
    cout << ": not found. (OK)\n";
  cout << "END Delete\n";
  

  cout << "Tree size: " << u_btree->size() << " elements. Tree height: " 
       << u_btree->height() << ".\n";

  tpie_stats_tree bts = u_btree->stats();
  delete u_btree;
  
  cout << "Block collection statistics (global):\n"
       << "\tGET BLOCK:    "
       << AMI_COLLECTION::gstats().get(BLOCK_GET) << "\n"
       << "\tPUT BLOCK:    "
       << AMI_COLLECTION::gstats().get(BLOCK_PUT) << "\n"
       << "\tNEW BLOCK     "
       << AMI_COLLECTION::gstats().get(BLOCK_NEW) << "\n"
       << "\tDELETE BLOCK: "
       << AMI_COLLECTION::gstats().get(BLOCK_DELETE) << "\n"
    ;
  cout << "Tree statistics:\n"
       << "\tREAD (LEAF+NODE):    " 
       << bts.get(LEAF_READ) + bts.get(NODE_READ) << "\n"
       << "\tCREATE (LEAF+NODE):  " 
       << bts.get(LEAF_CREATE) + bts.get(NODE_CREATE) << "\n"
       << "\tFETCH (LEAF+NODE):   " 
       << bts.get(LEAF_FETCH) + bts.get(NODE_FETCH) << "\n"
       << "\tWRITE (LEAF+NODE):   " 
       << bts.get(LEAF_WRITE) + bts.get(NODE_WRITE) << "\n"
       << "\tDELETE (LEAF+NODE):  " 
       << bts.get(LEAF_DELETE) + bts.get(NODE_DELETE) << "\n"
       << "\tRELEASE (LEAF+NODE): " 
       << bts.get(LEAF_RELEASE) + bts.get(NODE_RELEASE) << "\n"
    ;
  cout << "Stream statistics (global):\n"
       << "\tREAD ITEM:    "
       << stream_t::gstats().get(ITEM_READ) << "\n"
       << "\tWRITE ITEM:   "
       << stream_t::gstats().get(ITEM_WRITE) << "\n"
       << "\tSEEK ITEM:    "
       << stream_t::gstats().get(ITEM_SEEK) << "\n"
       << "\tREAD BLOCK:   "
       << stream_t::gstats().get(BLOCK_READ) << "\n"
       << "\tWRITE BLOCK:  "
       << stream_t::gstats().get(BLOCK_WRITE) << "\n"
    ;

  return 0;
}
