// Copyright (c) 1994 Darren Erik Vengroff
//
// File: mm.h (plus contents from mm_imps.h, now deprecated)
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/30/94
//
// $Id: mm.h,v 1.4 2005-11-08 17:21:02 adanner Exp $
//
#ifndef _MM_H
#define _MM_H

// Get the base class, enums, etc...
#include <mm_base.h>

// Get an implementation definition

// For now only single address space memory management is supported.
#ifdef MM_IMP_REGISTER
#include <mm_register.h>
#else
#error No MM implementation selected.
#endif

#endif // _MM_H 
