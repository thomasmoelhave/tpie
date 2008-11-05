#ifndef _MERGE_SORTED_RUNS_H
#define _MERGE_SORTED_RUNS_H

///////////////////////////////////////////////////////////////////////////
/// \file tpie/merge_sorted_runs.h
///  Contains the routine merge_sorted_runs used in several of TPIE's merge variants
/// \sa sorted_stream_merging
/// \internal \todo Document better
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <portability.h>

// Includes needed from TPIE
#include <mergeheap.h>     //For templated heaps
#include <quicksort.h>    //For templated qsort_items

#include <progress_indicator_base.h>


namespace tpie {

    namespace ami {

  /** Intended to signal the number of input streams in a merge */ 
	typedef TPIE_OS_SIZE_T arity_t;
	
  //////////////////////////////////////////////////////////////////////////
  /// This is a common merge routine for all of the AMI_merge_sorted,
  /// AMI_ptr_merge_sorted and AMI_key_merge_sorted entry points. 
	///
	/// It is also used by the sort entry points AMI_sort, AMI_ptr_sort and
  /// AMI_key_sort and by the routine      
  /// AMI_partition_and_merge.  Differences are encapsulated within the      
  /// merge heap object MergeHeap. It is assumed that MergeHeap#allocate()   
  /// was called before entering ami_merge_sorted_runs.                          
  /// ami_merge_sorted_runs takes both the max number of elements to read from any 
  /// stream and also a boolean flag for showing a progress indicator.
	///
	/// \anchor sorted_stream_merging \par Sorted Stream Merging:
	/// TPIE provides several merge entry points for merging sorted streams to
  /// produce a single, interleaved output stream.
  /// merge_sorted_runs has three polymorphs, namely the
  /// comparison operator, comparison class and the key-based
  /// polymorphs. 
  /// The comparison operator version tends to be the fastest and most straightforward to
  /// use. The comparison class version is comparable in speed (maybe
  /// slightly slower), but somewhat more flexible, as it can support
  /// multiple, different merges on the same keys.
	/// \internal \todo Doc: LA: Syntax of comparator for polymorphs? key-based?
  ////////////////////////////////////////////////////////////////////////////
	template <class T, class M>
	err  merge_sorted_runs(stream<T> **inStreams, arity_t arity,
			       stream<T> *outStream,  M* MergeHeap,
			       TPIE_OS_OFFSET cutoff=-1, 
			       progress_indicator_base* indicator = NULL) {
	    
	    TPIE_OS_SIZE_T i;
	    err ami_err;

	    //Pointers to current leading elements of streams
	    T** in_objects = new T*[arity];
	    TPIE_OS_OFFSET* nread = new TPIE_OS_OFFSET[arity]; 
	    
	    // **************************************************************
	    // * Read first element from stream. Do not rewind! We may read *
	    // * more elements from the same stream later.                  *
	    // **************************************************************
	    
	    for (i = 0; i < arity; i++) {
		if ((ami_err = inStreams[i]->read_item (&(in_objects[i]))) !=
		    NO_ERROR) {
		    
		    if (ami_err == END_OF_STREAM) {
			in_objects[i] = NULL;
		    } 
		    else {

			delete[] in_objects;
			delete[] nread;

			return ami_err;
		    }
		} 
		else {
		    MergeHeap->insert( in_objects[i], i );
		}
		nread[i]=1; 
		if (indicator) {
		    indicator->step_percentage();
		}
	    }

	    // *********************************************************
	    // * Build a heap from the smallest items of each stream   *
	    // *********************************************************
	    
	    MergeHeap->initialize ( );

	    // *********************************************************
	    // * Perform the merge until the inputs are exhausted.     *
	    // *********************************************************
	    while (MergeHeap->sizeofheap() > 0) {

		i = MergeHeap->get_min_run_id ();
		
		if ((ami_err = outStream->write_item (*in_objects[i]))
		    != NO_ERROR) {
		    
		    delete[] in_objects;
		    delete[] nread;
		    
		    return ami_err;
		}
		
		//Check if we read as many elements as we are allowed to
		if( (cutoff != -1) && (nread[i]>=cutoff)){
		    ami_err = END_OF_STREAM;
		} 
		else {
		    if ((ami_err = inStreams[i]->read_item (&(in_objects[i])))
			!= NO_ERROR) {
			
			if (ami_err != END_OF_STREAM) {
			    
			    delete[] in_objects;
			    delete[] nread;
			    
			    return ami_err;
			}
		    }
		    
		    if (indicator) {
			indicator->step_percentage();
		    }
		    
		} 
		
		if (ami_err == END_OF_STREAM) {
		    
		    MergeHeap->delete_min_and_insert (NULL);
		} 
		else { 

		    nread[i]++;
		    MergeHeap->delete_min_and_insert (in_objects[i]);
		    
		}
	    }  //  while
	    
	    //cleanup
	    delete[] in_objects;
	    delete[] nread;
	    
	    return NO_ERROR;
	}
	
	
  ///////////////////////////////////////////////////////////////////////////
  /// Merging with a heap that contains the records to be merged. CMPR is
  /// the class of the comparison object, and must contain the method
  /// compare() which is called from within the merge.
	///
	/// This is one of the merge entry points for merging without the   
	/// \ref merge_management_object used by TPIE's merge.
	/// These routines perform the special case of merging when the
	/// the required output is the original records interleaved
	/// according to a comparison operator or function.
	/// \internal \todo add comparison operator version
  ///////////////////////////////////////////////////////////////////////////
	template <class T, class CMPR>
	err merge_sorted(stream<T> **inStreams, arity_t arity,
			 stream<T> *outStream, CMPR *cmp) {
	    
	    // make a merge heap which uses the user's comparison object
	    // and initialize it
	    merge_heap_obj<T,CMPR> mrgheap (cmp);
	    mrgheap.allocate (arity);
	    
	    //Rewind all the input streams
	    for(int i=0; i<arity; i++){ 
		inStreams[i]->seek(0); 
	    }
	    
	    return merge_sorted_runs(inStreams, arity, outStream, mrgheap);
	}

	
  ///////////////////////////////////////////////////////////////////////////
	/// Merging with a heap that keeps a pointer to the records rather than
	/// the records themselves: \p CMPR is the class of the comparison object,
	/// and must contain the method compare()  which is called from within
	/// the merge.
	///
	/// This is one of the merge entry points for merging without the   
  /// \ref merge_management_object used by TPIE's merge.
  /// These routines perform the special case of merging when the
  /// the required output is the original records interleaved
  /// according to a comparison operator or function.
	/// \internal \todo Check that memory management is done right
  ///////////////////////////////////////////////////////////////////////////
	template <class T, class CMPR>
	err ptr_merge_sorted(stream<T> **inStreams, arity_t arity,
			     stream<T> *outStream, 
			     CMPR *cmp) {

	    // make a merge heap of pointers which uses the user's comparison
	    // object and initialize it
	    merge_heap_ptr_obj<T,CMPR> mrgheap (cmp);
	    mrgheap.allocate (arity);
	    
	    // Rewind all the input streams
	    for(int i=0; i<arity; i++){ 
		inStreams[i]->seek(0); 
	    }
	    
	    return merge_sorted_runs(inStreams, arity, outStream, mrgheap);
	}
	
  ///////////////////////////////////////////////////////////////////////////
	/// Merging with a heap that contains copies of the keys from the
	/// records being merged, rather than the records themselves.
	/// The comparison object "cmp", of (user-defined) class represented by
	/// CMPR, must have a member function called "compare" which is used
	/// for merging the input streams, and a member function called "copy"
	/// which is used for copying the key (of type KEY) from a record of
	/// type T (the type to be sorted).
	///
  /// This is one of the merge entry points for merging without the   
  /// \ref merge_management_object used by TPIE's merge.
  /// These routines perform the special case of merging when the
  /// the required output is the original records interleaved
  /// according to a comparison operator or function.
	/// \internal \todo Check that memory management is done right
	///////////////////////////////////////////////////////////////////////////
	template<class T, class KEY, class CMPR>
	err key_merge_sorted(stream<T> **inStreams, arity_t arity,
			     stream<T> *outStream, CMPR *cmp) {
	    
	    // make a key merge heap which uses the user's comparison object
	    // and initialize it
	    merge_heap_kobj<T,KEY,CMPR> mrgheap (cmp);
	    mrgheap.allocate (arity);
	    
	    // Rewind all the input streams
	    for(int i=0; i<arity; i++){ 
		inStreams[i]->seek(0); 
	    }
	    
	    return merge_sorted_runs ( inStreams, arity, outStream, mrgheap);
	}
	
    }  //  ami namespace
    
}  //  tpie namespace

#endif //_MERGE_SORTED_RUNS_H
