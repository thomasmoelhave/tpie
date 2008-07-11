// Copyright (c) 1994 Darren Erik Vengroff
//
// File: app_config.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/7/94
//

static char app_config_id[] = "$Id: app_config.cpp,v 1.5 2004-08-12 12:37:04 jan Exp $";

#include "app_config.h"

bool verbose = true;
TPIE_OS_SIZE_T test_mm_size = DEFAULT_TEST_MM_SIZE;
TPIE_OS_OFFSET test_size = DEFAULT_TEST_SIZE;
int random_seed = 17;

