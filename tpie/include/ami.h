// Copyright (c) 1994 Darren Erik Vengroff
//
// File: ami.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/19/94
//
// $Id: ami.h,v 1.6 1994-10-13 17:00:27 darrenv Exp $
//
#ifndef _AMI_H
#define _AMI_H

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

// Get templates for AMI_distribution_sweep().
#include <ami_dist_sweep.h>

// Get the size of main memory.
extern size_t AMI_mem_size(void);

#endif // _AMI_H 
