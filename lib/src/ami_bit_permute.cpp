// Copyright (c) 1995 Darren Vengroff
//
// File: ami_bit_permute.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/9/95
//

static char ami_bit_permute_id[] = "$Id: ami_bit_permute.cpp,v 1.1 1995-01-10 16:54:15 darrenv Exp $";

// This is just to avoid an error message since the string above is never
// referenced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___ami_bit_permute_id_compiler_fooler {
    char *pc;
    ___ami_bit_permute_id_compiler_fooler *next;
} the___ami_bit_permute_id_compiler_fooler = {
    ami_bit_permute_id,
    &the___ami_bit_permute_id_compiler_fooler
};



#include <ami_bit_permute.h>

AMI_bit_perm_object::AMI_bit_perm_object(const bit_matrix &A,
                                         const bit_matrix &c) :
                                                 mA(A), mc(c)
{
}

AMI_bit_perm_object::~AMI_bit_perm_object(void)
{
}

bit_matrix AMI_bit_perm_object::A(void)
{
    return mA;
}

bit_matrix AMI_bit_perm_object::c(void)
{
    return mc;
}


