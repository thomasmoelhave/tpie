// Copyright (c) 1994 Darren Erik Vengroff
//
// File: parse_args.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/7/94
//
// $Id: parse_args.cpp,v 1.1 2003-11-21 17:01:09 tavi Exp $
//

#include <unistd.h>
#include <string.h>

#include "app_config.h"
#include "parse_args.h"

void parse_args(int argc, char **argv, const char *as_opts,
                void (*parse_app_opt)(char opt, char *optarg))
{
    static char standard_opts[] = "";

    // All options, stnadard and application specific.
    char *all_opts;

    if (as_opts != NULL) {
        unsigned int l_aso;
        
        all_opts = new char[sizeof(standard_opts) + (l_aso = strlen(as_opts))];        strncpy(all_opts, standard_opts, sizeof(standard_opts));
        strncat(all_opts, as_opts, l_aso);
    } else {
        all_opts = standard_opts;
    }

    ///GetOpt go(argc, argv, all_opts);
    char c;

    ///while ((c = go()) != -1) {
    optarg = NULL;
    while((c = getopt(argc, argv, all_opts)) != -1) {
        switch (c) {
            default:
                parse_app_opt(c, optarg);
                break;
        }
    }
}

