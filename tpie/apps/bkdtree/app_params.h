// Copyright (C) 2001 Octavian Procopiuc
//
// File:    app_params.h
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// Runtime parameters for the kd-tree, K-D-B-tree and B-tree test
// suite.
//
// $Id: app_params.h,v 1.8 2004-11-17 22:45:30 adanner Exp $


#ifndef _APP_PARAMS_H
#define _APP_PARAMS_H

#include <portability.h>

// For STL's max()
#include <algorithm>
// TPIE streams.
#include <ami_stream.h>
// TPIE tree statistics.
#include <tpie_stats_tree.h>
// For AMI_KDTREE_GRID_SIZE, AMI_KDTREE_LOAD_SORT, AMI_KDTREE_LOAD_GRID.
// For split_heuristic_t.
#include <ami_kd_base.h>

#define MAX_PATH_LENGTH 122
#define MAX_VALUE 1000000000
#define MAX_RANDOM ((double)0x7fffffff)

// Default dimension.
#ifndef DIM
#define DIM 2
#endif

// Runtime parameters.
class app_params_t {
public:
  typedef int coord_t;
  typedef AMI_STREAM<AMI_record<coord_t, size_t, DIM> > stream_t;
  typedef AMI_point<coord_t, DIM> point_t;
  typedef AMI_record<coord_t, size_t, DIM> record_t;
  typedef AMI_record_key<coord_t, size_t, DIM> record_key_t;

  record_t mbr_lo;
  record_t mbr_hi;
  TPIE_OS_OFFSET point_count;
  int load_method;
  TPIE_OS_SIZE_T memory_limit;
  char base_file_name[MAX_PATH_LENGTH];
  char base_file_name_s[MAX_PATH_LENGTH];
  char base_file_name_t[MAX_PATH_LENGTH];
  char file_name_stats[MAX_PATH_LENGTH];
  char file_name_wquery[MAX_PATH_LENGTH];
  char nodup_file_name[MAX_PATH_LENGTH];
  char* structure_name;
  bool do_sort;
  bool do_load;
  bool do_wquery_from_file;
  bool do_verify_sorting;
  bool do_eliminate_duplicates;
  bool do_constrain_tiger_data;
  bool do_stress_test;
  bool do_print_tree;
  bool do_display;
  bool do_insert;
  bool do_logmethod;
  bool keep_tree;
  bool do_query_count_only;
  bool direct_io_bcc;
  double wquery_size_x;
  double wquery_size_y;
  float child_cache_fill;
  float bulk_load_fill;
  TPIE_OS_SIZE_T query_type;
  TPIE_OS_OFFSET wquery_count;
  TPIE_OS_SIZE_T grid_size;
  TPIE_OS_SIZE_T leaf_block_factor;
  TPIE_OS_SIZE_T node_block_factor;
  TPIE_OS_SIZE_T catalog_block_factor;
  TPIE_OS_SIZE_T leaf_size_max;
  TPIE_OS_SIZE_T node_size_max;
  TPIE_OS_SIZE_T max_intraroot_height;
  TPIE_OS_SIZE_T cached_blocks;
  TPIE_OS_SIZE_T node_cache_size;
  TPIE_OS_SIZE_T leaf_cache_size;
  TPIE_OS_SIZE_T B_for_LMB;
  stream_t *in_stream;
  stream_t *streams_sorted[DIM];
  ostringstream stats;
  split_heuristic_t split_heuristic;


  // Set default values.
  app_params_t(): mbr_lo(), mbr_hi() {
    point_count = 0;
    structure_name = "UNKNOWN"; // The application should override this.
    load_method = AMI_KDTREE_LOAD_SORT | AMI_KDTREE_LOAD_GRID;
    memory_limit = 64*1024*1024;
    do_sort = true;
    do_load = true;
    do_wquery_from_file = false;
    do_verify_sorting = false;
    do_eliminate_duplicates = false;
    do_constrain_tiger_data = false;
    do_stress_test = false;
    do_print_tree = false;
    do_display = false;
    do_insert = false;
    do_logmethod = false;
    keep_tree = false;
    do_query_count_only = false;
    direct_io_bcc = false; 
    wquery_size_x = 1.0;
    wquery_size_y = 1.0;
    // EPS-tree child cache fill factor.
    child_cache_fill = (float)1.0;
    // Bulk load fill factor. Applies to both leaves and nodes.
    bulk_load_fill = (float)0.998;
    // Query type. If 0, use native query, ie, 4-sided for kdtree,
    // 3-sided for epstree, 2-sided for Btree.
    query_type = 0;
    wquery_count = 0; // refers to the random window queries.
    grid_size = AMI_KDTREE_GRID_SIZE;
    leaf_block_factor = max((TPIE_OS_SIZE_T)16384/TPIE_OS_BLOCKSIZE(),
                            (TPIE_OS_SIZE_T)1);
    node_block_factor = max((TPIE_OS_SIZE_T)16384/TPIE_OS_BLOCKSIZE(),
                            (TPIE_OS_SIZE_T)1);
    // For the EPS-tree catalog nodes. A value of 0 means 2*node_block_factor.
    catalog_block_factor = 0;
    leaf_size_max = 0;
    node_size_max = 0;
    max_intraroot_height = 0;
    cached_blocks = 128;  // for the log. method.
    node_cache_size = 128; // max. number of nodes to be cached.
    leaf_cache_size = 128;  // max number of leaves to be cached.
    B_for_LMB = 0; // for LogmethodB.
    in_stream = NULL;
    for (int i = 0; i < DIM; i++)
      streams_sorted[i] = NULL;
    split_heuristic = CYCLICAL;
  }

  void write_block_stats(const tpie_stats_tree& bts) {
    // Shortcuts.
    TPIE_OS_SIZE_T lbf = leaf_block_factor;
    TPIE_OS_SIZE_T nbf = node_block_factor;
    stats << "BLOCKS:READ           " 
	  << bts.get(LEAF_READ) * lbf + bts.get(NODE_READ)  * nbf
	  << "\t l" << bts.get(LEAF_READ) 
	  << " x" << (TPIE_OS_OFFSET)lbf
	  << "\t n" << bts.get(NODE_READ)
	  << " x" << (TPIE_OS_OFFSET)nbf
	  << endl
	  << "BLOCKS:CREATE         " 
	  << bts.get(LEAF_CREATE) * lbf + bts.get(NODE_CREATE) *nbf
	  << "\t l" << bts.get(LEAF_CREATE) 
	  << " x" << (TPIE_OS_OFFSET)lbf
	  << "\t n" << bts.get(NODE_CREATE) 
	  << " x" << (TPIE_OS_OFFSET)nbf
	  << endl
	  << "BLOCKS:FETCH          " 
	  << bts.get(LEAF_FETCH) * lbf + bts.get(NODE_FETCH) * nbf
	  << "\t l" << bts.get(LEAF_FETCH) 
	  << " x" << (TPIE_OS_OFFSET)lbf
	  << "\t n" << bts.get(NODE_FETCH)
	  << " x" << (TPIE_OS_OFFSET)nbf
	  << endl
	  << "BLOCKS:WRITE          " 
	  << bts.get(LEAF_WRITE) * lbf + bts.get(NODE_WRITE) * nbf 
	  << "\t l" << bts.get(LEAF_WRITE) 
	  << " x" << (TPIE_OS_OFFSET)lbf
	  << "\t n" << bts.get(NODE_WRITE)
	  << " x" << (TPIE_OS_OFFSET)nbf
	  << endl
	  << "BLOCKS:DELETE         " 
	  << bts.get(LEAF_DELETE) * lbf + bts.get(NODE_DELETE)  * nbf
	  << "\t l" << bts.get(LEAF_DELETE) 
	  << " x" << (TPIE_OS_OFFSET)lbf
	  << "\t n" << bts.get(NODE_DELETE)
	  << " x" << (TPIE_OS_OFFSET)nbf
	  << endl
	  << "BLOCKS:RELEASE        " 
	  << bts.get(LEAF_RELEASE) * lbf + bts.get(NODE_RELEASE) * nbf 
	  << "\t l" << bts.get(LEAF_RELEASE)
	  << " x" << (TPIE_OS_OFFSET)lbf
	  << "\t n" << bts.get(NODE_RELEASE)
	  << " x" << (TPIE_OS_OFFSET)nbf
	  << endl
	  << "BLOCKS:COUNT          "
	  << bts.get(LEAF_COUNT) * lbf + bts.get(NODE_COUNT) * nbf
	  << "\t l" << bts.get(LEAF_COUNT)
	  << " x" << (TPIE_OS_OFFSET)lbf
	  << "\t n" << bts.get(NODE_COUNT)
	  << " x" << (TPIE_OS_OFFSET)nbf
	  << endl
      ;
  }
};

// The global parameters.
extern app_params_t params;

void usage(char* argv0);
void print_configuration(ostream& os = cerr);
void print_statistics(ostream& os = cerr);
void parse_args(int argc, char** argv);

template<class T> 
void add_to_stats(TPIE_OS_SIZE_T width, const char* header, T value) {
    params.stats << header << " " << value << endl;
}


#endif //_APP_PARAMS_H
