// Copyright (c) 1994 Darren Erik Vengroff
//
// File: ami.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/19/94
//
// $Id: ami.h,v 1.14 1999-02-03 17:03:48 tavi Exp $
//
#ifndef _AMI_H
#define _AMI_H

#ifndef AMI_VIRTUAL_BASE
#define AMI_VIRTUAL_BASE 0
#endif

// Include the configuration header.
#include <config.h>

// Some basic constants

// The name of the environment variable pointing to a tmp directory.
#define TMP_DIR_ENV "TMP"

// The name of a tmp directory to use if the env variable is not set.
#define TMP_DIR "/var/tmp"

// Get the base class, enums, etc...
#include <ami_base.h>

// Get the device description class
#include <ami_device.h>

// Get an implementation definition
#include <ami_imps.h>

// Get templates for ami_scan().
#include <ami_scan.h>

// Get templates for ami_merge().
#include <ami_merge.h>

// Get templates for ami_sort().
#include <ami_sort.h>

// Get templates for general permutation.
#include <ami_gen_perm.h>

// Get templates for bit permuting.
#include <ami_bit_permute.h>

// Get the size of main memory.
extern size_t AMI_mem_size(void);

#endif // _AMI_H 
