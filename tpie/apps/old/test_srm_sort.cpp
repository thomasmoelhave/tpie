//
// File: test_srm_sort.cpp
// Author: Rakesh Barve
//
// A test for SRM_sort().

//dh
#define MAX_BLOCKS_IN_A_STRIPE 2
#define PI_STREAM_MMB_LOGICAL_BLOCKSIZE_FACTOR 32

#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <iostream>

using std::cout;

// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

#define SCANBLOCKSIZE 144 
 
class scanstruct {
       public:
       unsigned long long keyval;
       char chararray[SCANBLOCKSIZE];

       friend int operator==(const scanstruct &x, const scanstruct &y)
             {return  (x.keyval ==  y.keyval);}
  
       friend int operator!=(const scanstruct &x, const scanstruct &y)
             {return  (x.keyval !=  y.keyval);}
  
       friend int operator<=(const scanstruct &x, const scanstruct &y)
             {return  (x.keyval <=  y.keyval);}
  
       friend int operator>=(const scanstruct &x, const scanstruct &y)
             {return  (x.keyval >=  y.keyval);}
  
       friend int operator<(const scanstruct &x, const scanstruct &y)
             {return  (x.keyval <  y.keyval);}
  
       friend int operator>(const scanstruct &x, const scanstruct &y)
             {return  (x.keyval >  y.keyval);}  
};


typedef scanstruct scantype;


// Define it all.
#include <ami.h>

#include <srm_merge.h>

// Utitlities for ascii output.
#include <ami_scan_utils.h>

static char def_srf[] = "/var/tmp/oss.txt";
static char def_rrf[] = "/var/tmp/osr.txt";

static char *sorted_results_filename = def_srf;
static char *rand_results_filename = def_rrf;

static bool report_results_random = false;
static bool report_results_sorted = false;
static bool sort_again = false;
static bool use_operator = false;
static bool kb_sort = false;

static const char as_opts[] = "R:S:rsaok";
void parse_app_opt(char c, char *optarg)
{
    switch (c) {
        case 'R':
            rand_results_filename = optarg;
        case 'r':
            report_results_random = true;
            break;
        case 'S':
            sorted_results_filename = optarg;
        case 's':
            report_results_sorted = true;
            break;
        case 'a':
            sort_again = !sort_again;
            break;
        case 'o':
            use_operator = !use_operator;
            break;
        case 'k':
            kb_sort = !kb_sort;
            break;
    }
}

//extern int register_new;


int main(int argc, char **argv)
{

    AMI_err ae;

    struct timeval tp1, tp2;

    parse_args(argc,argv,as_opts,parse_app_opt);

    if (verbose) {
        cout << "test_size = " << test_size << ".\n";
        cout << "test_mm_size = " << test_mm_size << ".\n";
        cout << "random_seed = " << random_seed << ".\n";
    } else {
        cout << test_size << ' ' << test_mm_size << ' ' << random_seed;
    }
    
    // Set the amount of main memory:
    MM_manager.resize_heap(test_mm_size);

    //register_new = 1;

#ifdef BTE_IMP_USER_DEFINED

    int blocks_in_a_stripe = MAX_BLOCKS_IN_A_STRIPE;
    int disk_arrays[2][2];
    disk_arrays[0][0] = 2;
    disk_arrays[0][1] = 3;
    disk_arrays[1][0] = 0;
    disk_arrays[1][1] = 1;

    char * patharray[] = {"/disks/disk0/",
                          "/disks/disk1/",
                            //"/disks/disk2/",
			             //"/disks/disk3/",
                          "/disks/disk4/",
                          "/disks/disk5/",
                          "/disks/disk7/",
                          "/disks/disk8/"};
    DiskSystem.ResetSystem(patharray,TOTAL_DISKS);
#endif



char InFileName[128],OutFileName[128];

#ifndef  BTE_IMP_USER_DEFINED
char * TmpPath = getenv("TMP");


strcpy(InFileName,TmpPath );
strcat(InFileName,"/InputFile");

strcpy(OutFileName,TmpPath );
strcat(OutFileName,"/OutputFile");

#else
strcpy(InFileName,"InputFile");
strcpy(OutFileName,"OutputFile");
#endif


//TRASH
cout << "Size of Key is " << sizeof(unsigned long long) << "\n";
cout << "Size of item is " << sizeof(scantype) << "\n";



AMI_STREAM<scantype> amis0(InFileName, AMI_READ_WRITE_STREAM);
AMI_STREAM<scantype> amis1(OutFileName,AMI_WRITEONLY_STREAM);

cout << "Constructed\n";

// Write some ints.
scantype Record;
unsigned long long fourth = test_size/4;

char *ptr1;

long longseed = (long) random_seed;
srand48(longseed);

        
for (int i=0; i < test_size; i++) {
  
        ptr1 = (char *)&(Record.keyval);
        *(unsigned int *)ptr1 = (unsigned int) mrand48();
        ptr1+=4;
        *(unsigned int *)ptr1 = (unsigned int) mrand48();
  
        //Record.keyval = fourth*(i % 4) + i/4; 

        if ((ae = amis0.write_item(Record))!= AMI_ERROR_NO_ERROR)
                                 {cout << "AMI_ERROR " << ae << "\n";}
    }





    amis0.persist(PERSIST_PERSISTENT);


    if (verbose) {
        cout << "Wrote the random values.\n";
        cout << "Stream length = " << amis0.stream_len() 
		   << " items;  " <<  amis0.stream_len()*sizeof(scantype)
             << " bytes.\n";
    }


   size_t sz_avail;

   if (MM_manager.available(&sz_avail) != MM_ERROR_NO_ERROR) { 
        return AMI_ERROR_MM_ERROR;
    }

//TRASH
    cout << "Sz before deleting stream is " << sz_avail << "\n";

   if (amis0.main_memory_usage(&sz_avail,MM_STREAM_USAGE_CURRENT)  
                                      ==  AMI_ERROR_NO_ERROR)
    cout << "Usage of amis0 is  " << sz_avail << "\n";

   if (amis1.main_memory_usage(&sz_avail,MM_STREAM_USAGE_CURRENT) 
                                      ==  AMI_ERROR_NO_ERROR) 
    cout << "Usage of amis1 is  " << sz_avail << "\n";

    
    amis0.persist(PERSIST_PERSISTENT);
    delete &amis0;
    //Just to free up space


       if (MM_manager.available(&sz_avail) != MM_ERROR_NO_ERROR) {
        return AMI_ERROR_MM_ERROR;
    }



//TRASH
    cout << "Sz after deleting stream is " << sz_avail << "\n";


    long dummylong = mrand48();
    unsigned long long dummykey;

    AMI_STREAM<scantype> amis00(InFileName,AMI_READ_WRITE_STREAM);

       if (MM_manager.available(&sz_avail) != MM_ERROR_NO_ERROR) {
        return AMI_ERROR_MM_ERROR;
    }
    
    
    
//TRASH
    cout << "Sz after reconstructing stream is " << sz_avail << "\n"; 

   if (amis00.main_memory_usage(&sz_avail,MM_STREAM_USAGE_CURRENT)
                                      ==  AMI_ERROR_NO_ERROR)
    cout << "Usage of amis0 is  " << sz_avail << "\n";

   if (amis1.main_memory_usage(&sz_avail,MM_STREAM_USAGE_CURRENT)
                                      ==  AMI_ERROR_NO_ERROR)
    cout << "Usage of amis1 is  " << sz_avail << "\n";
    
 
    AMI_STREAM<scantype> *inptr = &amis00;



    if (gettimeofday(&tp1,NULL) == -1) {perror("gettimeofday");}

    ae = srm_sort(inptr, &amis1, 0, dummylong, dummykey);

    if (gettimeofday(&tp2,NULL) == -1) {perror("gettimeofday");}
    

        cout << "Sorted them.\n";
        cout << "Sorted stream length = " << amis1.stream_len() << '\n';
        cout << "Time to sort is " << 
    ((double)(tp2.tv_sec-tp1.tv_sec)*1000000 + (double)(tp2.tv_usec-tp1.tv_usec))/1000 << " millisecs\n"; 
    
    cout << '\n';

    char *Name;

    /*

    amis1.name(&Name);
    amis1.persist(PERSIST_PERSISTENT);

    cout << "Destructing stream\n";
    delete &amis1;
    cout << "Constructing stream\n";

    */

    AMI_STREAM<scantype>*amis11 = &amis1;

	 //(Name, AMI_READ_WRITE_STREAM);

    cout << "Constructed stream\n";

    amis11->seek(0);
    cout << "About to verify\n";
    scantype  previous;
    scantype *nextint;

    previous.keyval = 0;

    unsigned long long  CNTR = 0;
  
    while (amis11->read_item(&nextint)!=AMI_ERROR_END_OF_STREAM)
          {
		 if (previous > *nextint) 
		   //if (nextint->keyval != CNTR) 
                                   { cout << "Sort errs\n";
                                    break;}
           if (CNTR == 0) cout << "First stream element = " << nextint->keyval << "\n";
           CNTR++;
           previous = *nextint;
           }

    cout << "Last  stream element = " << nextint->keyval << "\n";

    cout << "RAKESH:Sort verifier CNTR is " << CNTR << "\n";
    cout << "RAKESH:Stream is " << Name << "\n";

    delete amis11;

    //    delete &amis0;


    return 0;
}
    


// Instantiate all the templates we have used.

#ifdef NO_IMPLICIT_TEMPLATES

// Instantiate templates for streams of objects.
TEMPLATE_INSTANTIATE_STREAMS(scantype)

// Instantiate pi_streams
TEMPLATE_INSTANTIATE_PI_STREAMS(scantype)

// Instantiate templates for sorting objects.
TEMPLATE_INSTANTIATE_PI_SORT_OP(scantype, unsigned long long)

#endif

