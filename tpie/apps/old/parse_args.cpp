// Copyright (c) 1994 Darren Erik Vengroff
//
// File: parse_args.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/7/94
//


static char parse_args_id[] = "$Id: parse_args.cpp,v 1.9 2001-02-12 16:27:11 tavi Exp $";

#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <strstream.h>
#include <ctype.h>
#include <unistd.h>

#include "app_config.h"
#include "parse_args.h"

static size_t
parse_number(char *s) {
  size_t n, mult=1;
  int len = strlen(s);
  if(isalpha(s[len-1])) {
    switch(s[len-1]) {
    case 'M':
      mult = 1 << 20;
      break;
    case 'K':
      mult = 1 << 10;
      break;
    default:
      cerr << "bad number format: " << s << endl;
      exit(-1);
      break;
    }
    s[len-1] = '\0';
  }
  n = atol(s);
  return n * mult;
}

void parse_args(int argc, char **argv, const char *as_opts,
                void (*parse_app_opt)(char opt, char *optarg))
{
    static char standard_opts[] = "m:t:z:v";

    // All options, stnadard and application specific.
    char *all_opts;

    if (as_opts != NULL) {
      unsigned int l_aso;
      
      all_opts = new char[sizeof(standard_opts) + (l_aso = strlen(as_opts))]; 
      strncpy(all_opts, standard_opts, sizeof(standard_opts));
      strncat(all_opts, as_opts, l_aso);
    } else {
      all_opts = standard_opts;
    }

    char c;

    optarg = NULL;
    while((c = getopt(argc, argv, all_opts)) != -1) {
      switch (c) {
      case 'v':
	verbose = 1;
	break;
      case 'm':
	test_mm_size = parse_number(optarg);
	break;                
      case 't':
	test_size = parse_number(optarg);
	break;
      case 'z':
	random_seed = atol(optarg);
	break;
      default:
	parse_app_opt(c, optarg);
	break;
      }
    }
}

