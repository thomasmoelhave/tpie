// Copyright (c) 1994 Darren Vengroff
//
// File: bit.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/4/94
//

#include <versions.h>
VERSION(bit_cpp,"$Id: bit.cpp,v 1.6 2005-11-17 17:04:22 jan Exp $");

#include <bit.h>

using namespace tpie;

bit::bit(void) : data(0) {
    //  No code in this constructor.
}

bit::bit(bool b) : data(0) {
    data = (b == true);
}

bit::bit(int i) : data(0) {
    data = (i != 0);
}

bit::bit(long int i) : data(0) {
    data = (i != 0);
}

bit::operator bool(void) {
    return (data != 0);
}
        
bit::operator int(void) {
    return data;
}
        
bit::operator long int(void) {
    return data;
}
        
bit::~bit(void) {
    //  No code in this destructor.
}

bit bit::operator+=(bit rhs) {
    return *this = *this + rhs;
}
        
bit bit::operator*=(bit rhs) {
    return *this = *this + rhs;
}

bit operator+(bit op1, bit op2) {
    return bit(op1.data ^ op2.data);
}

bit operator*(bit op1, bit op2) {
    return bit(op1.data & op2.data);
}

std::ostream &operator<<(std::ostream &s, bit b) {
    return s << int(b.data);
}




