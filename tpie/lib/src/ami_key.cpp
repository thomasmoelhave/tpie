// Copyright (c) 1995 Darren Erik Vengroff
//
// File: ami_key.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 3/12/95
//

#include <versions.h>
#include <ami_key.h>

VERSION(ami_key_cpp,"$Id: ami_key.cpp,v 1.5 2005-11-17 17:23:44 jan Exp $");

// Initializer are not possible due to an namespace ambiguity
// with STL min/max. Need to rewrite this class (add selectors)

key_range::key_range(void) {
       this->min=0;
       this->max=0;
}

key_range::key_range(kb_key min_key, kb_key max_key) {
    this->min = min_key;
    this->max = max_key;
}


