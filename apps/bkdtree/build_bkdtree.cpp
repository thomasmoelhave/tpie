// Copyright (C) 2001-2003 Octavian Procopiuc
//
// File:    build_bkdtree.cpp
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// Not just a test file for the kd-tree, but a full suite for bulk
// loading, querying, printing and benchmarking the kd-tree and the
// log. method classes on the kd-tree. It uses the run-time parameters
// from app_params.cpp
//
// $Id: build_bkdtree.cpp,v 1.1 2003-09-25 17:45:03 tavi Exp $
//

#define DIRECTIO_STREAMS 0

#include <portability.h>

// STL stuff.
#include <functional>
// TPIE configuration: choose BTE, block size, etc.
#include "app_config.h"
// TPIE streams.
#include <ami_stream.h>
// TPIE timer.
#include <cpu_timer.h>
// TPIE tree statistics.
#include <tpie_stats_tree.h>
// Logarithmic method.
#include <ami_logmethod.h>
// The kd-tree. 
#include <ami_kdtree.h>
// Run-time parameters.
#include "app_params.h"


// Two types are available for the internal memory structure (Tree0)
// of the log. method.
#define TREE0_BTREE  0
#define TREE0_VECTOR 1
// Choose one of TREE0_VECTOR and TREE0_BTREE for the in-memory
// structure of the log. method.
#define TREE0_TYPE TREE0_VECTOR

// Template instantiations.

//template class AMI_kdtree_bin_node_default<int, DIM>;
//#define KDTREEintd AMI_kdtree<int, DIM, AMI_kdtree_bin_node_default<int, DIM> >
#define KDTREEintd AMI_kdtree<int, DIM, AMI_kdtree_bin_node_small<int, DIM> >
template class KDTREEintd;

#if (TREE0_TYPE==TREE0_BTREE)
#  include <ami_btree.h>
#  define BTREEintd AMI_btree<app_params_t::point_t, app_params_t::record_t, app_params_t::point_t::cmp, app_params_t::record_key_t>
   template class BTREEintd;
#  define LOGMETHOD2intd Logmethod2<app_params_t::point_t, app_params_t::record_t, KDTREEintd, AMI_kdtree_params, BTREEintd, AMI_btree_params>
#  define LOGMETHODBintd LogmethodB<app_params_t::point_t, app_params_t::record_t, KDTREEintd, AMI_kdtree_params, BTREEintd, AMI_btree_params>
#elif (TREE0_TYPE==TREE0_VECTOR)
#  include "d_vector.h"
#  define D_VECTORintd d_vector<app_params_t::point_t, app_params_t::record_t, app_params_t::record_key_t>
   template class D_VECTORintd;
#  define LOGMETHOD2intd Logmethod2<app_params_t::point_t, app_params_t::record_t, KDTREEintd, AMI_kdtree_params, D_VECTORintd, size_t>
#  define LOGMETHODBintd LogmethodB<app_params_t::point_t, app_params_t::record_t, KDTREEintd, AMI_kdtree_params, D_VECTORintd, size_t>
#endif

template class LOGMETHOD2intd;
template class LOGMETHODBintd;

bool tiger_constrained(const app_params_t::record_t& p) {
  // Constrain TIGER data to continental US.
  // Numbers represent longitude and latitude times 1 million.
  return (p[0] > -128000000 && p[0] < -65000000 && p[1] > 21000000 && p[1] < 50000000);
}


int main(int argc, char** argv) {

  KDTREEintd* kdtree;
  LOGMETHOD2intd *lm2;
  LOGMETHODBintd* lmB;
  AMI_kdtree_params kd_params;
  cpu_timer atimer;
  int i, ii;
  app_params_t::record_t p, q, *pp;
  double load_wall, sort_wall, load_io, sort_io;
  double wq_wall, wq_io;
  AMI_err err = AMI_ERROR_NO_ERROR;

  // <><><><><><><><><><><><><><><><><><><><>
  //    Initialize.
  // <><><><><><><><><><><><><><><><><><><><>

  // Log debugging info from the application, but not from the library. 
  tpie_log_init(TPIE_LOG_APP_DEBUG);
  // Write a warning in the log file when the memory limit is exceeded. 
  MM_manager.warn_memory_limit();
  parse_args(argc, argv);

  params.structure_name = "KD-TREE";

  kd_params.leaf_size_max = params.leaf_size_max;
  kd_params.node_size_max = params.node_size_max;
  kd_params.leaf_block_factor = params.leaf_block_factor;
  kd_params.node_block_factor = params.node_block_factor;
  kd_params.max_intraroot_height = params.max_intraroot_height;
  kd_params.node_cache_size = params.node_cache_size;
  kd_params.leaf_cache_size = params.leaf_cache_size;
  kd_params.grid_size = params.grid_size;

  if (params.do_logmethod) {

#if (TREE0_TYPE==TREE0_BTREE)
    Logmethod_params<AMI_kdtree_params, AMI_btree_params> lm_params;
    lm_params.tree0_params.leaf_block_factor = leaf_block_factor;
    lm_params.tree0_params.node_block_factor = node_block_factor;
    lm_params.tree0_params.node_cache_size = 64;
    lm_params.tree0_params.leaf_cache_size = (cached_blocks+7)/8 * 8;
#elif (TREE0_TYPE==TREE0_VECTOR)
    Logmethod_params<AMI_kdtree_params, size_t> lm_params;
    lm_params.tree0_params = params.cached_blocks;
#endif

    lm_params.cached_blocks = params.cached_blocks;
    lm_params.tree_params = kd_params;
    if (params.B_for_LMB == 0) {
      lm2 = new LOGMETHOD2intd(params.base_file_name_t, lm_params);
      lm2->persist(params.keep_tree ? PERSIST_PERSISTENT : PERSIST_DELETE);
      params.leaf_block_factor = lm2->params().tree_params.leaf_block_factor;
      params.node_block_factor = lm2->params().tree_params.node_block_factor;
    } else {
      LOGMETHODBintd::B = params.B_for_LMB;
      lmB = new LOGMETHODBintd(params.base_file_name_t, lm_params);
      lmB->persist(params.keep_tree ? PERSIST_PERSISTENT : PERSIST_DELETE);
      params.leaf_block_factor = lmB->params().tree_params.leaf_block_factor;
      params.node_block_factor = lmB->params().tree_params.node_block_factor;
    }

  } else {

    kdtree = new KDTREEintd(params.base_file_name_t, AMI_WRITE_COLLECTION, kd_params);
    if (kdtree->status() == AMI_KDTREE_STATUS_INVALID) {
      cerr << "Error opening the kdtree (see log for details). Aborting." << endl;
      delete kdtree;
      exit(1);
    }

    kdtree->persist(params.keep_tree ? PERSIST_PERSISTENT : PERSIST_DELETE);
    params.leaf_block_factor = kdtree->params().leaf_block_factor;
    params.node_block_factor = kdtree->params().node_block_factor;
    add_to_stats(0, "Node capacity:       ", kdtree->params().node_size_max);
    add_to_stats(0, "Leaf capacity:       ", kdtree->params().leaf_size_max);
  }


  // <><><><><><><><><><><><><><><><><><><><>
  //    Sort.
  // <><><><><><><><><><><><><><><><><><><><>
  
  if (params.do_sort && err == AMI_ERROR_NO_ERROR) {
    cerr << "Sorting..." << endl;
    atimer.start();
    if (params.do_logmethod)
      cerr << "Internal error: options do_logmethod and do_sort are incompatible." << endl;
    else
      err = kdtree->sort(params.in_stream, params.streams_sorted);
    atimer.stop();
    cerr << "Sorting timings: " << atimer << endl;
    sort_wall = atimer.wall_time();
    sort_io = sort_wall - atimer.user_time() - atimer.system_time();

    atimer.reset();
    delete params.in_stream;
    params.in_stream = NULL;
    params.stats << "SORT   (Wall IO %IO): "
	  << double(int(sort_wall*100))/100 << "\t "
	  << double(int(sort_io*100))/100 << "\t "
	  << int(sort_io*100/sort_wall) << endl;
  } else {
    //    params.streams_sorted[0] = params.in_stream;
  }

  // <><><><><><><><><><><><><><><><><><><><>
  //   Eliminate duplicates.
  // <><><><><><><><><><><><><><><><><><><><>

  if (params.do_eliminate_duplicates && err == AMI_ERROR_NO_ERROR) {
    // Temporary stream. 
    app_params_t::stream_t *tmps;
    char *sname, *tmpname;
    cerr << "Eliminating duplicates..." << flush;
    assert(params.streams_sorted[0] != NULL && params.streams_sorted[1] != NULL);
    for (ii = 0; ii < DIM; ii++) {
      params.streams_sorted[ii]->seek(0);
      params.streams_sorted[ii]->read_item(&pp);
      params.streams_sorted[ii]->persist(PERSIST_DELETE);
      params.streams_sorted[ii]->name(&sname);

      tmps = new app_params_t::stream_t;
      tmps->persist(PERSIST_PERSISTENT);
      tmps->name(&tmpname);
      if (!params.do_constrain_tiger_data || tiger_constrained(*pp))
	tmps->write_item(*pp);	
      p = *pp;
      while (params.streams_sorted[ii]->read_item(&pp) == AMI_ERROR_NO_ERROR) {
	if (!(p == *pp) && (!params.do_constrain_tiger_data || tiger_constrained(*pp))) {
	  tmps->write_item(*pp);
	  p = *pp;
	}
      }
      delete params.streams_sorted[ii];
      delete tmps;

      char mv_command[250];
      strcpy(mv_command, "/bin/mv ");
      strcat(mv_command, tmpname);
      strcat(mv_command, " ");
      strcat(mv_command, sname);
      if (system(mv_command) == -1) {
	perror("Error renaming the file");
	exit(1);
      }
      params.streams_sorted[ii] = new app_params_t::stream_t(sname, AMI_READ_STREAM);
      params.streams_sorted[ii]->persist(PERSIST_PERSISTENT);
      delete [] sname;
      delete [] tmpname;
    }
    cerr << "Done." << endl; 
  }


  // <><><><><><><><><><><><><><><><><><><><>
  //   Bulk load.
  // <><><><><><><><><><><><><><><><><><><><>

  if (params.do_load && err == AMI_ERROR_NO_ERROR) {
    cerr << "Loading..." << endl;
    atimer.start();
    if (params.do_logmethod)
      cerr << "Error: options do_logmethod and do_load are incompatible." << endl;
    else if (params.load_method & AMI_KDTREE_LOAD_SAMPLE) {
      assert(params.in_stream != NULL);
      err = kdtree->load_sample(params.in_stream);
      delete params.in_stream;
      params.in_stream = NULL;
    } else {
      err = kdtree->load_sorted(params.streams_sorted, 
				params.bulk_load_fill, 
				params.bulk_load_fill,
				params.load_method);
      for (ii = 0; ii < DIM; ii++)
	delete params.streams_sorted[ii];
    }
    atimer.stop();
    //    params.stats << "LOAD STREAM IO (R W M U S SC SD): " 
    //		 << BTE_stream_base_generic::statistics() << endl;
    //    BTE_stream_base_generic::stats_off();
    cerr << "\nLoading timings: " << atimer << endl;
    load_wall = atimer.wall_time();
    load_io = load_wall - atimer.user_time() - atimer.system_time();
    
    atimer.reset();
    params.stats << "Loading method:       " 
		 << (params.load_method & AMI_KDTREE_LOAD_BINARY? "BINARY ": (params.load_method & AMI_KDTREE_LOAD_SAMPLE ? "SAMPLE ": "GRID ")) 
		 << (params.load_method & AMI_KDTREE_LOAD_GRID ? params.grid_size: 0) << endl;
    params.stats << "LOAD:Wall_IO_%IO      "
		 << double(int(load_wall*1000)) / 1000 << "\t "
		 << double(int(load_io*100))/100 << "\t "
		 << int(load_io*100/load_wall) << endl;
    params.stats << "Binary nodes:         "
		 << kdtree->bin_node_count() << endl;
    params.stats << "Max intraroot height: "
		 << kdtree->params().max_intraroot_height << endl;
    params.stats << "Max intranode height: "
		 << kdtree->params().max_intranode_height << endl;
  }

  // <><><><><><><><><><><><><><><><><><><><>
  //    Insert.
  // <><><><><><><><><><><><><><><><><><><><>

  if (params.do_insert && err == AMI_ERROR_NO_ERROR) {
    cerr << "Inserting..." << endl;
    assert(params.in_stream != NULL);
    atimer.start();
    i = 0;
    params.in_stream->seek(0);
    if (params.do_logmethod) {
      if (params.B_for_LMB == 0) {
	while (i < params.point_count && 
	       params.in_stream->read_item(&pp) == AMI_ERROR_NO_ERROR) {
	  lm2->insert(*pp);
	  i++;
	}
      } else {
	while (i < params.point_count && 
	       params.in_stream->read_item(&pp) == AMI_ERROR_NO_ERROR) {
	  lmB->insert(*pp);
	  i++;
	}
      }
    } else {
      while (i < params.point_count && 
	     params.in_stream->read_item(&pp) == AMI_ERROR_NO_ERROR) {
	kdtree->insert(*pp);
	i++;
      }
    }
    atimer.stop();
    cerr << "\tInsert timings: " << atimer << endl;
    load_wall = atimer.wall_time();
    load_io = load_wall - atimer.user_time() - atimer.system_time();
    atimer.reset();
    delete params.in_stream;
    params.stats << "INSERT:WALL_IO_%IO   " 
	  << double(int(load_wall*100)) / 100 << "\t "
	  << double(int(load_io*100)) / 100 << "\t "
	  << (load_wall > 0 ? int(load_io*100/load_wall) : 0) << endl;
  }

  // <><><><><><><><><><><><><><><><><><><><>
  //    Query from ASCII file.
  // <><><><><><><><><><><><><><><><><><><><>
   
  if (params.do_wquery_from_file && err == AMI_ERROR_NO_ERROR) {
    ifstream ifs(params.file_name_wquery);
    if (!ifs) {
      cerr << argv[0] << ": Error opening window queries file " 
	   << params.file_name_wquery << endl;
    } else {
      size_t count = 0, result = 0;
      app_params_t::point_t lop, hip;
      app_params_t::stream_t *tempstr = (params.do_query_count_only ? NULL: new app_params_t::stream_t);
      cerr << "Window queries from file " 
	   << params.file_name_wquery << " ..." << endl;
      atimer.start();  

      for (ii = 0; ii < DIM; ii++)
	ifs >> lop[ii];
      for (ii = 0; ii < DIM; ii++)
	ifs >> hip[ii];

      while (!ifs.eof()) {
	count++;

	if (params.do_logmethod)
	  if (params.B_for_LMB == 0)
	    result += lm2->window_query(lop, hip, tempstr);
	  else
	    result += lmB->window_query(lop, hip, tempstr);
	else
	  if (params.query_type == 3) {
	    //                            _ 
	    // 3-sided query, like this: | |
	    //
	    hip[1] = max(lop[1], hip[1]);
	    lop[1] = -app_params_t::point_t::Inf;
	    result += kdtree->window_query(lop, hip, tempstr);
	  } else
	    result += kdtree->window_query(lop, hip, tempstr);

	for (ii = 0; ii < DIM; ii++)
	  ifs >> lop[ii];
	for (ii = 0; ii < DIM; ii++)
	  ifs >> hip[ii];
      }

      atimer.stop();
      cerr << "\tQuery timings: " << atimer << endl;
      wq_wall = atimer.wall_time();
      wq_io = wq_wall - atimer.user_time() - atimer.system_time();
      cerr << "\tFound " << result << " points." << endl;
      if (tempstr != NULL)
	cerr << "\t  (real: " << tempstr->stream_len() << " points)" << endl;

      atimer.reset();

      add_to_stats(0, "FQUERY:FILE          ", params.file_name_wquery);
      add_to_stats(0, "FQUERY:COUNT         ", count);
      add_to_stats(0, "FQUERY:RESULT        ", result);
      params.stats << "FQUERY:WALL_IO_%IO   "
	    << double(int(wq_wall*1000))/1000 << " "
	    << double(int(wq_io*100))/100 << " "
	    << int(wq_io*100/wq_wall) << endl;

      delete tempstr;
      cerr << "Done." << endl;
    }
  }


  // <><><><><><><><><><><><><><><><><><><><>
  //   Query using random boxes.
  // <><><><><><><><><><><><><><><><><><><><>

  if (params.wquery_count > 0 && err == AMI_ERROR_NO_ERROR) {
    app_params_t::point_t lop, hip;
    size_t result = 0;
    cerr << "Doing " << params.wquery_count << " window queries." << endl;
    int mbrlox, mbrloy, mbrdx, mbrdy;

    if (params.do_logmethod) {

    } else {
      mbrlox = kdtree->mbr_lo()[0];
      mbrloy = kdtree->mbr_lo()[1];
      mbrdx = kdtree->mbr_hi()[0] - kdtree->mbr_lo()[0];
      mbrdy = kdtree->mbr_hi()[1] - kdtree->mbr_lo()[1];
    }

    int wqdx = int(params.wquery_size_x / 100 * mbrdx);
    int wqdy = int(params.wquery_size_y / 100 * mbrdy);
   
    cerr << "  width: " << params.wquery_size_x << "% of " << mbrdx
	 << ", height: " << params.wquery_size_y << "% of " << mbrdy << endl;
    app_params_t::stream_t *tempstr = new app_params_t::stream_t;

    TPIE_OS_SRANDOM(time(NULL));
    atimer.start();  
    for (i = 0; i < params.wquery_count; i++) {
      lop[0] = TPIE_OS_RANDOM() % mbrdx + mbrlox - wqdx / 2;
      lop[1] = TPIE_OS_RANDOM() % mbrdy + mbrloy - wqdy / 2;
      hip[0] = lop[0] + wqdx;
      hip[1] = lop[1] + wqdy;
      if (params.do_logmethod)
	if (params.B_for_LMB == 0)
	  result += lm2->window_query(lop, hip, tempstr);
	else
	  result += lmB->window_query(lop, hip, tempstr);
      else
	kdtree->window_query(lop, hip, tempstr);
    }
    atimer.stop();
    cerr << "Query timings: " << atimer << endl;
    wq_wall = atimer.wall_time();
    wq_io = wq_wall - atimer.user_time() - atimer.system_time();
    cerr << "Found " << tempstr->stream_len() << " points." << endl;
    atimer.reset();

    add_to_stats(0, "RQUERY:Size_x        ", params.wquery_size_x);
    add_to_stats(0, "RQUERY:Size_y        ", params.wquery_size_y);
    add_to_stats(0, "RQUERY:Count         ", params.wquery_count);
    add_to_stats(0, "RQUERY:Result        ", tempstr->stream_len());
    add_to_stats(4, "RQUERY:Wall          ", double(int(wq_wall*100))/100);
    add_to_stats(6, "RQUERY:IO            ", double(int(wq_io*100))/100);
    add_to_stats(2, "RQUERY:PercentIO     ", int(wq_io*100/wq_wall));
    delete tempstr;
  }


  // <><><><><><><><><><><><><><><><><><><><>
  //    Clean up and print statistics.
  // <><><><><><><><><><><><><><><><><><><><>
 
  tpie_stats_tree bts;

  if (params.do_logmethod) {
    if (params.B_for_LMB == 0) {
      params.point_count = lm2->size();
      bts = lm2->stats();
      delete lm2;
    } else {
      params.point_count = lmB->size();
      bts = lmB->stats();
      delete lmB;
    }
  } else {
    params.point_count = kdtree->size();
    bts = kdtree->stats();
    if (params.do_print_tree)
      kdtree->print(cout);
    delete kdtree;
  }

  if (err == AMI_ERROR_NO_ERROR) {
    params.write_block_stats(bts);

    print_configuration();
    print_statistics();
    
    // Write statistics to a file (append).
    ofstream ofs(params.file_name_stats, ios::app);
    
    print_configuration(ofs);
    print_statistics(ofs);
  } else {
    cerr << "An error ocurred. See log for details." << endl;
  }

  return 0;
}
