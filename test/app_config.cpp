// Copyright (c) 1994 Darren Erik Vengroff
//
// File: app_config.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/7/94
//

static char app_config_id[] = "$Id: app_config.cpp,v 1.6 2004-02-05 17:49:10 jan Exp $";

#include "app_config.h"

#ifdef NDEBUG
bool verbose = false;
#else
bool verbose = true;
#endif

size_t test_mm_size = DEFAULT_TEST_MM_SIZE;
size_t test_size = DEFAULT_TEST_SIZE;
int random_seed = 17;

