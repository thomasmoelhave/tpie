// Copyright (c) 1995 Darren Erik Vengroff
//
// File: persist.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 4/7/95
//
// $Id: persist.h,v 1.1 1995-06-20 18:43:25 darrenv Exp $
//
// Persistence flags for TPIE streams.
//
#ifndef _PERSIST_H
#define _PERSIST_H


enum persistence {
    // Delete the stream from the disk when it is destructed.
    PERSIST_DELETE = 0,
    // Do not delete the stream from the disk when it is destructed.
    PERSIST_PERSISTENT,
    // Delete each block of data from the disk as it is read.
    PERSIST_READ_ONCE
};

#endif // _PERSIST_H 
