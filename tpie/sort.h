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

#include <tpie/comparator.h>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort elements of a stream using the given STL-style comparator
/// object.
///////////////////////////////////////////////////////////////////////////////
template<class T, class Compare>
void stlsort(file_stream<T> &instream, file_stream<T> &outstream,
			 Compare comp, progress_indicator_base & indicator) {

	ami::Internal_Sorter_Obj<T,Compare> myInternalSorter(comp);
	ami::merge_heap_obj<T,Compare>      myMergeHeap(comp);
	sort_manager< T, ami::Internal_Sorter_Obj<T,Compare>, ami::merge_heap_obj<T,Compare> > 
	mySortManager(&myInternalSorter, &myMergeHeap);

	mySortManager.sort(&instream, &outstream, &indicator);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort elements of a stream using the less-than operator.
///////////////////////////////////////////////////////////////////////////////
template<class T>
void sort(file_stream<T> &instream, file_stream<T> &outstream,
		  tpie::progress_indicator_base* indicator=NULL) {
	std::less<T> comp;
	stlsort(instream, outstream, comp, *indicator);
}

///////////////////////////////////////////////////////////////////////////////
/// \deprecated \brief Sort elements of a stream using the given AMI-style
/// comparator.
///////////////////////////////////////////////////////////////////////////////
template<class T, class TCompare>
void sort(file_stream<T> &instream, file_stream<T> &outstream,
		  TCompare *tpiecomp, progress_indicator_base* indicator) {
	TPIE2STL_cmp<T, TCompare> comp(tpiecomp);
	stlsort(instream, outstream, comp, *indicator);
}


// ********************************************************************
// *                                                                  *
// * Duplicates of the above versions that only use 2x space and      *
// * overwrite the original input stream                              *
// *                                                                  *
// ********************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort elements of a stream in-place using the given STL-style
/// comparator object.
///////////////////////////////////////////////////////////////////////////////
template<class T, class Compare>
void stlsort(file_stream<T> &instream, Compare comp,
			 progress_indicator_base & indicator) {
	stlsort(instream, instream, comp, indicator);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort elements of a stream in-place using the less-than operator.
///////////////////////////////////////////////////////////////////////////////
template<class T>
void sort(file_stream<T> &instream, 
		  progress_indicator_base &indicator) {
	sort(instream, instream, &indicator);
}

///////////////////////////////////////////////////////////////////////////////
/// \deprecated \brief Sort elements of a stream in-place using the given
/// AMI-style comparator.
///////////////////////////////////////////////////////////////////////////////
template<class T, class CMPR>
void sort(file_stream<T> &instream, 
		  CMPR &cmp, progress_indicator_base &indicator) {
	sort(instream, instream, &cmp, &indicator);
}

}  //  tpie namespace

#include <tpie/sort_deprecated.h>

#endif // _AMI_SORT_H 
