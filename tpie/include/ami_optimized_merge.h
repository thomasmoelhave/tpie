//
// File: optimized_ami_merge.h
// Author: Rakesh Barve
//
// Function: AMI_partition_and_merge() was modified from Darren's
// original version, so as to ensure "sequential access."
// The function AMI_single_merge(), which uses a merge management 
// object and a priority queue class to carry out internal memory
// merging computation, now has a "pure C" alternative that seems to
// perform better by a huge margin: THis function is called 
// MIAMI_single_merge() and is based on a simple heap data structure
// straight out of CLR (Introduction to ALgorithms) in Simple_Heap.h
// There is also a merge using  replacement selection based run formation.
// There is also a provision to use a run formation that uses 
// a quicksort using only keys of the items; there is a provision to 
// to use templated heaps to implement the merge.

//TO DO: substream_count setting; don't depend on current_stream_len

#ifndef _OPT_AMI_MERGE_H
#define _OPT_AMI_MERGE_H

#include <ami_ptr.h>
#include <fstream.h>

// For log() and such as needed to compute tree heights.
#include <math.h>

#include <sys/time.h>


typedef int AMI_merge_flag;
typedef unsigned int arity_t;

#define CONST 


//Defined at the end of the file, declared here.

#define RF_VERIFY_OPT 0
#define SORT_VERIFY_OPT 0
#define WANT_AMI_READ_STREAMS 0
#define WANT_AMI_WRITEONLY_STREAMS 1
#define TRY_SYSTEM_QSORT_INTS 0
#define  ALT_DISK_CONFIG  0



//For templated heaps
#include <mergeheap.h>

//For templated qsort_items
#include <quicksort.h>

// Get the base class, enums, etc...
#include <ami_base.h>

// Get the device description class
#include <ami_device.h>

// Get an implementation definition
#include <ami_imps.h>




#ifdef BTE_IMP_STDIO
#define BTE_PATH_NAME_LEN BTE_STDIO_PATH_NAME_LEN
#endif
#ifdef BTE_IMP_UFS
#define BTE_PATH_NAME_LEN BTE_UFS_PATH_NAME_LEN
#endif
#ifdef BTE_IMP_MMB
#define BTE_PATH_NAME_LEN BTE_MMB_PATH_NAME_LEN
#endif
#ifdef BTE_IMP_USER_DEFINED
#define BTE_PATH_NAME_LEN BTE_STRIPED_PATH_NAME_LEN
#endif




template<class KEY> class run_formation_item
{
public:
KEY Key;
unsigned int RecordPtr;
unsigned int Loser; 
short RunNumber;
unsigned int ParentExt;      
unsigned int ParentInt;

friend int operator==(const run_formation_item &x, const run_formation_item &y)
             {return  (x.Key ==  y.Key);};

friend int operator!=(const run_formation_item &x, const run_formation_item &y)
             {return  (x.Key !=  y.Key);}  ; 

friend int operator<=(const run_formation_item &x, const run_formation_item &y)
             {return  (x.Key <=  y.Key);};

friend int operator>=(const run_formation_item &x, const run_formation_item &y)
            {return  (x.Key >=  y.Key);};

friend int operator<(const run_formation_item &x, const run_formation_item &y)
             {return  (x.Key <  y.Key);};

friend int operator>(const run_formation_item &x, const run_formation_item &y)
             {return  (x.Key >  y.Key);};

 };



template<class T, class KEY> AMI_err Run_Formation_Algo_R_Key( AMI_STREAM<T>
*, arity_t , AMI_STREAM<T> **, char * , size_t , int * , int ** , int ,
int , int, KEY);

template<class T, class KEY> AMI_err MIAMI_single_merge_Key(AMI_STREAM<T> **,
 arity_t , AMI_STREAM<T> *, int , KEY);

template<class T, class KEY> AMI_err MIAMI_single_merge_Key_scan(AMI_STREAM<T> **,
 arity_t , AMI_STREAM<T> *, int , KEY);


static inline void stream_name_generator(char *prepre, char * pre, int id, char * dest)
{
  char tmparray[5];

  strcpy(dest,prepre);
  if (strcmp(dest,"") != 0) strcat(dest,"/");
  strcat(dest,pre);
  sprintf(tmparray,"%d",id);
  strcat(dest,tmparray);
}





// Recursive division of a stream and then merging back together.
template<class T>
AMI_err AMI_partition_and_merge_stream(AMI_STREAM<T> *instream,
                                AMI_STREAM<T> *outstream)
{ 
    AMI_err ae;
    off_t len;
    size_t sz_avail, sz_stream;
    size_t sz_substream;

    //Monitoring 
    struct timeval tp1, tp2;
    double tp_dub1,tp_dub2,tp_dub3;



    unsigned int ii, jj, kk;
    int ii_streams;

    //Monitoring
    char *Cache_For_Names;
    char *working_disk;
    

    // Figure out how much memory we've got to work with.

    if (MM_manager.available(&sz_avail) != MM_ERROR_NO_ERROR) {
        return AMI_ERROR_MM_ERROR;
    }

    //Monitoring
    cout << "RAKESH: Avail memory (new keyless fun) is " << sz_avail << "\n";

    //Conservatively assume that the memory for buffers for 
    //the two streams is unallocated; so we need to subtract.

    if ((ae = instream->main_memory_usage(&sz_stream,
                                              MM_STREAM_USAGE_MAXIMUM)) !=
                                              AMI_ERROR_NO_ERROR) {
            return ae;
        }                                     

    if ((ae = instream->main_memory_usage(&sz_substream,
                                              MM_STREAM_USAGE_OVERHEAD)) !=
                                              AMI_ERROR_NO_ERROR) {

              return ae;
        }   

    sz_avail -= 2*sz_stream;

    cout << "Avail memory after accounting for input streams' memory" << sz_avail << endl;


    working_disk = getenv("TMP");

    // If the whole input can fit in main memory then just call
    // AMI_main_mem_merge() to deal with it by loading it once and
    // processing it.

    len = instream->stream_len();
    instream->seek(0);
    
    cout << "Instream length is " << len << "\n";

    
    if ((len * sizeof(T)) <= sz_avail) 

          {
           
           T * next_item;
           T * mm_stream = new T[len];

           for (int i = 0; i <  len; i++)
           {
            if ((ae =  instream->read_item(&next_item)) != AMI_ERROR_NO_ERROR)
                                                  return ae;
            mm_stream[i] = *next_item;
           }

           quicker_sort_op((T *)mm_stream,len);

            for (int i = 0; i <  len; i++)
           {
            if ((ae = outstream->write_item( mm_stream[i])) 
                                     != AMI_ERROR_NO_ERROR)
                                                 return ae;
           }

    } else {

        // The number of substreams that the original input stream
        // will be split into.
        
        arity_t original_substreams;

        // The length, in terms of stream objects of type T, of the
        // original substreams of the input stream.  The last one may
        // be shorter than this.
        
        size_t sz_original_substream;

        // The initial temporary stream, to which substreams of the
        // original input stream are written.

        //RAKESH
        AMI_STREAM<T> **initial_tmp_stream;
        
        // The number of substreams that can be merged together at once.

        arity_t merge_arity;

        // A pointer to the buffer in main memory to read a memory load into.
        T *mm_stream;

        
        // Loop variables:

        // The stream being read at the current level.
        
        //RAKESH
        AMI_STREAM<T> **current_input;

        // The output stream for the current level if it is not outstream.

        //RAKESH
        AMI_STREAM<T> **intermediate_tmp_stream;
        
       //RAKESH  FIX THIS: Need to generate random strings using
	   //tmpname() or something like that.
	   char * prefix_name[] = {"Tempo_stream0", "Tempo_stream1"};
        char itoa_str[5];


        // The size of substreams of *current_input that are being
        // merged.  The last one may be smaller.  This value should be
        // sz_original_substream * (merge_arity ** k) where k is the
        // number of iterations the loop has gone through.
        

        //Merge Level
        unsigned int k;

        off_t sub_start, sub_end;

        // How many substreams will there be?  The main memory
        // available to us is the total amount available, minus what
        // is needed for the input stream and the temporary stream.




//RAKESH
// In our case merge_arity is determined differently than in the original
// implementation of AMI_partition_and_merge since we use several streams
// in each level.
// In our case net main memory required to carry out an R-way merge is
// (R+1)*MM_STREAM_USAGE_MAXIMUM  {R substreams for input runs, 1 stream for output}
// + R*MM_STREAM_USAGE_OVERHEAD   {One stream for each active input run: but while
//				   the substreams use buffers, streams don't}
// + (R+1)*m_obj->space_usage_per_stream();
//
// The net memory usage for an R-way merge is thus
// R*(sz_stream + sz_substeam + m_obj->space_usage_per_stream()) + sz_stream +
// m_obj->space_usage_per_stream();
//
        
     
        

	   //To support a binary merge, need space for max_stream_usage
	   //for at least three stream objects.

         if (sz_avail <= 3*(sz_stream + sz_substream 
                            + sizeof(merge_heap_element<T>))

                            ) 
         {
   
		 cout << "Insuff memory\n";
            return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
         }
 
        
       sz_original_substream = (sz_avail)/sizeof(T);

        // Round the original substream length off to an integral
        // number of chunks.  This is for systems like HP-UX that
        // cannot map in overlapping regions.  It is also required for
        // BTE's that are capable of freeing chunks as they are
        // read.

        {
         size_t sz_chunk_size = instream->chunk_size();
            
         sz_original_substream = sz_chunk_size *
                ((sz_original_substream + sz_chunk_size - 1) /
                 sz_chunk_size);
        }

        original_substreams = (len + sz_original_substream - 1) /
            sz_original_substream;
        
        // Account for the space that a merge object will use.

        {
		//Availabe memory for input stream objects is given by 
		//sz_avail minus the space occupied by output stream objects.
            size_t sz_avail_during_merge = sz_avail - 
                                           sz_stream - sz_substream;


		  //This conts the per-input stream memory cost.
            size_t sz_stream_during_merge =sz_stream + sz_substream +
                 sizeof(merge_heap_element<T>);
           

            //Compute merge arity
            merge_arity = sz_avail_during_merge/sz_stream_during_merge;


//TRASH
            cout << "Original mem size available " << sz_avail << "\n";
            cout << "Max stream overhead " << sz_stream << "\n";
            cout <<"Merge Heap Element Size "<< sizeof(merge_heap_element<T>)
                                             << "\n";
            cout << "Availablity after accounting for output stream" 
                 << sz_avail_during_merge << "\n";
	       cout << "Overhead Per input run " 
                 <<  sz_stream_during_merge << "\n";
            cout << "Thus merge arity is " << merge_arity << "\n";	
        }

        // Make sure that the AMI is willing to provide us with the
        // number of substreams we want.  It may not be able to due to
        // operating system restrictions, such as on the number of
        // regions that can be mmap()ed in.

        {
            int ami_available_streams = instream->available_streams();

            cout << "Available_streams " << ami_available_streams << "\n";

            if (ami_available_streams != -1) {
                    if (ami_available_streams <= 5) {
                    return AMI_ERROR_INSUFFICIENT_AVAILABLE_STREAMS;
                }
                
                if (merge_arity > (arity_t)ami_available_streams - 2) {
                    merge_arity = ami_available_streams - 2;
                    LOG_INFO("Reduced merge arity due to AMI restrictions.\n");
				//Monitor
                    cout << "debug: Merge arity reduced to " 
                                      << merge_arity << "\n";
                }
            }
        }
        
        LOG_INFO("AMI_partition_and_merge(): merge arity = " <<
                                          merge_arity << ".\n");
        

        if (merge_arity < 2) {

	   cout << "Insufficient memory: merge_arity is " << merge_arity << "\n";
        LOG_FATAL("Insufficient memory for AMI_partition_and_merge()");
        LOG_FLUSH_LOG;

        return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
        }

 
//#define MINIMIZE_INITIAL_SUBSTREAM_LENGTH
#ifdef MINIMIZE_INITIAL_SUBSTREAM_LENGTH
        
        // Make the substreams as small as possible without increasing
        // the height of the merge tree.

        {
            // The tree height is the ceiling of the log base merge_arity
            // of the number of original substreams.
            
            double tree_height = log((double)original_substreams) /
                log((double)merge_arity);

            tp_assert(tree_height > 0,
                      "Negative or zero tree height!");
           
            tree_height = ceil(tree_height);

            // See how many substreams we could possibly fit in the
            // tree without increasing the height.

            double max_original_substreams = pow((double)merge_arity,
                                                 tree_height);

            tp_assert(max_original_substreams >= original_substreams,
                      "Number of permitted substreams was reduced.");

            // How big will such substreams be?

            double new_sz_original_substream = ceil((double)len /
                                                    max_original_substreams);

            tp_assert(new_sz_original_substream <= sz_original_substream,
                      "Size of original streams increased.");

            sz_original_substream = (size_t)new_sz_original_substream;

            LOG_INFO("Memory constraints set original substreams = " <<
                     original_substreams << '\n');
            
            original_substreams = (len + sz_original_substream - 1) /
                sz_original_substream;

            LOG_INFO("Tree height constraints set original substreams = " <<
                     original_substreams << '\n');
        }
                
#endif // MINIMIZE_INITIAL_SUBSTREAM_LENGTH



        // Create a temporary stream, then iterate through the
        // substreams, processing each one and writing it to the
        // corresponding substream of the temporary stream.

//RAKESH        
//TRASH
       cout << "RAKESH original substreams " << original_substreams << "\n";
       cout << "Merge arity " << merge_arity << ".\n";


       unsigned int run_lengths[2][merge_arity]
                   [(original_substreams+merge_arity-1)/merge_arity];

       int Sub_Start[merge_arity];
    
       for (int i = 0; i < 2; i++)
           for (int j = 0; j < merge_arity; j++)
              for (int k1 = 0; 
                   k1 <  (original_substreams+merge_arity-1)/merge_arity;
                   k1++)
                      run_lengths[i][j][k1] = 0;                      




       initial_tmp_stream = new (AMI_STREAM<T> *)[merge_arity];
       mm_stream = new T[sz_original_substream];

       
        tp_assert(mm_stream != NULL, "Misjudged available main memory.");

        if (mm_stream == NULL) {
		cout << "Misjudged available main memory.\n";
            return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
        }

        instream->seek(0);


        tp_assert(original_substreams * sz_original_substream - len <
                  sz_original_substream,
                  "Total substream length too long or too many.");

        tp_assert(len - (original_substreams - 1) * sz_original_substream <=
                  sz_original_substream,
                  "Total substream length too short or too few.");        

//RAKESH
     size_t check_size = 0;    
     int current_stream = merge_arity-1;


     int runs_in_current_stream = 0;
     int * desired_runs_in_stream = new int[merge_arity];
     char new_stream_name[BTE_PATH_NAME_LEN];

     //For the first stream:
     
    for (ii_streams = 0; ii_streams < merge_arity; ii_streams ++)
    {

 //Figure out how many runs go in each one of merge_arity streams?
 // If there are 12 runs to be distributed among 5 streams, the first 
 //three get 2 and the last two  get 3 runs 


    if (ii_streams < (merge_arity - (original_substreams % merge_arity)) )   
      desired_runs_in_stream[ii_streams] = original_substreams/merge_arity;
     
    else 
      desired_runs_in_stream[ii_streams] = 
                               (original_substreams+ merge_arity -1)/
                                merge_arity;
    }                  



#ifndef BTE_IMP_USER_DEFINED

//    new_name_from_prefix(prefix_name[0],current_stream, new_stream_name);

    //The assumption here is that working_disk is the name of the specific 
    //directory in which the temporary/intermediate streams will be made.
    //By default, I think we shd 

      stream_name_generator(working_disk, 
                           prefix_name[0], 
                           current_stream, 
                           new_stream_name);
#endif 

#ifdef BTE_IMP_USER_DEFINED
        stream_name_generator("",
                             prefix_name[0], 
                             current_stream, 
                             new_stream_name);
        
#endif


        initial_tmp_stream[current_stream] = new AMI_STREAM<T>(
                                                new_stream_name);


        initial_tmp_stream[current_stream]->persist(PERSIST_PERSISTENT); 

	   //Monitor        
        initial_tmp_stream[current_stream]->name(&Cache_For_Names);
        cout << "constructed stream " << Cache_For_Names << "\n";
        if (gettimeofday(&tp1,NULL) == -1) {perror("gettimeofday");}


        ii = 0;
        while (ii < original_substreams) { 
            off_t mm_len;

            // Make sure that the current_stream is supposed to get a run
         
            if (desired_runs_in_stream[current_stream] > runs_in_current_stream )
	    { 
               if (ii == original_substreams - 1) {
                mm_len = len % sz_original_substream;

                // If it is an exact multiple, then the mod will come
                // out 0, which is wrong.

                if (!mm_len) {
                    mm_len = sz_original_substream;
                }
            } else {
                mm_len = sz_original_substream;
            }
            

#if DEBUG_ASSERTIONS
            off_t mm_len_bak = mm_len;
#endif

            // Read a memory load out of the input stream one item at a time,
		  // fill up the key array at the same time.

           {
           T * next_item;
           for (int i = 0; i <  mm_len; i++)
           {
            if ((ae =  instream->read_item(&next_item)) != AMI_ERROR_NO_ERROR)
                                                  return ae;
            mm_stream[i] = *next_item;
           }

		 //Sort the array.

           quicker_sort_op((T *)mm_stream,mm_len);


            for (int i = 0; i <  mm_len; i++)
           {
            if ((ae = initial_tmp_stream[current_stream]->write_item(
                                               mm_stream[i]))
                                                     != AMI_ERROR_NO_ERROR)
                                                                 return ae;
			
           }

           run_lengths[0][current_stream][runs_in_current_stream] = mm_len;

           }


            runs_in_current_stream++;
            ii++;

	 }

//RAKESH        
    if (runs_in_current_stream == desired_runs_in_stream[current_stream])
         {
              
               check_size += 
			initial_tmp_stream[current_stream]->stream_len(); 

                // We do not want old streams hanging around
                // occuping memory. We know how to get the streams
                // since we can generate their names

			//Monitor
             cout << "Packed "<< runs_in_current_stream << 
              "runs into stream " << new_stream_name << 
              "of length " 
            << initial_tmp_stream[current_stream]->stream_len() << "\n";

              delete initial_tmp_stream[current_stream];

              if (check_size < instream->stream_len())  {
                            
                  current_stream = (current_stream + merge_arity - 1) 
                                    % merge_arity;

#ifndef BTE_IMP_USER_DEFINED
 //    new_name_from_prefix(prefix_name[0],current_stream, new_stream_name);

               stream_name_generator(working_disk, 
                                     prefix_name[0], 
                                     current_stream, 
                                     new_stream_name);
  
#endif

#ifdef BTE_IMP_USER_DEFINED
                stream_name_generator("",
                                      prefix_name[0], 
                                      current_stream, 
                                      new_stream_name);
#endif 



                initial_tmp_stream[current_stream] =
                           new AMI_STREAM<T>(new_stream_name);



                
                initial_tmp_stream[current_stream]->persist(PERSIST_PERSISTENT);



                // Number of runs packed into 
                // the stream just constructed now

                runs_in_current_stream = 0;
                }
	    }


        }


        delete initial_tmp_stream[current_stream]; 
        if (mm_stream) delete  mm_stream;
        

	   //Monitor
        if (gettimeofday(&tp2,NULL) == -1) {perror("gettimeofday");}
        tp_dub1 = tp2.tv_sec-tp1.tv_sec;
        tp_dub2 = tp2.tv_usec-tp1.tv_usec;
        cout << "RF Pass:\nNumber of runs=" <<  
         (len + sz_original_substream- 1) /sz_original_substream << "\nTime=" <<
        (tp_dub1*1000000 + tp_dub2)/1000 << " millisecs\n";



        // Make sure the total length of the temporary stream is the
        // same as the total length of the original input stream.


        tp_assert(instream->stream_len() == check_size,
                  "Stream lengths do not match:" <<
                  "\n\tinstream->stream_len() = " << instream->stream_len() <<
                  "\n\tinitial_tmp_stream->stream_len() = " <<
                  check_size << ".\n");

        // Set up the loop invariants for the first iteration of hte
        // main loop.


        instream->persist(PERSIST_PERSISTENT);
	   //ATTENTION: You were deleting this, but you shdnt be.
        //delete instream;



        current_input = initial_tmp_stream;
 


        // Pointers to the substreams that will be merged.
//RAKESH        
       AMI_STREAM<T> **the_substreams = new (AMI_STREAM<T> *)[merge_arity];

        k = 0;
        
        // The main loop.  At the outermost level we are looping over
        // levels of the merge tree.  Typically this will be very
        // small, e.g. 1-3.

        
        T dummykey;   // This is for the last arg to 
	                 // MIAMI_single_merge_Key()
	                 // which necessitated due to type unificatuon problems

        // The number of substreams to be processed at any merge level.
        arity_t substream_count;


        for (substream_count  = original_substreams;
             substream_count > 1;
             substream_count = (substream_count + merge_arity - 1)
                               /merge_arity
            )
		{

		//Monitor
          if (gettimeofday(&tp1,NULL) == -1) {perror("gettimeofday");}                

            
            // Set up to process a given level.
//RAKESH
            tp_assert(len == check_size,
                      "Current level stream not same length as input." <<
                      "\n\tlen = " << len <<
                      "\n\tcurrent_input->stream_len() = " <<
                      check_size << ".\n");

            check_size = 0;




            

            
            // Do we have enough main memory to merge all the
            // substreams on the current level into the output stream?
            // If so, then we will do so, if not then we need an
            // additional level of iteration to process the substreams
            // in groups.


            if (substream_count <= merge_arity) {

//RAKESH   Open up the substream_count streams in which the
//         the runs input to the current merge level are packed
//         The names of these streams (storing the input runs)
//         can be constructed from  prefix_name[k % 2]

    
            
          for(ii= merge_arity - substream_count; ii < merge_arity; ii++) 

                {

		
#ifndef BTE_IMP_USER_DEFINED

                stream_name_generator(working_disk, 
                                      prefix_name[k % 2], 
                                      (int) ii, 
                                      new_stream_name);

#endif

#ifdef  BTE_IMP_USER_DEFINED
                stream_name_generator("",
                                      prefix_name[k % 2], 
                                      (int) ii, 
                                      new_stream_name);
#endif




                current_input[ii] = new AMI_STREAM<T>(new_stream_name);
                current_input[ii]->persist(PERSIST_DELETE);
 
                }



                // Merge them into the output stream.

		//Monitor
               cout << "RAKESH: Reached just before (last pass) AMI_single_merge\n";


                ae = MIAMI_single_merge_Key(
                                  (current_input+merge_arity-substream_count),
                                   substream_count,
				               outstream,
                                   0,
                                   dummykey
                                   );

			 //Monitor
                cout << "Length of output stream after MERGE is " << outstream->stream_len() << "\n";


                if (ae != AMI_ERROR_NO_ERROR) {
			   //Monitor
                   cout << "Sorting error : AMI_ERROR " << ae << "\n";
                   LOG_FATAL("AMI_ERROR ");
                   LOG_FATAL(ae);
                   LOG_FATAL(" returned by  MIAMI_single_merge_Key()");
                   LOG_FLUSH_LOG; 
                   return ae;
                }

			 //Monitor
                cout << "RAKESH: Reached just after (last pass) AMI_single_merge\n";

                
                  
                // Delete the streams input to the above merge.

			 //Monitor
                cout << "Just before deleting streams after last pass\n";


                for (ii = merge_arity - substream_count; 
                     ii < merge_arity; 
                     ii++) {

                   delete current_input[ii];


                }

                delete[] current_input;
                delete[] the_substreams;

			 //Monitor
                if (gettimeofday(&tp2,NULL) == -1) {perror("gettimeofday");}
                tp_dub1 = tp2.tv_sec-tp1.tv_sec;  
                tp_dub2 = tp2.tv_usec-tp1.tv_usec;
                cout << "Final Pass:\n Arity=" << substream_count << 
                "\nTime=" << (tp_dub1*1000000 + tp_dub2)/1000 << " millisecs\n";
                
            } else {

		    //Monitor
               cout << "RAKESH: More than one pass\n";
               LOG_INFO("Merging substreams to an intermediate stream.\n");

                // Create the array of merge_arity stream pointers that
		      // will each point to a stream containing runs output
		      // at the current level k. 

                intermediate_tmp_stream = new (AMI_STREAM<T> *)
                                          [merge_arity];       



//RAKESH   Open up the merge_arity streams in which the
//         the runs input to the current merge level are packed
//         The names of these streams (storing the input runs)
//         can be constructed from  prefix_name[k % 2]

		for (ii=0; ii < merge_arity; ii++)
                	{

//                	new_name_from_prefix(prefix_name[k % 2],(int) ii,
//                                            new_stream_name);

#ifndef BTE_IMP_USER_DEFINED
                    stream_name_generator(working_disk, 
                                          prefix_name[k % 2], 
                                          (int) ii, 
                                          new_stream_name);
#endif

#ifdef BTE_IMP_USER_DEFINED
		          stream_name_generator("",
                                          prefix_name[k % 2], 
                                          (int) ii, 
                                          new_stream_name);
#endif



                     current_input[ii] = new AMI_STREAM<T>(new_stream_name);

                     current_input[ii]->persist(PERSIST_DELETE);

                	}



                // Fool the OS into unmapping the current block of the
                // input stream so that blocks of the substreams can
                // be mapped in without overlapping it.  This is
                // needed for correct execution on HU-UX.
//RAKESH
//                current_input->seek(0);



           current_stream = merge_arity-1;

        	//For the first stream that we use to pack some    
		//of the output runs of the current merge level k.


//                new_name_from_prefix(prefix_name[(k+1) % 2],0,
//                                            new_stream_name);
                  
#ifndef BTE_IMP_USER_DEFINED

                stream_name_generator(working_disk, 
                                      prefix_name[(k+1) % 2], 
                                      current_stream , 
                                      new_stream_name);
#endif

#ifdef BTE_IMP_USER_DEFINED
                stream_name_generator("", 
                                      prefix_name[(k+1) % 2], 
                                      current_stream , 
                                      new_stream_name);
#endif 




         intermediate_tmp_stream[current_stream] = new
                                    AMI_STREAM<T>(new_stream_name);


         intermediate_tmp_stream[current_stream]->persist(PERSIST_PERSISTENT);

                 int remaining_number_of_output_runs = 
			  (substream_count +  merge_arity - 1)/merge_arity;


                for (ii_streams = 0; ii_streams < merge_arity; ii_streams ++)
                {
	            // If there are 12 runs to be distributed among 5 streams, 
                 // the first three get 2 and the last two  get 3 runs   

    		       if (ii_streams < 
                    (merge_arity - 
                    (remaining_number_of_output_runs % merge_arity)) )   
                   
		            desired_runs_in_stream[ii_streams] = 
                                       remaining_number_of_output_runs/
                                       merge_arity;
     
   	            else 
                      desired_runs_in_stream[ii_streams] = 
                                      (remaining_number_of_output_runs 
                                                    + merge_arity -1)/
                                                           merge_arity;
 
                 Sub_Start[ii_streams] = 0;

               }           


               runs_in_current_stream = 0;
               unsigned int merge_number = 0;

			//Monitor
               

                // Loop through the substreams of the current stream,
                // merging as many as we can at a time until all are
                // done with.


                for (sub_start = 0, ii = 0, jj = 0;
                     ii < substream_count;
                     ii++, jj++) {

                     if (run_lengths[k % 2][merge_arity-1-jj][merge_number]!=0)
				  {                    

                       sub_start =  Sub_Start[merge_arity-1-jj];

                       sub_end = sub_start + 
                       run_lengths[k % 2][merge_arity-1-jj][merge_number] - 1;


                       Sub_Start[merge_arity-1-jj]+=
                       run_lengths[k % 2][merge_arity-1-jj][merge_number];

                       run_lengths[k % 2][merge_arity-1-jj][merge_number]
                                  = 0;
                      }
                    else
                      {
				    //This weirdness is caused by the way bte substream
				    //constructor was designed.

                       sub_end = Sub_Start[merge_arity-1-jj]-1;
                       sub_start = sub_end + 1;

                       ii--;

                      }

 
				 //Open the new substream
                  current_input[merge_arity-1-jj]->new_substream(AMI_READ_STREAM,
                                                sub_start,
                                                sub_end,
                                                (AMI_base_stream<T> **)
                                                (the_substreams + jj));
                               
                    // The substreams are read-once.
                    // If we've got all we can handle or we've seen
                    // them all, then merge them.
                    

                    if ((jj >= merge_arity - 1) ||
                        (ii == substream_count - 1)) {
                        
                        tp_assert(jj <= merge_arity - 1,
                                  "Index got too large.");


				    //Check if the stream into which runs are cuurently 
				    //being packed has got its share of runs. If yes,
				    //delete that stream and construct a new stream 
				    //appropriately.
 
                    if (desired_runs_in_stream[current_stream] == runs_in_current_stream)
                    {
                     
				  //Make sure that the deleted stream persists on disk.
                      intermediate_tmp_stream[current_stream]->persist(
                                                       PERSIST_PERSISTENT);

                      delete intermediate_tmp_stream[current_stream];

                      current_stream = (current_stream + merge_arity -1) 
                                       % merge_arity;


                      // Unless the current level is over, we've to generate 
				  //a new stream for the next set of runs.

                       if (remaining_number_of_output_runs > 0) {

//                        new_name_from_prefix(prefix_name[(k+1) % 2],
//                              current_stream, new_stream_name);


#ifndef BTE_IMP_USER_DEFINED
                          stream_name_generator(working_disk, 
                                                prefix_name[(k+1) % 2], 
                                                (int) current_stream, 
                                                new_stream_name);
#endif

#ifdef BTE_IMP_USER_DEFINED
                          stream_name_generator("",
                                                prefix_name[(k+1) % 2], 
                                                (int) current_stream, 
                                                new_stream_name);
#endif


                        intermediate_tmp_stream[current_stream] = new
                                    AMI_STREAM<T>(new_stream_name);



                        intermediate_tmp_stream[current_stream]->persist(
                                                     PERSIST_PERSISTENT);
    			         runs_in_current_stream = 0;
                       }
				}

				//Monitor
                      cout << "RAKESH: Calling merge\n";

                      ae = MIAMI_single_merge_Key(the_substreams,
                                       jj+1,
                                       intermediate_tmp_stream[current_stream],
                                       0,
                                       dummykey
                                       );
                        
                      if (ae != AMI_ERROR_NO_ERROR) {
                            return ae;
                                                    }
                     
                      for (ii_streams = 0; ii_streams < jj+1; ii_streams++)
                      run_lengths[(k+1)%2][current_stream]
                                  [runs_in_current_stream]+= 
                                  the_substreams[ii_streams]->stream_len();
            

				  //Monitor                        
                      cout << "After call from AMI_single_merge\n";
                      merge_number++;

				  //Monitor
                      cout << "Length of stream containing output of merge of MERGE is " << intermediate_tmp_stream[current_stream]->stream_len() << "\n";

         
                       
                    //Decrement the counter corresp to number of runs 
			    // still to be formed at current level


  			      remaining_number_of_output_runs--;
                        
                        // Delete input substreams. jj is currently the index
                        // of the largest, so we want to bump it up before the
                        // idiomatic loop.

				 //Monitor
                  cout << "RAKESH:Deleting substreams\n";

                  for (jj++; jj--; ) {
                          delete the_substreams[merge_arity-1-jj];
                          }

			   //Monitor
                  cout << "RAKESH: Deleted them\n";
                        // Now jj should be -1 so that it gets bumped
                        // back up to 0 before the next iteration of
                        // the outer loop.
                        tp_assert((jj == -1), "Index not reduced to -1.");
                        
//RAKESH		The number of runs in the current_stream
//			goes up by 1.

                   runs_in_current_stream ++;
                        
                   }               

                }

                delete intermediate_tmp_stream[current_stream];


                

                // Get rid of the current input streams and use the ones
			 //output at the current level.
//RAKESH

                for (ii = 0; ii < merge_arity; ii++) delete current_input[ii];
                delete[] current_input;   

                current_input = (AMI_STREAM<T> **) intermediate_tmp_stream;
            	    
            }

            if (substream_count > merge_arity) {

               if (gettimeofday(&tp2,NULL) == -1) {perror("gettimeofday");}


               tp_dub1 = tp2.tv_sec-tp1.tv_sec;  
               tp_dub2 = tp2.tv_usec-tp1.tv_usec;

               cout << "Intmdt Pass:\n Arity=" << merge_arity <<
               "\nTime=" << (tp_dub1*1000000 + tp_dub2)/1000 << " millisecs\n";
    

              } 
            
            k++;

        }

	   //Monitor
        cout << "Number of passes including runformation pass is " << k+1 << "\n";
//RAKESH
//        delete [] the_substreams;
        
        return AMI_ERROR_NO_ERROR;
   
	   }
    }
 





// Recursive division of a stream and then merging back together.
template<class T, class KEY>
AMI_err AMI_partition_and_merge_Key(AMI_STREAM<T> *instream,
                                AMI_STREAM<T> *outstream, 
                                int keyoffset, KEY dummykey)
{ 
    AMI_err ae;
    off_t len;
    size_t sz_avail, sz_stream;
    size_t sz_substream;

    //Monitoring 
    struct timeval tp1, tp2;
    double tp_dub1,tp_dub2,tp_dub3;



    unsigned int ii, jj, kk;
    int ii_streams;

    //Monitoring
    char *Cache_For_Names;
    char *working_disk;
    

    // Figure out how much memory we've got to work with.

    if (MM_manager.available(&sz_avail) != MM_ERROR_NO_ERROR) {
        return AMI_ERROR_MM_ERROR;
    }

    //Monitoring
    cout << "RAKESH: Avail memory is " << sz_avail << "\n";

    //Conservatively assume that the memory for buffers for 
    //the two streams is unallocated; so we need to subtract.

    if ((ae = instream->main_memory_usage(&sz_stream,
                                              MM_STREAM_USAGE_MAXIMUM)) !=
                                              AMI_ERROR_NO_ERROR) {
            return ae;
        }                                     

    if ((ae = instream->main_memory_usage(&sz_substream,
                                              MM_STREAM_USAGE_OVERHEAD)) !=
                                              AMI_ERROR_NO_ERROR) {

              return ae;
        }   

    sz_avail -= 2*sz_stream;

    cout << "Avail memory after accounting for input streams' memory" << sz_avail << endl;


    working_disk = getenv("TMP");

    // If the whole input can fit in main memory then just call
    // AMI_main_mem_merge() to deal with it by loading it once and
    // processing it.

    len = instream->stream_len();
    instream->seek(0);
    
    cout << "Instream length is " << len << "\n";

    
    if ((len * sizeof(T)) <= sz_avail) {

           if (len * (sizeof(T)*sizeof(qsort_item<KEY>)) > sz_avail)
           // ie if you have dont have space for separate
           // keysorting (good cache performance) followed by permuting 
          {
           
           T * next_item;
           T * mm_stream = new T[len];

           for (int i = 0; i <  len; i++)
           {
            if ((ae =  instream->read_item(&next_item)) != AMI_ERROR_NO_ERROR)
                                                  return ae;
            mm_stream[i] = *next_item;
           }

           quicker_sort_op((T *)mm_stream,len);

            for (int i = 0; i <  len; i++)
           {
            if ((ae = outstream->write_item( mm_stream[i])) 
                                     != AMI_ERROR_NO_ERROR)
                                                 return ae;
           }
         }
         else
         {
         //Use qsort on keys followed by permuting
         T * mm_stream = new T[len];
         qsort_item<KEY> *qs_array = new (qsort_item<KEY>)[len];
         T * next_item;

         for (int i = 0; i <  len; i++)
          {
           if ((ae =  instream->read_item(&next_item)) != AMI_ERROR_NO_ERROR)
                                                  return ae;
            mm_stream[i] = *next_item;
            qs_array[i].keyval = * (KEY *)((char *)next_item+keyoffset);
            qs_array[i].source = i;
           }

          quicker_sort_op((qsort_item<KEY> *)qs_array,len);

          for (int i = 0; i <  len; i++)
          {
            if ((ae = outstream->write_item(mm_stream[qs_array[i].source]))
                                           != AMI_ERROR_NO_ERROR)
                                           return ae;
            }
          }
          
          return AMI_ERROR_NO_ERROR;

    } else {

        // The number of substreams that the original input stream
        // will be split into.
        
        arity_t original_substreams;

        // The length, in terms of stream objects of type T, of the
        // original substreams of the input stream.  The last one may
        // be shorter than this.
        
        size_t sz_original_substream;

        // The initial temporary stream, to which substreams of the
        // original input stream are written.

        //RAKESH
        AMI_STREAM<T> **initial_tmp_stream;
        
        // The number of substreams that can be merged together at once.

        arity_t merge_arity;

        // A pointer to the buffer in main memory to read a memory load into.
        T *mm_stream;

        
        // Loop variables:

        // The stream being read at the current level.
        
        //RAKESH
        AMI_STREAM<T> **current_input;

        // The output stream for the current level if it is not outstream.

        //RAKESH
        AMI_STREAM<T> **intermediate_tmp_stream;
        
       //RAKESH  FIX THIS: Need to generate random strings using
	   //tmpname() or something like that.
	   char * prefix_name[] = {"Tempo_stream0", "Tempo_stream1"};
        char itoa_str[5];


        // The size of substreams of *current_input that are being
        // merged.  The last one may be smaller.  This value should be
        // sz_original_substream * (merge_arity ** k) where k is the
        // number of iterations the loop has gone through.
        

        //Merge Level
        unsigned int k;

        off_t sub_start, sub_end;

        // How many substreams will there be?  The main memory
        // available to us is the total amount available, minus what
        // is needed for the input stream and the temporary stream.




//RAKESH
// In our case merge_arity is determined differently than in the original
// implementation of AMI_partition_and_merge since we use several streams
// in each level.
// In our case net main memory required to carry out an R-way merge is
// (R+1)*MM_STREAM_USAGE_MAXIMUM  {R substreams for input runs, 1 stream for output}
// + R*MM_STREAM_USAGE_OVERHEAD   {One stream for each active input run: but while
//				   the substreams use buffers, streams don't}
// + (R+1)*m_obj->space_usage_per_stream();
//
// The net memory usage for an R-way merge is thus
// R*(sz_stream + sz_substeam + m_obj->space_usage_per_stream()) + sz_stream +
// m_obj->space_usage_per_stream();
//
        
     
        

	   //To support a binary merge, need space for max_stream_usage
	   //for at least three stream objects.

         if (sz_avail <= 3*(sz_stream + sz_substream 
                            + sizeof(merge_heap_element<KEY>))
                           //+ sz_stream + sizeof(merge_heap_element<KEY>)
                            ) 
         {
   
		 cout << "Insuff memory\n";
            return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
         }
 
        
       sz_original_substream = (sz_avail)/(sizeof(T)+sizeof(qsort_item<KEY>));

        // Round the original substream length off to an integral
        // number of chunks.  This is for systems like HP-UX that
        // cannot map in overlapping regions.  It is also required for
        // BTE's that are capable of freeing chunks as they are
        // read.

        {
         size_t sz_chunk_size = instream->chunk_size();
            
         sz_original_substream = sz_chunk_size *
                ((sz_original_substream + sz_chunk_size - 1) /
                 sz_chunk_size);
        }

        original_substreams = (len + sz_original_substream - 1) /
            sz_original_substream;
        
        // Account for the space that a merge object will use.

        {
		//Availabe memory for input stream objects is given by 
		//sz_avail minus the space occupied by output stream objects.
            size_t sz_avail_during_merge = sz_avail - 
                                           sz_stream - sz_substream;


		  //This conts the per-input stream memory cost.
            size_t sz_stream_during_merge =sz_stream + sz_substream +
                 sizeof(merge_heap_element<KEY>);
           

            //Compute merge arity
            merge_arity = sz_avail_during_merge/sz_stream_during_merge;


//TRASH
            cout << "Original mem size available " << sz_avail << "\n";
            cout << "Max stream overhead " << sz_stream << "\n";
            cout <<"Merge Heap Element Size "<< sizeof(merge_heap_element<KEY>)
                                             << "\n";
            cout << "Availablity after accounting for output stream" 
                 << sz_avail_during_merge << "\n";
	       cout << "Overhead Per input run " 
                 <<  sz_stream_during_merge << "\n";
            cout << "Thus merge arity is " << merge_arity << "\n";	
        }

        // Make sure that the AMI is willing to provide us with the
        // number of substreams we want.  It may not be able to due to
        // operating system restrictions, such as on the number of
        // regions that can be mmap()ed in.

        {
            int ami_available_streams = instream->available_streams();

            cout << "Available_streams " << ami_available_streams << "\n";

            if (ami_available_streams != -1) {
                    if (ami_available_streams <= 5) {
                    return AMI_ERROR_INSUFFICIENT_AVAILABLE_STREAMS;
                }
                
                if (merge_arity > (arity_t)ami_available_streams - 2) {
                    merge_arity = ami_available_streams - 2;
                    LOG_INFO("Reduced merge arity due to AMI restrictions.\n");
				//Monitor
                    cout << "debug: Merge arity reduced to " 
                                      << merge_arity << "\n";
                }
            }
        }
        
        LOG_INFO("AMI_partition_and_merge_Key(): merge arity = " <<
                                          merge_arity << ".\n");
        

        if (merge_arity < 2) {

	   cout << "Insufficient memory: merge_arity is " << merge_arity << "\n";
        LOG_FATAL("Insufficient memory for AMI_partition_and_merge_Key()");
        LOG_FLUSH_LOG;

        return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
        }

 
//#define MINIMIZE_INITIAL_SUBSTREAM_LENGTH
#ifdef MINIMIZE_INITIAL_SUBSTREAM_LENGTH
        
        // Make the substreams as small as possible without increasing
        // the height of the merge tree.

        {
            // The tree height is the ceiling of the log base merge_arity
            // of the number of original substreams.
            
            double tree_height = log((double)original_substreams) /
                log((double)merge_arity);

            tp_assert(tree_height > 0,
                      "Negative or zero tree height!");
           
            tree_height = ceil(tree_height);

            // See how many substreams we could possibly fit in the
            // tree without increasing the height.

            double max_original_substreams = pow((double)merge_arity,
                                                 tree_height);

            tp_assert(max_original_substreams >= original_substreams,
                      "Number of permitted substreams was reduced.");

            // How big will such substreams be?

            double new_sz_original_substream = ceil((double)len /
                                                    max_original_substreams);

            tp_assert(new_sz_original_substream <= sz_original_substream,
                      "Size of original streams increased.");

            sz_original_substream = (size_t)new_sz_original_substream;

            LOG_INFO("Memory constraints set original substreams = " <<
                     original_substreams << '\n');
            
            original_substreams = (len + sz_original_substream - 1) /
                sz_original_substream;

            LOG_INFO("Tree height constraints set original substreams = " <<
                     original_substreams << '\n');
        }
                
#endif // MINIMIZE_INITIAL_SUBSTREAM_LENGTH



        // Create a temporary stream, then iterate through the
        // substreams, processing each one and writing it to the
        // corresponding substream of the temporary stream.

//RAKESH        
//TRASH
       cout << "RAKESH original substreams " << original_substreams << "\n";
       cout << "Merge arity " << merge_arity << ".\n";


       unsigned int run_lengths[2][merge_arity]
                   [(original_substreams+merge_arity-1)/merge_arity];

       int Sub_Start[merge_arity];
    
       for (int i = 0; i < 2; i++)
           for (int j = 0; j < merge_arity; j++)
              for (int k1 = 0; 
                   k1 <  (original_substreams+merge_arity-1)/merge_arity;
                   k1++)
                      run_lengths[i][j][k1] = 0;                      




       initial_tmp_stream = new (AMI_STREAM<T> *)[merge_arity];
       mm_stream = new T[sz_original_substream];

       qsort_item<KEY> * qs_array = new (qsort_item<KEY>)[sz_original_substream];



        tp_assert(mm_stream != NULL, "Misjudged available main memory.");

        if (mm_stream == NULL) {
		cout << "Misjudged available main memory.\n";
            return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
        }

        instream->seek(0);


        tp_assert(original_substreams * sz_original_substream - len <
                  sz_original_substream,
                  "Total substream length too long or too many.");

        tp_assert(len - (original_substreams - 1) * sz_original_substream <=
                  sz_original_substream,
                  "Total substream length too short or too few.");        

//RAKESH
     size_t check_size = 0;    
     int current_stream = merge_arity-1;


     int runs_in_current_stream = 0;
     int * desired_runs_in_stream = new int[merge_arity];
     char new_stream_name[BTE_PATH_NAME_LEN];

     //For the first stream:
     
    for (ii_streams = 0; ii_streams < merge_arity; ii_streams ++)
    {

 //Figure out how many runs go in each one of merge_arity streams?
 // If there are 12 runs to be distributed among 5 streams, the first 
 //three get 2 and the last two  get 3 runs 


    if (ii_streams < (merge_arity - (original_substreams % merge_arity)) )   
      desired_runs_in_stream[ii_streams] = original_substreams/merge_arity;
     
    else 
      desired_runs_in_stream[ii_streams] = 
                               (original_substreams+ merge_arity -1)/
                                merge_arity;
    }                  



#ifndef BTE_IMP_USER_DEFINED

//    new_name_from_prefix(prefix_name[0],current_stream, new_stream_name);

    //The assumption here is that working_disk is the name of the specific 
    //directory in which the temporary/intermediate streams will be made.
    //By default, I think we shd 

      stream_name_generator(working_disk, 
                           prefix_name[0], 
                           current_stream, 
                           new_stream_name);
#endif 

#ifdef BTE_IMP_USER_DEFINED
        stream_name_generator("",
                             prefix_name[0], 
                             current_stream, 
                             new_stream_name);
        
#endif


        initial_tmp_stream[current_stream] = new AMI_STREAM<T>(
                                                new_stream_name);


        initial_tmp_stream[current_stream]->persist(PERSIST_PERSISTENT); 

	   //Monitor        
        initial_tmp_stream[current_stream]->name(&Cache_For_Names);
        cout << "constructed stream " << Cache_For_Names << "\n";
        if (gettimeofday(&tp1,NULL) == -1) {perror("gettimeofday");}


        ii = 0;
        while (ii < original_substreams) { 
            off_t mm_len;

            // Make sure that the current_stream is supposed to get a run
         
            if (desired_runs_in_stream[current_stream] > runs_in_current_stream )
	    { 
               if (ii == original_substreams - 1) {
                mm_len = len % sz_original_substream;

                // If it is an exact multiple, then the mod will come
                // out 0, which is wrong.

                if (!mm_len) {
                    mm_len = sz_original_substream;
                }
            } else {
                mm_len = sz_original_substream;
            }
            

#if DEBUG_ASSERTIONS
            off_t mm_len_bak = mm_len;
#endif

            // Read a memory load out of the input stream one item at a time,
		  // fill up the key array at the same time.

           {
           T * next_item;
           for (int i = 0; i <  mm_len; i++)
           {
            if ((ae =  instream->read_item(&next_item)) != AMI_ERROR_NO_ERROR)
                                                  return ae;
            mm_stream[i] = *next_item;
            qs_array[i].keyval = * (KEY *)((char *)next_item+keyoffset);
            qs_array[i].source = i;
           }

		 //Sort the key array.

           quicker_sort_op((qsort_item<KEY> *)qs_array,mm_len);

		 //Now permute the memoryload as per the sorted key array.

            for (int i = 0; i <  mm_len; i++)
           {
            if ((ae = initial_tmp_stream[current_stream]->write_item(
                                               mm_stream[qs_array[i].source]))
                                                     != AMI_ERROR_NO_ERROR)
                                                                 return ae;
			
           }

           run_lengths[0][current_stream][runs_in_current_stream] = mm_len;

           }


            runs_in_current_stream++;
            ii++;

	 }

//RAKESH        
    if (runs_in_current_stream == desired_runs_in_stream[current_stream])
         {
              
               check_size += 
			initial_tmp_stream[current_stream]->stream_len(); 

                // We do not want old streams hanging around
                // occuping memory. We know how to get the streams
                // since we can generate their names

			//Monitor
             cout << "Packed "<< runs_in_current_stream << 
              "runs into stream " << new_stream_name << 
              "of length " 
            << initial_tmp_stream[current_stream]->stream_len() << "\n";

              delete initial_tmp_stream[current_stream];

              if (check_size < instream->stream_len())  {
                            
                  current_stream = (current_stream + merge_arity - 1) 
                                    % merge_arity;

#ifndef BTE_IMP_USER_DEFINED
 //    new_name_from_prefix(prefix_name[0],current_stream, new_stream_name);

               stream_name_generator(working_disk, 
                                     prefix_name[0], 
                                     current_stream, 
                                     new_stream_name);
  
#endif

#ifdef BTE_IMP_USER_DEFINED
                stream_name_generator("",
                                      prefix_name[0], 
                                      current_stream, 
                                      new_stream_name);
#endif 



                initial_tmp_stream[current_stream] =
                           new AMI_STREAM<T>(new_stream_name);



                
                initial_tmp_stream[current_stream]->persist(PERSIST_PERSISTENT);



                // Number of runs packed into 
                // the stream just constructed now

                runs_in_current_stream = 0;
                }
	    }


        }


        delete initial_tmp_stream[current_stream]; 
        if (mm_stream) delete  mm_stream;
        if (qs_array) delete qs_array;

	   //Monitor
        if (gettimeofday(&tp2,NULL) == -1) {perror("gettimeofday");}
        tp_dub1 = tp2.tv_sec-tp1.tv_sec;
        tp_dub2 = tp2.tv_usec-tp1.tv_usec;
        cout << "RF Pass:\nNumber of runs=" <<  
         (len + sz_original_substream- 1) /sz_original_substream << "\nTime=" <<
        (tp_dub1*1000000 + tp_dub2)/1000 << " millisecs\n";



        // Make sure the total length of the temporary stream is the
        // same as the total length of the original input stream.


        tp_assert(instream->stream_len() == check_size,
                  "Stream lengths do not match:" <<
                  "\n\tinstream->stream_len() = " << instream->stream_len() <<
                  "\n\tinitial_tmp_stream->stream_len() = " <<
                  check_size << ".\n");

        // Set up the loop invariants for the first iteration of hte
        // main loop.


        instream->persist(PERSIST_PERSISTENT);
	   //ATTENTION: You were deleting this, but you shdnt be.
        //delete instream;



        current_input = initial_tmp_stream;
 


        // Pointers to the substreams that will be merged.
//RAKESH        
       AMI_STREAM<T> **the_substreams = new (AMI_STREAM<T> *)[merge_arity];

        k = 0;
        
        // The main loop.  At the outermost level we are looping over
        // levels of the merge tree.  Typically this will be very
        // small, e.g. 1-3.

        
        KEY dummykey; // This is for the last arg to 
	                 // AMI_partition_and_merge_Key()
	                 // which necessitated due to type unificatuon problems

        // The number of substreams to be processed at any merge level.
        arity_t substream_count;


        for (substream_count  = original_substreams;
             substream_count > 1;
             substream_count = (substream_count + merge_arity - 1)
                               /merge_arity
            )
		{

		//Monitor
          if (gettimeofday(&tp1,NULL) == -1) {perror("gettimeofday");}                

            
            // Set up to process a given level.
//RAKESH
            tp_assert(len == check_size,
                      "Current level stream not same length as input." <<
                      "\n\tlen = " << len <<
                      "\n\tcurrent_input->stream_len() = " <<
                      check_size << ".\n");

            check_size = 0;




            

            
            // Do we have enough main memory to merge all the
            // substreams on the current level into the output stream?
            // If so, then we will do so, if not then we need an
            // additional level of iteration to process the substreams
            // in groups.


            if (substream_count <= merge_arity) {

//RAKESH   Open up the substream_count streams in which the
//         the runs input to the current merge level are packed
//         The names of these streams (storing the input runs)
//         can be constructed from  prefix_name[k % 2]

    
            
          for(ii= merge_arity - substream_count; ii < merge_arity; ii++) 

                {

		
#ifndef BTE_IMP_USER_DEFINED

                stream_name_generator(working_disk, 
                                      prefix_name[k % 2], 
                                      (int) ii, 
                                      new_stream_name);

#endif

#ifdef  BTE_IMP_USER_DEFINED
                stream_name_generator("",
                                      prefix_name[k % 2], 
                                      (int) ii, 
                                      new_stream_name);
#endif




                current_input[ii] = new AMI_STREAM<T>(new_stream_name);
                current_input[ii]->persist(PERSIST_DELETE);
 
                }



                // Merge them into the output stream.

		//Monitor
               cout << "RAKESH: Reached just before (last pass) AMI_single_merge\n";


                ae = MIAMI_single_merge_Key(
                                  (current_input+merge_arity-substream_count),
                                   substream_count,
				               outstream,
                                   keyoffset,
                                   dummykey
                                   );

			 //Monitor
                cout << "Length of output stream after MERGE is " << outstream->stream_len() << "\n";


                if (ae != AMI_ERROR_NO_ERROR) {
			   //Monitor
                   cout << "Sorting error : AMI_ERROR " << ae << "\n";
                   LOG_FATAL("AMI_ERROR ");
                   LOG_FATAL(ae);
                   LOG_FATAL(" returned by  MIAMI_single_merge_Key()");
                   LOG_FLUSH_LOG; 
                   return ae;
                }

			 //Monitor
                cout << "RAKESH: Reached just after (last pass) AMI_single_merge\n";

                
                  
                // Delete the streams input to the above merge.

			 //Monitor
                cout << "Just before deleting streams after last pass\n";


                for (ii = merge_arity - substream_count; 
                     ii < merge_arity; 
                     ii++) {

                   delete current_input[ii];


                }

                delete[] current_input;
                delete[] the_substreams;

			 //Monitor
                if (gettimeofday(&tp2,NULL) == -1) {perror("gettimeofday");}
                tp_dub1 = tp2.tv_sec-tp1.tv_sec;  
                tp_dub2 = tp2.tv_usec-tp1.tv_usec;
                cout << "Final Pass:\n Arity=" << substream_count << 
                "\nTime=" << (tp_dub1*1000000 + tp_dub2)/1000 << " millisecs\n";
                
            } else {

		    //Monitor
               cout << "RAKESH: More than one pass\n";
               LOG_INFO("Merging substreams to an intermediate stream.\n");

                // Create the array of merge_arity stream pointers that
		      // will each point to a stream containing runs output
		      // at the current level k. 

                intermediate_tmp_stream = new (AMI_STREAM<T> *)
                                          [merge_arity];       



//RAKESH   Open up the merge_arity streams in which the
//         the runs input to the current merge level are packed
//         The names of these streams (storing the input runs)
//         can be constructed from  prefix_name[k % 2]

		for (ii=0; ii < merge_arity; ii++)
                	{

//                	new_name_from_prefix(prefix_name[k % 2],(int) ii,
//                                            new_stream_name);

#ifndef BTE_IMP_USER_DEFINED
                    stream_name_generator(working_disk, 
                                          prefix_name[k % 2], 
                                          (int) ii, 
                                          new_stream_name);
#endif

#ifdef BTE_IMP_USER_DEFINED
		          stream_name_generator("",
                                          prefix_name[k % 2], 
                                          (int) ii, 
                                          new_stream_name);
#endif



                     current_input[ii] = new AMI_STREAM<T>(new_stream_name);

                     current_input[ii]->persist(PERSIST_DELETE);

                	}



                // Fool the OS into unmapping the current block of the
                // input stream so that blocks of the substreams can
                // be mapped in without overlapping it.  This is
                // needed for correct execution on HU-UX.
//RAKESH
//                current_input->seek(0);



           current_stream = merge_arity-1;

        	//For the first stream that we use to pack some    
		//of the output runs of the current merge level k.


//                new_name_from_prefix(prefix_name[(k+1) % 2],0,
//                                            new_stream_name);
                  
#ifndef BTE_IMP_USER_DEFINED

                stream_name_generator(working_disk, 
                                      prefix_name[(k+1) % 2], 
                                      current_stream , 
                                      new_stream_name);
#endif

#ifdef BTE_IMP_USER_DEFINED
                stream_name_generator("", 
                                      prefix_name[(k+1) % 2], 
                                      current_stream , 
                                      new_stream_name);
#endif 




         intermediate_tmp_stream[current_stream] = new
                                    AMI_STREAM<T>(new_stream_name);


         intermediate_tmp_stream[current_stream]->persist(PERSIST_PERSISTENT);

                 int remaining_number_of_output_runs = 
			  (substream_count +  merge_arity - 1)/merge_arity;


                for (ii_streams = 0; ii_streams < merge_arity; ii_streams ++)
                {
	            // If there are 12 runs to be distributed among 5 streams, 
                 // the first three get 2 and the last two  get 3 runs   

    		       if (ii_streams < 
                    (merge_arity - 
                    (remaining_number_of_output_runs % merge_arity)) )   
                   
		            desired_runs_in_stream[ii_streams] = 
                                       remaining_number_of_output_runs/
                                       merge_arity;
     
   	            else 
                      desired_runs_in_stream[ii_streams] = 
                                      (remaining_number_of_output_runs 
                                                    + merge_arity -1)/
                                                           merge_arity;
 
                 Sub_Start[ii_streams] = 0;

               }           


               runs_in_current_stream = 0;
               unsigned int merge_number = 0;

			//Monitor
               

                // Loop through the substreams of the current stream,
                // merging as many as we can at a time until all are
                // done with.


                for (sub_start = 0, ii = 0, jj = 0;
                     ii < substream_count;
                     ii++, jj++) {

                     if (run_lengths[k % 2][merge_arity-1-jj][merge_number]!=0)
				  {                    

                       sub_start =  Sub_Start[merge_arity-1-jj];

                       sub_end = sub_start + 
                       run_lengths[k % 2][merge_arity-1-jj][merge_number] - 1;


                       Sub_Start[merge_arity-1-jj]+=
                       run_lengths[k % 2][merge_arity-1-jj][merge_number];

                       run_lengths[k % 2][merge_arity-1-jj][merge_number]
                                  = 0;
                      }
                    else
                      {
				    //This weirdness is caused by the way bte substream
				    //constructor was designed.

                       sub_end = Sub_Start[merge_arity-1-jj]-1;
                       sub_start = sub_end + 1;

                       ii--;

                      }

 
				 //Open the new substream
                  current_input[merge_arity-1-jj]->new_substream(AMI_READ_STREAM,
                                                sub_start,
                                                sub_end,
                                                (AMI_base_stream<T> **)
                                                (the_substreams + jj));
                               
                    // The substreams are read-once.
                    // If we've got all we can handle or we've seen
                    // them all, then merge them.
                    

                    if ((jj >= merge_arity - 1) ||
                        (ii == substream_count - 1)) {
                        
                        tp_assert(jj <= merge_arity - 1,
                                  "Index got too large.");


				    //Check if the stream into which runs are cuurently 
				    //being packed has got its share of runs. If yes,
				    //delete that stream and construct a new stream 
				    //appropriately.
 
                    if (desired_runs_in_stream[current_stream] == runs_in_current_stream)
                    {
                     
				  //Make sure that the deleted stream persists on disk.
                      intermediate_tmp_stream[current_stream]->persist(
                                                       PERSIST_PERSISTENT);

                      delete intermediate_tmp_stream[current_stream];

                      current_stream = (current_stream + merge_arity -1) 
                                       % merge_arity;


                      // Unless the current level is over, we've to generate 
				  //a new stream for the next set of runs.

                       if (remaining_number_of_output_runs > 0) {

//                        new_name_from_prefix(prefix_name[(k+1) % 2],
//                              current_stream, new_stream_name);


#ifndef BTE_IMP_USER_DEFINED
                          stream_name_generator(working_disk, 
                                                prefix_name[(k+1) % 2], 
                                                (int) current_stream, 
                                                new_stream_name);
#endif

#ifdef BTE_IMP_USER_DEFINED
                          stream_name_generator("",
                                                prefix_name[(k+1) % 2], 
                                                (int) current_stream, 
                                                new_stream_name);
#endif


                        intermediate_tmp_stream[current_stream] = new
                                    AMI_STREAM<T>(new_stream_name);



                        intermediate_tmp_stream[current_stream]->persist(
                                                     PERSIST_PERSISTENT);
    			         runs_in_current_stream = 0;
                       }
				}

				//Monitor
                      cout << "RAKESH: Calling merge\n";

                      ae = MIAMI_single_merge_Key(the_substreams,
                                       jj+1,
                                       intermediate_tmp_stream[current_stream],
                                       keyoffset,
                                       dummykey
                                       );
                        
                      if (ae != AMI_ERROR_NO_ERROR) {
                            return ae;
                                                    }
                     
                      for (ii_streams = 0; ii_streams < jj+1; ii_streams++)
                      run_lengths[(k+1)%2][current_stream]
                                  [runs_in_current_stream]+= 
                                  the_substreams[ii_streams]->stream_len();
            

				  //Monitor                        
                      cout << "After call from AMI_single_merge\n";
                      merge_number++;

				  //Monitor
                      cout << "Length of stream containing output of merge of MERGE is " << intermediate_tmp_stream[current_stream]->stream_len() << "\n";

         
                       
                    //Decrement the counter corresp to number of runs 
			    // still to be formed at current level


  			      remaining_number_of_output_runs--;
                        
                        // Delete input substreams. jj is currently the index
                        // of the largest, so we want to bump it up before the
                        // idiomatic loop.

				 //Monitor
                  cout << "RAKESH:Deleting substreams\n";

                  for (jj++; jj--; ) {
                          delete the_substreams[merge_arity-1-jj];
                          }

			   //Monitor
                  cout << "RAKESH: Deleted them\n";
                        // Now jj should be -1 so that it gets bumped
                        // back up to 0 before the next iteration of
                        // the outer loop.
                        tp_assert((jj == -1), "Index not reduced to -1.");
                        
//RAKESH		The number of runs in the current_stream
//			goes up by 1.

                   runs_in_current_stream ++;
                        
                   }               

                }

                delete intermediate_tmp_stream[current_stream];


                

                // Get rid of the current input streams and use the ones
			 //output at the current level.
//RAKESH

                for (ii = 0; ii < merge_arity; ii++) delete current_input[ii];
                delete[] current_input;   

                current_input = (AMI_STREAM<T> **) intermediate_tmp_stream;
            	    
            }

            if (substream_count > merge_arity) {

               if (gettimeofday(&tp2,NULL) == -1) {perror("gettimeofday");}


               tp_dub1 = tp2.tv_sec-tp1.tv_sec;  
               tp_dub2 = tp2.tv_usec-tp1.tv_usec;

               cout << "Intmdt Pass:\n Arity=" << merge_arity <<
               "\nTime=" << (tp_dub1*1000000 + tp_dub2)/1000 << " millisecs\n";
    

              } 
            
            k++;

        }

	   //Monitor
        cout << "Number of passes including runformation pass is " << k+1 << "\n";
//RAKESH
//        delete [] the_substreams;
        
        return AMI_ERROR_NO_ERROR;
   
	   }
    }
 



template<class T, class KEY>
AMI_err AMI_replacement_selection_and_merge_Key(AMI_STREAM<T> *instream,
                                AMI_STREAM<T> *outstream, int keyoffset,
                                KEY dummykey)
{
    AMI_err ae;
    off_t len;
    size_t sz_avail, sz_stream;
    size_t sz_substream;

    //Monitor
    struct timeval tp1, tp2;
    double tp_dub1,tp_dub2,tp_dub3;
    

 
    unsigned int ii, jj, kk;
    int ii_streams;

    char *Cache_For_Names;

#ifndef BTE_IMP_USER_DEFINED
    char *working_disk;
#endif

    // Figure out how much memory we've got to work with.

    if (MM_manager.available(&sz_avail) != MM_ERROR_NO_ERROR) {
        return AMI_ERROR_MM_ERROR;
    }

    //Monitor
    cout << "RAKESH: Avail memory is " << sz_avail << "\n";

#ifndef BTE_IMP_USER_DEFINED
    working_disk = getenv("TMP");
#endif

    // If the whole input can fit in main memory then just call
    // AMI_main_mem_merge() to deal with it by loading it once and
    // processing it.

    len = instream->stream_len();
    instream->seek(0);

    //Monitor
    cout << "Instream length is " << len << "\n";


    if ((len * sizeof(T)) <= sz_avail) {

          if (len * (sizeof(T)*sizeof(qsort_item<KEY>)) > sz_avail)
           // ie if you have dont have space for separate
           // keysorting (good cache performance) followed by permuting 
          {
           
           T * next_item;
           T * mm_stream = new T[len];

           for (int i = 0; i <  len; i++)
           {
            if ((ae =  instream->read_item(&next_item)) != AMI_ERROR_NO_ERROR)
                                                  return ae;
            mm_stream[i] = *next_item;
           }

           quicker_sort_op((T *)mm_stream,len);

            for (int i = 0; i <  len; i++)
           {
            if ((ae = outstream->write_item( mm_stream[i])) 
                                     != AMI_ERROR_NO_ERROR)
                                                 return ae;
           }
         }
         else
         {
         //Use qsort on keys followed by permuting

         T * mm_stream = new T[len];
         qsort_item<KEY> *qs_array = new (qsort_item<KEY>)[len];
         T * next_item;

         for (int i = 0; i <  len; i++)
          {
           if ((ae =  instream->read_item(&next_item)) != AMI_ERROR_NO_ERROR)
                                                  return ae;
            mm_stream[i] = *next_item;
            qs_array[i].keyval = * (KEY *)((char *)next_item+keyoffset);
            qs_array[i].source = i;
           }

          quicker_sort_op(( qsort_item<KEY> *)qs_array,len);

          for (int i = 0; i <  len; i++)
          {
            if ((ae = outstream->write_item(mm_stream[qs_array[i].source]))
                                           != AMI_ERROR_NO_ERROR)
                                           return ae;
            }
          }
          
          return AMI_ERROR_NO_ERROR;

    } else {

        // The number of substreams that the original input stream
        // will be split into

        arity_t original_substreams;

        // The length, in terms of stream objects of type T, of the
        // original substreams of the input stream.  The last one may
        // be shorter than this.

        size_t sz_original_substream;

        // The initial temporary stream, to which substreams of the
        // original input stream are written.

        AMI_STREAM<T> **initial_tmp_stream;

        // The number of substreams that can be merged together at once.

        arity_t merge_arity;

        // A pointer to the buffer in main memory to read a memory load into.
        T *mm_stream;


        // Loop variables:

        // The stream being read at the current level.


        int runs_in_current_stream;

//RAKESH
        AMI_STREAM<T> **current_input;

        // The output stream for the current level if it is not outstream.

//RAKESH
        AMI_STREAM<T> **intermediate_tmp_stream;

	   //TO DO
//RAKESH  (Hard coded prefixes) Ideally you be asking TPIE to give new names
        char * prefix_name[] = {"Tempo_stream0", "Tempo_stream1"};
        char itoa_str[5];


        // The size of substreams of *current_input that are being
        // merged.  The last one may be smaller.  This value should be
        // sz_original_substream * (merge_arity ** k) where k is the
        // number of iterations the loop has gone through.

        size_t current_substream_len;

        // The exponenent used to verify that current_substream_len is
        // correct.

        unsigned int k;

        off_t sub_start, sub_end;

        // How many substreams will there be?  The main memory
        // available to us is the total amount available, minus what
        // is needed for the input stream and the temporary stream.



        size_t mergeoutput_v;


        if ((ae = instream->main_memory_usage(&sz_stream,
                                              MM_STREAM_USAGE_MAXIMUM)) !=
                                              AMI_ERROR_NO_ERROR) {
            return ae;
        }

        if ((ae = instream->main_memory_usage(&sz_substream,
                                              MM_STREAM_USAGE_OVERHEAD)) !=
                                              AMI_ERROR_NO_ERROR) {

              return ae;
        }

        
        //Conservatively assume that the input and output streams
	   //have not been accounted for in the bte_stream.

        sz_avail -= 2*sz_stream;

        sz_original_substream = sz_avail - 2 * sz_stream;

//      Here the above var is in bytes: in AMI_partition_and_merge,
//      its in number of items of type T.



//RAKESH
// In our case merge_arity is determined differently than in the original
// implementation of AMI_partition_and_merge since we use several streams
// in each level.
// In our case net main memory required to carry out an R-way merge is
// (R+1)*MM_STREAM_USAGE_MAXIMUM  {R substreams for input runs, 1 stream for output}
// + R*MM_STREAM_USAGE_OVERHEAD   {One stream for each active input run: but while
//                                 the substreams use buffers, streams don't}
// + (R+1)*m_obj->space_usage_per_stream();
//
// The net memory usage for an R-way merge is thus
// R*(sz_stream + sz_substeam + m_obj->space_usage_per_stream()) + sz_stream +
// m_obj->space_usage_per_stream();
//





	   //We can probably make do with a little less memory
	   //if there is only a single binary merge pass required
	   //but its too specialized a case to optimize for.

         if (sz_avail <= 3*(sz_stream + sz_substream 
                            + sizeof(merge_heap_element<KEY>))
                           //+ sz_stream + sizeof(merge_heap_element<KEY>)
                            ) 
		 {

		   //Monitor
            cout << "RAKESH:Insufficient mem\n";
            LOG_FATAL("Insufficient Memory for AMI_replacement_selection_and_merge_Key()");
            LOG_FLUSH_LOG;
            return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
        }






        // Round the original substream length off to an integral
        // number of chunks.  This is for systems like HP-UX that
        // cannot map in overlapping regions.  It is also required for
        // BTE's that are capable of freeing chunks as they are
        // read.

        {
            size_t sz_chunk_size = instream->chunk_size();

            //RAKESH: Why is this the ceiling instead of being the floor?

            sz_original_substream = sz_chunk_size *
                ((sz_original_substream + sz_chunk_size - 1) /
                 sz_chunk_size);
        }




//The foll qty is a "to be determined" qty since the run lengths
// resulting from replacement selection are unknown.


        // Account for the space that a merge object will use.

        {
            size_t sz_avail_during_merge =
                sz_avail - sz_stream - sz_substream - sz_stream - 
                                    sizeof(merge_heap_element<KEY>);

            size_t sz_stream_during_merge =sz_stream + sz_substream +
                 sizeof(merge_heap_element<KEY>);

            merge_arity = sz_avail_during_merge/sz_stream_during_merge;

		  //Monitor
            cout << "Original mem size available " << sz_avail << "\n";
            cout << "Max stream overhead " << sz_stream << "\n";
            cout << "Merge Obj Overhead " <<  sizeof(merge_heap_element<KEY>)   << "\n";
            cout << "Availablity after accounting for output stream" 
                                    << sz_avail_during_merge << "\n";
            cout << "Overhead Per input run " <<  sz_stream_during_merge << "\n";
            cout << "Thus merge arity is " << merge_arity << "\n";

        }

        // Make sure that the AMI is willing to provide us with the
        // number of substreams we want.  It may not be able to due to
        // operating system restrictions, such as on the number of
        // regions that can be mmap()ed in.

        {
            int ami_available_streams = instream->available_streams();

            if (ami_available_streams != -1) {
                if (ami_available_streams <= 4) {
                    return AMI_ERROR_INSUFFICIENT_AVAILABLE_STREAMS;
                }

                if (merge_arity > (arity_t)ami_available_streams - 2) {
                    merge_arity = ami_available_streams - 2;
                    LOG_INFO("Reduced merge arity due to AMI restrictions.\n");
                    cout << "debug: Merge arity reduced to " << merge_arity << "\n";
                }
            }
        }

        LOG_INFO("AMI_replacement_selection_and_merge(): merge arity = " <<
                 merge_arity << ".\n");

        if (merge_arity < 2) {
            LOG_FATAL("Insufficient Memory for AMI_replacement_selection_and_merge_Key()");
            LOG_FLUSH_LOG;
            return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
        }



        // Create a temporary stream, then iterate through the
        // substreams, processing each one and writing it to the
        // corresponding substream of the temporary stream.
//RAKESH

	   //Monitor
        cout << "RAKESH original substreams " << original_substreams << "\n";
        cout << "Merge arity " << merge_arity << ".\n";


        instream->seek(0);


     size_t check_size = 0;

     char computed_prefix[BTE_PATH_NAME_LEN];
     char new_stream_name[BTE_PATH_NAME_LEN];



	//Compute a prefix that will be sent to the run formation function,
	//since that is where the initial runs are formed.

#ifndef BTE_IMP_USER_DEFINED
       strcpy(computed_prefix,working_disk);
       strcat(computed_prefix,"/");
       strcat(computed_prefix,prefix_name[0]);
#endif


#ifdef BTE_IMP_USER_DEFINED
        strcpy(computed_prefix,prefix_name[0]);
#endif


	  //Monitor
       cout << "Computed_prefix is " << computed_prefix << "\n";

	  //Conservatie estimate of the max possible number of runs during
	  //run formation.
       int MaxRuns = instream->stream_len()/
         (sz_original_substream/(sizeof(run_formation_item<KEY>)+sizeof(T)));

	  //Monitor
       cout << "Size of each array to hold run lengths for a given stream is "
            << (MaxRuns+merge_arity-1)/merge_arity << "\n";

	  //Arrays to store the number of runs in each of the streams formed 
	  //during each pass and the length of each of the runs.

       int RunsInStream[2][merge_arity], 
           RunLengths[2][merge_arity][(MaxRuns+merge_arity-1)/merge_arity];



        for (int i = 0; i < merge_arity; i++) {
					 RunsInStream[0][i]=0;
					 RunsInStream[1][i]=0;
				      }


         KEY dummykey; // This is only for the last argument to 
	                  //Run_Formation() that was added because of type 
	                  //unifcation problems.
        

   
	    //Monitor
         if (gettimeofday(&tp1,NULL) == -1) {perror("gettimeofday");}

	    //Call the run formation function.

         if ((ae = Run_Formation_Algo_R_Key( instream,
                                        merge_arity,
                                        initial_tmp_stream,
                                        computed_prefix,
                                        sz_original_substream,
                                        RunsInStream[0],
                                        (int **) RunLengths[0],
                                        (MaxRuns+merge_arity-1)/merge_arity,
                                        keyoffset, dummykey
                                       ))  != AMI_ERROR_NO_ERROR)
                                       {
                                        cout << "Run Formation error\n";
                                        cout << "AMI Error " << ae << "\n";
                                       }



	    //Monitor
        if (gettimeofday(&tp2,NULL) == -1) {perror("gettimeofday");}


        // Make sure the total length of the temporary stream is the
        // same as the total length of the original input stream.

        arity_t run_count =0;

        for (int i=0; i < merge_arity; i++) {
               for (int j=0; j < RunsInStream[0][i]; j++)  {

               check_size+=RunLengths[0][i][j];
              } 
                       
          run_count+=RunsInStream[0][i];
          }

	   //Monitor
        tp_dub1 = tp2.tv_sec-tp1.tv_sec;
        tp_dub2 = tp2.tv_usec-tp1.tv_usec;
    
        cout << "RF Pass:\nNumber of runs=" <<
        run_count <<
         "\nTime=" << (tp_dub1*1000000 + tp_dub2)/1000 << " millisecs\n";


        if (check_size != instream->stream_len()) 
            {
		    //Monitor
            cout << "Run formation input stream is of length " << instream->stream_len() << "\n";
            cout << "but outputs " << check_size << " items\n";
            return AMI_ERROR_IO_ERROR;
            } 


        tp_assert(instream->stream_len() == check_size,
                  "Stream lengths do not match:" <<
                  "\n\tinstream->stream_len() = " << instream->stream_len() <<
                  "\n\tinitial_tmp_stream->stream_len() = " <<
                  check_size << ".\n");

        // Set up the loop invariants for the first iteration of the
        // main loop.

        //instream->persist(PERSIST_PERSISTENT);
	   // delete instream;


        current_input = new (AMI_STREAM<T> *)[merge_arity]; 
        arity_t next_level_run_count;
        int run_start[merge_arity];


        // Pointers to the substreams that will be merged.

//RAKESH
       AMI_STREAM<T> **the_substreams = new (AMI_STREAM<T> *)[merge_arity];

        k = 0;

        // The main loop.  At the outermost level we are looping over
        // levels of the merge tree.  Typically this will be very
        // small, e.g. 1-3.



        while (run_count > 1) {

		//Monitor
          if (gettimeofday(&tp1,NULL) == -1) {perror("gettimeofday");}



            // Set up to process a given level.
//RAKESH

            // Do we have enough main memory to merge all the
            // substreams on the current level into the output stream?
            // If so, then we will do so, if not then we need an
            // additional level of iteration to process the substreams
            // in groups.


            if (run_count <= merge_arity) {

//RAKESH   Open up the run_count streams in which the
//         the runs input to the current merge level are packed
//         The names of these streams (storing the input runs)
//         can be constructed from  prefix_name[k % 2]



          for(ii= merge_arity - run_count; ii < merge_arity; ii++)
                {

#ifndef BTE_IMP_USER_DEFINED 

                stream_name_generator(working_disk, 
                                      prefix_name[k % 2], 
                                      (int) ii, 
                                      new_stream_name);

#endif

#ifdef  BTE_IMP_USER_DEFINED

                stream_name_generator("",
                                      prefix_name[k % 2], 
                                      (int) ii, 
                                      new_stream_name);

#endif 

                current_input[ii] = new AMI_STREAM<T>(new_stream_name);
                current_input[ii]->persist(PERSIST_DELETE);
                
                }


                
                // Merge them into the output stream.

		//Monitor
               cout << "RAKESH: Reached just before (last pass)
AMI_single_merge\n";

                ae = MIAMI_single_merge_Key(
                                      (current_input+ merge_arity-run_count),
                                      run_count,
                                       outstream,
                                       keyoffset,
                                       dummykey
                                       ); 

			 //Monitor
                cout << "Length of output stream after MERGE is " << outstream->stream_len() << " \n";


                if (ae != AMI_ERROR_NO_ERROR) {
			   //Monitor

                  cout << "MIAMI_single_merge_Key error : AMI_ERROR " << ae << "\n";
                    return ae;
                }

                cout << "RAKESH: Reached just after (last pass) AMI_single_merge\n";

                
                // Delete the substreams.
//RAKESH

                for (ii = merge_arity - run_count; ii < merge_arity; ii++) {

                   delete current_input[ii];

                }

                // And the current input, which is an intermediate stream
                // of some kind.



                 delete[] current_input;
                 delete[] the_substreams;
                
			  //Monitor
                if (gettimeofday(&tp2,NULL) == -1) {perror("gettimeofday");}

                tp_dub1 = tp2.tv_sec-tp1.tv_sec; 
                tp_dub2 = tp2.tv_usec-tp1.tv_usec;

                cout << "Final Pass:\n Arity=" << run_count <<
                "\nTime=" << (tp_dub1*1000000 + tp_dub2)/1000 << " millisecs\n";


                run_count = 1;

            } else {

                
		    //Monitor
               cout << "RAKESH: More than one pass\n";
               LOG_INFO("Merging substreams to an intermediate stream.\n");

                // Create the array of merge_arity stream pointers that
                // will each point to a stream containing runs output
                // at the current level k.

               // Note that the array RunLengths[k % 2][ii] contains lengths of
               // the RunsInStream[k % 2][ii] runs in current_input stream
               // ii. 


			//Number of runs in the next level.
              next_level_run_count = (run_count + merge_arity - 1)/merge_arity; 

              intermediate_tmp_stream = new (AMI_STREAM<T> *) [merge_arity];


//RAKESH   Open up the merge_arity streams in which the
//         the runs input to the current merge level are packed
//         The names of these streams (storing the input runs)
//         can be constructed from  prefix_name[k % 2]

                for (ii=0; ii < merge_arity; ii++)
                        {

//                      new_name_from_prefix(prefix_name[k % 2],(int) ii,
//                                            new_stream_name);

#ifndef BTE_IMP_USER_DEFINED

                        stream_name_generator(working_disk, 
                                              prefix_name[k % 2], 
                                              (int) ii, 
                                              new_stream_name);
#endif

#ifdef BTE_IMP_USER_DEFINED

                        stream_name_generator("",
                                              prefix_name[k % 2], 
                                              (int) ii, 
                                              new_stream_name);

#endif


				    //Construct the stream
                        current_input[ii]  = new AMI_STREAM<T>(new_stream_name);
 
                      
                        current_input[ii]->persist(PERSIST_DELETE);
                         
                        }



			 //Stream counter
                int current_stream = merge_arity-1;

                //Construct the first stream that we use to pack some
                //of the output runs of the current merge level k.


#ifndef BTE_IMP_USER_DEFINED 


                stream_name_generator(working_disk, 
                                      prefix_name[(k+1) % 2], 
                                      current_stream , 
                                      new_stream_name);
#endif
     
#ifdef BTE_IMP_USER_DEFINED
                stream_name_generator("", 
                                      prefix_name[(k+1) % 2], 
                                      current_stream , 
                                      new_stream_name);


#endif  



        intermediate_tmp_stream[current_stream]  = new AMI_STREAM<T>(
                                                          new_stream_name);





        intermediate_tmp_stream[current_stream]->persist(PERSIST_PERSISTENT);


	   //Number of output runs that remain to be generated at this level.
        int remaining_number_of_output_runs =
                          (run_count +  merge_arity - 1)/merge_arity;


	   //Determine the number of runs that will go in each of the streams
	   //that will be output at this level.

         for (ii_streams = 0; ii_streams < merge_arity; ii_streams ++)
               {

                       if (ii_streams <
                             (merge_arity -  
                             (remaining_number_of_output_runs % merge_arity)) 
                          )

                            RunsInStream[(k+1) % 2][ii_streams] =
                                remaining_number_of_output_runs/ merge_arity;
    
                       else
                            RunsInStream[(k+1) % 2][ii_streams] =
                               (remaining_number_of_output_runs + merge_arity -1)/
                                                           merge_arity;
                       run_start[ii_streams]=0;
				   
				   //Monitor
                       cout << "Stream " << ii_streams << " shd get " << 
					RunsInStream[(k+1) % 2][ii_streams] << "streams\n";

               }


               runs_in_current_stream = 0;


                // Loop through the substreams of the current stream,
                // merging as many as we can at a time until all are
                // done with.


                mergeoutput_v = 0;
                int merge_number = 0;
                for ( ii = 0, jj = 0;
                     ii < run_count;
                     ii++, jj++) {


			   //Runs can be of various lengths; so need to have 
			   //appropriate starting and ending points for substreams.

                    sub_start = run_start[merge_arity - 1 - jj];

                    sub_end = sub_start +
                    RunLengths[k % 2][merge_arity - 1 - jj] [merge_number]-1;

                    run_start[merge_arity - 1 - jj]+= 
                    RunLengths[k % 2][merge_arity - 1 - jj] [merge_number];



				//The weirdness below is because of the nature of the
				// substream arguments.

                    if (sub_end >= 
                        current_input[merge_arity-1-jj]->stream_len()) 
                    {
                          
                        sub_end = 
                          current_input[merge_arity-1-jj]->stream_len() - 1;

                        if (sub_start > 
                            current_input[merge_arity-1-jj]->stream_len())

                              sub_start = sub_end+1;

                    }



                    mergeoutput_v+= sub_end - sub_start +1;


                    if (sub_end  - sub_start + 1 == 0) ii--;

                  //NOTE:If the above condition is true it means that 
			   // the run just  encountered is a dummy run;
                  // the last merge of a pass  has
                  //   ( merge_arity - (run_count % merge_arity) )
                  // dummy runs; no other merge of the pass has any dummy run.


				//Monitor
                    cout << "Stream " << merge_arity - 1 - jj <<
				" sub_start " << sub_start << ", sub_end " << sub_end << "\n";
         

                   current_input[merge_arity-1-jj]->new_substream(
                                                AMI_READ_STREAM,
                                                sub_start,
                                                sub_end,
                                                (AMI_base_stream<T> **)
                                                (the_substreams + jj));


                    // If we've got all we can handle or we've seen
                    // them all, then merge them.


                    if ((jj >= merge_arity - 1) || (ii == run_count - 1)) {

                        tp_assert(jj <= merge_arity - 1, "Index got too large.");




				    //Check to see if the current intermediate_tmp_stream
				    //contains as many runs as it should; if yes, then
				    //destroy (with PERSISTENCE) that stream and 
				    //construct the next intermediate_tmp_stream. 

                    if (RunsInStream[(k+1)%2][current_stream] 
                                              == runs_in_current_stream)

                    {

                     intermediate_tmp_stream[current_stream]->persist(
                                                      PERSIST_PERSISTENT);

                     delete intermediate_tmp_stream[current_stream];

                     current_stream = (current_stream + merge_arity -1) % merge_arity;


                     // Unless the current level is over, we've to 
				 //generate a new stream for the next set of runs.

                       if (remaining_number_of_output_runs > 0) {


#ifndef BTE_IMP_USER_DEFINED 

                        stream_name_generator(working_disk, 
                                              prefix_name[(k+1) % 2], 
                                              (int) current_stream, 
                                              new_stream_name);

#endif
     

#ifdef BTE_IMP_USER_DEFINED

                          stream_name_generator("",
                                                prefix_name[(k+1) % 2], 
                                                (int) current_stream,
					                       new_stream_name);


#endif 


                           intermediate_tmp_stream[current_stream] = 
                                                          new AMI_STREAM<T>(
                                                          new_stream_name);


                           intermediate_tmp_stream[current_stream]->persist(
                                                         PERSIST_PERSISTENT);
                           runs_in_current_stream = 0;
                       }
                      }


                        // The merge should append to the output stream, since
                        // AMI_single_merge() does not rewind the
                        // output before merging.

                      ae = MIAMI_single_merge_Key(the_substreams,
                                              jj+1,
                                       intermediate_tmp_stream[current_stream],
                                       keyoffset,
                                       dummykey 
                                       );

                      if (ae != AMI_ERROR_NO_ERROR) {
				    //Monitor
				    cout << "MIAMI_single_merge_Key Error " << ae << "\n";
                            return ae;
                                                    }

				  //Monitor
                      cout << "Length of stream " 
                      << current_stream << " with merge output is "
                      << intermediate_tmp_stream[current_stream]->stream_len() << "\n";

                      RunLengths[(k+1) % 2][current_stream]
                                //[merge_number/merge_arity] = mergeoutput_v;
                                [runs_in_current_stream]  = mergeoutput_v;

				  //Monitor
                      cout << "Recorded output run length is " <<  mergeoutput_v << "\n";


                      merge_number++;


//RAKESH Decrement the counter corresp to number of runs still to be
// formed at current level

                        mergeoutput_v = 0;
                        remaining_number_of_output_runs--;


                        // Delete the substreams.  jj is currently the index
                        // of the largest, so we want to bump it up before the
                        // idiomatic loop.

                         for (jj++; jj--; ) {
                          delete the_substreams[jj];
                          }

                        // Now jj should be -1 so that it gets bumped
                        // back up to 0 before the next iteration of
                        // the outer loop.
                        tp_assert((jj == -1), "Index not reduced to -1.");

//RAKESH                Advance the starting position within each of the
//                      current_input streams by the input run length
//                      of merge level k.



//RAKESH                The number of runs in the current_stream
//                      goes up by 1.

                        runs_in_current_stream ++;


                    }

                }

                delete intermediate_tmp_stream[current_stream];

              // Get rid of the current input stream and use the next one.

                for (ii = 0; ii < merge_arity; ii++) { 
                                                delete current_input[ii];
                                                }

                delete[] current_input;

                current_input = (AMI_STREAM<T> **) intermediate_tmp_stream;

                run_count = next_level_run_count;
            }


            if (run_count != 1) {

		    //Monitor
               if (gettimeofday(&tp2,NULL) == -1) {perror("gettimeofday");}
                

              tp_dub1 = tp2.tv_sec-tp1.tv_sec;
               tp_dub2 = tp2.tv_usec-tp1.tv_usec;

               cout << "Intmdt Pass:\n Arity=" << merge_arity <<
               "\nTime=" << (tp_dub1*1000000 + tp_dub2)/1000 << " millisecs\n";


              }


            k++;


        }


        cout << "Number of passes including runformation pass is " << k+1 << "\n";

        return AMI_ERROR_NO_ERROR;
    }
}







template<class T, class KEY> AMI_err MIAMI_single_merge_Key(AMI_STREAM<T> **instreams, arity_t arity, AMI_STREAM<T> *outstream, int keyoffset, KEY dummykey)
{


// Pointers to current leading elements of streams
T * in_objects[arity+1];
int i,j;
AMI_err ami_err;

//The number of actual heap elements at any time: can change even 
//after the merge begins because
// whenever some stream gets completely depleted, heapsize decremnents by one.

int heapsize_H;

class merge_heap_element<KEY> * K_Array = 
                            new (merge_heap_element<KEY>)[arity+1];

T merge_out;

    // Rewind and read the first item from every stream.

    j = 1;
    for (i = 0; i < arity ; i++ ) {
        if ((ami_err = instreams[i]->seek(0)) != AMI_ERROR_NO_ERROR) {
            return ami_err;
        }

        if ((ami_err = instreams[i]->read_item(&(in_objects[i]))) !=
            AMI_ERROR_NO_ERROR) {
            if (ami_err == AMI_ERROR_END_OF_STREAM) {
                in_objects[i] = NULL;
             
            } else {
               return ami_err;
            }
            // Set the taken flags to 0 before we call intialize()
        } else {
         
         K_Array[j].key = *(KEY *)((char *)in_objects[i]+keyoffset);
         K_Array[j].run_id = i;
         j++; 


        
        }
    }

    unsigned int NonEmptyRuns = j-1;


    merge_heap<KEY> Main_Merge_Heap(K_Array, NonEmptyRuns);

    while (Main_Merge_Heap.sizeofheap())
    {
    i = Main_Merge_Heap.get_min_run_id();
    if ((ami_err = outstream->write_item(*in_objects[i])) 
                                    != AMI_ERROR_NO_ERROR)
                   {
				 //Monitor
				 cout << "Write item error in MIAMI_single..._Key()\n";
                    return ami_err;
                   }

    if ((ami_err = instreams[i]->read_item(&(in_objects[i])))
                                       != AMI_ERROR_NO_ERROR)
                   {
                   if (ami_err != AMI_ERROR_END_OF_STREAM)
                     {
				   //Monitor
                     cout << "Read_item error in MIAMI_single_merge\n";
                     return ami_err;
		           }
                   }

    if (ami_err == AMI_ERROR_END_OF_STREAM)
                  {
                   Main_Merge_Heap.delete_min_and_insert((KEY *)NULL);
                  }
    else 
	   	        {
                   Main_Merge_Heap.delete_min_and_insert(
                                 (KEY *)((char *)in_objects[i]+keyoffset)
                                                        );
                 } 
    }

    return AMI_ERROR_NO_ERROR;

}


// We assume that instream is the already constructed input stream 
// to be sorted by AMI_partition and merge. 
// We assume that the outstreams array is yet to be constructed.
//We assume that available_mem is the value of available memory *after*
// accounting for the fact that at any time during run formation,
// the input stream and one output stream will be active (so their
// memory-usage will have to have been taken into consideration.)
// We assume that computed_prefix is such that it contains the 
//appropriate intermediate stream name up to Tempo_stream: so  
// we need to only add the integer at the end to get a stream name.


template<class T, class KEY> AMI_err MIAMI_single_merge_Key_scan(AMI_STREAM<T> **instreams, arity_t arity, AMI_STREAM<T> *outstream, int keyoffset, KEY dummykey)
{


// Pointers to current leading elements of streams
T * in_objects[arity+1];
int i,j;
AMI_err ami_err;

//The number of actual heap elements at any time: can change even 
//after the merge begins because
// whenever some stream gets completely depleted, heapsize decremnents by one.

int heapsize_H;

class merge_heap_element<KEY> * K_Array = new (merge_heap_element<KEY>)[arity+1];

T merge_out;

//RAKESH:
//for (i=0; i < arity; i++)
//       cout << "STream " << i << " of length " << instreams[i]->stream_len() << "\n";

    // Rewind and read the first item from every stream.
    j = 1;
    for (i = 0; i < arity ; i++ ) {
        if ((ami_err = instreams[i]->seek(0)) != AMI_ERROR_NO_ERROR) {
		//TRASH
            cout << "Seek error in MIAMI merge\n";
            return ami_err;
        }

        if ((ami_err = instreams[i]->read_item(&(in_objects[i]))) !=
            AMI_ERROR_NO_ERROR) {
            if (ami_err == AMI_ERROR_END_OF_STREAM) {
                in_objects[i] = NULL;
             
            } else {
		    //TRASH 
               cout << "RAKESH: read_item AMI_stream error" << ami_err << "\n";
               return ami_err;
            }
            // Set the taken flags to 0 before we call intialize()
        } else {
#if DEBUG_PERFECT_MERGE
    input_count++;
#endif  
         
         K_Array[j].key = *(KEY *)((char *)in_objects[i]+keyoffset);
         K_Array[j].run_id = i;
         j++; 


        
        }
    }

    unsigned int NonEmptyRuns = j-1;


    merge_heap<KEY> Main_Merge_Heap(K_Array, NonEmptyRuns);

#if SORT_VERIFY_OPT 
    int MIAMI_Cntr = 0;
#endif
    while (Main_Merge_Heap.sizeofheap())
    {
    i = Main_Merge_Heap.get_min_run_id();
    //if ((ami_err = outstream->write_item(*in_objects[i])) 
    //                                != AMI_ERROR_NO_ERROR)
    //               {
    //                cout << "RAKESH: write_item error in MIAMI_single_merge\n";
    //                return ami_err;
    //                }

    if ((ami_err = instreams[i]->read_item(&(in_objects[i])))
                                       != AMI_ERROR_NO_ERROR)
                   {
                   if (ami_err != AMI_ERROR_END_OF_STREAM)
                     {
				   //TRASH
                     cout << "RAKESH: read_item error in MIAMI_single_merge\n";
                     return ami_err;
		           }
                   }

    if (ami_err == AMI_ERROR_END_OF_STREAM)
                  {
                   Main_Merge_Heap.delete_min_and_insert((KEY *)NULL);

#if SORT_VERIFY_OPT
			    //                   cout <<  "Heap outputted " << MIAMI_Cntr << " elements\n";
#endif
                  
                  }
    else 
	   	        {
                   Main_Merge_Heap.delete_min_and_insert(
                                 (KEY *)((char *)in_objects[i]+keyoffset)
                                                        );
                 } 

    }

#if SORT_VERIFY_OPT
    //    cout << "Heap outputted " << MIAMI_Cntr << " elements\n";
#endif

    return AMI_ERROR_NO_ERROR;

}


// We assume that instream is the already constructed input stream 
// to be sorted by AMI_partition and merge. 
// We assume that the outstreams array is yet to be constructed.
//We assume that available_mem is the value of available memory *after*
// accounting for the fact that at any time during run formation,
// the input stream and one output stream will be active (so their
// memory-usage will have to have been taken into consideration.)
// We assume that computed_prefix is such that it contains the 
//appropriate intermediate stream name up to Tempo_stream: so  
// we need to only add the integer at the end to get a stream name.



template<class T, class KEY> 
AMI_err Run_Formation_Algo_R_Key( AMI_STREAM<T> *instream, 
                                                arity_t arity, 
                                                AMI_STREAM<T> **outstreams,
                                               char * computed_prefix, 
                                                size_t available_mem,                      
                                                int * LRunsInStream,
                                                int ** LRunLengths,
                                                int dim2_LRunLengths,                                                          int offset_to_key,
                                                KEY dummykey)
                                      
{

//TRASH
cout << "Entered Run_Formation_Algo_R\n";

char local_copy[BTE_PATH_NAME_LEN];

strcpy(local_copy,computed_prefix);

AMI_err ami_err;

//For now we are assuming that the key is of type int 
//and that the offset of the key within an item of type
//T is offset_to_key=0

//Define the proper structure for algorithm R of Vol 3

/*
struct rf_item{ 
               T  Record;
               int Loser;
               int RunNumber;
               int ParentExt;
               int ParentInt;
              }  ;
*/

//What is called "P" in algorithm R in Vol 3. (Avg run length is 2P)
unsigned int Number_P = 
              available_mem/(sizeof(run_formation_item<KEY>)+sizeof(T));

run_formation_item<KEY> *Array_X = new (run_formation_item<KEY>)[Number_P];
T * Item_Array = new T[Number_P];

//TRASH
cout << "Allocated memory for P= " << Number_P << " to Array_X in Run Formation routine\n";
cout << "Size of rf_item is " << sizeof(run_formation_item<KEY>) << "\n";
cout << "Avail mem to run formation phase is " << available_mem << "\n";

T * ptr_to_record;
unsigned int tempint;
unsigned int Var_T;
unsigned int curr_run_length=0;

//We will  write first run into stream arity-1, then next run into
//stream arity-2, and so on in a round robin manner.
int current_stream = arity - 1;

char int_to_string[5], new_stream_name[BTE_PATH_NAME_LEN];


// In the case we opt to go in for an alt_disk_config,
// currently we aasume that there are four disks and that 
// the input stream is on disks 2 and 3 
#if ALT_DISK_CONFIG
    int blocks_in_a_stripe = MAX_BLOCKS_IN_A_STRIPE;
    int disk_arrays[2][2];
    disk_arrays[0][0] = 0;
    disk_arrays[0][1] = 1;
    disk_arrays[1][0] = 2;
    disk_arrays[1][1] = 3;
#endif


outstreams = new (AMI_STREAM<T> *)[arity];


int * Cast_Var = (int *)LRunLengths;
int MaxRuns = instream->stream_len()/(available_mem/sizeof(run_formation_item<KEY>));


int RF_Cntr=0;

short RMAX = 0;
short RC = 0;
KEY LASTKEY; //Should be guaranteed to be initialized to something greater than 
             //the key value of the first item in instream, for correctness.

int Q = 0;
short RQ = 0;

for (unsigned int j=0; j < Number_P; j++)
    {
     Array_X[j].Loser = j;
     Array_X[j].RunNumber = 0;
     Array_X[j].ParentExt = (Number_P+j)/2;
     Array_X[j].ParentInt = j/2;
     Array_X[j].RecordPtr = j;
    }

Step_R2:
    if (RQ != RC)  {
                   if (RC >= 1) 
                   {   
//TRASH
//delete the previous run
//                 cout << "Curr run length is " << curr_run_length << "\n"; 

                   Cast_Var[current_stream*dim2_LRunLengths 
                         + LRunsInStream[current_stream]] = curr_run_length;

                   LRunsInStream[current_stream]++;

//                 cout << "RAKESH: Destructing stream " << current_stream << "\n";

                   delete outstreams[current_stream];

//TRASH
                   cout << "Run of length " <<  curr_run_length << 
                           " output to stream " << current_stream << "\n";
                   

                   current_stream = (current_stream + arity - 1) % arity;
                   RF_Cntr+=curr_run_length;
                   }
                     

                   if (RQ > RMAX) goto Step_End;
                   else RC = RQ;

                  // Now construct the possibly previously destroyed stream for
                  // new run and seek to its end.

                  //Compute the name for the stream
                  sprintf(int_to_string,"%d",current_stream);
                  strcpy(new_stream_name,local_copy);
                  strcat(new_stream_name,int_to_string);
 


                 // Use the appropriate constructor.
                 //BEGIN CONSTRUCT STREAM

#ifdef BTE_IMP_USER_DEFINED

#if ALT_DISK_CONFIG   //*********************************************************

                  outstreams[current_stream] =      new AMI_STREAM<T>(new_stream_name,
                                                                     AMI_WRITEONLY_STREAM,
                                 		                     blocks_in_a_stripe,
                                                  		     disk_arrays[0]);

#else // ************************************************************************

		  outstreams[current_stream] =      new AMI_STREAM<T>(new_stream_name

#if WANT_AMI_WRITEONLY_STREAMS
                 		                                     ,AMI_WRITEONLY_STREAM
#endif 
                                   		                     );

#endif //****************************************************************************

#else  //! BTE_IMP_USER_DEFINED******************************************************

                  outstreams[current_stream] =        new AMI_STREAM<T>(new_stream_name

#if WANT_AMI_WRITEONLY_STREAMS
                 		                                       ,AMI_WRITEONLY_STREAM
#endif
                                 		                        );

#endif //****************************************************************************


                  //END CONSTRUCT STREAM


         	  outstreams[current_stream]->persist(PERSIST_PERSISTENT);
                  cout << "RAKESH: Seeking to " << 
                 outstreams[current_stream]->stream_len() << " for "
                 << new_stream_name << "\n";
 
                  outstreams[current_stream]->seek(outstreams[current_stream]->stream_len());


                  // Now set length of currently being formed run to zero.
 
                  curr_run_length = 0;

//TRASH
//                cout << "Constructed stream " << new_stream_name << "\n";
                 }

// End of Step_R2


Step_R3:  
        if  (RQ == 0)   
               {

                if ((ami_err = instream->read_item(&ptr_to_record)) 
                                                          != AMI_ERROR_NO_ERROR)
                    {
                     if (ami_err == AMI_ERROR_END_OF_STREAM) {
                                   RQ = RMAX+1;
                                   goto Step_R5;
                                   }
                     return ami_err;
                    }

   
			 //Copy the most recently read item into item array loc
			 // Array_X[Q].RecordPtr            

                Item_Array[Array_X[Q].RecordPtr] = *ptr_to_record;
                Array_X[Q].Key = *(KEY *)((char *)ptr_to_record+offset_to_key);
 
        //The above portion is actually carried out in Step R4 in Vol 3's 
         // description of Algorithm R. But here we carry it out in Step
         // R3 itself so that we can efficiently simulate LASTKEY=Infinity
             

         //We've made sure that we read the first record from instream
         // Now we set LASTKEY to be one more than that first record's key
         // so that it simulates LASTKEY=Infinity


                LASTKEY = * (KEY *) ((char *)ptr_to_record+offset_to_key);
                ++LASTKEY; //LASTKEY = LASTKEY+1;
                
                

               }

         else 

               {

                if ((ami_err = outstreams[current_stream]->write_item(
                                      Item_Array[Array_X[Q].RecordPtr]))
                                                  != AMI_ERROR_NO_ERROR)
                    {
                     return ami_err; 
                    }
                  
                LASTKEY= Array_X[Q].Key;
 
                curr_run_length++;

            // The foll portion is actually carried out in Step R4 in Vol 3's 
            // description of Algorithm R. But here we carry it out in Step
            // R3 itself so that we can efficiently simulate LASTKEY=Infinity
 
                if ((ami_err = instream->read_item(&ptr_to_record))   
                                                          != AMI_ERROR_NO_ERROR)
                    {
                     if (ami_err == AMI_ERROR_END_OF_STREAM) {
                                   RQ = RMAX+1;
                                   goto Step_R5;
                                   }
                     return ami_err;
                    }

                Item_Array[Array_X[Q].RecordPtr] = *ptr_to_record;
                Array_X[Q].Key=*(KEY *)((char *)ptr_to_record+offset_to_key); 

			}


Step_R4: // Array_X[Q] already contains a new item from input stream.
         if (Array_X[Q].Key < LASTKEY)  {

                     // Array_X[Q].Record cannot go into the present run so :

                     RQ = RQ+1;
                     if (RQ > RMAX) RMAX = RQ;
                     }

Step_R5: 

         Var_T = Array_X[Q].ParentExt;

Step_R6:
        if (
            (Array_X[Var_T].RunNumber < RQ) || 
            (
             (Array_X[Var_T].RunNumber == RQ ) && 
             // KEY(LOSER(T)) < KEY(Q)
              Array_X[Array_X[Var_T].Loser].Key <  Array_X[Q].Key) 
           )

           { 
             // Swap LOSER(T) and Q 
             tempint = Array_X[Var_T].Loser;
             Array_X[Var_T].Loser = Q;
             Q = tempint;
            
             //Swap RN(T) and RQ
             tempint = Array_X[Var_T].RunNumber;
             Array_X[Var_T].RunNumber=RQ;
             RQ = tempint;
            }

Step_R7:
        if (Var_T == 1)  {goto Step_R2;}
        else
            {
             Var_T = Array_X[Var_T].ParentInt;
             goto Step_R6;
            }

Step_End: delete [] Array_X;
          delete [] outstreams;


		//TRASH
cout << "Elements scanned during RF : " << RF_Cntr << "\n";
for (int i = 0; i < arity; i++) 
    {
     cout << "Stream " << i << " has " << LRunsInStream[i] << " runs of lengths : \n";
     for (int j = 0; j < LRunsInStream[i]; j++)
         cout << " " << Cast_Var[i*dim2_LRunLengths+j] << " ";
     cout << "\n";
     }


return AMI_ERROR_NO_ERROR;

}



#ifdef NO_IMPLICIT_TEMPLATES

#define TEMPLATE_INSTANTIATE_CACHE_PERF_SORT(T,KEY)			\
template class run_formation_item<KEY>;                         \
TEMPLATE_INSTANTIATE_QSORT_ITEM(KEY)                             \
TEMPLATE_INSTANTIATE_QUICKER_SORT_OP(qsort_item<KEY>)            \
TEMPLATE_INSTANTIATE_QUICKER_SORT_OP(T)                          \
TEMPLATE_INSTANTIATE_MERGE_HEAP(KEY)                             \
template AMI_err AMI_partition_and_merge_Key(AMI_STREAM<T> *,	\
                                         AMI_STREAM<T> *,		\
                                         int, KEY);              \
template AMI_err AMI_replacement_selection_and_merge_Key(AMI_STREAM<T> *, \
                                                     AMI_STREAM<T> *,   \
                                                     int, KEY);           \
template AMI_err Run_Formation_Algo_R_Key( AMI_STREAM<T> *instream,     \
                                                arity_t arity,          \
                                             AMI_STREAM<T> **outstreams,\
                                               char *,                  \
                                                size_t available_mem,   \
                                                int *RunsInStream,      \
                                                int **RunLengths,       \
				                            int dim2_RunLengths,    \
                                                int offset_to_key,      \
                                                KEY dummykey);          \
template AMI_err MIAMI_single_merge_Key_scan(AMI_STREAM<T> **instreams,  \
                                  arity_t arity,                        \
                                  AMI_STREAM<T> *outstream,             \
                                   int keyoffset, KEY dummykey);        \
template AMI_err MIAMI_single_merge_Key(AMI_STREAM<T> **instreams,      \
                                  arity_t arity,                        \
                                  AMI_STREAM<T> *outstream,             \
                                  int keyoffset, KEY dummykey);


#define TEMPLATE_INSTANTIATE_MERGE_STREAMS(T)                     \
TEMPLATE_INSTANTIATE_QUICKER_SORT_OP(T)                          \
TEMPLATE_INSTANTIATE_MERGE_HEAP(T)                               \
template AMI_err AMI_partition_and_merge_stream(AMI_STREAM<T> *instream,   \
                                                AMI_STREAM<T> *outstream); \
template AMI_err MIAMI_single_merge_Key(AMI_STREAM<T> **instreams,         \
                                        arity_t arity,                     \
                                        AMI_STREAM<T> *outstream,          \
                                        int keyoffset, T dummykey);
#endif
#endif
