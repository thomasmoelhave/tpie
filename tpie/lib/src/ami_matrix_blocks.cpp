// Copyright (c) 1994 Darren Vengroff
//
// File: ami_matrix_blocks.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/11/94
//

#include <versions.h>
VERSION(ami_matrix_blocks_cpp,"$Id: ami_matrix_blocks.cpp,v 1.3 2000-01-10 22:28:45 hutchins Exp $");

#include <sys/types.h>
#include <ami_base.h>
#include <ami_gen_perm_object.h>
#include <ami_matrix_blocks.h>

perm_matrix_into_blocks::perm_matrix_into_blocks(unsigned int rows,
                                                 unsigned int cols,
                                                 unsigned int block_extent) :
                                                         r(rows),
                                                         c(cols),
                                                         be(block_extent)
{
}

perm_matrix_into_blocks::~perm_matrix_into_blocks()
{
}

AMI_err perm_matrix_into_blocks::initialize(off_t len)
{
    return ((off_t) (r * c) == len) ? AMI_ERROR_NO_ERROR : AMI_MATRIX_BOUNDS;
}

off_t perm_matrix_into_blocks::destination(off_t source)
{
    tp_assert(r % be == 0, "Rows not a multiple of block extent.");
    tp_assert(c % be == 0, "Cols not a multiple of block extent.");

    // What row and column are the source in?

    off_t src_row = source / c;
    off_t src_col = source % c;

    // How many rows of blocks come before the one the source is in?

    off_t src_brow = src_row / be;

    // How many blocks in the row of blocks that the source is in come
    // before the block the source is in?

    off_t src_bcol = src_col / be;

    // Number of objects in block rows above.

    off_t obj_b_above = src_brow * be * c;

    // Number of objects in blocks in the same block row before it.

    off_t obj_b_left = src_bcol * be * be;
    
    // Position in block

    off_t bpos = (src_row - be * src_brow) * be +
        (src_col - be * src_bcol);
    
    return obj_b_above + obj_b_left + bpos;
}


perm_matrix_outof_blocks::perm_matrix_outof_blocks(unsigned int rows,
                                                   unsigned int cols,
                                                   unsigned int block_extent) :
                                                           r(rows),
                                                           c(cols),
                                                           be(block_extent)
{
}

perm_matrix_outof_blocks::~perm_matrix_outof_blocks()
{
}

AMI_err perm_matrix_outof_blocks::initialize(off_t len)
{
    return ((off_t) (r * c) == len) ? AMI_ERROR_NO_ERROR : AMI_MATRIX_BOUNDS;
}

off_t perm_matrix_outof_blocks::destination(off_t source)
{
    tp_assert(r % be == 0, "Rows not a multiple of block extent.");
    tp_assert(c % be == 0, "Cols not a multiple of block extent.");

    // How many full blocks come before source?

    off_t src_blocks_before = source / (be * be);

    // How many rows of blocks are above the block source is in?

    off_t src_brow = src_blocks_before / (c / be);

    // How many blocks in the current row are before the block src is in?

    off_t src_bleft = src_blocks_before % (c / be); 
    
    // What is the position of source in its block?

    off_t src_pos_in_block = source % (be * be);

    // What is the row of the source in its block?

    off_t src_row_in_block = src_pos_in_block / be;

    // What is the col of the source in its block?

    off_t src_col_in_block = src_pos_in_block % be;

    // Number of items in block rows above src.

    off_t items_brow_above = src_brow * c * be;

    // Number of items in the current block row above src.

    off_t items_curr_brow_above = src_row_in_block * c;

    // Number of items in item row to left of source.

    off_t items_left_in_row = (src_bleft * be) + src_col_in_block;

    // Add up everything before it.
    
    return items_brow_above + items_curr_brow_above + items_left_in_row;
}

