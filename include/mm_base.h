// Copyright (c) 1994 Darren Erik Vengroff
//
// File: mm_base.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/30/94
//
// $Id: mm_base.h,v 1.1 1994-06-03 13:27:30 dev Exp $
//
#ifndef _MM_BASE_H
#define _MM_BASE_H

// MM Error codes
enum MM_err {
    MM_ERROR_NO_ERROR = 0,
    MM_ERROR_INSUFFICIENT_SPACE,
    MM_ERROR_UNDERFLOW
};

// The base class for all memory management objects.
class MM_base_manager
{
};

#endif // _MM_BASE_H 
