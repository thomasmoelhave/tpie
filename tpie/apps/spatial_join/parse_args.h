// Copyright (c) 1994 Darren Erik Vengroff
//
// File:         parse_args.h
// Author:       Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created:      10/7/94
// Description:  Declaration of function parse_args()
//
// $Id: parse_args.h,v 1.1 2003-11-21 17:01:09 tavi Exp $
//
#ifndef _PARSE_ARGS_H
#define _PARSE_ARGS_H

#include "app_config.h"

// Parse arguments for flags common to all test apps, as well as those
// specific to this particular app.  aso points to a string of app
// specific options, and parse_app is a pointer to a function, tpyically
// just a big switch, to handle them.

void parse_args(int argc, char **argv, const char *aso = NULL,
                void (*parse_app)(char opt, char *optarg) = NULL);
                
#endif // _PARSE_ARGS_H 
