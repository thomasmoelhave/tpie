// file: apm_dh.h 

#ifndef _APM_DH_H
#define _APM_DH_H
// **************************************************************************
// *                                                                        *
// *  This include file contains common routines                            *
// *    AMI_single_merge                                                    *
// *    AMI_partition_and_merge                                             *
// *  used in several of TPIE's sorting variants                            *
// *                                                                        *
// **************************************************************************

#include <math.h>		// For log(), etc  to compute tree heights.
#include <sys/time.h>
#include <assert.h>
#include <fstream.h>

#include <ami.h>
#include <ami_ptr.h>
#include <mergeheap_dh.h>	   //For templated heaps
#include <quicksort.h>		//For templated qsort_items
#include <ami_base.h>		// Get the base class, enums, etc...
#include <ami_device.h>		// Get the device description class
#include <ami_imps.h>		// Get an implementation definition

typedef int          AMI_merge_flag;
typedef unsigned int arity_t;

static inline void
makeName (char *prepre, char *pre, int id, char *dest)
{
   char tmparray[5];

   strcpy (dest, prepre);
   strcat (dest, pre);
   sprintf (tmparray, "%d", id);
   strcat (dest, tmparray);
}

// **************************************************************************
// *                                                                        *
// *   A M I _ s i n g l e _ m e r g e _ d h                                *
// *                                                                        *
// **************************************************************************

template < class T, class M >
    AMI_err 
AMI_single_merge_dh (AMI_STREAM < T > **inStreams, arity_t arity,
		     AMI_STREAM < T > *outStream,  M MergeHeap  )
{

   unsigned int i;
   AMI_err      ami_err;

   //Pointers to current leading elements of streams
   T *in_objects[arity + 1];

   // **************************************************************
   // * Rewind each input stream and read its first element.       *
   // **************************************************************

   for (i = 0; i < arity; i++) {

      if ((ami_err = inStreams[i]->seek (0)) != AMI_ERROR_NO_ERROR) {
	 return ami_err;
      }
      if ((ami_err = inStreams[i]->read_item (&(in_objects[i]))) !=
	  AMI_ERROR_NO_ERROR) {
	 if (ami_err == AMI_ERROR_END_OF_STREAM) {
	    in_objects[i] = NULL;
	 } else {
	    return ami_err;
	 }
      } else {
         MergeHeap.insert( in_objects[i], i );
      }
   }

   // *********************************************************
   // * Build a heap from the smallest items of each stream   *
   // *********************************************************

   MergeHeap.initialize ( );

   // *********************************************************
   // * Perform the merge until the inputs are exhausted.     *
   // *********************************************************
   while (MergeHeap.sizeofheap ()) {

      i = MergeHeap.get_min_run_id ();
 
      if ((ami_err = outStream->write_item (*in_objects[i]))
	  != AMI_ERROR_NO_ERROR) {
	 return ami_err;
      }

      if ((ami_err = inStreams[i]->read_item (&(in_objects[i])))
	  != AMI_ERROR_NO_ERROR) {
	 if (ami_err != AMI_ERROR_END_OF_STREAM) {
	    return ami_err;
	 }
      }

      if (ami_err == AMI_ERROR_END_OF_STREAM) {
	 MergeHeap.delete_min_and_insert ((T *) NULL);
      } else {
	 MergeHeap.delete_min_and_insert (in_objects[i]);
      }
   }	//while

   return AMI_ERROR_NO_ERROR;
}

// **************************************************************************
// *                                                                        *
// *   A M I _ p a r t i t i o n _ a n d _ m e r g e _ d h                  *
// *                                                                        *
// **************************************************************************

template < class T, class M >
    AMI_err
AMI_partition_and_merge_dh (AMI_STREAM < T > *inStream,
			    AMI_STREAM < T > *outStream, M mgmt_obj)
{
   AMI_err      ae;
   off_t        len;
   size_t       sz_avail, szStream;
   size_t       szSubstream;

   unsigned int ii, jj;
   unsigned int iiStreams;

   char         *working_disk;

   LOG_DEBUG_ID ("AMI_partition_and_merge START");

   // Figure out how much memory we've got to work with.

   sz_avail = MM_manager.memory_available ();

   //Conservatively assume that the memory for buffers for 
   //the two streams is unallocated; so we need to subtract.

   if ((ae = inStream->main_memory_usage 
            (&szStream,MM_STREAM_USAGE_MAXIMUM)) != AMI_ERROR_NO_ERROR) {
      LOG_DEBUG_ID ("Error returned from main_memory_usage(.,MM_STREAM_USAGE_MAXIMUM");
      return ae; // LOG_FATAL was reported in main_memory_usage
   }
   if ((ae = inStream->main_memory_usage 
            (&szSubstream,MM_STREAM_USAGE_OVERHEAD)) != AMI_ERROR_NO_ERROR) {
      LOG_DEBUG_ID ("Error returned from inStream->main_memory_usage");
      return ae;
   }
   sz_avail -= 2 * szStream;

   working_disk = ami_single_temp_name ("Temp");

   // ***************************************************************
   // * If the input stream fits into main memory, special case     *
   // ***************************************************************

   len = inStream->stream_len ();
   inStream->seek (0);

   if (mgmt_obj.sort_fits_in_memory (inStream, sz_avail)){
      if ((ae= mgmt_obj.main_mem_operate_init (len) ) !=
	       AMI_ERROR_NO_ERROR) {
         LOG_FATAL_ID ("main_mem_operate_init failed");
         return ae;
      }
      if ((ae = mgmt_obj.main_mem_operate (inStream, outStream, len)) !=
	 AMI_ERROR_NO_ERROR) {
	 LOG_FATAL_ID ("main_mem_operate failed");
	 return ae;
      }
      mgmt_obj.main_mem_operate_cleanup ();

      return AMI_ERROR_NO_ERROR;
   };				// else 

   // ******************************************************************
   // * Input stream too large for main memory, use general merge sort *
   // ******************************************************************

   LOG_DEBUG_ID( "Beginning general merge sort." );

   // The number of substreams that the original input stream
   // will be split into.

   arity_t origSubstreams;

   // The length, in terms of stream objects of type T, of the
   // original substreams of the input stream.  The last one may
   // be shorter than this.

   size_t szOrigSubstream;

   // The initial temporary stream, to which substreams of the
   // original input stream are written.

   AMI_STREAM < T > **initialTmpStream;

   // The number of substreams that can be merged together at once.

   arity_t mrgArity;

   // A pointer to the buffer in main memory to read a memory load into.

   //T *mmStream;

   // Loop variables:

   // The stream being read at the current level.

   AMI_STREAM < T > **currInput;

   // The output stream for the current level if it is not outStream.

   AMI_STREAM < T > **tmpStream;

   //RAKESH  FIX THIS: Need to generate random strings using
   //tmpname() or something like that.
   char *prefixName[] = { "_0_", "_1_" };

   // The size of substreams of *currInput that are being
   // merged.  The last one may be smaller.  This value should be
   // szOrigSubstream * (mrgArity ** mrgHgt) where mrgHgt is the
   // number of iterations the loop has gone through.
   
   unsigned int mrgHgt;                 //Merge Level
   off_t        sub_start, sub_end;

   // How many substreams will there be?  The main memory
   // available to us is the total amount available, minus what
   // is needed for the input stream and the temporary stream.

   //RAKESH In our case mrgArity is determined differently than in the
   //orig implementation of AMI_partition_and_merge since we use
   //several streams in each level.  In our case net main memory
   //required to carry out an R-way merge is

   // (R+1)*MM_STREAM_USAGE_MAXIMUM  {R substr input runs, 1 stream output}
   // + R*MM_STREAM_USAGE_OVERHEAD   {1 stream each active input run: but 
   //                                 substreams use buffers, streams don't}
   // + (R+1)*m_obj->space_usage_per_stream();
   //
   // The net memory usage for an R-way merge is thus
   // R*(szStream + sz_substream + space_usage_per_stream()) + szStream +
   // space_usage_per_stream();

   //To support a binary merge, need space for max_stream_usage
   //for at least three stream objects.

   if (sz_avail <= 3 *(szStream + szSubstream + sizeof(heap_element < T >))) {
      LOG_FATAL_ID ("Insufficient Memory for AMI_partition_and_merge");
      return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
   }

   szOrigSubstream = (sz_avail) / sizeof (T);

   // Round the original substream length up to an integral number of
   // chunks.  This is for systems like HP-UX that cannot map in
   // overlapping regions.  It is also required for BTE's that are
   // capable of freeing chunks as they are read.

   size_t chunkSize = inStream->chunk_size ();
   szOrigSubstream = chunkSize *((szOrigSubstream + chunkSize-1) / chunkSize);
   origSubstreams = (len + szOrigSubstream - 1) / szOrigSubstream;

   //Available memory for input stream objects is given by 
   //sz_avail minus the space occupied by output stream objects.
   size_t sz_avail_during_merge = sz_avail - szStream - szSubstream;

   //This counts the per-input stream memory cost.
   size_t szPerInputStream = szStream + szSubstream + sizeof(heap_element<T>);

   //Compute merge arity
   mrgArity = sz_avail_during_merge / szPerInputStream;

   // Make sure that the AMI is willing to provide us with the
   // number of substreams we want.  It may not be able to due to
   // operating system restrictions, such as on the number of
   // regions that can be mmap()ed in.
   
   int availableStreams = inStream->available_streams ();

   if (availableStreams != -1) {
      if (availableStreams <= 5) {
	 LOG_FATAL_ID ("Not enough substreams available to perform merge.");
	 return AMI_ERROR_INSUFFICIENT_AVAILABLE_STREAMS;
      }

      if (mrgArity > (arity_t) availableStreams - 2) {
	 mrgArity = availableStreams - 2;
	 LOG_WARNING_ID ("Reduced merge arity due to AMI restrictions.\n");
      }
   }

   LOG_DEBUG_ID ("merge arity = " << mrgArity << ".\n");

   if (mrgArity < 2) {
      LOG_FATAL_ID ("Merge arity < 2! Insufficient memory for a merge.");
      return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
   }

//#define MINIMIZE_INITIAL_SUBSTREAM_LENGTH
#ifdef MINIMIZE_INITIAL_SUBSTREAM_LENGTH
   LOG_DEBUG_ID ("Minimizing initial run lengths without increasing" <<
		    "the height of the merge tree.");
   // Make the substreams as small as possible without increasing
   // the height of the merge tree.

   // The tree height is the ceiling of the log base mrgArity of the
   // number of original substreams.

   double tree_height = log((double)origSubstreams) / log((double)mrgArity);

   tp_assert (tree_height > 0, "Negative or zero tree height!");

   tree_height = ceil (tree_height);

   // See how many substreams we could possibly fit in the
   // tree without increasing the height.

   double maxOrigSubstreams = pow ((double) mrgArity, tree_height);

   tp_assert (maxOrigSubstreams >= origSubstreams,
		 "Number of permitted substreams was reduced.");

   // How big will such substreams be?

   double new_szOrigSubstream = ceil ((double) max lenOrigSubstreams);

   tp_assert (new_szOrigSubstream <= szOrigSubstream,
		 "Size of original streams increased.");

   szOrigSubstream = (size_t) new_szOrigSubstream;

   LOG_DEBUG_ID ("Memory constraints set original substreams = " <<
		    origSubstreams << '\n');

   origSubstreams = (len + szOrigSubstream - 1) / szOrigSubstream;

   LOG_DEBUG_ID ("Tree height constraints set original substreams = "
		    << origSubstreams << '\n');
#endif				// MINIMIZE_INITIAL_SUBSTREAM_LENGTH

   // Create a temporary stream, then iterate through the
   // substreams, processing each one and writing it to the
   // corresponding substream of the temporary stream.

   unsigned int runLens[2][mrgArity][(origSubstreams+mrgArity-1) / mrgArity];
   int          Sub_Start[mrgArity];

   // for (int i = 0; i < 2; i++)
   //    for (int j = 0; j < mrgArity; j++)
   //       for (int k1 = 0; 
   //            k1 <  (origSubstreams+mrgArity-1)/mrgArity;
   //            k1++)
   //          runLens[i][j][k1] = 0;                      

   memset ((void *) runLens, 0,
   	   2 * mrgArity * ((origSubstreams + mrgArity - 1) /
   			      mrgArity) * sizeof (unsigned int));

   initialTmpStream = new (AMI_STREAM < T > *)[mrgArity];
   LOG_DEBUG_ID("Allocated space for " << mrgArity <<
                 " initialTmpStream pointers\n");

   if ((ae= mgmt_obj.main_mem_operate_init(szOrigSubstream)) !=
	    AMI_ERROR_NO_ERROR) {
      LOG_FATAL_ID ("main_mem_operate_init failed");
      return ae;
   }
   
   inStream->seek (0);

   tp_assert (origSubstreams * szOrigSubstream - len < szOrigSubstream,
	      "Total substream length too long or too many.");
   tp_assert (len - (origSubstreams - 1) * szOrigSubstream <= szOrigSubstream,
	      "Total substream length too short or too few.");

   size_t check_size            = 0;
   int    currStream            = mrgArity - 1;
   int    runsInCurrStream      = 0;
   int    reqdRuns [mrgArity];
   char   newName [BTE_PATH_NAME_LEN];

   //For the first stream:
   for (iiStreams = 0; iiStreams < mrgArity; iiStreams++) {

      // Figure out how many runs go in each one of mrgArity
      // streams?  If there are 12 runs to be distributed among 5
      // streams, the first three get 2 and the last two get 3 runs

      if (iiStreams < (mrgArity - (origSubstreams % mrgArity)))
	 reqdRuns[iiStreams] = origSubstreams / mrgArity;
      else
	 reqdRuns[iiStreams] = (origSubstreams + mrgArity - 1) / mrgArity;
   }

#ifdef BTE_IMP_USER_DEFINED
   makeName ("", prefixName[0], currStream, newName);
#else
   //The assumption here is that working_disk is the name of the specific 
   //directory in which the temporary/intermediate streams will be made.
   makeName (working_disk, prefixName[0], currStream, newName);
#endif

   initialTmpStream[currStream] = new AMI_STREAM < T > (newName);
   LOG_DEBUG_ID("Allocated space for initialTmpStream[" << currStream <<
                 "]\n");
   initialTmpStream[currStream]->persist (PERSIST_PERSISTENT);

   ii = 0;
   while (ii < origSubstreams) {
      off_t mm_len;

      // ****************************************************************
      // * Make sure that the current stream is supposed to get a run   *
      // ****************************************************************

      if (reqdRuns[currStream] > runsInCurrStream) {
	 if (ii == origSubstreams - 1) {
	    mm_len = len % szOrigSubstream;

	    // If it is an exact multiple, then the mod will come out
	    // 0, which is wrong.

	    if (!mm_len) {
	       mm_len = szOrigSubstream;
	    }
	 } else {
	    mm_len = szOrigSubstream;
	 }

         if ((ae = mgmt_obj.main_mem_operate (inStream, initialTmpStream[currStream], mm_len)) !=
	    AMI_ERROR_NO_ERROR) {
	    LOG_FATAL_ID ("main_mem_operate failed");
	    return ae;
         }

         runLens[0][currStream][runsInCurrStream] = mm_len;
	 runsInCurrStream++;
	 ii++;
      } // end  if (reqdRuns[currStream] > runsInCurrStream)

      if (runsInCurrStream == reqdRuns[currStream]) {

	 check_size += initialTmpStream[currStream]->stream_len ();

	 // We do not want old streams hanging around
	 // occuping memory. We know how to get the streams
	 // since we can generate their names

	 if (initialTmpStream[currStream]) {
	    delete initialTmpStream[currStream];
	    initialTmpStream[currStream] = NULL;
	 }

	 if (check_size < inStream->stream_len ()) {
	    currStream = (currStream + mrgArity - 1) % mrgArity;
#ifdef BTE_IMP_USER_DEFINED
	    makeName ("", prefixName[0], currStream, newName);
#else
	    makeName (working_disk, prefixName[0], currStream, newName);
#endif
	    initialTmpStream[currStream] = new AMI_STREAM < T > (newName);
            LOG_DEBUG_ID("Allocated space for initialTmpStream[" << 
                 currStream << "]\n");
	    initialTmpStream[currStream]->persist (PERSIST_PERSISTENT);
	    runsInCurrStream = 0;
	 }
      } // end if (runsInCurrStream == reqdRuns[currStream])
   } // end while (ii < origSubstreams)

   if (initialTmpStream[currStream]) {
      delete initialTmpStream[currStream];
      initialTmpStream[currStream] = NULL;
   }

   //if (mmStream) {
   //   delete[]mmStream;
   //   mmStream = NULL;
   //}
   mgmt_obj.main_mem_operate_cleanup ();

   // Make sure the total length of the temporary stream is the
   // same as the total length of the original input stream.
   tp_assert (inStream->stream_len () == check_size,
	      "Stream lengths do not match:" <<
	      "\n\tinStream->stream_len() = " << inStream->stream_len ()
	      << "\n\tinitialTmpStream->stream_len() = " << check_size
	      << ".\n");

   LOG_DEBUG_ID ("Initial number of runs " << origSubstreams << "\n");
   LOG_DEBUG_ID ("Merge arity is " << mrgArity << "\n");

   // Pointers to the substreams that will be merged.
      
   AMI_STREAM < T > **theSubstreams = new (AMI_STREAM < T > *)[mrgArity];
   LOG_DEBUG_ID("Allocated space for " << mrgArity << 
                " theSubstreams pointers\n");

   mrgHgt = 0;
   currInput = initialTmpStream;
   // Allocate room for the merge heap
   mgmt_obj.MergeHeap.allocate( mrgArity );

   // ***********************************************************************
   // *                                                                     *
   // * The main loop.  At the outermost level we are looping over          *
   // * levels of the merge tree.  Typically this will be very small,       *
   // * e.g. 1-3.                                                           *
   // *                                                                     *
   // ***********************************************************************

   // The number of substreams to be processed at any merge level.
   arity_t ssCount;

   for (ssCount = origSubstreams; ssCount > 1;
        ssCount = (ssCount + mrgArity - 1) / mrgArity) {

      // Set up to process a given level.

      tp_assert (len == check_size,
		 "Current level stream not same length as input." <<
		 "\n\tlen = " << len <<
		 "\n\tcurrInput->stream_len() = " <<
		 check_size << ".\n");

      check_size = 0;

      // Do we have enough main memory to merge all the
      // substreams on the current level into the output stream?
      // If so, then we will do so, if not then we need an
      // additional level of iteration to process the substreams
      // in groups.

      if (ssCount <= mrgArity) {

         // Open up the ssCount streams in which the
         // the runs input to the current merge level are packed
         // The names of these streams (storing the input runs)
         // can be constructed from  prefixName[mrgHgt % 2]

	 for (ii = mrgArity - ssCount; ii < mrgArity; ii++) {

#ifdef  BTE_IMP_USER_DEFINED
	    makeName ("", prefixName[mrgHgt % 2], (int) ii, newName);
#else
	    makeName (working_disk, prefixName[mrgHgt % 2], (int) ii, newName);
#endif
	    currInput[ii] = new AMI_STREAM < T > (newName);
            LOG_DEBUG_ID("Allocated space for currInput[" << ii << "]\n");
	    currInput[ii]->persist (PERSIST_DELETE);
	 }

	 // Merge them into the output stream.
	 // currInput is address( address (the first input stream) )

         ae = mgmt_obj.single_merge (currInput + mrgArity - ssCount, 
                                    ssCount, outStream );

	 if (ae != AMI_ERROR_NO_ERROR) {
            LOG_FATAL_ID ("AMI_ERROR " << ae << " returned by single_merge");
	    return ae;
	 }

	 // Delete the streams input to the above merge.
	 for (ii = mrgArity - ssCount; ii < mrgArity; ii++) {
	    if (currInput[ii]) {
	       delete currInput[ii];
	       currInput[ii] = NULL;
	    }
	 }
	 if (currInput) {
	    delete[]currInput;
	    currInput = initialTmpStream = NULL;
	 }
	 if (theSubstreams) {
	    delete[]theSubstreams;
	    theSubstreams = NULL;
	 }

      } else { // (ssCount > mrgArity)

	 LOG_DEBUG_ID ("Merging substreams to intermediate streams.\n");

	 // Create the array of mrgArity stream pointers that
	 // will each point to a stream containing runs output
	 // at the current level mrgHgt. 

	 tmpStream = new (AMI_STREAM < T > *)[mrgArity];
         LOG_DEBUG_ID("Allocated space for " << mrgArity << 
                      " tmpStream pointers\n");

         // Open up the mrgArity streams in which the
         // the runs input to the current merge level are packed
         // The names of these streams (storing the input runs)
         // can be constructed from  prefixName[mrgHgt % 2]

	 for (ii = 0; ii < mrgArity; ii++) {
#ifdef BTE_IMP_USER_DEFINED
	    makeName ("", prefixName[mrgHgt % 2], (int) ii, newName);
#else
	    makeName (working_disk, prefixName[mrgHgt % 2], (int) ii, newName);
#endif
	    currInput[ii] = new AMI_STREAM < T > (newName);
            LOG_DEBUG_ID("Allocated space for currInput[" << ii << "]\n");
	    currInput[ii]->persist (PERSIST_DELETE);
	 }

	 // Fool the OS into unmapping the current block of the
	 // input stream so that blocks of the substreams can
	 // be mapped in without overlapping it.  This is
	 // needed for correct execution on HP-UX.
         //RAKESH
         //                currInput->seek(0);

	 currStream = mrgArity - 1;

	 //For the first stream that we use to pack some    
	 //of the output runs of the current merge level mrgHgt.

#ifdef BTE_IMP_USER_DEFINED
	 makeName("", prefixName[(mrgHgt + 1) % 2], currStream, newName);
#else
	 makeName(working_disk, prefixName[(mrgHgt + 1) % 2], currStream, newName);
#endif

	 tmpStream[currStream] = new AMI_STREAM< T >(newName);
         LOG_DEBUG_ID("Allocated space for tmpStream[" << currStream << "]\n");
	 tmpStream[currStream]->persist (PERSIST_PERSISTENT);

	 int outRunsLeft = (ssCount + mrgArity - 1) / mrgArity;

	 for (iiStreams = 0; iiStreams < mrgArity; iiStreams++) {

	    // If there are 12 runs to be distributed among 5 streams, 
	    // the first three get 2 and the last two  get 3 runs   

	    if (iiStreams < (mrgArity - (outRunsLeft % mrgArity)))
	       reqdRuns[iiStreams] = outRunsLeft / mrgArity;
	    else
	       reqdRuns[iiStreams] = (outRunsLeft + mrgArity-1) / mrgArity;

	    Sub_Start[iiStreams] = 0;
	 }

	 unsigned int mergeNo = 0;
	 runsInCurrStream = 0;

         // **************************************************************
	 // * Loop through the substreams of the current stream, merging *
	 // * as many as we can at a time until all are done with.       *
         // **************************************************************

         //  [mrgHgt % 2]   lets us alternate between indices 0 and 1.
         //  jj        is number of substreams to be added so far to 
         //            the merge.
         //  ssx       is the index of the substream currently being 
         //            considered. ssx = mrgArity - 1 - jj


	 for (ii = 0, sub_start = 0, jj = 0; ii < ssCount; ii++) {

            int ssx = mrgArity - 1 - jj;
	    if (runLens[mrgHgt % 2][ssx][mergeNo]!= 0) {
	       sub_start       = Sub_Start[ssx];
	       sub_end         = sub_start + runLens[mrgHgt%2][ssx][mergeNo] - 1;
	       Sub_Start[ssx] += runLens[mrgHgt % 2][ssx][mergeNo];
	       runLens[mrgHgt % 2][ssx][mergeNo] = 0;
	    } else {
	       sub_end   = Sub_Start[ssx] - 1;
	       sub_start = sub_end + 1;
	       ii--;
	    }

	    //Open the new substream
	    currInput[ssx]-> new_substream 
                     ( AMI_READ_STREAM, sub_start, sub_end,
		       (AMI_base_stream < T > **) (theSubstreams + jj) );

            // ***********************************************************
	    // * The substreams are read-once.  If we've got all we can  *
	    // * handle or we've seen them all, then merge them.         *
            // ***********************************************************

	    if ( (jj >= mrgArity - 1)||(ii == ssCount - 1) ) {

	       tp_assert( jj<=mrgArity-1, "Index got too large." );

	       //Check if the stream into which runs are cuurently
	       //being packed has got its share of runs. If yes,
	       //'delete' that stream and construct a new stream

	       if (reqdRuns[currStream] == runsInCurrStream) {

		  //Make sure that the deleted stream persists on disk.
		  tmpStream[currStream]->persist (PERSIST_PERSISTENT);
		  delete tmpStream[currStream];

		  currStream = (currStream + mrgArity - 1) % mrgArity;

		  // Unless the current level is over, we've to
		  // generate a new stream for the next set of runs.

		  if (outRunsLeft > 0) {
#ifdef BTE_IMP_USER_DEFINED
		     makeName ("", prefixName[(mrgHgt + 1) % 2],
				       (int) currStream, newName);
#else
		     makeName (working_disk, prefixName[(mrgHgt + 1) % 2],
				       (int) currStream, newName);
#endif
		     tmpStream[currStream] = new AMI_STREAM < T > (newName);
                     LOG_DEBUG_ID("Allocated space for tmpStream[" << currStream << "]\n");
		     tmpStream[currStream]->persist(PERSIST_PERSISTENT);
		     runsInCurrStream = 0;
		  }
	       }

	       ae = mgmt_obj.single_merge ( theSubstreams, jj + 1,
		     tmpStream [currStream] );
	       if (ae != AMI_ERROR_NO_ERROR) {
		  LOG_FATAL_ID ("AMI_single_merge error");
		  return ae;
	       }

	       for (iiStreams = 0; iiStreams < jj + 1; iiStreams++)
		  runLens[(mrgHgt + 1) % 2][currStream][runsInCurrStream] 
                      += theSubstreams[iiStreams]->stream_len ();

	       mergeNo++;

	       //Decrement the number of runs still to be formed at
	       //current level

	       outRunsLeft--;

	       // Delete input substreams. jj is the index of the last one.

	       for (iiStreams = 0; iiStreams < jj + 1; iiStreams++) {
		  if (theSubstreams[iiStreams]) {
		     delete theSubstreams[iiStreams];
		     theSubstreams[iiStreams] = NULL;
		  }
	       }

	       jj = 0;
	       runsInCurrStream++;
	    } else { // Keep adding substreams
	       jj++;
	    }
	 }

	 if (tmpStream[currStream]) {
	    delete tmpStream[currStream];
	    tmpStream[currStream] = NULL;
	 }

	 // Get rid of the current input streams and use the ones
	 // output at the current level.

	 for (ii = 0; ii < mrgArity; ii++)
	    if (currInput[ii]) {
	       delete currInput[ii];
	    }
	 if (currInput) {
	    delete[]currInput;
	    currInput = NULL;
	 }
	 currInput = (AMI_STREAM < T > **)  tmpStream;
      }
      mrgHgt++;
   }
   //Monitoring prints.
   LOG_DEBUG_ID ("Number of passes incl run formation is " << mrgHgt+1 << "\n");
   LOG_DEBUG_ID ("AMI_partition_and_merge END");
   return AMI_ERROR_NO_ERROR;
   // Deallocate the merge heap
   mgmt_obj.MergeHeap.deallocate( );
}
#endif _APM_DH_H
