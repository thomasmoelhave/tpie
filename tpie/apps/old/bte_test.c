
//Author: Rakesh Barve
//
//COMMENT: This program may need to be modified if we want to use
//         munmap() operations during MMAP_TEST.
//
//This file contains a program that can be used to determine the 
//streaming speeds attained by BTE_stdio, BTE_mmb, and BTE_ufs
//streams on a given system. The performance of these implementations
//can vary significantly from system to system; and the information 
//that can be attained  can be used to decide which implementation
//is appropriate for a given system. 

//The C program simulates the buffering and I/O mechanisms
//used by each one of the BTE stream implementations so that 
//the ``raw'' (only in the sense that there is no TPIE layer between
//the program and the filesystem) streaming speed of an 
//I/O-buffering mechanism combination.
//
//To use the program,  #define one of  MMAP_TEST, READ_WRITE_TEST or
//STDIO_TEST to be 1 depending on whether you want to test the streaming
//speed of  BTE_mmb, BTE_ufs or  BTE_stdio; #define the other two 
//flags to 0. Also #define the BLOCKSIZE_BASE parameter to be equal
//to the underlying operating system blocksize. 
//
//NOTE: The BTE_mmb stream being simulated here is one that has 
//an mmap() based prefetching; this configuration is typically expected
//to be the best possible mmap()-based BTE implementation. The ftruncate()
//necessary while appending data to an to a file is done once every
//twenty mmaps(). 
//
//Then compile the program using a C compiler. In order to test the streaming 
//performance of  BTE streams of any type T (of size ItemSize), the program 
//first writes out some specified number NumStreams of BTE streams containing 
//a specified number NumItems of items of size ItemSize. Then, it carries
//out a perfect NumStreams-way interleaving of the streams via a simple
//merge like process, writing the output to an output stream. During the 
//computation, each of the NumStreams streams input to the merge and 
//the stream being output by the merge uses either one (when READ_WRITE_TEST
//or STDIO_TEST are set to 1)  or two (when MMAP_TEST is set to 1) buffers.
//In case of STDIO_TEST, the buffers are not maintained in the program
//but by the stdio library. In the case of MMAP_TEST or READ_WRITE_TEST,
//each buffer is set to be of size BLOCK_FACTOR times BLOCKSIZE_BASE, and
//each I/O operation corresponds to a buffer-sized operation. 

//In order to test the streaming performance of a BTE stream with 
//items_in_block items of size ItemSize in each block (that is, the
//block corresponding to the theoretical I/O model), the command line 
//sequence is:
//
//bte_test NumItems ItemSize NumStreams BLOCK_FACTOR items_in_block DataFile 
//
//The output of the program (streaming speed) is appended to the file
//DataFile. The streaming speed, alternatively called I/O Bandwidth,
//is given in units of MB/s.


#include<stdio.h>
#include <stdlib.h>
#include <sys/time.h> 
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/*extern int munmap(caddr_t addr, int len);*/



#define MMAP_TEST 0
#define READ_WRITE_TEST 0
#define STDIO_TEST 1
#define BLOCKSIZE_BASE 4096 
#define DEC_ALPHA 1

#define MAP_OVERWRITE 0x1000   
#define MAX_STREAMS 100

main(int argc, char * argv[])
{

/* We assume that blocksize is a multiple of itemsize */

int ret_val;
int NumItems = atoi(argv[1]);
int ItemSize = atoi(argv[2]);
//int random_seed = atoi(argv[3]);
int NumStreams = atoi(argv[3]);
int blocksize = atoi(argv[4])*BLOCKSIZE_BASE;
int items_in_block = blocksize/ItemSize;
char *Datafilename = argv[5];

/*FILE *fileptr[MAX_STREAMS];*/
FILE **fileptr, *outfileptr, *datafile;

char *ptr_current;
int checklength;
int i,j, cur_stream;
int randint ;
int max = -1;
int CNTR = 0;
struct timeval starttimestruct,endtimestruct, previous_stamp, curr_stamp;
double mean_time=0;

/* char *buf_current[MAX_STREAMS], *buf_next[MAX_STREAMS], *curr_item[MAX_STREAMS], *temp_buf_ptr; */
char **buf_current, **buf_next, **curr_item, *temp_buf_ptr;
char  **try_current,  **try_next;
char *outbuf_current = NULL;
char *outbuf_next = NULL;
char *outcurr_item = NULL;
char *Item;
long starttime,endtime;
double dendtime, dum1,dum2,dum3;
char *envdir = getenv("TMP");

/* int fd[MAX_STREAMS];*/
int *fd, outfd;

/*int item_cnt[MAX_STREAMS];*/
int *item_cnt;
int outitem_cnt = 0;

/*int block_offset[MAX_STREAMS];*/
int *block_offset;
int outblock_offset = 0;
int temp_offset;

/*int f_filelen[MAX_STREAMS];*/
int *f_filelen;
int outf_filelen = 0;
int Max_Verify = -1;

char **Filename;
char outFilename[128];
char int_string[4];

if (NumStreams > MAX_STREAMS) {printf("Too many streams\n"); exit(1);}

printf("Note that if the number of streams being merged is too large,\n");
printf("there is a possibility that due to shortage of `really available'\n");
printf("physical memory, the program incurs VM paging overhead when in fact\n");
printf("it is not intended that this happens\n");

/*Space allocations for arrays; each element corresponds to a stream */
fileptr = (FILE **)malloc(NumStreams*sizeof(FILE *));
buf_current = (char **)malloc(NumStreams*sizeof(char *));
try_current =  (char **)malloc(NumStreams*sizeof(char *));
buf_next = (char **)malloc(NumStreams*sizeof(char *));
try_next =  (char **)malloc(NumStreams*sizeof(char *));
curr_item = (char **)malloc(NumStreams*sizeof(char *));
fd = (int *)malloc(NumStreams*sizeof(int));
item_cnt = (int *)malloc(NumStreams*sizeof(int));
block_offset = (int *)malloc(NumStreams*sizeof(int));
f_filelen = (int *)malloc(NumStreams*sizeof(int));
Filename = (char **)malloc(NumStreams*sizeof(char *));

strcpy(outFilename,envdir);
strcat(outFilename,"/ShuffleOut");


/* Open input streams */
for ( j=0; j < NumStreams; j++)
{
Filename[j] = malloc(128);
sprintf(int_string,"%d",j);
strcpy(Filename[j],envdir);
strcat(Filename[j],"/TESTFILE");
strcat(Filename[j],int_string);

#if  STDIO_TEST
if ((fileptr[j] = fopen(Filename[j], "a+")) == NULL)
   {perror("Fopen"); printf("Fname is %s\n",Filename[j]);exit(1);}
#endif

#if !STDIO_TEST
  if ((fd[j] = open(Filename[j],  O_RDWR | O_CREAT | O_EXCL,
                             S_IRUSR | S_IWUSR |
                             S_IRGRP | S_IWGRP |
                             S_IROTH | S_IWOTH)) == -1)  
     {perror("open failed"); exit(1);}
#endif
}

if ((Item = (char *)malloc(ItemSize))==NULL)
    {perror("Malloc failed");exit(1);}

/* Allocate space for write buffers and initialize other stuff*/
#if READ_WRITE_TEST
for (j = 0; j < NumStreams; j++) 
{
if ((buf_current[j] = (char *)malloc(blocksize)) == NULL)
   {perror("Malloc failed for buf_current");exit(1);}
curr_item[j] = buf_current[j];
f_filelen[j] = 0;
block_offset[j] = 0;
item_cnt[j] = 0;
}
#endif

/* Nullify buffer pointers and initiialize other stuff */
#if MMAP_TEST
for (j=0; j < NumStreams; j++)
{
   buf_current[j] = buf_next[j] = (char *)0;
   block_offset[j] = 0;
   item_cnt[j] = 0;
   f_filelen[j] = 0;
}
#endif


printf("Starting to write\n");

//srandom((unsigned)random_seed);

for (cur_stream = 0; cur_stream < NumStreams; cur_stream++)
{
for (i = 0; i < NumItems; i++) 
    {
    *( int *)Item = random();
    Max_Verify = ((Max_Verify < *( int *)Item)?(*( int *)Item):Max_Verify);

#if  STDIO_TEST
    if (fwrite(Item, ItemSize, 1, fileptr[cur_stream]) != 1)
       { perror("Fwrite"); printf("Iteration %d\n",i); exit(1); }
#endif

#if READ_WRITE_TEST
   *(int *)curr_item[cur_stream] = *( int *)Item;
   (char *)curr_item[cur_stream] = (char *) curr_item[cur_stream] + ItemSize;
   if (++item_cnt[cur_stream] == items_in_block)
      {item_cnt[cur_stream]=0;
       if (write(fd[cur_stream],(char *)buf_current[cur_stream],blocksize) != blocksize)
           { perror("write");printf("Iteration %d\n",i); exit(1); }
       curr_item[cur_stream] = buf_current[cur_stream]; 
      }
#endif

#if MMAP_TEST

    if (item_cnt[cur_stream]== 0) {

      if (f_filelen[cur_stream] < block_offset[cur_stream] + 2*blocksize ) {
          if  (ftruncate(fd[cur_stream], block_offset[cur_stream] +
                20*     // ftruncate space for 20 blocks at end of file.
                 blocksize ))
                            {perror("ftruncate"); exit(1);}
      f_filelen[cur_stream] = block_offset[cur_stream] +  20*blocksize;
    }


    if (block_offset[cur_stream] == 0) { 

#if !DEC_ALPHA
            buf_current[cur_stream] = 
                    (char *)(mmap((caddr_t)buf_current[cur_stream], blocksize,
                            PROT_READ | PROT_WRITE,
#ifdef SYSTYPE_BSD
                            // Note:  MAP_FIXED does not seem to work as
                            // advertized in the man pages in OSF 1.3 on
                            // a DEC alpha.  When mapping in from different
                            // offsets of the same file, the same address
                            // is used.
                            MAP_FILE | MAP_VARIABLE |
#endif  
                            MAP_SHARED, fd[cur_stream], 
                            block_offset[cur_stream]));

#else

     buf_current[cur_stream] = 
                    (char *)(mmap((caddr_t)buf_current[cur_stream], blocksize,
                            PROT_READ | PROT_WRITE,
                            MAP_FILE |
                            (buf_current[cur_stream] ?  MAP_FIXED : MAP_VARIABLE) |
                            MAP_OVERWRITE |
                            MAP_SHARED,
                            fd[cur_stream], block_offset[cur_stream])); 
#endif

    if (buf_current[cur_stream] == (char *) -1)
                            {perror("mmap buf_current error"); exit(1);}
    

                            }
    else                    {

     temp_buf_ptr = buf_current[cur_stream];
     buf_current[cur_stream] = buf_next[cur_stream];
     buf_next[cur_stream] = temp_buf_ptr;

                            }

#if !DEC_ALPHA
            buf_next[cur_stream] 
                    = (char *)(mmap((caddr_t)buf_next[cur_stream], blocksize,
                            PROT_READ | PROT_WRITE,
#ifdef SYSTYPE_BSD
                            // Note:  MAP_FIXED does not seem to work as
                            // advertized in the man pages in OSF 1.3 on
                            // a DEC alpha.  When mapping in from different
                            // offsets of the same file, the same address
                            // is used.
                            MAP_FILE | MAP_VARIABLE |
#endif
                            MAP_SHARED, 
                        fd[cur_stream], block_offset[cur_stream]+blocksize));
     
#else
    buf_next[cur_stream] = 
                       (char *)(mmap((caddr_t)buf_next[cur_stream], blocksize,
                            PROT_READ | PROT_WRITE,
                            MAP_FILE |
                            (buf_next[cur_stream] ?  MAP_FIXED : MAP_VARIABLE) |
                            MAP_OVERWRITE |
                            MAP_SHARED,
                        fd[cur_stream], block_offset[cur_stream]+blocksize));
#endif
   if (buf_next[cur_stream] == (char *) -1)
                            {perror("mmap buf_next error"); 
                             printf("Number of iterations is %d\n",i);
                             exit(1);}

    curr_item[cur_stream] = buf_current[cur_stream];
    block_offset[cur_stream]+=blocksize;
                        }
    *(int *)curr_item[cur_stream] = *( int *)Item;
    (char *)curr_item[cur_stream] = (char *) curr_item[cur_stream] + ItemSize;
    if (++item_cnt[cur_stream] == items_in_block) item_cnt[cur_stream]=0;
#endif
 
    }

#if READ_WRITE_TEST
if (item_cnt[cur_stream] != 0)
    { 
    if (write(fd[cur_stream],(char *)buf_current[cur_stream],item_cnt[cur_stream]) != item_cnt[cur_stream])
         { perror("write");printf("Stream %d\n",cur_stream); exit(1); }
    }
#endif

#if MMAP_TEST
   /*temp_offset = block_offset[cur_stream]-blocksize; 
   if (munmap((caddr_t)buf_current[cur_stream], blocksize)) {perror("Unmap error");exit(1);}
   if (munmap((caddr_t)buf_next[cur_stream],blocksize)) {perror("Unmap error");exit(1);}
   */
#endif   


#if !STDIO_TEST 
    if (close(fd[cur_stream])) {perror("Close file"); exit(1);} 
	       // Sometimes on the MMB kernel not doing this
               // after accessing a file in writeonly mode
               // leads to seg faults or something.
#else 
    if (fclose(fileptr[cur_stream])) {perror("FClose file"); exit(1);}
        
#endif

printf("Finished writing %d random items to stream %d\n",i, cur_stream);
}

printf("The actual max is %d\n",Max_Verify);


#if  STDIO_TEST
for (j=0; j < NumStreams; j++) 
 {
  if ((fileptr[j] = fopen(Filename[j], "a+")) == NULL) {perror("Fopen: Reopening"); exit(1);}
 }
if ((outfileptr = fopen(outFilename, "a+")) == NULL) {perror("Fopen: Outputfile"); exit(1);}
#endif
     
#if !STDIO_TEST
for (j=0; j < NumStreams; j++) 
 {
    if ((fd[j] = open(Filename[j], O_RDONLY)) == -1) {
    perror("Open:Reopening"); exit(1);        
    }

 }
if ((outfd = open(outFilename, O_RDWR | O_CREAT | O_EXCL,
                             S_IRUSR | S_IWUSR |
                             S_IRGRP | S_IWGRP |
                             S_IROTH | S_IWOTH)) == -1)
     {perror("Open: Reopening"); exit(1);}  
#endif

for (j = 0; j < NumStreams; j++)
{   
 item_cnt[j] = 0;
 block_offset[j] = 0;
#if STDIO_TEST
 if (fseek(fileptr[j], 0, SEEK_SET)!=0) { perror("Fseek"); exit(1); }
#endif

#if !STDIO_TEST
 if (lseek(fd[j],0,SEEK_SET) != 0) {perror("Lseek");exit(1);  }
#endif

#if MMAP_TEST
   buf_current[j] = buf_next[j] = NULL;
#endif
}

/* item_cnt and block_offset for output file have already been initialized;
   even the buffer pointers have been set to be NULL. In case of READ_WRITE_TEST
   however, we need to malloc space for a write buffer for the output stream 
*/

#if READ_WRITE_TEST
outbuf_current = (char *)malloc(blocksize);
outcurr_item = outbuf_current;
#endif


if (gettimeofday(&starttimestruct,NULL) == -1) {perror("gettimeofday");}    
printf("Starting perfect shuffle\n");
printf("That is, we perfectly shuffle the %d items of each one of the %d streams\n",NumItems,NumStreams);
printf("and write them into an output stream. In the process we also compute\n");
printf("the maximum of all the items of all streams. The performance of this computation is\n");
printf("is expected to mimic the *best* possible performance that a %d-way external merge\n",NumStreams);
printf("can have.\n"); 
printf("If %d is 1, this computation mimics the time to scan thru all stream elements,\n",NumStreams);
printf(" and write them out into a different stream.\n"); 

cur_stream = 0;

for (i = 0; i < NumItems*NumStreams; i++)
    {

/* First we read the item from appropriate input stream */
#if  STDIO_TEST
    if ((ret_val = fread(Item, ItemSize, 1, fileptr[cur_stream])) != 1)
       { perror("Fread"); printf("Iteration %d: fread ret val is %d\n",i, ret_val); exit(1); }
#endif

#if READ_WRITE_TEST
    if (item_cnt[cur_stream]==0) {
       if ((ret_val = read(fd[cur_stream],(char *)buf_current[cur_stream],blocksize)) != blocksize)
           { if (i/NumStreams + i % NumStreams  + ret_val/items_in_block >  NumItems)  
             { perror("read");printf("Iteration %d\n",i); exit(1); }
           }
       curr_item[cur_stream] = buf_current[cur_stream];
       }
    if (++item_cnt[cur_stream] == items_in_block) item_cnt[cur_stream]=0;
   *( int *)Item= *(int *)curr_item[cur_stream];
   (char *)curr_item[cur_stream] = (char *) curr_item[cur_stream] + ItemSize;
#endif

#if MMAP_TEST
    if (item_cnt[cur_stream]== 0) {

    if (block_offset[cur_stream] == 0) {

#if !DEC_ALPHA
            buf_current[cur_stream] = (char *)(mmap((caddr_t)buf_current[cur_stream], blocksize,
                            PROT_READ,
#ifdef SYSTYPE_BSD
                            // Note:  MAP_FIXED does not seem to work as
                            // advertized in the man pages in OSF 1.3 on
                            // a DEC alpha.  When mapping in from different
                            // offsets of the same file, the same address
                            // is used.
                            MAP_FILE | MAP_VARIABLE |
#endif
                            MAP_SHARED, fd[cur_stream], block_offset[cur_stream]));
    
#else

     buf_current[cur_stream] = (char *)(mmap((caddr_t)buf_current[cur_stream], blocksize,
                            PROT_READ,
                            MAP_FILE |
                           (buf_current[cur_stream] ?  MAP_FIXED : MAP_VARIABLE) |
                            MAP_SHARED,
                            fd[cur_stream], block_offset[cur_stream]));

#endif
    /*
    printf("buf_current[%d] is %d\n",cur_stream,buf_current[cur_stream]);
    printf("buf_current[%d] is %d\n",1-cur_stream,buf_current[1-cur_stream]);
    */


    
     if (buf_current[cur_stream] == (char *) -1)
                            {perror("mmap buf_current error"); exit(1);}
                            }
    else                    {
     temp_buf_ptr = buf_current[cur_stream];
     buf_current[cur_stream] = buf_next[cur_stream];
     buf_next[cur_stream] = temp_buf_ptr;
                            } 

#if !DEC_ALPHA
            buf_next[cur_stream] = (char *)(mmap((caddr_t)buf_next[cur_stream], blocksize,      
                            PROT_READ,
#ifdef SYSTYPE_BSD
                            // Note:  MAP_FIXED does not seem to work as
                            // advertized in the man pages in OSF 1.3 on
                            // a DEC alpha.  When mapping in from different
                            // offsets of the same file, the same address
                            // is used.
                            MAP_FILE | MAP_VARIABLE |
#endif
                            MAP_SHARED, 
                            fd[cur_stream], block_offset[cur_stream]+blocksize));
     
#else

    buf_next[cur_stream] = (char *)(mmap((caddr_t)buf_next[cur_stream], blocksize,
                            PROT_READ,
                            MAP_FILE |
                            (buf_next[cur_stream] ?  MAP_FIXED : MAP_VARIABLE) |
                            MAP_SHARED,  
                            fd[cur_stream], block_offset[cur_stream]+blocksize));
#endif
    if (buf_next[cur_stream] == (char *) -1)
                            {perror("mmap buf_next error"); exit(1);}

                            
    curr_item[cur_stream] = buf_current[cur_stream];
    block_offset[cur_stream]+=blocksize;
                        }    
    *(int *)Item = *( int *)curr_item[cur_stream]; 
    (char *)curr_item[cur_stream] = (char *) curr_item[cur_stream] + ItemSize; 
    if (++item_cnt[cur_stream] == items_in_block) item_cnt[cur_stream]=0;
#endif 

    if (i == 0) max =  *(int *)Item;
    else max = ((max < *(int *)Item)?(*(int *)Item):max);

    /*printf("Read item %d is %d\n",i,*(int *)Item);*/

/* Now we write into output stream: Basically its the same code as 
   the previous code used while generating random nos for the stream
   except that this is for a single stream */

#if  STDIO_TEST
    if (fwrite(Item, ItemSize, 1, outfileptr) != 1)
       { perror("Fwrite"); printf("Iteration %d\n",i); exit(1); }
#endif

#if READ_WRITE_TEST
   *(int *)outcurr_item = *( int *)Item;
   (char *)outcurr_item = (char *) outcurr_item + ItemSize;
   if (++outitem_cnt == items_in_block)
      {outitem_cnt=0;
       if (write(outfd,(char *)outbuf_current,blocksize) != blocksize)
           { perror("write");printf("Iteration %d\n",i); exit(1); }
       outcurr_item = outbuf_current;
      }
#endif

#if MMAP_TEST

    if (outitem_cnt== 0) {

      if (outf_filelen < outblock_offset + 2*blocksize ) {
          if  (ftruncate(outfd, outblock_offset +
                20*     // ftruncate space for 20 blocks at end of file.
                 blocksize ))
                            {perror("ftruncate"); exit(1);}
      outf_filelen = outblock_offset +  20*blocksize;
    }


    if (outblock_offset == 0) {

#if !DEC_ALPHA
            outbuf_current = (char *)(mmap((caddr_t)outbuf_current, blocksize,
                            PROT_READ | PROT_WRITE,
#ifdef SYSTYPE_BSD
                            // Note:  MAP_FIXED does not seem to work as
                            // advertized in the man pages in OSF 1.3 on
                            // a DEC alpha.  When mapping in from different
                            // offsets of the same file, the same address
                            // is used.
                            MAP_FILE | MAP_VARIABLE |
#endif
                            MAP_SHARED, outfd, outblock_offset));

#else

     outbuf_current = (char *)(mmap((caddr_t)outbuf_current, blocksize,
                            PROT_READ | PROT_WRITE,
                            MAP_FILE |
                            (outbuf_current ?  MAP_FIXED : MAP_VARIABLE) |
                            MAP_OVERWRITE |
                            MAP_SHARED,
                            outfd, outblock_offset));
#endif
    
    if (outbuf_current == (char *) -1)
                            {perror("mmap buf_current error"); exit(1);}

                            }
    else                    {

     temp_buf_ptr = outbuf_current;
     outbuf_current = outbuf_next;
     outbuf_next = temp_buf_ptr;

                            }

#if !DEC_ALPHA
            outbuf_next = (char *)(mmap((caddr_t)outbuf_next, blocksize,
                            PROT_READ | PROT_WRITE,
#ifdef SYSTYPE_BSD
                            // Note:  MAP_FIXED does not seem to work as
                            // advertized in the man pages in OSF 1.3 on
                            // a DEC alpha.  When mapping in from different
                            // offsets of the same file, the same address
                            // is used.
                            MAP_FILE | MAP_VARIABLE |
#endif
                            MAP_SHARED, outfd, outblock_offset+blocksize));

#else
    outbuf_next = (char *)(mmap((caddr_t)outbuf_next, blocksize,
                            PROT_READ | PROT_WRITE,
                            MAP_FILE |
                            (outbuf_next ?  MAP_FIXED : MAP_VARIABLE) |
                            MAP_OVERWRITE |
                            MAP_SHARED,
                            outfd, outblock_offset+blocksize));
#endif
   if (outbuf_next == (char *) -1)
                            {perror("mmap buf_next error");
                             printf("Number of iterations is %d\n",i);
                             exit(1);}

    outcurr_item = outbuf_current;
    outblock_offset+=blocksize;
    }    
    *(int *)outcurr_item = *( int *)Item;
    (char *)outcurr_item = (char *) outcurr_item + ItemSize;
    if (++outitem_cnt == items_in_block) outitem_cnt=0;
#endif

    if (++cur_stream == NumStreams) cur_stream = 0;
    }

#if READ_WRITE_TEST
if (outitem_cnt != 0)
    {
    if (write(outfd,(char *)outbuf_current,outitem_cnt) != outitem_cnt)
         { perror("write");printf("Iteration %d\n",i); exit(1); }
    }
#endif

    
if (gettimeofday(&endtimestruct,NULL) == -1) {perror("gettimeofday");}    

    starttime = starttimestruct.tv_sec*1000000 + starttimestruct.tv_usec;
    endtime = endtimestruct.tv_sec*1000000 +  endtimestruct.tv_usec;
    dendtime = endtime;    
 
    printf("Max is %d\n", max);


    dum1 = 2*ItemSize*NumStreams*NumItems;
    dum2 = endtime-starttime;
    dum3 = 1024*1024/1000000;
    printf("Net time taken is %ld microsecs \n", endtime-starttime);
    printf("Net BW in MB/s is %f \n", dum1/(dum3*dum2));

/*     ((2*ItemSize*NumStreams*NumItems)*1000000)/((1024*1024)*(dendtime-starttime)));*/

    if ((datafile = fopen(Datafilename, "a+")) == NULL)
   {perror("Fopen"); printf("Fname is %s\n",Filename[j]);exit(1);}
#if READ_WRITE_TEST
    fprintf(datafile,"READ_WRITE_TEST\n");
#endif
#if MMAP_TEST
    fprintf(datafile,"MMAP_TEST\n");
#endif
#if STDIO_TEST
    fprintf(datafile,"STDIO_TEST\n");
#endif
    fprintf(datafile,"B=%d, Itemsize=%d, NumStreams=%d, StreamLength=%d\n",
                     blocksize, ItemSize, NumStreams, NumItems);
    fprintf(datafile,"Time is %f secs, IO BW is %f MB/s\n",
           dum2/1000000, dum1/(dum3*dum2)
     );
    fprintf(datafile,"***********************************\n");
    if (fclose(datafile)) {perror("Fclose data file"); exit(1);}    
}

     
















