// Copyright (c) 1994 Darren Vengroff
//
// File: ami_matrix_blocks.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/11/94
//
// $Id: ami_matrix_blocks.h,v 1.3 2003-09-27 06:18:05 tavi Exp $
//
#ifndef _AMI_MATRIX_BLOCKS_H
#define _AMI_MATRIX_BLOCKS_H

// Get definitions for working with Unix and Windows
#include <portability.h>
// Get AMI_gen_perm_object.
#include <ami_gen_perm_object.h>

class perm_matrix_into_blocks : public AMI_gen_perm_object {
private:
    unsigned int r,c,be;
public:    
    perm_matrix_into_blocks(unsigned int rows, unsigned int cols,
                            unsigned int block_extent);
    virtual ~perm_matrix_into_blocks();
    AMI_err initialize(TPIE_OS_OFFSET len);
    TPIE_OS_OFFSET destination(TPIE_OS_OFFSET source);
};

class perm_matrix_outof_blocks : public AMI_gen_perm_object {
private:
    unsigned int r,c,be;
public:    
    perm_matrix_outof_blocks(unsigned int rows, unsigned int cols,
                             unsigned int block_extent);
    virtual ~perm_matrix_outof_blocks();
    AMI_err initialize(TPIE_OS_OFFSET len);
    TPIE_OS_OFFSET destination(TPIE_OS_OFFSET source);
};


#endif // _AMI_MATRIX_BLOCKS_H 



