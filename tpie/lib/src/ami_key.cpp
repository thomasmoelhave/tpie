// Copyright (c) 1995 Darren Erik Vengroff
//
// File: ami_key.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 3/12/95
//


static char ami_radix_dist_id[] = "$Id: ami_key.cpp,v 1.1 1995-06-20 18:39:26 darrenv Exp $";

#include <ami_key.h>

key_range::key_range(kb_key min_key, kb_key max_key) :
        min(min_key), max(max_key)
{
}

key_range::key_range(void) :
        min(0), max(0)
{
}

