// Copyright (c) 1994 Darren Erik Vengroff
//
// File: ami_sort.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 6/10/94
//
// $Id: ami_sort.h,v 1.8 2000-04-17 02:03:31 hutchins Exp $
//
#ifndef _AMI_SORT_H
#define _AMI_SORT_H

#define CONST const

#ifdef AMI_IMP_SINGLE
#include <ami_sort_single.h>
#include <ami_optimized_sort.h>
#include <ami_sort_single_dh.h>
#endif

#endif // _AMI_SORT_H 
