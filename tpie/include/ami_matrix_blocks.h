// Copyright (c) 1994 Darren Vengroff
//
// File: ami_matrix_blocks.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/11/94
//
// $Id: ami_matrix_blocks.h,v 1.1 1995-01-10 16:50:22 darrenv Exp $
//
#ifndef _AMI_MATRIX_BLOCKS_H
#define _AMI_MATRIX_BLOCKS_H

class perm_matrix_into_blocks : public AMI_gen_perm_object {
private:
    unsigned int r,c,be;
public:    
    perm_matrix_into_blocks(unsigned int rows, unsigned int cols,
                            unsigned int block_extent);
    virtual ~perm_matrix_into_blocks();
    AMI_err initialize(off_t len);
    off_t destination(off_t source);
};

class perm_matrix_outof_blocks : public AMI_gen_perm_object {
private:
    unsigned int r,c,be;
public:    
    perm_matrix_outof_blocks(unsigned int rows, unsigned int cols,
                             unsigned int block_extent);
    virtual ~perm_matrix_outof_blocks();
    AMI_err initialize(off_t len);
    off_t destination(off_t source);
};


#endif // _AMI_MATRIX_BLOCKS_H 



