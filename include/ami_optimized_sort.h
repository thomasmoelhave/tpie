// Optimized merge sorting for the AMI_IMP_SINGLE implementation.
//
//file: ami_optimized_sort.h
#ifndef _AMI_SORT_OPTIMIZED_H
#define _AMI_SORT_OPTIMIZED_H

#ifndef AMI_IMP_SINGLE
#warning Including __FILE__ when AMI_IMP_SINGLE undefined.
#endif

#include <ami_merge.h>
#include <ami_optimized_merge.h>


//------------------------------------------------------------
template<class T>
AMI_err 
AMI_optimized_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream) {
  
  return AMI_partition_and_merge(instream, outstream);
}

//------------------------------------------------------------
template<class T>
AMI_err 
AMI_optimized_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
	 int (*cmp)(CONST T&, CONST T&)) {

  return AMI_partition_and_merge(instream, outstream, cmp);
}


//------------------------------------------------------------
template<class T>
AMI_err 
AMI_optimized_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
	 int keyoffset, KEY dummykey) {
  
  return AMI_partition_and_merge(instream, outstream, keyoffset, dummykey);
}
                          

#endif
