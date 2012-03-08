// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008-2012, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

#ifndef _AMI_SORT_H
#define _AMI_SORT_H

///////////////////////////////////////////////////////////////////////////
/// \file sort.h
///  Contains sorting algorithms.
///  
/// \anchor sortingspace_in_tpie \par In-place Variants for Sorting in TPIE:
/// One can sort given an input stream and output stream,
/// or just an input stream. When just an input stream is specified, the
/// original input elements are deleted the input stream is rewritten with the
/// sorted output elements. If both the input stream and output stream are
/// specified, the original input elements are saved. During sorting, a
/// temporary copy of each element is stored on disk as part of intermediate
/// sorting results. If N is the size on disk of the original input stream,
/// the polymorphs of sorting with both input and output streams use 3N
/// space, whereas if just an input stream is specified, 2N space is used.
/// If the original unsorted input stream is not needed after sorting, it is
/// recommended that users use the sort() polymorph with with just
/// an input stream, to save space and avoid having to maintain both an input
/// and output stream. 
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// The class that actually does the sorting
#include <tpie/sort_manager.h>
#include <tpie/mergeheap.h>
#include <tpie/internal_sort.h>

#include <tpie/progress_indicator_base.h>
#include <tpie/progress_indicator_null.h>

namespace tpie {

  //////////////////////////////////////////////////////////////////////////
  /// A version of sort that takes an input stream of elements of type
  /// T, and an output stream, and and uses the < operator to sort,
  /// see also \ref sorting_in_tpie "Sorting in TPIE".
  /// 
  /// \anchor comp_sorting \par Comparing within Sorting:
  /// TPIE's sort() has two polymorphs, namely the comparison
  /// operator and comparison class polymorphs. The comparison operator
  /// version tends to be the fastest and most straightforward to use. The
  /// comparison class version is comparable in speed (maybe slightly
  /// slower), but somewhat more flexible, as it can support multiple,
  /// different sorts on the same keys.
  /// 
  /// \par Comparison operator version:
  /// This version works on streams of
  /// objects for which the operator "<" is defined.
  /// 
  /// \par Comparison class version:
  /// This version of sort() uses a method of a user-defined
  /// comparison object to determine the order of two input objects. The
  /// object must have a public member function named compare(),
  /// having the following prototype:
  ///
  /// <tt>inline int compare (const KEY & k1, const KEY & k2);</tt>
  /// 
  /// The user-written compare() function computes the order of
  /// the two user-defined keys \p k1  and \p k2, and
  /// returns -1, 0, or +1 to indicate that <tt>k1<k2</tt>, <tt>k1==k2</tt>, or
  /// <tt>k1>k2</tt> respectively.
  ///
  /// \internal \deprecated \par  Deprecated Comparison Function Sorting:
  /// Earlier TPIE versions allowed a sort that used a C-style
  /// comparison function to sort. However, comparison functions cannot be
  /// inlined, so each comparison requires one function call. Given that the
  /// comparison operator < and comparison object classes can be inlined and
  /// have better performance while providing the exact same functionality,
  /// comparison functions have been removed from TPIE. If you can provide us
  /// with a compelling argument on why they should be in here, we may consider
  /// adding them again, but you must demonstrate that comparision functions
  /// can outperform other methods in at least some cases or give an example
  /// were it is impossible to use a comparison operator or comparison object.
  //////////////////////////////////////////////////////////////////////////
	template<class T>
	void sort(file_stream<T> &instream, file_stream<T> &outstream,
		 tpie::progress_indicator_base* indicator=NULL)	{
	    ami::Internal_Sorter_Op<T> myInternalSorter;
	    ami::merge_heap_op<T>      myMergeHeap;
	    sort_manager< T, ami::Internal_Sorter_Op<T>, ami::merge_heap_op<T> > 
		mySortManager(&myInternalSorter, &myMergeHeap);

		mySortManager.sort(&instream, &outstream, indicator);
	}


  ///////////////////////////////////////////////////////////////////////////
  /// A version of sort that takes an input stream of elements of
  /// type T, an output stream, and a user-specified comparison
  /// object. The comparison object "cmp", of (user-defined) class
  /// represented by CMPR, must have a member function called "compare"
  /// which is used for sorting the input stream; see also 
  /// \ref comp_sorting "Comparing within Sorting".
  ///////////////////////////////////////////////////////////////////////////
  template<class T, class CMPR>
  void sort(file_stream<T> &instream, file_stream<T> &outstream,
		 CMPR *cmp, progress_indicator_base* indicator) {
	    ami::Internal_Sorter_Obj<T,CMPR> myInternalSorter(cmp);
	    ami::merge_heap_obj<T,CMPR>      myMergeHeap(cmp);
	    sort_manager< T, ami::Internal_Sorter_Obj<T,CMPR>, ami::merge_heap_obj<T,CMPR> > 
		mySortManager(&myInternalSorter, &myMergeHeap);

		mySortManager.sort(&instream, &outstream, indicator);
	}


// ********************************************************************
// *                                                                  *
// * Duplicates of the above versions that only use 2x space and      *
// * overwrite the original input stream                              *
// *                                                                  *
// ********************************************************************/

  ///////////////////////////////////////////////////////////////////////////
  /// In-place sorting variant of \ref sort(stream<T> *instream_ami, stream<T> *outstream_ami, tpie::progress_indicator_base* indicator=NULL),
  /// see also \ref sortingspace_in_tpie "In-place Variants for Sorting in TPIE".
  ///////////////////////////////////////////////////////////////////////////
	template<class T>
	void sort(file_stream<T> &instream, 
		 progress_indicator_base &indicator) {
	    ami::Internal_Sorter_Op<T> myInternalSorter;
	    ami::merge_heap_op<T>      myMergeHeap;
	    sort_manager< T, ami::Internal_Sorter_Op<T>, ami::merge_heap_op<T> > 
		mySortManager(&myInternalSorter, &myMergeHeap);
	    
		mySortManager.sort(&instream, &instream, &indicator);
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// In-place sorting variant of \ref  sort(stream<T> *instream_ami, stream<T> *outstream_ami, CMPR *cmp, progress_indicator_base* indicator=NULL), 
	/// see also \ref sortingspace_in_tpie "In-place Variants for Sorting in TPIE".
	///////////////////////////////////////////////////////////////////////////
	template<class T, class CMPR>
	void sort(file_stream<T> &instream, 
		 CMPR &cmp, progress_indicator_base &indicator) {
		ami::Internal_Sorter_Obj<T,CMPR> myInternalSorter(&cmp);
	    ami::merge_heap_obj<T,CMPR>      myMergeHeap(&cmp);
	    sort_manager< T, ami::Internal_Sorter_Obj<T,CMPR>, ami::merge_heap_obj<T,CMPR> > 
		mySortManager(&myInternalSorter, &myMergeHeap);

		mySortManager.sort(&instream, &instream, &indicator);
	}

}  //  tpie namespace

#include <tpie/sort_deprecated.h>

#endif // _AMI_SORT_H 
