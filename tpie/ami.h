// Copyright (c) 1994 Darren Erik Vengroff
//
// File: ami.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/19/94
//
// $Id: ami.h,v 1.19 2003-04-17 11:59:40 jan Exp $
//
#ifndef _AMI_H
#define _AMI_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// Get a stream implementation.
#include <tpie/stream.h>

// Get templates for ami_scan().
#include <tpie/scan.h>

// Get templates for ami_merge().
#include <tpie/merge.h>

// Get templates for ami_sort().
#include <tpie/sort.h>

// Get templates for general permutation.
#include <tpie/gen_perm.h>

// Get templates for bit permuting.
//#include <bit_permute.h>

// Get a collection implementation.
#include <tpie/coll.h>

// Get a block implementation.
#include <tpie/block.h>

// Get templates for AMI_btree.
#include <tpie/btree.h>

#endif // _AMI_H 
