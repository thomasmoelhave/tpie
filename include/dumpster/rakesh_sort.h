//
// File: rakesh_sort.h
// $Id: rakesh_sort.h,v 1.1 2005-11-09 11:43:20 adanner Exp $
//
// Optimized merge sorting.
//
#ifndef _RAKESH_SORT_H
#define _RAKESH_SORT_H

// Get definitions for working with Unix and Windows
#include <portability.h>

#ifndef AMI_STREAM_IMP_SINGLE
#  warning Including __FILE__ when AMI_STREAM_IMP_SINGLE undefined.
#endif

#include <rakesh_merge.h>

//------------------------------------------------------------
template<class T>
AMI_err 
AMI_optimized_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream) {
  
  return AMI_partition_and_merge(instream, outstream);
}

//------------------------------------------------------------
template<class T, class KEY>
AMI_err 
AMI_optimized_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
	 int keyoffset, KEY dummykey) {
  
  return AMI_partition_and_merge(instream, outstream, keyoffset, dummykey);
}
                          

#endif
