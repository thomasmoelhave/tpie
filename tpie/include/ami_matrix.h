// Copyright (c) 1994 Darren Vengroff
//
// File: ami_matrix.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/9/94
//
// $Id: ami_matrix.h,v 1.1 1994-12-16 21:46:50 darrenv Exp $
//
#ifndef _AMI_MATRIX_H
#define _AMI_MATRIX_H

#include <matrix.h>

#include <ami_matrix_pad.h>
#include <ami_matrix_blocks.h>
#include <ami_stream_arith.h>

template<class T>
class AMI_matrix : public AMI_STREAM<T> {
private:
    unsigned int r,c;
public:
    AMI_matrix(unsigned int row, unsigned int col);
    ~AMI_matrix(void);
    unsigned int rows();
    unsigned int cols();
};

template<class T>
AMI_matrix<T>::AMI_matrix(unsigned int row, unsigned int col) :
        r(row), c(col), AMI_STREAM<T>((unsigned int)0, row*col)
{
}

template<class T>
AMI_matrix<T>::~AMI_matrix(void)
{
}

template<class T>
unsigned int AMI_matrix<T>::rows(void)
{
    return r;
}

template<class T>
unsigned int AMI_matrix<T>::cols(void)
{
    return c;
}

// Add two matrices.

template<class T>
AMI_err AMI_matrix_add(AMI_matrix<T> &op1, AMI_matrix<T> &op2,
                       AMI_matrix<T> &res)
{
    AMI_scan_add<T> sa;

    // We should do some bound checking here.

    return AMI_scan((AMI_base_stream<T> *)&op1, (AMI_base_stream<T> *)&op2,
                    &sa, (AMI_base_stream<T> *)&res);
}

// Subtract.

template<class T>
AMI_err AMI_matrix_sub(AMI_matrix<T> &op1, AMI_matrix<T> &op2,
                       AMI_matrix<T> &res)
{
    AMI_scan_sub<T> ss;

    // We should do some bound checking here.
    
    return AMI_scan((AMI_base_stream<T> *)&op1, (AMI_base_stream<T> *)&op2,
                    &ss, (AMI_base_stream<T> *)&res);
}


// Matrix multiply.

// For standard (non-Strassen) matrix multiply, there are at least two
// algorithms with the same asymptotic complexity.  There is the 4 way
// divide and conquer algorithms of Vitter and Shriver and there is
// the technique which divides the matrix into blocks of size
// $sqrt(M/B) \times sqrt(M/B)$.  The latter has a smaller constant and is
// simpler to implement, so we chose to use it.

template<class T>
AMI_err AMI_matrix_mult(AMI_matrix<T> &op1, AMI_matrix<T> &op2,
                        AMI_matrix<T> &res)
{
    AMI_err ae;
    
    size_t sz_avail;
    size_t mm_matrix_extent;
    size_t single_stream_usage;    
    
    // Check bounds on the matrices to make sure they match up.
    if ((op1.cols() != op2.rows()) || (res.rows() != op1.rows()) ||
        (res.cols() != op2.cols())) {
        return AMI_MATRIX_BOUNDS;
    }
    
    // Check available main memory.
    if (MM_manager.available(&sz_avail) != MM_ERROR_NO_ERROR) {
        return AMI_ERROR_MM_ERROR;
    }
    
    // How much memory does a single streamneed in the worst case?
    
    if ((ae = op1.main_memory_usage(&single_stream_usage,
                                    MM_STREAM_USAGE_MAXIMUM)) !=
                                    AMI_ERROR_NO_ERROR) {
        return ae;
    }
    
    // Will the problem fit in main memory?

    {
        size_t sz_op1 = op1.rows() * op1.cols() * sizeof(T);
        size_t sz_op2 = op2.rows() * op2.cols() * sizeof(T);
        size_t sz_res = res.rows() * res.cols() * sizeof(T);
    
        if (sz_avail > sz_op1 + sz_op2 + sz_res + 3 * single_stream_usage +
            3 * sizeof(matrix<T>)) {

            unsigned int ii,jj;
            T *tmp_read;
            
            // Main memory copies of the matrices.
            matrix<T> mm_op1(op1.rows(), op1.cols());
            matrix<T> mm_op2(op2.rows(), op2.cols());
            matrix<T> mm_res(res.rows(), res.cols());
            
            // Read in the matrices and solve in main memory.

            op1.seek(0);
            for (ii = 0; ii < op1.rows(); ii++ ) {
                for (jj = 0; jj < op1.cols(); jj++ ) {
                    ae = op1.read_item(&tmp_read);
                    if (ae != AMI_ERROR_NO_ERROR) {
                        return ae;
                    }
                    mm_op1[ii][jj] = *tmp_read;
                }
            }
                
            op2.seek(0);
            for (ii = 0; ii < op2.rows(); ii++ ) {
                for (jj = 0; jj < op2.cols(); jj++ ) {
                    ae = op2.read_item(&tmp_read);
                    if (ae != AMI_ERROR_NO_ERROR) {
                        return ae;
                    }
                    mm_op2[ii][jj] = *tmp_read;
                }
            }
            
            perform_mult_in_place((matrix_base<T> &)mm_op1,
                                  (matrix_base<T> &)mm_op2,
                                  (matrix_base<T> &)mm_res);
                        
            // Write out the result.
            res.seek(0);
            for (ii = 0; ii < res.rows(); ii++ ) {
                for (jj = 0; jj < res.cols(); jj++ ) {
                    ae = res.write_item(mm_res[ii][jj]);
                    if (ae != AMI_ERROR_NO_ERROR) {
                        return ae;
                    }
                }
            }
            
            return AMI_ERROR_NO_ERROR;
        }

    }

    // We now know the problem does not fit in main memory.

    {
                    
        unsigned int num_active_streams = 4 + 4;
        size_t mm_matrix_space;
        size_t single_stream_usage;
    
        // What is the maximum extent of any matrix we will try to
        // load into memory?  We may have up to four in memory at any
        // given time.  To be safe, let each one have a stream behind
        // it and let there be some additional active streams such as
        // ....

        if ((ae = op1.main_memory_usage(&single_stream_usage,
                                        MM_STREAM_USAGE_MAXIMUM)) !=
                                        AMI_ERROR_NO_ERROR) {
            return ae;
        }
        
        mm_matrix_space = sz_avail - num_active_streams * single_stream_usage;

        mm_matrix_space /= 3;
            
        mm_matrix_extent = (unsigned int)sqrt((double)mm_matrix_space /
                                              sizeof(T));    

        // How many rows and cols in padded matrices.
        
        unsigned int rowsp1 = mm_matrix_extent * (((op1.rows() - 1) /
                                                   mm_matrix_extent) + 1);
        unsigned int colsp1 = mm_matrix_extent * (((op1.cols() - 1) /
                                                  mm_matrix_extent) + 1);

        unsigned int rowsp2 = mm_matrix_extent * (((op2.rows() - 1) /
                                                   mm_matrix_extent) + 1);
        unsigned int colsp2 = mm_matrix_extent * (((op2.cols() - 1) /
                                                   mm_matrix_extent) + 1);
        
        // Padded matrices.

        AMI_matrix<T> *op1p = new AMI_matrix<T>(rowsp1, colsp1);
        AMI_matrix<T> *op2p = new AMI_matrix<T>(rowsp2, colsp2);
                    
        // Scan each matrix to pad it out with zeroes as needed.

        {
            AMI_matrix_pad<T> smp1(op1.rows(), op1.cols(), mm_matrix_extent);
            AMI_matrix_pad<T> smp2(op2.rows(), op2.cols(), mm_matrix_extent);

            ae = AMI_scan((AMI_base_stream<T> *)&op1, &smp1,
                          (AMI_base_stream<T> *)op1p);
            if (ae != AMI_ERROR_NO_ERROR) {
                return ae;
            }
            ae = AMI_scan((AMI_base_stream<T> *)&op2, &smp2,
                          (AMI_base_stream<T> *)op2p);
            if (ae != AMI_ERROR_NO_ERROR) {
                return ae;
            }
        }

        // Permuted padded matrices.

        AMI_matrix<T> *op1pp = new AMI_matrix<T>(rowsp1, colsp1);
        AMI_matrix<T> *op2pp = new AMI_matrix<T>(rowsp2, colsp2);

        AMI_matrix<T> *respp = new AMI_matrix<T>(rowsp1, colsp2);

	// Permute each padded matrix into block order.  The blocks
        // are in row major order and the elements within the blocks
        // are in row major order.

        {
            perm_matrix_into_blocks pmib1(rowsp1, colsp1, mm_matrix_extent);
            perm_matrix_into_blocks pmib2(rowsp2, colsp2, mm_matrix_extent);

            ae = AMI_general_permute((AMI_STREAM<T> *)op1p,
                                     (AMI_STREAM<T> *)op1pp,
                                     (AMI_gen_perm_object *)&pmib1); 
            if (ae != AMI_ERROR_NO_ERROR) {
                return ae;
            }

            ae = AMI_general_permute((AMI_STREAM<T> *)op2p,
                                     (AMI_STREAM<T> *)op2pp,
                                     (AMI_gen_perm_object *)&pmib2); 
            if (ae != AMI_ERROR_NO_ERROR) {
                return ae;
            }
        }

        // We are done with the padded but unpermuted matrices.

        delete op1p;
        delete op2p;
            
        // Now run the standard matrix multiplication algorithm over
        // the blocks.  To multiply two blocks, we read them into main
        // memory.  The blocks of the result are accumulated one by
        // one in main memory and then written out.
        
        {
            matrix<T> mm_op1(mm_matrix_extent, mm_matrix_extent);
            matrix<T> mm_op2(mm_matrix_extent, mm_matrix_extent);
            matrix<T> mm_accum(mm_matrix_extent, mm_matrix_extent);
            
            unsigned int ii,jj,kk;
            T *tmp_read;

            respp->seek(0);
            
            // ii loops over block rows of op1pp.

            for (ii = 0; ii < rowsp1 / mm_matrix_extent; ii++ ) {

                // jj loops over block cols of op2pp.

                for (jj = 0; jj < colsp2 / mm_matrix_extent; jj++ ) {

                    // These are for looping over rows and cols of MM
                    // matrices.
                    unsigned int ii1,jj1;

                    // Clear the temporary result.
                    for (ii1 = 0; ii1 < mm_matrix_extent; ii1++ ) {
                        for (jj1 = 0; jj1 < mm_matrix_extent; jj1++ ) {
                            mm_accum[ii1][jj1] = 0;
                        }
                    }

                    // kk loops over the cols of op1pp and rows of
                    // op2pp at the same time.

                    tp_assert(rowsp2 == colsp1, "Matrix extent mismatch.");
                    
                    for (kk = 0; kk < rowsp2 / mm_matrix_extent; kk++ ) {

                        // Read a block from op1pp.

                        op1pp->seek(ii * colsp1 * mm_matrix_extent +
                                    kk * mm_matrix_extent * mm_matrix_extent);
                        
                        for (ii1 = 0; ii1 < mm_matrix_extent; ii1++ ) {
                            for (jj1 = 0; jj1 < mm_matrix_extent; jj1++ ) {
                                ae = op1pp->read_item(&tmp_read);
                                if (ae != AMI_ERROR_NO_ERROR) {
                                    return ae;
                                }
                                mm_op1[ii1][jj1] = *tmp_read; 
                            }
                        }

                        // Read a block from op2pp.

                        op2pp->seek(kk * colsp2 * mm_matrix_extent +
                                    jj * mm_matrix_extent * mm_matrix_extent);
                        
                        for (ii1 = 0; ii1 < mm_matrix_extent; ii1++ ) {
                            for (jj1 = 0; jj1 < mm_matrix_extent; jj1++ ) {
                                ae = op2pp->read_item(&tmp_read);
                                if (ae != AMI_ERROR_NO_ERROR) {
                                    return ae;
                                }
                                mm_op2[ii1][jj1] = *tmp_read; 
                            }
                        }

                        // Multiply in MM and add to the running sum.
                        
                        perform_mult_add_in_place((matrix_base<T> &)mm_op1,
                                                  (matrix_base<T> &)mm_op2,
                                                  (matrix_base<T> &)mm_accum);
                    }

                    // We now have the complete result for a block of
                    // respp, so write it out.

                    for (ii1 = 0; ii1 < mm_matrix_extent; ii1++ ) {
                        for (jj1 = 0; jj1 < mm_matrix_extent; jj1++ ) {
                            ae = respp->write_item(mm_accum[ii1][jj1]);
                            if (ae != AMI_ERROR_NO_ERROR) {
                                return ae;
                            }
                        }
                    }                                        
                }
            }            
        }

        // We are done with the padded and permuted operators.

        delete op1pp;
        delete op2pp;
        
        // Permute the result from scan block order back into row
        // major order.

        AMI_matrix<T> *resp = new AMI_matrix<T>(rowsp1, colsp2);

        {
            perm_matrix_outof_blocks pmob(rowsp1, colsp1, mm_matrix_extent);

            ae = AMI_general_permute((AMI_STREAM<T> *)respp,
                                     (AMI_STREAM<T> *)resp,
                                     (AMI_gen_perm_object *)&pmob); 
            if (ae != AMI_ERROR_NO_ERROR) {
                return ae;
            }            
        }

        // We are done with the padded and permuted result.

        delete respp;
        
        // Scan to strip the padding from the output matrix.

        {
            AMI_matrix_unpad<T> smup(op1.rows(), op2.cols(),
                                      mm_matrix_extent);

            ae = AMI_scan((AMI_base_stream<T> *)resp, &smup,
                          (AMI_base_stream<T> *)&res);
            if (ae != AMI_ERROR_NO_ERROR) {
                return ae;
            }
        
        }

        // We are done with the padded but unpermuted result.

        delete resp;

    }
    
    return AMI_ERROR_NO_ERROR;
}


#ifdef NO_IMPLICIT_TEMPLATES

#define TEMPLATE_INSTANTIATE_AMI_MATRIX(T)				\
template class AMI_matrix<T>;						\
TEMPLATE_INSTANTIATE_AMI_MATRIX_PAD(T)					\
TEMPLATE_INSTANTIATE_MATRIX(T)						\
template AMI_err AMI_matrix_mult(AMI_matrix<T> &op1,			\
                                 AMI_matrix<T> &op2,			\
                                 AMI_matrix<T> &res);			\
template AMI_err AMI_matrix_add(AMI_matrix<T> &op1,			\
                                AMI_matrix<T> &op2,			\
                                AMI_matrix<T> &res);			\
template AMI_err AMI_matrix_sub(AMI_matrix<T> &op1,			\
                                AMI_matrix<T> &op2,			\
                                AMI_matrix<T> &res);			\
TEMPLATE_INSTANTIATE_GEN_PERM(T)					\
TEMPLATE_INSTANTIATE_STREAM_ADD(T)					\
TEMPLATE_INSTANTIATE_STREAM_SUB(T)

#endif



#endif // _AMI_MATRIX_H 


