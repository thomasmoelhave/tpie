// File: ami_sort_single_dh.h
//
// This file contains the templated routines
//     1) AMI_sort:
//          a) AMI_err AMI_sort(AMI_STREAM<T> *instream, 
//                              AMI_STREAM<T> *outstream)
//          b) AMI_err AMI_sort(AMI_STREAM<T> *instream, 
//                              AMI_STREAM<T> *outstream,
//                              int (*cmp)(CONST T&, CONST T&))
//          c) AMI_err AMI_sort(AMI_STREAM<T> *instream, 
//                              AMI_STREAM<T> *outstream,
//                              CMPR *cmp)
//     2) AMI_ptr_sort
//          a) AMI_err AMI_ptr_sort(AMI_STREAM<T> *instream, 
//                                  AMI_STREAM<T> *outstream)
//          b) AMI_err AMI_ptr_sort(AMI_STREAM<T> *instream, 
//                                  AMI_STREAM<T> *outstream,
//                                  int (*cmp)(CONST T&, CONST T&))
//          c) AMI_err AMI_ptr_sort(AMI_STREAM<T> *instream, 
//                                  AMI_STREAM<T> *outstream,
//                                  CMPR *cmp)
//     3) AMI_key_sort
//          a) AMI_err AMI_key_sort(AMI_STREAM<T> *instream, 
//                                  AMI_STREAM<T> *outstream,
//                                  KEY dummykey, 
//                                  CMPR *cmp) 

// and the supporting routines
//     sort_manager : a base class for sort managers
//     sort_manager_op : used in 1a, 2a
//     sort_manager_obj : used in 1b, 2b
//     sort_manager_cmp : used in 1c, 2c
//     sort_manager_kobj : used in 3a
// 

// The routines AMI_partition_and_merge and AMI_single_merge are also
// used, and can be found in file apm_dh.h.  Besides the sort manager
// class, the sort routines are parameterized by an appropriate "merge
// heap class" and instantiate an object of that type. These classes
// can be found in file mergeheap_dh.h.

// 	$Id: ami_sort_single_dh.h,v 1.9 2001-04-23 19:33:12 hutchins Exp $	
// TODO:
//      (1) 'space_usage_overhead' is really overhead per item. Let's
//      change it to 'space_usage_per_item' which is consistent with
//      'space_usage_per_stream'.
 
#ifndef _AMI_SORT_SINGLE_DH_H
#define _AMI_SORT_SINGLE_DH_H

#ifndef AMI_IMP_SINGLE
#warning Including __FILE__ when AMI_IMP_SINGLE undefined.
#endif

// For use in core by main_mem_operate().
#include <quicksort.h>

#include <ami_merge.h>
#include <ami_optimized_merge.h>
#include <apm_dh.h>

template <class KEY, class CMPR>
class _QSIC {
public:
    CMPR *UsrObject;
    inline int compare ( CONST qsort_item<KEY> &x, CONST qsort_item<KEY> &y) {
        return (*UsrObject).compare( x.keyval, y.keyval );
    };
};

// A class of merge objects for merge sorting objects of type T.  We
// will actually use one of three subclasses of this class which use
// either a comparison function, a comparison object,  or the binary 
// comparison operator <.

template <class T, class Q>
class sort_manager {
private:
protected:
    T                   *mmStream;     // ptr to buffer for a memoryload
    size_t              mm_len;        // size of a memory load
    size_t              item_overhead; // space overhead per item
public:
            sort_manager(void);
            ~sort_manager(void);
    bool    sort_fits_in_memory(AMI_STREAM<T> *, size_t sz_avail);
    AMI_err main_mem_operate_init(size_t);
    AMI_err main_mem_operate_cleanup();
    size_t  space_usage_per_stream(void);
    size_t  space_usage_overhead(void);
};

template<class T, class Q>
sort_manager<T,Q>::sort_manager (void) : item_overhead(0) 
{
}

template<class T, class Q>
sort_manager<T,Q>::~sort_manager(void)
{
}

template<class T,class Q>
inline bool sort_manager<T,Q>::sort_fits_in_memory (AMI_STREAM <T> *instream, size_t sz_avail) {
    // See if we have enough room to sort in memory
    //    if (instream->stream_len()*(sizeof(T)+MM_manager.space_overhead()+item_overhead) <= sz_avail){ 2001/04/22 dh
    if (instream->stream_len()*(sizeof(T)+item_overhead) <= sz_avail){
        LOG_DEBUG_ID("sort_manager.sort_fits_in_memory: sort fits in mem");
        return true;
    }
    else {
        LOG_DEBUG_ID("sort_manager.sort_fits_in_memory: sort doesn't fit in mem");
        return false;
    };
}

template<class T,class Q>
inline AMI_err sort_manager<T,Q>::main_mem_operate_init(size_t szMemoryLoad) {
   LOG_DEBUG_ID("Starting sort_manager.main_mem_operate_init");
   item_overhead = 0;
   mm_len        = szMemoryLoad;
   mmStream      = new T[szMemoryLoad];
   //if (mmStream == NULL) {
   //   LOG_FATAL_ID ("Misjudged available main memory.");
   //   return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
   //}
   LOG_DEBUG_ID("Ending sort_manager.main_mem_operate_init");
   return AMI_ERROR_NO_ERROR;
}

template<class T,class Q>
inline AMI_err sort_manager<T,Q>::main_mem_operate_cleanup() {
   LOG_DEBUG_ID("Starting sort_manager.main_mem_operate_cleanup");
   if (mmStream) {
      delete[]mmStream;
      mmStream = NULL;
   }
   LOG_DEBUG_ID("Ending sort_manager.main_mem_operate_cleanup");
}

template<class T, class Q>
size_t sort_manager<T,Q>::space_usage_per_stream(void)
{
    return sizeof(arity_t) + sizeof(T);
}

template<class T,class Q>
size_t sort_manager<T,Q>::space_usage_overhead(void){
    return item_overhead;
}


// *********************************************************************
// *                                                                   *
// * Operator based merge sort manager.                                *
// *                                                                   *
// *********************************************************************

template <class T, class Q>
class sort_manager_op  : public sort_manager<T,Q>  {
private:
public:
                sort_manager_op(void);    
                ~sort_manager_op(void);    
    Q           MergeHeap;
    AMI_err     main_mem_operate( AMI_STREAM <T>*, AMI_STREAM <T>*, size_t);
    AMI_err     single_merge(AMI_STREAM<T> **, arity_t, AMI_STREAM<T> * );
};    

template<class T,class Q>
sort_manager_op<T,Q>::sort_manager_op(void){
}

template<class T,class Q>
sort_manager_op<T,Q>::~sort_manager_op(void){
}

template<class T,class Q>
inline AMI_err sort_manager_op<T,Q>::single_merge ( 
                               AMI_STREAM < T > **inStreams, arity_t arity,
                               AMI_STREAM < T > *outStream ) {
   return AMI_single_merge_dh<T,Q>( inStreams, arity, outStream, MergeHeap );
};

template<class T,class Q>
inline AMI_err sort_manager_op<T,Q>::main_mem_operate (
                               AMI_STREAM <T> *inStream, 
                               AMI_STREAM <T> *outStream, size_t runSize ) {
   AMI_err ae;
   T    *next_item;

   LOG_DEBUG_ID("starting sort_manager_op.main_mem_operate. runSize is " << 
                runSize );
   tp_assert ( runSize <= mm_len, "memory load larger than buffer.");

   // Read a memory load out of the input stream one item at a time,
   // if key sorting fill up the key array at the same time.

   for (int i = 0; i < runSize; i++) {
      if ((ae=inStream->read_item (&next_item)) != AMI_ERROR_NO_ERROR) {
         LOG_FATAL_ID ("sort_manager_op.main_mem_operate: AMI read error " << 
                       ae);
         return ae;
      }
      mmStream[i] = *next_item;
   }

   //Sort the array.
   LOG_DEBUG_ID("sort_manager_op.main_mem_operate: calling quicker_sort_op" <<
                " for" << runSize << " items");
   quicker_sort_op ((T *) mmStream, runSize);

   LOG_DEBUG_ID("sort_manager_op.main_mem_operate: " << 
                "starting main_mem_operate write out");
   for (int i = 0; i < runSize; i++) {
      if ((ae = outStream->write_item (mmStream[i])) 
           != AMI_ERROR_NO_ERROR) {
         LOG_FATAL_ID ("sort_manager_op.main_mem_operate: AMI write error " <<
                       ae );
         return ae;
      }
   }
   LOG_DEBUG_ID("sort_manager_op.main_mem_operate: " << 
                "returning from main_mem_operate");
   return AMI_ERROR_NO_ERROR;
}


// *********************************************************************
// *                                                                   *
// * Comparison object based merge sort manager.                       *
// *                                                                   *
// *********************************************************************

template <class T, class Q, class CMPR>
class sort_manager_obj   : public sort_manager<T,Q>  {
private:
    CMPR         *cmp_o;
public:
                 sort_manager_obj(CMPR *cmp);
                 ~sort_manager_obj(void);    
    Q            MergeHeap;
    AMI_err      main_mem_operate(AMI_STREAM <T>*, AMI_STREAM <T> *, size_t);
    AMI_err      single_merge(AMI_STREAM<T> **, arity_t, AMI_STREAM<T> * );
    // size_t       space_usage_overhead(void);
};   

template<class T, class Q, class CMPR>
sort_manager_obj<T,Q,CMPR>::sort_manager_obj(CMPR *cmp) : MergeHeap(cmp)
{
    cmp_o = cmp;
}

template<class T, class Q, class CMPR>
sort_manager_obj<T,Q,CMPR>::~sort_manager_obj(void)
{
}

template<class T,class Q,class CMPR>
inline AMI_err sort_manager_obj<T,Q,CMPR>::single_merge ( 
                               AMI_STREAM < T > **inStreams, arity_t arity,
                               AMI_STREAM < T > *outStream ) {
   return AMI_single_merge_dh<T,Q>( inStreams, arity, outStream, MergeHeap );
};

template<class T, class Q, class CMPR>
inline AMI_err sort_manager_obj<T,Q,CMPR>::main_mem_operate(AMI_STREAM <T>*inStream, AMI_STREAM <T> *outStream, size_t runSize)
{
   AMI_err ae;
   T    *next_item;

   LOG_DEBUG_ID("starting sort_manager_obj.main_mem_operate. runSize is " << 
                runSize );
   tp_assert ( runSize <= mm_len, "memory load larger than buffer.");

   // Read a memory load out of the input stream one item at a time,
   // if key sorting fill up the key array at the same time.

   for (int i = 0; i < runSize; i++) {
      if ((ae=inStream->read_item (&next_item)) != AMI_ERROR_NO_ERROR) {
         LOG_FATAL_ID ("sort_manager_obj.main_mem_operate: read error" <<
                       ae);
         return ae;
      }
      mmStream[i] = *next_item;
   }

   //Sort the array.

   LOG_DEBUG_ID("sort_manager_obj.main_mem_operate: calling quicker_sort_obj");
   quicker_sort_obj ((T *) mmStream, runSize, cmp_o);

   LOG_DEBUG_ID("sort_manager_obj.main_mem_operate: starting write out");
   for (int i = 0; i < runSize; i++) {
      if ((ae = outStream->write_item (mmStream[i])) 
           != AMI_ERROR_NO_ERROR) {
         LOG_FATAL_ID ("sort_manager_obj.main_mem_operate: write error " << 
                       ae );
         return ae;
      }
   }
   LOG_DEBUG_ID("returning from sort_manager_obj.main_mem_operate");
   return AMI_ERROR_NO_ERROR;
}

//template<class T, class Q, class CMPR>
//size_t sort_manager_obj<T,Q,CMPR>::space_usage_overhead(void)
//{
//}

// *********************************************************************
// *                                                                   *
// * Comparison function based merge sort manager.                     *
// *                                                                   *
// *********************************************************************

template <class T, class Q>
class sort_manager_cmp   : public sort_manager<T,Q>  {
private:
    int (*cmp_f)(CONST T&, CONST T&);
public:
    Q              MergeHeap;
                   sort_manager_cmp (int (*cmp)(CONST T&, CONST T&));
                   ~sort_manager_cmp(void);    
    AMI_err        initialize();
    AMI_err        main_mem_operate(AMI_STREAM <T>*, AMI_STREAM <T>*, size_t);
    AMI_err        single_merge(AMI_STREAM<T> **, arity_t, AMI_STREAM<T> * );
    //size_t         space_usage_overhead(void);
};   


template<class T,class Q>
sort_manager_cmp<T,Q>::sort_manager_cmp(int (*cmp)(CONST T&, CONST T&) )
    : MergeHeap ( cmp ) {
    cmp_f = cmp;
}

template<class T, class Q>
sort_manager_cmp<T,Q>::~sort_manager_cmp(void) {
}

template<class T,class Q>
inline AMI_err sort_manager_cmp<T,Q>::single_merge ( 
                               AMI_STREAM < T > **inStreams, arity_t arity,
                               AMI_STREAM < T > *outStream ) {
   return AMI_single_merge_dh<T,Q>( inStreams, arity, outStream, MergeHeap );
};

template<class T,class Q>
inline AMI_err sort_manager_cmp<T,Q>::main_mem_operate(AMI_STREAM <T>*inStream, AMI_STREAM <T> *outStream, size_t runSize) {

   AMI_err ae;
   T    *next_item;

   LOG_DEBUG_ID("starting sort_manager_cmp.main_mem_operate. runSize is " << 
                runSize );
   tp_assert ( runSize <= mm_len, "memory load larger than buffer.");

   // Read a memory load out of the input stream one item at a time,
   // if key sorting fill up the key array at the same time.

   for (int i = 0; i < runSize; i++) {
      if ((ae=inStream->read_item (&next_item)) != AMI_ERROR_NO_ERROR) {
         LOG_FATAL_ID ("sort_manager_cmp.main_mem_operate: read error " <<
                       ae );
         return ae;
      }
      mmStream[i] = *next_item;
   }

   //Sort the array.

   LOG_DEBUG_ID("sort_manager_cmp.main_mem_operate: calling quicker_sort_cmp");
   quicker_sort_cmp ((T *) mmStream, runSize, cmp_f);

   LOG_DEBUG_ID("sort_manager_cmp.main_mem_operate: starting write out");
   for (int i = 0; i < runSize; i++) {
      if ((ae = outStream->write_item (mmStream[i])) 
           != AMI_ERROR_NO_ERROR) {
         LOG_FATAL_ID ("sort_manager_cmp.main_mem_operate: write error " <<
                       ae );
         return ae;
      }
   }
   LOG_DEBUG_ID("returning from sort_manager_cmp.main_mem_operate");
   return AMI_ERROR_NO_ERROR;
}

//template<class T,class Q>
//size_t sort_manager_cmp<T,Q>::space_usage_overhead(void){
//}

//template<class T, class Q>
//AMI_err sort_manager_cmp<T,Q>::initialize( ){
//}


// *********************************************************************
// *                                                                   *
// * Key + Object based merge sort manager.                          *
// *                                                                   *
// *********************************************************************

template <class T, class Q, class KEY, class CMPR>
class sort_manager_kobj  : public sort_manager<T,Q> {
private:
    qsort_item<KEY> *qs_array;
  //size_t          item_overhead;
    CMPR            *UsrObject;             
public:
                    sort_manager_kobj(CMPR *);    
                    ~sort_manager_kobj(void);    
    Q               MergeHeap;
    AMI_err         main_mem_operate_init(size_t szMemoryLoad);
    AMI_err         main_mem_operate(AMI_STREAM <T>*, AMI_STREAM <T>*, size_t);
    AMI_err         main_mem_operate_cleanup();
    AMI_err         single_merge(AMI_STREAM<T> **, arity_t, AMI_STREAM<T> * );
  // size_t          space_usage_overhead(void);
    inline int compare ( CONST qsort_item<KEY> &x, CONST qsort_item<KEY> &y) {
        return (*UsrObject).compare( x.keyval, y.keyval );};
};    

template<class T,class Q,class KEY,class CMPR>
sort_manager_kobj<T,Q,KEY,CMPR>::sort_manager_kobj(CMPR *cmp) : MergeHeap(cmp)
{
    UsrObject = cmp;
    item_overhead = sizeof(qsort_item<KEY>);
}

template<class T,class Q,class KEY,class CMPR>
sort_manager_kobj<T,Q,KEY,CMPR>::~sort_manager_kobj(void){
}

template<class T,class Q, class KEY,class CMPR>
inline AMI_err sort_manager_kobj<T,Q,KEY,CMPR>::single_merge ( 
                               AMI_STREAM < T > **inStreams, arity_t arity,
                               AMI_STREAM < T > *outStream ) {
   return AMI_single_merge_dh<T,Q>( inStreams, arity, outStream, MergeHeap);
};

template<class T,class Q,class KEY,class CMPR>
inline AMI_err sort_manager_kobj<T,Q,KEY,CMPR>::main_mem_operate_init(size_t szMemoryLoad) {
   LOG_DEBUG_ID("Starting sort_manager_kobj.main_mem_operate_init");
   mm_len        = szMemoryLoad;
   LOG_DEBUG_ID("sort_manager_kobj.main_mem_operate_init: allocating " 
                << szMemoryLoad*sizeof(T) << 
                "bytes for array mmStream<T>[" << szMemoryLoad << "]");
   mmStream      = new T[szMemoryLoad];
   LOG_DEBUG_ID("sort_manager_kobj.main_mem_operate_init: allocating " 
                << szMemoryLoad*sizeof(qsort_item <KEY>) << 
                "bytes for qs_array["<< szMemoryLoad << "]");
   qs_array      = new (qsort_item <KEY>)[szMemoryLoad];
   LOG_DEBUG_ID("Ending sort_manager_kobj.main_mem_operate_init");
   return AMI_ERROR_NO_ERROR;
}

template<class T,class Q,class KEY,class CMPR>
inline AMI_err sort_manager_kobj<T,Q,KEY,CMPR>::main_mem_operate(AMI_STREAM <T> *inStream, AMI_STREAM <T> *outStream, size_t runSize ) {

   AMI_err ae;
   T    *next_item;

   LOG_DEBUG_ID("starting sort_manager_kobj.main_mem_operate. runSize is " << 
                runSize );
   tp_assert ( runSize <= mm_len, "memory load larger than buffer.");

   // Read a memory load out of the input stream one item at a time,
   // if key sorting fill up the key array at the same time.

   for (int i = 0; i < runSize; i++) {
      if ((ae=inStream->read_item (&next_item)) != AMI_ERROR_NO_ERROR) {
         LOG_FATAL_ID ("sort_manager_kobj.main_mem_operate: read error " <<
                       ae);
         return ae;
      }
      mmStream[i] = *next_item;
      (*UsrObject).copy (&qs_array[i].keyval, *next_item);
      qs_array[i].source = i;
   }

   //Sort the array.
   LOG_DEBUG_ID("sort_manager_kobj.main_mem_operate: calling quicker_sort_obj");
   quicker_sort_obj ((qsort_item<KEY> *)qs_array, runSize, this );

   LOG_DEBUG_ID("starting sort_manager_kobj.main_mem_operate write out");
   for (int i = 0; i < runSize; i++) {
       if ((ae = outStream->write_item(mmStream[qs_array[i].source])) 
           != AMI_ERROR_NO_ERROR) {
          LOG_FATAL_ID ("sort_manager_kobj.main_mem_operate: write error " <<
                        ae);
         return ae;
      }
   }
   LOG_DEBUG_ID("returning from sort_manager_kobj.main_mem_operate");
   return AMI_ERROR_NO_ERROR;
}

template<class T,class Q,class KEY,class CMPR>
inline AMI_err sort_manager_kobj<T,Q,KEY,CMPR>::main_mem_operate_cleanup() {
   LOG_DEBUG_ID("Starting sort_manager_kobj.main_mem_operate_cleanup");
   if (mmStream) {
      delete[]mmStream;
      mmStream = NULL;
   }
   if (qs_array) {
      delete[]qs_array;
      qs_array = NULL;
   }
   LOG_DEBUG_ID("Ending sort_manager_kobj.main_mem_operate_cleanup");
   return AMI_ERROR_NO_ERROR;
}

//template<class T,class Q,class KEY,class CMPR>
//size_t sort_manager_kobj<T,Q,KEY,CMPR>::space_usage_overhead(void){
//    return item_overhead;
//}

// *******************************************************************
// *                                                                 *
// *               AMI_mergeX                                        *
// *                                                                 *
// *  This is a temporary AMI_merge entry point and should be removed
// *  soon. 2001/02/26  dh.                                          *
// ******************************************************************* 
template < class T, class CMPR >
    AMI_err 
AMI_mergeX (AMI_STREAM < T > **inStreams, arity_t arity,
		     AMI_STREAM < T > *outStream,   CMPR *cmp)
{
    merge_heap_dh_obj<T,CMPR> MergeHeap (cmp);
    MergeHeap.allocate (arity);
    return AMI_single_merge_dh ( inStreams, 
                                 arity, 
                                 outStream, 
                                 MergeHeap);  
}
// *******************************************************************
// *                                                                 *
// *           The actual AMI_merge calls                            *
// *                                                                 *
// *  These are the AMI_merge entry points for merging without the   *
// *  'merge_management_object' used by AMI_generalized_merge.      *
// *  These routines perform the special case of merging when the    *
// *  the required output is the original records interleaved        *
// *  according to a comparison operator or function.                *
// *                                                                 *
// ******************************************************************* 

// Merging with a heap that contains the records to be merged: CMPR is
// the class of the comparison object, and must contain the method
// 'compare' which is called from within the merge.

// TODO: 
//       1) check that memory management is done right 
//       2) add comparison operator and comparison function versions
//       3) watch out for conflicts with AMI_merge versions in
//       ami_optimized_merge.h

template < class T, class CMPR >
    AMI_err 
AMI_merge (AMI_STREAM < T > **inStreams, 
           arity_t arity,
	   AMI_STREAM < T > *outStream,   
           CMPR *cmp)
{
   // make a merge heap which uses the user's comparison object
   // and initialize it
   merge_heap_dh_obj<T,CMPR> mrgheap (cmp);
   mrgheap.allocate (arity);

   AMI_err ae = AMI_single_merge_dh ( inStreams, arity, outStream, mrgheap);  
   return ae;
}

// Merging with a heap that keeps a pointer to the records rather than
// the records themselves: CMPR is the class of the comparison object,
// and must contain the method 'compare' which is called from within
// the merge.

// TODO: 
//       1) check that memory management is done right 

template < class T, class CMPR >
    AMI_err 
AMI_ptr_merge (AMI_STREAM < T > **inStreams, 
           arity_t arity,
	   AMI_STREAM < T > *outStream,   
           CMPR *cmp)
{
   // make a merge heap of pointers which uses the user's comparison
   // object and initialize it
   merge_heap_pdh_obj<T,CMPR> mrgheap (cmp);
   mrgheap.allocate (arity);

   AMI_err ae = AMI_single_merge_dh ( inStreams, arity, outStream, mrgheap);  
   return ae;
}

// Merging with a heap that contains copies of the keys from the
// records being merged, rather than the records themselves: 

// The comparison object "cmp", of (user-defined) class represented by
// CMPR, must have a member function called "compare" which is used
// for merging the input streams, and a member function called "copy"
// which is used for copying the key (of type KEY) from a record of
// type T (the type to be sorted).

// TODO: 
//       1) check that memory management is done right 

template<class T, class KEY, class CMPR>
    AMI_err 
AMI_key_merge (AMI_STREAM < T > **inStreams, 
           arity_t arity,
	   AMI_STREAM < T > *outStream,   
           CMPR *cmp)
{
   // make a key merge heap which uses the user's comparison object
   // and initialize it
   merge_heap_dh_kobj<T,KEY,CMPR> mrgheap (cmp);
   mrgheap.allocate (arity);

   AMI_err ae = AMI_single_merge_dh ( inStreams, arity, outStream, mrgheap);  
   return ae;
}

// *******************************************************************
// *                                                                 *
// *           The actual AMI_sort calls                             *
// *                                                                 *
// ******************************************************************* 

// A version of AMI_sort that takes an input stream of elements of
// type T, an output stream, and a user-specified comparison function

template<class T>
AMI_err AMI_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
                 int (*cmp)(CONST T&, CONST T&))
{
    return AMI_partition_and_merge_dh(instream, outstream,
           sort_manager_cmp< T, merge_heap_dh_cmp<T> > (cmp) );
}

// A version of AMI_sort that takes an input stream of elements of type
// T, and an output stream, and and uses the < operator to sort

template<class T>
AMI_err AMI_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream)
{
    return AMI_partition_and_merge_dh(instream, outstream,
           sort_manager_op< T, merge_heap_dh_op<T> > () );
}

// A version of AMI_sort that takes an input stream of elements of
// type T, an output stream, and a user-specified comparison
// object. The comparison object "cmp", of (user-defined) class
// represented by CMPR, must have a member function called "compare"
// which is used for sorting the input stream.

template<class T, class CMPR>
AMI_err AMI_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
                 CMPR *cmp)
{
    return AMI_partition_and_merge_dh( instream, outstream,
     sort_manager_obj<T,merge_heap_dh_obj<T,CMPR>,CMPR>( cmp ) );
}

// ********************************************************************
// *                                                                  *
// *  These are the versions that keep a heap of pointers to records  *
// *                                                                  *
// ********************************************************************
// A version of AMI_sort that takes an input stream of elements of type
// T, and an output stream, and and uses the < operator to sort

template<class T>
AMI_err AMI_ptr_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream)
{
    return AMI_partition_and_merge_dh(instream, outstream,
           sort_manager_op< T, merge_heap_pdh_op<T> > () );
}
// A version of AMI_sort that takes an input stream of elements of
// type T, an output stream, and a user-specified comparison function

template<class T>
AMI_err AMI_ptr_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
                 int (*cmp)(CONST T&, CONST T&))
{
    return AMI_partition_and_merge_dh(instream, outstream,
           sort_manager_cmp< T, merge_heap_pdh_cmp<T> > (cmp) );
}
// A version of AMI_sort that takes an input stream of elements of
// type T, an output stream, and a user-specified comparison
// object. The comparison object "cmp", of (user-defined) class
// represented by CMPR, must have a member function called "compare"
// which is used for sorting the input stream.

template<class T, class CMPR>
AMI_err AMI_ptr_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
                 CMPR *cmp)
{
    return AMI_partition_and_merge_dh (instream, outstream,
     sort_manager_obj<T,merge_heap_pdh_obj<T,CMPR>,CMPR> (cmp) );
}

// ********************************************************************
// *                                                                  *
// *  This version keeps a heap of keys to records                    *
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
AMI_err 
AMI_key_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
	 KEY dummykey, CMPR *cmp) {
    return AMI_partition_and_merge_dh( instream, outstream, 
       sort_manager_kobj<T,merge_heap_dh_kobj<T,KEY,CMPR>,KEY,CMPR>( cmp ) );
}
                          
                          
#endif // _AMI_SORT_SINGLE_DH_H 
