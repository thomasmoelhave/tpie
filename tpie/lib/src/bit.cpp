// Copyright (c) 1994 Darren Vengroff
//
// File: bit.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/4/94
//

static char bit_id[] = "$Id: bit.cpp,v 1.1 1995-01-10 16:55:18 darrenv Exp $";

// This is just to avoid an error message since the string above is never
// referenced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___bit_id_compiler_fooler {
    char *pc;
    ___bit_id_compiler_fooler *next;
} the___bit_id_compiler_fooler = {
    bit_id,
    &the___bit_id_compiler_fooler
};

#include <bit.h>

bit::bit(void)
{
}

bit::bit(bool b)
{
    data = (b == true);
}

bit::bit(int i)
{
    data = (i != 0);
}

bit::bit(long int i)
{
    data = (i != 0);
}

bit::operator bool(void)
{
    return data;
}
        
bit::operator int(void)
{
    return data;
}
        
bit::operator long int(void)
{
    return data;
}
        
bit::~bit(void)
{
}

bit bit::operator+=(bit rhs)
{
    return *this = *this + rhs;
}
        
bit bit::operator*=(bit rhs)
{
    return *this = *this + rhs;
}

bit operator+(bit op1, bit op2)
{
    return bit(op1.data ^ op2.data);
}


bit operator*(bit op1, bit op2)
{
    return bit(op1.data & op2.data);
}


ostream &operator<<(ostream &s, bit b)
{
    return s << int(b.data);
}




