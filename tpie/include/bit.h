// Copyright (c) 1994 Darren Vengroff
//
// File: bit.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/4/94
//
// $Id: bit.h,v 1.4 2003-04-20 06:38:39 tavi Exp $
//
#ifndef _BIT_H
#define _BIT_H

// Get definitions for working with Unix and Windows
#include <portability.h>

#include <iostream>

// A bit with two operarators, addition (= XOR) and multiplication (=
// AND).
class bit {
private:
    char data;
public:
    bit(void);
    bit(bool);
    bit(int);
    bit(long int);
    ~bit(void);

    operator bool(void);
    operator int(void);
    operator long int(void);
    
    bit operator+=(bit rhs);
    bit operator*=(bit rhs);
    
    friend bit operator+(bit op1, bit op2);
    friend bit operator*(bit op1, bit op2);

    friend std::ostream &operator<<(std::ostream &s, bit b);
};

#endif // _BIT_H 
