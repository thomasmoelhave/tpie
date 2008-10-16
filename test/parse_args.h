//
// File: parse_args.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/7/94
//
// $Id: parse_args.h,v 1.5 2005-02-15 00:20:03 tavi Exp $
//
#ifndef _PARSE_ARGS_H
#define _PARSE_ARGS_H

#include "app_config.h"
#include <portability.h>
#include "getopts.h"


// Parse arguments for flags common to all test apps, as well as those
// specific to this particular app.  aso points to a string of app
// specific options, and parse_app is a pointer to a function, tpyically
// just a big switch, to handle them.
void parse_args(int argc, char **argv, struct options *application_opts,
		void (*parse_app_opts)(int idx, char *opt_arg), 
		bool stop_if_no_args = true);              
#endif // _PARSE_ARGS_H 
