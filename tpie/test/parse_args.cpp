// Copyright (c) 1994 Darren Erik Vengroff
//
// File: parse_args.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/7/94
//



static char parse_args_id[] = "$Id: parse_args.cpp,v 1.2 1995-01-10 16:47:10 darrenv Exp $";

#include <GetOpt.h>
#include <strstream.h>

#include "app_config.h"

#include "parse_args.h"

void parse_args(int argc, char **argv, const char *as_opts,
                void (*parse_app_opt)(char opt, char *optarg))
{
    static char standard_opts[] = "m:t:z:v";

    // All options, stnadard and application specific.
    char *all_opts;

    if (as_opts != NULL) {
        unsigned int l_aso;
        
        all_opts = new char[sizeof(standard_opts) + (l_aso = strlen(as_opts))];        strncpy(all_opts, standard_opts, sizeof(standard_opts));
        strncat(all_opts, as_opts, l_aso);
    } else {
        all_opts = standard_opts;
    }

    GetOpt go(argc, argv, all_opts);
    char c;

    while ((c = go()) != -1) {
        switch (c) {
            case 'v':
                verbose = !verbose;
                break;
            case 'm':
                istrstream(go.optarg,strlen(go.optarg)) >> test_mm_size;
                break;                
            case 't':
                istrstream(go.optarg,strlen(go.optarg)) >> test_size;
                break;
            case 'z':
                istrstream(go.optarg,strlen(go.optarg)) >> random_seed;
                break;
            default:
                parse_app_opt(c, go.optarg);
                break;
        }
    }
}

