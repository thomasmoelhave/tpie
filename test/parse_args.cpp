//
// File: parse_args.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/7/94
//
#include "parse_args.h"
#include <cstdlib>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <algorithm>

// The new getopts() argument-parsing function.
#include "getopts.h"
#include <tpie/stream.h>

static TPIE_OS_OFFSET parse_number(char *s) {
  TPIE_OS_OFFSET n; 
  TPIE_OS_OFFSET mult = 1;
  size_t len = strlen(s);
  if(isalpha(s[len-1])) {
    switch(s[len-1]) {
    case 'G':
    case 'g':
      mult = 1 << 30;
      break;
    case 'M':
    case 'm':
      mult = 1 << 20;
      break;
    case 'K':
    case 'k':
      mult = 1 << 10;
      break;
    default:
      std::cerr << "Error parsing arguments: bad number format: " << s << "\n";
      exit(-1);
      break;
    }
    s[len-1] = '\0';
  }
  n = TPIE_OS_OFFSET(atof(s) * mult);
  return n;
}

void parse_args(int argc, char **argv, struct options *application_opts,
		void (*parse_app_opts)(int idx, char *opt_arg), bool stop_if_no_args) {
  bool do_exit = false;

  static struct options standard_opts[] = {
    { 1, "memory", "Set TPIE memory limit", "m", 1 },
    { 2, "verbose", "Set verbose flag", "v", 0 },
    { 3, "test-size", "Set test input size (no. of items)", "t", 1 },
    { 4, "random-seed", "Set random seed", "z", 1 },
  };
  static struct options null_opt = { 0,  NULL, NULL, NULL, 0 };

  static size_t l_std_o = 4;
  size_t l_app_o = 0;
  while (application_opts[l_app_o].number != 0) {
    ++l_app_o;
  }
  struct options *all_opts;
  if (application_opts == NULL && l_app_o != 0) {
    std::cerr << "Error parsing arguments: NULL pointer to options array." << std::endl;
    return;
  }
  
  size_t l_all_o = l_app_o + l_std_o + 1; // add 1 for the null option.

  all_opts = new struct options[l_all_o];
  size_t i;
  for (i = 0; i < l_std_o; i++) {
    all_opts[i] = standard_opts[i];
  }
  for (i = 0; i < l_app_o; i++) {
    all_opts[i+l_std_o] = application_opts[i];
  }
  all_opts[l_app_o+l_std_o] = null_opt;

  if (stop_if_no_args && argc == 1) {
    getopts_usage(argv[0], all_opts);
    exit(0);
  }

  int idx;
  TPIE_OS_SIZE_T mm_sz = DEFAULT_TEST_MM_SIZE;
  unsigned int rnd_seed = DEFAULT_RANDOM_SEED;
  char *opt_arg;

  while ((idx = getopts(argc, argv, all_opts, &opt_arg)) != 0) {
    switch (idx) {
    case 1: 
        // mm_size should be small.
      mm_sz = std::max(size_t(128*1024), static_cast<TPIE_OS_SIZE_T>(parse_number(opt_arg)));
      break;
    case 2:
      verbose = true; 
     TP_LOG_APP_DEBUG_ID("Setting verbose flag.");
      break;
    case 3: 
      test_size = parse_number(opt_arg); 
      break;
    case 4: 
	rnd_seed = std::max(1u, static_cast<unsigned int>(parse_number(opt_arg)));
      break;
    default:
      parse_app_opts(idx, opt_arg);
      break;
    }
  }

  // Set memory limit.
  MM_manager.set_memory_limit(mm_sz);
  //LOG_APP_DEBUG_ID2("Setting TPIE memory size to: ", mm_sz);

  TPIE_OS_SRANDOM(rnd_seed);
  //LOG_APP_DEBUG_ID2("Setting random seed to: ", rnd_seed);
  
  delete [] all_opts;
  if (do_exit) {
    exit(0);
  }
}

