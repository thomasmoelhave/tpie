// Copyright (c) 1994 Darren Vengroff
//
// File: bit.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/4/94
//
// $Id: bit.h,v 1.1 1994-12-16 21:47:41 darrenv Exp $
//
#ifndef _BIT_H
#define _BIT_H

#include <ostream.h>

// A bit with two operarators, addition (= XOR) and multiplication (=
// AND).
class bit {
private:
    char data;
public:
    bit(void);
    bit(bool);
    bit(int);
    ~bit(void);

    bit operator+=(bit rhs);
    bit operator*=(bit rhs);
    
    friend bit operator+(bit op1, bit op2);
    friend bit operator*(bit op1, bit op2);

    friend ostream &operator<<(ostream &s, bit b);
};

#endif // _BIT_H 
