//
// File: parse_args.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/7/94
//
// $Id: parse_args.h,v 1.3 2003-09-11 18:45:57 jan Exp $
//
#ifndef _PARSE_ARGS_H
#define _PARSE_ARGS_H

#include "app_config.h"
#include "getopt.h"

void usage(void (*usage_app)(void) = NULL);

// Parse arguments for flags common to all test apps, as well as those
// specific to this particular app.  aso points to a string of app
// specific options, and parse_app is a pointer to a function, tpyically
// just a big switch, to handle them.
void parse_args(int argc, char **argv, const char *aso = NULL,
                void (*parse_app_opt)(char opt, char *optarg) = NULL);
                
#endif // _PARSE_ARGS_H 
