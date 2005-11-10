//
// File: ami_sort.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 6/10/94
//
// $Id: ami_sort.h,v 1.13 2005-11-10 11:54:57 jan Exp $
//
#ifndef _AMI_SORT_H
#define _AMI_SORT_H

// Get definitions for working with Unix and Windows
#include <portability.h>

// The class that actually does the sorting
#include <sort_manager.h>
#include <mergeheap.h>
#include <internal_sort.h>

#include <progress_indicator_base.h>

// *******************************************************************
// *                                                                 *
// *           The actual AMI_sort calls                             *
// *                                                                 *
// *******************************************************************

// ********************************************************************
// *                                                                  *
// *  These first two versions use a mergeheap on actual objecet      *
// *                                                                  *
// ********************************************************************

// A version of AMI_sort that takes an input stream of elements of type
// T, and an output stream, and and uses the < operator to sort
template<class T>
AMI_err AMI_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
                 progress_indicator_base* indicator=NULL)
{
  return sort_manager< T, Internal_Sorter_Op<T>, merge_heap_op<T> >
    (Internal_Sorter_Op<T>(), merge_heap_op<T>() ).sort
    (instream, outstream, indicator);
}

// A version of AMI_sort that takes an input stream of elements of
// type T, an output stream, and a user-specified comparison
// object. The comparison object "cmp", of (user-defined) class
// represented by CMPR, must have a member function called "compare"
// which is used for sorting the input stream.
template<class T, class CMPR>
AMI_err AMI_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
    CMPR *cmp, progress_indicator_base* indicator=NULL)
{
  return sort_manager<T, Internal_Sorter_Obj<T, CMPR>,
         merge_heap_obj<T,CMPR> >( Internal_Sorter_Obj<T, CMPR>(cmp),
             merge_heap_obj<T,CMPR>(cmp) ).sort
              (instream, outstream, indicator);
}

// ********************************************************************
// *                                                                  *
// *  These versions build  a heap on pointers to objects             *
// *                                                                  *
// ********************************************************************

// A version of AMI_sort that takes an input stream of elements of type
// T, and an output stream, and and uses the < operator to sort
template<class T>
AMI_err AMI_ptr_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
                 progress_indicator_base* indicator=NULL)
{
  return sort_manager< T, Internal_Sorter_Op<T>, merge_heap_ptr_op<T> >
    (Internal_Sorter_Op<T>(), merge_heap_ptr_op<T>()).sort
    (instream, outstream, indicator);
}

// A version of AMI_sort that takes an input stream of elements of
// type T, an output stream, and a user-specified comparison
// object. The comparison object "cmp", of (user-defined) class
// represented by CMPR, must have a member function called "compare"
// which is used for sorting the input stream.
template<class T, class CMPR>
AMI_err AMI_ptr_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
    CMPR *cmp, progress_indicator_base* indicator=NULL)
{
  return sort_manager<T, Internal_Sorter_Obj<T, CMPR>,
         merge_heap_ptr_obj<T,CMPR> >( Internal_Sorter_Obj<T, CMPR>(cmp),
             merge_heap_ptr_obj<T,CMPR>(cmp) ).sort
             (instream, outstream, indicator);
}

// ********************************************************************
// *                                                                  *
// *  This version keeps a heap of keys to records, separating small  *
// *  keys from large records and reducing data movement in the heap  *
// *  when objects are very large but the sort key is small           *
// *                                                                  *
// ********************************************************************
// A version of AMI_sort that takes an input stream of elements of
// type T, an output stream, a key specification, and a user-specified
// comparison object.

// The key specification consists of an example key, which is used to
// infer the type of the key field. The comparison object "cmp", of
// (user-defined) class represented by CMPR, must have a member
// function called "compare" which is used for sorting the input
// stream, and a member function called "copy" which is used for
// copying the key of type KEY from a record of type T (the type to be
// sorted).
template<class T, class KEY, class CMPR>
AMI_err  AMI_key_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
    KEY dummykey, CMPR *cmp, progress_indicator_base* indicator=NULL)
{
  return sort_manager<T, Internal_Sorter_KObj<T, KEY, CMPR>,
        merge_heap_kobj<T,KEY,CMPR> >( Internal_Sorter_KObj<T,KEY,CMPR>(cmp),             merge_heap_kobj<T,KEY,CMPR>(cmp) ).sort
          (instream, outstream, indicator);
}

// ********************************************************************
// *                                                                  *
// * Duplicates of the above versions that only use 2x space and      *
// * overwrite the original input stream                              *
// *                                                                  *
// ********************************************************************

// object heaps, < operator comparisons
template<class T>
AMI_err AMI_sort(AMI_STREAM<T> *instream, progress_indicator_base* indicator=NULL)
{
  return sort_manager< T, Internal_Sorter_Op<T>, merge_heap_op<T> >
  (Internal_Sorter_Op<T>(), merge_heap_op<T>() ).sort
  (instream, indicator);
}

// object heaps, comparison object comparisions
template<class T, class CMPR>
AMI_err AMI_sort(AMI_STREAM<T> *instream, CMPR *cmp, progress_indicator_base* indicator=NULL)
{
  return sort_manager<T, Internal_Sorter_Obj<T, CMPR>,
         merge_heap_obj<T,CMPR> >( Internal_Sorter_Obj<T, CMPR>(cmp),
             merge_heap_obj<T,CMPR>(cmp) ).sort
           (instream, indicator);
}

// ptr heaps, < operator comparisons
template<class T>
AMI_err AMI_ptr_sort(AMI_STREAM<T> *instream, progress_indicator_base* indicator=NULL)
{
  return sort_manager< T, Internal_Sorter_Op<T>, merge_heap_ptr_op<T> >
    (Internal_Sorter_Op<T>(), merge_heap_ptr_op<T>()).sort
    (instream, indicator);
}

// ptr heaps, comparison object comparisions
template<class T, class CMPR>
AMI_err AMI_ptr_sort(AMI_STREAM<T> *instream, CMPR *cmp, progress_indicator_base* indicator=NULL)
{
  return sort_manager<T, Internal_Sorter_Obj<T, CMPR>,
         merge_heap_ptr_obj<T,CMPR> >( Internal_Sorter_Obj<T, CMPR>(cmp),
             merge_heap_ptr_obj<T,CMPR>(cmp) ).sort
           (instream, indicator);
}

// key/object heaps, key/object comparisons 
template<class T, class KEY, class CMPR>
AMI_err  AMI_key_sort(AMI_STREAM<T> *instream, KEY dummykey, CMPR *cmp,
    progress_indicator_base* indicator=NULL)
{
  return sort_manager<T, Internal_Sorter_KObj<T, KEY, CMPR>,
        merge_heap_kobj<T,KEY,CMPR> >( Internal_Sorter_KObj<T,KEY,CMPR>(cmp),             merge_heap_kobj<T,KEY,CMPR>(cmp) ).sort
          (instream, indicator);
}

/*
DEPRECATED: comparison function sorting
Earlier TPIE versions allowed a sort that used a C-style
comparison function to sort. However, comparison functions cannot be
inlined, so each comparison requires one function call. Given that the
comparison operator < and comparison object classes can be inlined and
have better performance while providing the exact same functionality,
comparison functions have been removed from TPIE. If you can provide us
with a compelling argument on why they should be in here, we may consider
adding them again, but you must demonstrate that comparision functions
can outperform other methods in at least some cases or give an example
were it is impossible to use a comparison operator or comparison object

Sincerely,
the management
*/

#endif // _AMI_SORT_H 
