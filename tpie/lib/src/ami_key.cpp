// Copyright (c) 1995 Darren Erik Vengroff
//
// File: ami_key.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 3/12/95
//

#include <versions.h>
#include <ami_key.h>

VERSION(ami_key_cpp,"$Id: ami_key.cpp,v 1.2 2000-01-10 21:38:58 hutchins Exp $");


key_range::key_range(kb_key min_key, kb_key max_key) :
        min(min_key), max(max_key)
{
}

key_range::key_range(void) :
        min(0), max(0)
{
}

