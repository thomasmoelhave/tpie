//
// File: bte_stream_ufs.h (formerly bte_ufs.h)
// Author: Rakesh Barve <rbarve@cs.duke.edu>
//
// $Id: bte_stream_ufs.h,v 1.1 2002-01-06 18:47:09 tavi Exp $
//
// BTE streams with blocks IOed using read()/write().  This particular
// implementation explicitly manages blocks, and only ever maps in one
// block at a time.  This relies on the filesystem to do lookahead. It
// is assumed for the purpose of memory calculations that for each
// block used by TPIE, the filesystem uses up another block of the
// same size.
//
// Completely different from the old bte_ufs.h since this does
// blocking like bte_mmb, only it uses read()/write() to do so.
//
//

#ifndef _BTE_STREAM_UFS_H
#define _BTE_STREAM_UFS_H

//  BTE_IMPLICIT_FS_READAHEAD means we account for buffers potentially
//  used due to file system read-ahead assuming space for one block is
//  used by filesystem for each TPIE block even though we do not
//  actually do prefetching; by default this is set, and should be
//  eventually set by user in app_config.h; in the current version,
//  since there is no BCC defined, it is set here;
#define BTE_IMPLICIT_FS_READAHEAD 1

//the code for double buffering is not here..
#define UFS_DOUBLE_BUFFER 0

// Either double buffer explicitly using aio or aio can be used Darren
// style or more directly. Using it directly will probably be better,
// but right now that is not supported. (Solaris and Digital/FreeBSD
// use different aio interfaces.
#if BTE_UFS_READ_AHEAD	

#if !USE_LIBAIO && !UFS_DOUBLE_BUFFER
#error BTE_UFS_READ_AHEAD requested, but no double buff mechanism in config.
#endif
//#include <sys/asynch.h>                               

#define BTE_UFS_MM_BUFFERS 2
#endif

#if UFS_DOUBLE_BUFFER
#error At present explicit DOUBLE BUFFER not supported.
#endif

// The double buffering mechanism will use lib_aio on Solaris and the
// asynch.h interface on Digital Unix and FreeBSD.  Gut feeling is
// that if file access is maintained sequential performance with both
// UFS_DOUBLE_BUFFER and USE_LIBAIO set off is best.

#if USE_LIBAIO
#if !HAVE_LIBAIO
#error USE_LIBAIO requested, but aio library not in configuration.
#endif
#if UFS_DOUBLE_BUFFER
#error Darren-style USE_LIBAIO requested, but so is DOUBLE BUFFER
#endif
#endif

#if BTE_IMPLICIT_FS_READAHEAD
#if BTE_UFS_READ_AHEAD
#error both EXPLICIT and IMPLICIT READAHEAD set.
#endif
#define BTE_UFS_MM_BUFFERS 2
#endif

// This is if we want to cheat: That is we don't count at all the amount
// of memory used by the filesystem to buffer our data.
#if (!BTE_IMPLICIT_FS_READAHEAD) && (!BTE_UFS_READ_AHEAD)
#define BTE_UFS_MM_BUFFERS 1
#endif

// Include the registration based memory manager.
#define MM_IMP_REGISTER
#include <mm.h>

#include <bte_stream_base.h>

// We need a variety of OS constants for the mmap() implementation.
extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
}
#if !HAVE_PROTOTYPE_FTRUNCATE
extern "C" int ftruncate (int fd, off_t length);
#endif

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

// This code makes assertions and logs errors.
#include <tpie_assert.h>
#include <tpie_log.h>
#include <assert.h>

// Stats code.
#include <bte_mmb_stats.h>

#ifndef BTE_UFS_LOGICAL_BLOCKSIZE_FACTOR
#define BTE_UFS_LOGICAL_BLOCKSIZE_FACTOR 1
#endif

#define UFS_HEADER_MAGIC_NUMBER	0xFEDCBA

// A base class for all BTE_single_disk<T> classes.
class BTE_single_disk_base {
 protected:
   static bool f_stats;
   static bte_mmb_stats stats;
   static int remaining_streams;
 public:
   // Gathering and providing statisitics.<T> classes. implementation.ount
       static void reset_stats (void);
   static void stats_on ();
   static void stats_off ();
   static const bte_stats & statistics (void);
};

//
// A header structure that will appear in the first block of a BTE stream.
// This structure will genarally not occupy the entire first block, but 
// to preserve block boundaries, nothing else will be added to the block.
//
// If more information is needed in the header, it should be added to a
// newer version of this structure.
//
class ufs_stream_header {
 public:
   unsigned int magic_number;	// Set to UFS_HEADER_MAGIC_NUMBER
   unsigned int version;	// Should be 1 for current version.
   unsigned int length;		// # of bytes in this structure.
   off_t item_logical_eof;
   // The number of items in the stream.
   size_t item_size;		// The size of each item in the stream.
   size_t block_size;		// The size of a physical block on the device
   // where this stream resides.
   unsigned int items_per_block;
};

#define BTE_UFS_PATH_NAME_LEN 128

//
// BTE_single_disk<T>
//
// This is a class template for the implementation of a 
// BTE stream of objects of type T such that the entire stream 
// resides on a single disk.  This version maps in only one
// block of the file at a time. The striped_stream class, such
// that it is comprised of several single disk streams, has  
// a member function that is a friend of this class.
//

template < class T > class BTE_single_disk:
public BTE_stream_base < T >, public BTE_single_disk_base {
 private:

   int fd;			// descriptor of the mapped file.

   size_t os_blocksize;

   size_t logical_blocksize;

   int itemsize_div_blocksize;

   unsigned int substream_level;	// How deeply is this stream nested.

   off_t f_offset;		// Offset of the current item in the file.  This

   // is the logical offset of the item within the
   // file, that is, the place we would have to lseek()
   // to in order to read() or write() the item if
   // we were using ordinary (i.e. non-mmap()) file
   // access methods.

   off_t f_eos;			// Offset just past the end of the last item in the

   // stream.  If this is a substream, we can't write
   // here or anywhere beyond.

   off_t f_bos;			// Beginning of the file.  Can't write before here.

   off_t f_filelen;

   ufs_stream_header *header;	// A pointer to the mapped in 

   // header block for the stream.

   T *current;			// The current item (mapped in)

   T *curr_block;		// A pointer to the beginning of the currently mapped

   // block.

   int block_valid;		// Non-zero if current points to a valid, mapped in

   // block.

   int block_dirty;		// If block_valid is one, then block_dirty is 1 if and 

   // only if mapped block is
   // dirty; obviously block_dirty is always 0 for r_only streams.

   off_t curr_block_file_offset;	// When block_valid is one, this is the Offset of

   // curr_block in the underlying Unix file.

   unsigned int blocksize_items;

#if UFS_DOUBLE_BUFFER
   // for use in double buffering, when one is implemented using
   // the aio interface.

   T *next_block;		// ptr to next block 

   off_t f_next_block;		// position of next block

   int have_next_block;		// is next block mapped?

#endif	/* UFS_DOUBLE_BUFFER */

   int r_only;			// Non-zero if this stream was opened for reading 

   // only. 

   int os_errno;		// A place to cache OS error values.  It is normally

   // set after each call to the OS.

   char path[BTE_UFS_PATH_NAME_LEN];

#if USE_LIBAIO

   // A buffer to read the first word of each OS block in the next logical
   // block for read ahead.
   int read_ahead_buffer[BTE_UFS_LOGICAL_BLOCKSIZE_FACTOR];

   // Results of asyncronous I/O.
   aio_result_t aio_results[BTE_UFS_LOGICAL_BLOCKSIZE_FACTOR];

#endif	/* USE_LIBAIO */

#if BTE_UFS_READ_AHEAD

   // Read ahead into the next logical block.
   void read_ahead (void);

#endif

   ufs_stream_header *map_header (void);

   inline BTE_err validate_current (void);

   inline BTE_err invalidate_current (void);

   BTE_err map_current (void);
   BTE_err unmap_current (void);

   inline BTE_err advance_current (void);

   inline BTE_err register_memory_allocation (size_t sz);
   inline BTE_err register_memory_deallocation (size_t sz);

   inline off_t item_off_to_file_off (off_t item_off);
   inline off_t file_off_to_item_off (off_t item_off);

   persistence per;		// The persistence status of this stream.

 public:
   // Constructors
   BTE_single_disk (const char *dev_path, BTE_stream_type st);

   BTE_single_disk (BTE_stream_type st);
   BTE_single_disk (BTE_single_disk < T > &s);

   // A substream constructor.
   BTE_single_disk (BTE_single_disk * super_stream,
		    BTE_stream_type st, off_t sub_begin, off_t sub_end);

   // A psuedo-constructor for substreams.
   BTE_err new_substream (BTE_stream_type st, off_t sub_begin,
			  off_t sub_end,

			  BTE_stream_base < T > **sub_stream);

   // Query memory usage
   BTE_err main_memory_usage (size_t * usage, MM_stream_usage usage_type);

   // Return the number of items in the stream.
   off_t stream_len (void);

   // Return the path name in newly allocated space.
   BTE_err name (char **stream_name);

   // Move to a specific position in the stream.
   BTE_err seek (off_t offset);

   // Truncate the stream.
   BTE_err truncate (off_t offset);

   // Destructor
   ~BTE_single_disk (void);

   B_INLINE BTE_err read_item (T ** elt);
   B_INLINE BTE_err write_item (const T & elt);

   int read_only (void) {
      return r_only;
   };

   int available_streams (void);

   off_t chunk_size (void);

   // Tell the stream whether to leave its data on the disk or not
   // when it is destructed.
   void persist (persistence);

};

//
// This constructor creates a stream whose contents are taken from the
// file whose path is given.
//

template < class T >
    BTE_single_disk < T >::BTE_single_disk (const char *dev_path,
					    const BTE_stream_type st)
{

   status_ = BTE_STREAM_STATUS_NO_STATUS;

   if (f_stats) {
      stats.record_create ();
   }
   // Reduce the number of streams avaialble.
   if (remaining_streams == 0) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL_ID ("BTE internal error: cannot open more streams.");
      return;
   } else
      remaining_streams--;

   // Cache the path name
   if (strlen (dev_path) > BTE_UFS_PATH_NAME_LEN - 1) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL_ID ("Path name \"" << dev_path << "\" too long.");
      return;
   }

   strncpy (path, dev_path, BTE_UFS_PATH_NAME_LEN);

   // Find out the block size from the file system and set the
   // logical block size.
#ifdef _SC_PAGE_SIZE
   os_blocksize = sysconf (_SC_PAGE_SIZE);
#else
   os_blocksize = getpagesize ();
#endif

   logical_blocksize = os_blocksize * BTE_UFS_LOGICAL_BLOCKSIZE_FACTOR;

   blocksize_items = logical_blocksize / sizeof (T);

   itemsize_div_blocksize = (logical_blocksize % sizeof (T) == 0);

   // We can't handle streams of large objects.
   if (sizeof (T) > logical_blocksize) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL_ID ("Object size is larger than os block size.");
      return;
   }
   // This is a top level stream
   substream_level = 0;

   per = PERSIST_DELETE;

   switch (st) {
   case BTE_READ_STREAM:

      r_only = 1;

      // Open the file for reading.
      if ((fd =::open (path, O_RDONLY)) == -1) {
	 status_ = BTE_STREAM_STATUS_INVALID;
	 os_errno = errno;
	 LOG_FATAL_ID ("open() failed to open " << path);
	 LOG_FATAL_ID (strerror (os_errno));
	 assert (0);
	 return;
      }

      header = map_header ();

      // Do some error checking on the header, such as to make sure that
      // it is a stream of objects of the right size and that it has
      // the correct header version.
      if ((header->magic_number != (unsigned int) UFS_HEADER_MAGIC_NUMBER)
	  || (header->item_size != sizeof (T))
	  || (header->block_size != os_blocksize)
	  || (header->items_per_block != blocksize_items)) {
	 LOG_FATAL ("Invalid header for " << path);
	 return;
      }
      // Set the eos marker appropriately.
      f_eos = item_off_to_file_off (header->item_logical_eof);

      if (header->item_logical_eof >= 1) {
	 if (f_eos - item_off_to_file_off (header->item_logical_eof - 1) -
	     sizeof (T) > 0) {
	    //Meaning, 1. sizeof (T) does not divide the 
	    //logical blocksize. 2. the last item in the stream is
	    //the last item that could have been placed
	    //on its logical block (so that the valid file
	    //offset as far as TPIE goes, is the beginning
	    //of a new block and so strictly greater than
	    //the byte offset at which the last item ends.)
	    //In this situation, after reading the last item
	    //and f_offset gets incremented, it is strictly
	    //less than f_eos; as a result the check 
	    //(f_eos <= f_offset)? in ::read_item() gets
	    //beaten when it shouldn't.
	    //To remedy, we simply reset f_eos in this circumstance
	    //to be just past the last item's byte offset.

	    f_eos =
		item_off_to_file_off (header->item_logical_eof - 1) +
		sizeof (T);
	 }
      }
      // Get ready to read the first item out of the file.
      f_offset = f_bos = os_blocksize;
      curr_block = current = NULL;
      block_valid = 0;

      //dirty bit and a field to remember the file offset of mapped in block.
      block_dirty = 0;
      curr_block_file_offset = 0;
      break;

   case BTE_WRITE_STREAM:
   case BTE_WRITEONLY_STREAM:
   case BTE_APPEND_STREAM:

      r_only = 0;

      // Open the file for writing.  First we will try to open 
      // is with the O_EXCL flag set.  This will fail if the file
      // already exists.  If this is the case, we will call open()
      // again without it and read in the header block.
      if ((fd =::open (path, O_RDWR | O_CREAT | O_EXCL,
		       S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
		       S_IROTH | S_IWOTH)) == -1) {

	 // Try again, hoping the file already exists.
	 if ((fd =::open (path, O_RDWR)) == -1) {

	    status_ = BTE_STREAM_STATUS_INVALID;
	    os_errno = errno;
	    LOG_FATAL_ID ("open() failed to open " << path);
	    LOG_FATAL_ID (strerror (os_errno));
	    return;
	 }
	 // Map in the header.
	 header = map_header ();

	 // Do some error checking. Set the current location. 
	 // If we are appending, set it to the end.
	 if (
	     (header->magic_number !=
	      (unsigned int) UFS_HEADER_MAGIC_NUMBER)
	     || (header->item_size != sizeof (T))
	     || (header->block_size != os_blocksize)
	     || (header->items_per_block != blocksize_items)) {
	    LOG_FATAL_ID ("Invalid header for " << path);
	    return;
	 }

	 f_offset = f_bos = os_blocksize;	// Right after the header.
	 current = curr_block = NULL;
	 block_valid = 0;

	 block_dirty = 0;
	 curr_block_file_offset = 0;

	 f_eos = item_off_to_file_off (header->item_logical_eof);

	 if (header->item_logical_eof >= 1) {
	    if (f_eos -
		item_off_to_file_off (header->item_logical_eof - 1) -
		sizeof (T) > 0) {
	       //Meaning, 1. sizeof (T) does not divide the 
	       //logical blocksize. 2. the last item in the stream is
	       //the last item that could have been placed
	       //on its logical block (so that the valid file
	       //offset as far as TPIE goes, is the beginning
	       //of a new block and so strictly greater than
	       //the byte offset at which the last item ends.)
	       //In this situation, after reading the last item
	       //and f_offset gets incremented, it is strictly
	       //less than f_eos; as a result the check 
	       //(f_eos <= f_offset)? in ::read_item() gets
	       //beaten when it shouldn't.
	       //To remedy, we simply reset f_eos in this circumstance
	       //to be just past the last item's byte offset.

	       f_eos =
		   item_off_to_file_off (header->item_logical_eof - 1) +
		   sizeof (T);
	    }
	 }

	 if (st == BTE_APPEND_STREAM) {
	    f_offset = f_eos;
	 }

      } else {			// The file was just created.

	 // Create and map in the header.      
	 // File does not exist, so first establish
	 // a mapping and then write into the file via the mapping.
	 header = map_header ();

	 // The file ptr corresp to stream is set pointing to one 
	 // location immediately following os_blocksize.
	 header->magic_number = UFS_HEADER_MAGIC_NUMBER;
	 header->version = 1;
	 header->length = sizeof (*header);
	 header->item_logical_eof = 0;
	 header->item_size = sizeof (T);
	 header->block_size = os_blocksize;
	 header->items_per_block = logical_blocksize / sizeof (T);

	 // Set out current location to be in the first block,
	 // ready to write.  We won't actually map in the first
	 // block of data until the writing begins.  
	 f_offset = f_bos = f_eos = os_blocksize;
	 current = curr_block = NULL;
	 block_valid = 0;

	 block_dirty = 0;
	 curr_block_file_offset = 0;
      }

      break;
   }				// end of switch

#if UFS_DOUBLE_BUFFER

   next_block = NULL;
   f_next_block = 0;
   have_next_block = 0;

#endif

   // Memory-usage for the header and 
   // the stream buffers are registered automatically by Darren's modified new() function.

   f_filelen = lseek (fd, 0, SEEK_END);

#if  BTE_IMPLICIT_FS_READAHEAD
   //register_memory_allocation(logical_blocksize);
#endif

   register_memory_allocation (sizeof (BTE_single_disk < T >));
};

// A constructor to create a temporary stream.
template < class T >
    BTE_single_disk < T >::BTE_single_disk (BTE_stream_type st)
{
   tp_assert (0, "This constructor is under construction");

   status_ = BTE_STREAM_STATUS_INVALID;
   return;

   // This function has not yet been implemented.

   // Generate a unique name for the file to be created.

   // Create the file with the appropriate type.
};

// A substream constructor.
// sub_begin is the item offset of the first item in the stream.
// sub_end is the item offset that of the last item in the stream.
// Thus, f_eos in the new substream will be set to point one item beyond
// this.
//
// For example, if a stream contains [A,B,C,D,...] then substream(1,3)
// will contain [B,C,D].
//
template < class T >
    BTE_single_disk < T >::BTE_single_disk (BTE_single_disk * super_stream,
					    BTE_stream_type st,
					    off_t sub_begin, off_t sub_end)
{
   status_ = BTE_STREAM_STATUS_NO_STATUS;

   if (f_stats) {
      stats.record_create ();
   }
   // Reduce the number of streams avaialble.
   if (remaining_streams == 0) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL_ID ("BTE internal error: cannot open more streams.");
      return;
   } else
      remaining_streams--;

   if (super_stream->status_ == BTE_STREAM_STATUS_INVALID) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL_ID ("BTE internal error: super stream is invalid.");
      return;
   }

   if (super_stream->r_only && (st != BTE_READ_STREAM)) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL_ID
	  ("BTE internal error: super stream is read only and substream is not.");
      return;
   }
   // If you are going to access a substream of
   // a previously created (super)stream we want to make sure that the 
   // superstream 's currently valid block, if any, is committed to
   // the underlying Unix file. Note that with memory mapped implementation
   // such a "committing" is automatic but in our case we need to keep
   // track of such things.

   if (!super_stream->r_only && super_stream->block_valid) {

      super_stream->unmap_current ();

      if (super_stream->status_ == BTE_STREAM_STATUS_INVALID) {
	 status_ = BTE_STREAM_STATUS_INVALID;
	 LOG_FATAL_ID ("BTE internal error: super stream is invalid.");
	 return;
      }
   }
   // Copy the relevant fields from the super_stream.
   fd = super_stream->fd;
   os_blocksize = super_stream->os_blocksize;
   logical_blocksize = super_stream->logical_blocksize;
   blocksize_items = super_stream->blocksize_items;
   itemsize_div_blocksize = super_stream->itemsize_div_blocksize;
   header = super_stream->header;
   substream_level = super_stream->substream_level + 1;

   per = PERSIST_PERSISTENT;

   // The arguments sub_start and sub_end are logical item positions
   // within the stream.  We need to convert them to offsets within
   // the stream where items are found.

   off_t super_item_begin = file_off_to_item_off (super_stream->f_bos);

   f_bos = item_off_to_file_off (super_item_begin + sub_begin);
   f_eos = item_off_to_file_off (super_item_begin + sub_end + 1);

   assert (f_bos <= f_eos);	// sanity check

   if (super_item_begin + sub_end + 1 >= 1) {
      if (f_eos - item_off_to_file_off (super_item_begin + sub_end) -
	  sizeof (T) > 0) {
	 //Meaning, 1. sizeof (T) does not divide the
	 //logical blocksize. 2. the last item in the stream is
	 //the last item that could have been placed
	 //on its logical block (so that the valid file
	 //offset as far as TPIE goes, is the beginning
	 //of a new block and so strictly greater than
	 //the byte offset at which the last item ends.)
	 //In this situation, after reading the last item
	 //and f_offset gets incremented, it is strictly
	 //less than f_eos; as a result the check
	 //(f_eos <= f_offset)? in ::read_item() gets
	 //beaten when it shouldn't.
	 //To remedy, we simply reset f_eos in this circumstance
	 //to be just past the last item's byte offset.
	 f_eos =
	     item_off_to_file_off (super_item_begin + sub_end) +
	     sizeof (T);
      }
   }

   assert (f_bos <= f_eos);	// sanity check

   f_filelen = super_stream->f_filelen;

   if (f_eos > super_stream->f_eos) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL_ID
	  ("BTE internal error: reached beyond super stream eof.");
      return;
   }

   f_offset = f_bos;
   current = curr_block = NULL;
   block_valid = 0;
   block_dirty = 0;
   curr_block_file_offset = 0;

#if UFS_DOUBLE_BUFFER

   next_block = NULL;
   f_next_block = 0;
   have_next_block = 0;

#endif

   r_only = super_stream->r_only;

   strncpy (path, super_stream->path, BTE_UFS_PATH_NAME_LEN);

#if  BTE_IMPLICIT_FS_READAHEAD

   //    register_memory_allocation(logical_blocksize);

#endif

   // Register memory_usage for the object corresp to the substream.
   register_memory_allocation (sizeof (BTE_single_disk < T >));

}

// A psuedo-constructor for substreams.  This serves as a wrapper for
// the constructor above in order to get around the fact that one
// cannot have virtual constructors.
template < class T >
    BTE_err BTE_single_disk < T >::new_substream (BTE_stream_type st,
						  off_t sub_begin,
						  off_t sub_end,
						  BTE_stream_base < T >
						  **sub_stream)
{
   // Check permissions.
   if ((st != BTE_READ_STREAM) && ((st != BTE_WRITE_STREAM) || r_only)) {
      *sub_stream = NULL;
      return BTE_ERROR_PERMISSION_DENIED;
   }

   tp_assert (((st == BTE_READ_STREAM) && r_only) ||
	      (st == BTE_READ_STREAM),
	      "Bad things got through the permisssion checks.");

   BTE_single_disk < T > *sub =
       new BTE_single_disk < T > (this, st, sub_begin, sub_end);

   *sub_stream = (BTE_stream_base < T > *)sub;

   return BTE_ERROR_NO_ERROR;
}

template < class T > BTE_single_disk < T >::~BTE_single_disk (void)
{

   if (f_stats) {
      stats.record_delete ();
   }
   // If the stream is already invalid for some reason, then don't
   // worry about anything.
   if (status_ == BTE_STREAM_STATUS_INVALID) {
      LOG_FATAL_ID ("BTE internal error: invalid stream in destructor.");
      return;
   }
   // Increase the number of streams avaialble.
   if (remaining_streams >= 0) {
      remaining_streams++;
   }
   // If this is writable and not a substream, then put the logical
   // eos back into the header before unmapping it.
   if (!r_only && !substream_level) {
      header->item_logical_eof = file_off_to_item_off (f_eos);
   }
   // Unmap the current block if necessary.
   if (block_valid) {
      unmap_current ();
   }
   // If this is not a substream then close the file.
   if (!substream_level) {
      if (lseek (fd, 0, SEEK_SET) != 0) {
	 status_ = BTE_STREAM_STATUS_INVALID;
	 os_errno = errno;
	 LOG_FATAL_ID ("lseek() failed to move past header of " << path);
	 LOG_FATAL_ID (strerror (os_errno));
	 assert (0);
	 return;
      }
      // If a writeable stream, write back the header.
      if (!r_only)
	 if (write (fd, (char *) header, sizeof (ufs_stream_header))
	     != sizeof (ufs_stream_header)) {
	    status_ = BTE_STREAM_STATUS_INVALID;
	    os_errno = errno;
	    LOG_FATAL_ID ("write() failed during stream destruction for "
			  << path);
	    LOG_FATAL_ID (strerror (os_errno));
	    assert (0);
	    return;
	 }

      if (header)
	 delete header;

      if (::close (fd)) {
	 os_errno = errno;
	 LOG_FATAL_ID ("Failed to close() " << path);
	 LOG_FATAL_ID (strerror (os_errno));
	 assert (0);
	 return;
      }
      // If it should not persist, unlink the file.
      if (per == PERSIST_DELETE) {
	 if (r_only)
	    LOG_WARNING_ID
		("PERSIST_DELETE persistence for read-only stream in " <<
		 path);
	 else if (unlink (path)) {
	    os_errno = errno;
	    LOG_WARNING_ID ("unlink() failed during destruction of " <<
			    path);
	    LOG_WARNING_ID (strerror (os_errno));
	 }
      }

   }				// end of if (!substream_level) 

   if (curr_block) {
     delete [] curr_block; // should be vector delete -RW

      //If you really want to be anal about memory calculation
      //consistency then if BTE_IMPLICIT_FS_READAHEAD flag is
      //set you shd register a memory deallocation of 
      //logical_blocksize AT THIS POINT of time in code.
      //At present, since we havent registered allocation
      //for these ``implicitly read-ahead'' blocks, we dont
      //register the dealloc either
   }
#if UFS_DOUBLE_BUFFER
   //Have to think this out since if UFS_DOUBLE_BUFFERING is implemented
   //there is the possibility that the aio_read for the next block is
   //ongoing at the time of the destruction, in which case trying to
   //delete next_block may cause a run-time error. Most probably
   // the aio read op may have to be suspended if ongoing. 
   if (next_block)
      delete [] next_block;	// use vector delete -RW
#endif

   register_memory_deallocation (sizeof (BTE_single_disk < T >));

}

template < class T >
    B_INLINE BTE_err BTE_single_disk < T >::read_item (T ** elt)
{
   BTE_err bte_err;

   if (f_stats) {
      stats.record_read ();
   }
   // Make sure we are not currently at the EOS.
   if (f_offset >= f_eos) {
      tp_assert (f_eos == f_offset, "Can't read past eos.");
      return BTE_ERROR_END_OF_STREAM;
   }
   // Validate the current block.
   if ((bte_err = validate_current ()) != BTE_ERROR_NO_ERROR) {
      return bte_err;
   }
   // Check and make sure that the current pointer points into the current block.
   tp_assert (((unsigned int) ((char *) current - (char *) curr_block) <=
	       (unsigned int) (logical_blocksize - sizeof (T))),
	      "current is past the end of the current block");
   tp_assert (((char *) current - (char *) curr_block >= 0),
	      "current is before the begining of the current block");

   // Read
   *elt = current;

   // Advance the current pointer.
   advance_current ();

   // If we are in a substream, there should be no way for f_current to pass f_eos.
   tp_assert (!substream_level || (f_offset <= f_eos),
	      "Got past eos in a substream.");

   return BTE_ERROR_NO_ERROR;

}

template < class T >
    B_INLINE BTE_err BTE_single_disk < T >::write_item (const T & elt)
{
   BTE_err bte_err;

   if (f_stats) {
      stats.record_write ();
   }
   // This better be a writable stream.
   if (r_only) {
      return BTE_ERROR_READ_ONLY;
   }
   // Make sure we are not currently at the EOS of a substream.
   if (substream_level && (f_eos <= f_offset)) {
      tp_assert (f_eos == f_offset, "Went too far in a substream.");
      return BTE_ERROR_END_OF_STREAM;
   }
   // Validate the current block.
   if ((bte_err = validate_current ()) != BTE_ERROR_NO_ERROR) {
      return bte_err;
   }
   // Check and make sure that the current pointer points into the current
   // block.
   tp_assert (((unsigned int) ((char *) current - (char *) curr_block) <=
	       (unsigned int) (logical_blocksize - sizeof (T))),
	      "current is past the end of the current block");
   tp_assert (((char *) current - (char *) curr_block >= 0),
	      "current is before the begining of the current block");

   // write
   *current = elt;
   block_dirty = 1;

   // Advance the current pointer.
   advance_current ();

   // If we are in a substream, there should be no way for f_current to
   // pass f_eos.
   tp_assert (!substream_level || (f_offset <= f_eos),
	      "Got past eos in a substream.");

   // If we moved past eos, then update eos unless we are in a
   // substream, in which case EOS will be returned on the next call.
   if ((f_offset > f_eos) && !substream_level) {
      // disable the assertion below because it is violated when
      // the end of a block is reached and the item size does not
      // divide the block size completely (so there is some space left)
      // tp_assert(f_offset == f_eos + sizeof(T), "Advanced too far somehow.");
      f_eos = f_offset;
   }

   return BTE_ERROR_NO_ERROR;
}

// Query memory usage
// Note that in a substream we do not charge for the memory used by
// the header, since it is accounted for in the 0 level superstream.
template < class T >
    BTE_err BTE_single_disk < T >::main_memory_usage (size_t * usage,
						      MM_stream_usage
						      usage_type)
{
   switch (usage_type) {
   case MM_STREAM_USAGE_OVERHEAD:
      *usage =
	  (sizeof (*this) +
	   (((header == NULL) || substream_level) ? 0 : os_blocksize));
      break;
   case MM_STREAM_USAGE_BUFFER:
      *usage = BTE_UFS_MM_BUFFERS * logical_blocksize;
      break;
   case MM_STREAM_USAGE_CURRENT:
      *usage =
	  (sizeof (*this) +
	   (((header == NULL) || substream_level) ? 0 : os_blocksize) +
	   ((curr_block == NULL) ? 0 : BTE_UFS_MM_BUFFERS *
	    logical_blocksize));
      break;
   case MM_STREAM_USAGE_MAXIMUM:
      *usage = (sizeof (*this) + BTE_UFS_MM_BUFFERS * logical_blocksize +
		(substream_level ? 0 : os_blocksize));
      break;
   case MM_STREAM_USAGE_SUBSTREAM:
      *usage = (sizeof (*this) + BTE_UFS_MM_BUFFERS * logical_blocksize);
      break;
   }

   return BTE_ERROR_NO_ERROR;
};

// Return the number of items in the stream.
template < class T > off_t BTE_single_disk < T >::stream_len (void)
{
   return file_off_to_item_off (f_eos) - file_off_to_item_off (f_bos);
};

// Return the path name in newly allocated space.
template < class T >
    BTE_err BTE_single_disk < T >::name (char **stream_name)
{
   int len = strlen (path);

   tp_assert (len < BTE_UFS_PATH_NAME_LEN, "Path length is too long.");

   char *new_path = new char[len + 1];

   strncpy (new_path, path, len + 1);
   *stream_name = new_path;

   return BTE_ERROR_NO_ERROR;
};

// Move to a specific position.
template < class T > BTE_err BTE_single_disk < T >::seek (off_t offset)
{
   BTE_err be;
   off_t new_offset;

   if (f_stats) {
      stats.record_seek ();
   }

   if ((offset < 0) ||
       (offset >
	file_off_to_item_off (f_eos) - file_off_to_item_off (f_bos))) {
      LOG_WARNING_ID ("seek() out of range (off/bos/eos)");
      LOG_WARNING_ID (offset);
      LOG_WARNING_ID (file_off_to_item_off (f_bos));
      LOG_WARNING_ID (file_off_to_item_off (f_eos));
      return BTE_ERROR_OFFSET_OUT_OF_RANGE;
   }
   // Compute the new offset
   new_offset =
       item_off_to_file_off (file_off_to_item_off (f_bos) + offset);

   // If it is not in the same block as the current position then
   // invalidate the current block.
   // if (((new_offset - os_blocksize) / logical_blocksize) !=
   //    ((f_offset - os_blocksize) / logical_blocksize)) {

   // The above was the old code which was wrong: we also need to check that
   // we have the correct block mapped in (f_offset does not always point into
   // the current block!)

   if (
       ((size_t) ((char *) current - (char *) curr_block) >=
	logical_blocksize)
       || (((new_offset - os_blocksize) / logical_blocksize) !=
	   ((f_offset - os_blocksize) / logical_blocksize))) {
      if (block_valid && ((be = unmap_current ()) != BTE_ERROR_NO_ERROR)) {
	 return be;
      }
   } else {
      if (block_valid) {

	 // We have to adjust current.
	 register off_t internal_block_offset;

	 internal_block_offset =
	     file_off_to_item_off (new_offset) % blocksize_items;
	 current = curr_block + internal_block_offset;
      }
   }

   f_offset = new_offset;

   return BTE_ERROR_NO_ERROR;
};

// Truncate the stream.
template < class T > BTE_err BTE_single_disk < T >::truncate (off_t offset)
{
   BTE_err be;
   off_t new_offset;
   off_t block_offset;

   // Sorry, we can't truncate a substream.
   if (substream_level) {
      return BTE_ERROR_STREAM_IS_SUBSTREAM;
   }

   if (offset < 0) {
      return BTE_ERROR_OFFSET_OUT_OF_RANGE;
   }
   // Compute the new offset
   new_offset =
       item_off_to_file_off (file_off_to_item_off (f_bos) + offset);

   // If it is not in the same block as the current position then
   // invalidate the current block.
   // We also need to check that we have the correct block mapped in (f_offset
   // does not always point into the current block!) - see comment in seek()
   if (
       ((unsigned int) ((char *) current - (char *) curr_block) >=
	logical_blocksize)
       || (((new_offset - os_blocksize) / logical_blocksize) !=
	   ((f_offset - os_blocksize) / logical_blocksize))) {
      if (block_valid && ((be = unmap_current ()) != BTE_ERROR_NO_ERROR)) {
	 return be;
      }
   }
   // If it is not in the same block as the current end of stream
   // then truncate the file to the end of the new last block.
   if (((new_offset - os_blocksize) / logical_blocksize) !=
       ((f_eos - os_blocksize) / logical_blocksize)) {

      // Determine the offset of the block that new_offset is in.
      block_offset = ((new_offset - os_blocksize) / logical_blocksize)
	  * logical_blocksize + os_blocksize;
      f_filelen = block_offset + logical_blocksize;
      if (ftruncate (fd, block_offset + logical_blocksize)) {
	 os_errno = errno;
	 LOG_FATAL_ID ("Failed to ftruncate() to the new end of " << path);
	 LOG_FATAL_ID (strerror (os_errno));
	 return BTE_ERROR_OS_ERROR;
      }
   }
   // Reset the current position to the end.    
   f_offset = f_eos = new_offset;

   return BTE_ERROR_NO_ERROR;
};

// Map in the header from the file.  This assumes that the path
// has been cached in path and that the file has been opened and
// fd contains a valid descriptor.
template < class T >
    ufs_stream_header * BTE_single_disk < T >::map_header (void)
{
   off_t file_end;
   ufs_stream_header *ptr_to_header;

   // If the underlying file is not at least long enough to contain
   // the header block, then, assuming the stream is writable, we have
   // to create the space on disk by doing an explicit write().  
   if ((file_end = lseek (fd, 0, SEEK_END)) < (off_t) os_blocksize) {
      if (r_only) {
	 status_ = BTE_STREAM_STATUS_INVALID;
	 LOG_FATAL_ID ("No header block in read only stream " << path);
	 return NULL;
      } else {

	 // A writable stream, but it doesn't have a header block, which means
	 // the file was just created and we have to leave space for the header
	 // block at the beginning of the fille.
	 // In this case we choose simply to allocate space
	 // for header fields and return a pointer to ufs_stream_header
	 // but first we write a dummy os_blocksize sized block at the beginning
	 // of the file.   This will trigger off sequential write optimizations that
	 // are useful unless non-sequential accesses to data are made.

	 char *tmp_buffer = new char[os_blocksize];

	 if (lseek (fd, 0, SEEK_SET) != 0) {
	    os_errno = errno;
	    LOG_FATAL_ID ("Failed to lseek() in stream " << path);
	    LOG_FATAL_ID (strerror (os_errno));
	    return NULL;
	 }

	 if (write (fd, tmp_buffer, os_blocksize) !=
	     (ssize_t) os_blocksize) {
	    os_errno = errno;
	    LOG_FATAL_ID ("Failed to write() in stream " << path);
	    LOG_FATAL_ID (strerror (os_errno));
	    return NULL;
	 }

	 delete [] tmp_buffer;	// use vector delete -RW

	 ptr_to_header = new ufs_stream_header;
	 if (ptr_to_header != NULL) {
	    return ptr_to_header;
	 } else {
	    os_errno = errno;
	    LOG_FATAL_ID ("Failed to alloc space for header.");
	    LOG_FATAL_ID (strerror (os_errno));
	    status_ = BTE_STREAM_STATUS_INVALID;
	    return NULL;
	 }
      }
   }
   // Map in the header block.  If the stream is writable, the header 
   // block should be too.

   //Instead of mmap() we simply read in the os_blocksize leading
   // bytes of the file, copy the leading sizeof(ufs_stream_header) bytes
   // of the os_blocksize bytes into the ptr_to_header structure and return
   // ptr_to_header. Note that even though we could have read only the first
   // sizeof(ufs_stream_header) of the file 
   // we choose not to do so in order to avoid confusing sequential prefetcher. 

   char *tmp_buffer = new char[os_blocksize];

   if (lseek (fd, 0, SEEK_SET) != 0) {
      os_errno = errno;
      LOG_FATAL_ID ("Failed to lseek() in stream " << path);
      LOG_FATAL_ID (strerror (os_errno));
      return NULL;
   }

   if (read (fd, (char *) tmp_buffer, os_blocksize) !=
       (ssize_t) os_blocksize) {
      os_errno = errno;
      LOG_FATAL_ID ("Failed to read() in stream " << path);
      LOG_FATAL_ID (strerror (os_errno));
      return NULL;
   }
    
   ptr_to_header = new ufs_stream_header;
   memcpy(ptr_to_header, tmp_buffer, sizeof(ufs_stream_header));
   delete [] tmp_buffer;	// should use vector delete -RW
   return ptr_to_header;

   // The following comment is probably wrong. -RW

   // Note that with this implementation of map_header()
   // and the header strucure, we do not consume one os_blocksize from memory
   // as the mmap() based implementation does.
   // MMB Need to change MEMORY USAGE since now the header occupies
   // only minimal space
};

//
// Make sure the current block is mapped in and all internal pointers are
// set as appropriate.  
//  
template < class T >
    inline BTE_err BTE_single_disk < T >::validate_current (void)
{
   unsigned int block_space;	// The space left in the current block.
   BTE_err bte_err;

   // If the current block is valid and current points into it and has
   // enough room in the block for a full item, we are fine.  If it is
   // valid but there is not enough room, unmap it.
   if (block_valid) {
      if ((block_space = logical_blocksize -
	   ((char *) current - (char *) curr_block)) >= sizeof (T)) {
	 return BTE_ERROR_NO_ERROR;
      } else {			// Not enough room left.
	 if ((bte_err = unmap_current ()) != BTE_ERROR_NO_ERROR) {
	    return bte_err;
	 }
	 f_offset += block_space;
      }
   }
   // The current block is invalid, since it was either invalid to start
   // with or we just invalidated it because we were out of space.
   tp_assert (!block_valid, "Block is already mapped in.");

   // Now map it the block.
   return map_current ();

};

template < class T >
    inline BTE_err BTE_single_disk < T >::invalidate_current (void)
{
   // We should currently have a valid block.
   tp_assert (block_valid, "No block is mapped in.");
   block_valid = 0;

   return BTE_ERROR_NO_ERROR;
}

// Map in the current block.
// f_offset is used to determine what block is needed.
template < class T > BTE_err BTE_single_disk < T >::map_current (void)
{
   off_t block_offset;
   int do_mmap = 0;

   // We should not currently have a valid block.
   tp_assert (!block_valid, "Block is already mapped in.");

   if (f_stats) {
      stats.record_map ();
   }
   // Determine the offset of the block that the current item is in.
   block_offset = ((f_offset - os_blocksize) / logical_blocksize)
       * logical_blocksize + os_blocksize;

   // If the block offset is beyond the logical end of the file, then
   // we either record this fact and return (if the stream is read
   // only) or ftruncate() out to the end of the current block.
   if (f_filelen < block_offset + (off_t) logical_blocksize) {
      if (r_only) {
	 return BTE_ERROR_END_OF_STREAM;
      } else {

	 // An assumption here is that !r_only implies that you won't try to read
	 // items beyond offset block_offset. This is justified because one invariant
	 // being maintained is that file length is os_blocksize + an INTEGRAL
	 // number of Logical Blocks: By this invariant, since lseek returns something 
	 //  smaller than block_offset + logical_blocksize - 1 (meaning that filesize
	 // is smaller than block_offset + logical_blocksize), 
	 //
	 // A consequence of this assumption is that the block being mapped in
	 // is being written/appended. Now while using mmapped I/O, what this 
	 // means is that we need to first ftruncate() and then map in the requisite
	 // block. On the other hand, if we are using the read()/write() BTE, we
	 // simply do nothing: the unmap_current() call executed during
	 // validate_current() and before map_current() would have ensured that
	 // we do not overwrite some previously mapped block. 

	 // Not mapped I/O  
	 // means we assume we are using the read()/write() BTE
	 // This means we do an unmap_current() in validate_current()
	 // just before  map_current() so there's no danger of overwriting
	 // a dirty block.  

	 if (curr_block == NULL) {
#if(0)
	   // new should never return NULL -RW
	   // compiler doesnt like casting up from char to T 
	   if ((curr_block = (T *) new char[logical_blocksize]) == NULL) {
		 status_ = BTE_STREAM_STATUS_INVALID;
		 os_errno = errno;
	       LOG_FATAL_ID ("new failed to allocate memory.");
	       LOG_FATAL_ID (strerror (os_errno));
	       return BTE_ERROR_OS_ERROR;
	    }
#endif
	   curr_block = new T[(sizeof(T)-1+logical_blocksize)/sizeof(T)];

	    // If you really want to be anal about memory calculation
	    // consistency then if BTE_IMPLICIT_FS_READAHEAD flag is
	    // set you shd register a memory allocation of 
	    // logical_blocksize AT THIS POINT of time in code.
	 }

	 block_valid = 1;
	 curr_block_file_offset = block_offset;
	 block_dirty = 0;

	 register off_t internal_block_offset;

	 internal_block_offset =
	     file_off_to_item_off (f_offset) % blocksize_items;

	 current = curr_block + internal_block_offset;

	 return BTE_ERROR_NO_ERROR;

      }
   }
   // If the current block is already mapped in by this process then
   // some systems, (e.g. HP-UX), will not allow us to map it in
   // again.  This presents all kinds of problems, not only with
   // sub/super-stream interactions, which we could probably detect
   // by looking back up the path to the level 0 stream, but also
   // with overlapping substreams, which are very hard to detect
   // since the application can build them however it sees fit.  We
   // can also have problems if we break a stream into two substreams
   // such that their border is in the middle of a block, and then we
   // read to the end of the fisrt substream while we are still at
   // the beginning of the second.

#if UFS_DOUBLE_BUFFER
   if (have_next_block && (block_offset == f_next_block)) {
      T *temp;

      temp = curr_block;
      curr_block = next_block;
      next_block = temp;
      have_next_block = 0;
   } else {
      do_mmap = 1;
   }
#else
   do_mmap = 1;
#endif

   if (do_mmap) {

      if (lseek (fd, block_offset, SEEK_SET) != block_offset) {
	 status_ = BTE_STREAM_STATUS_INVALID;
	 os_errno = errno;
	 LOG_FATAL_ID ("lseek() to  block at " << block_offset <<
		       " in file ");
	 LOG_FATAL_ID (path << ": " << strerror (os_errno));
	 return BTE_ERROR_OS_ERROR;
      }

      if (curr_block == NULL) {

#if(0)
	   // new should never return NULL -RW
	   // compiler doesnt like casting up from char to T 
	 if ((curr_block = (T *) new char[logical_blocksize]) == NULL) {
	    status_ = BTE_STREAM_STATUS_INVALID;
	    os_errno = errno;
	    LOG_FATAL_ID ("new failed to allocate memory.");
	    LOG_FATAL_ID (strerror (os_errno));
	    return BTE_ERROR_OS_ERROR;
	 }
#endif
	 curr_block = new T[(sizeof(T)-1+logical_blocksize)/sizeof(T)];

	 //If you really want to be anal about memory calculation
	 //consistency then if BTE_IMPLICIT_FS_READAHEAD flag is
	 //set you shd register a memory allocation of 
	 //logical_blocksize AT THIS POINT of time in code.
      }

      if (read (fd, (char *) curr_block, logical_blocksize) !=
	  (ssize_t) logical_blocksize) {
	 status_ = BTE_STREAM_STATUS_INVALID;
	 os_errno = errno;
	 LOG_FATAL_ID ("read() failed to read in block at " << block_offset
		       << " in file ");
	 LOG_FATAL (path << " : " << strerror (os_errno));
	 return BTE_ERROR_OS_ERROR;
      }

   }

   block_valid = 1;
   curr_block_file_offset = block_offset;
   block_dirty = 0;

#if BTE_UFS_READ_AHEAD

   // Start the asyncronous read of the next logical block.
   read_ahead ();

#endif

   // The offset, in terms of number of items, that current should
   // have relative to curr_block.

   register off_t internal_block_offset;

   internal_block_offset =
       file_off_to_item_off (f_offset) % blocksize_items;

   current = curr_block + internal_block_offset;

   return BTE_ERROR_NO_ERROR;

};

template < class T > BTE_err BTE_single_disk < T >::unmap_current (void)
{
   off_t lseek_retval;

   // We should currently have a valid block.
   tp_assert (block_valid, "No block is mapped in.");

   if (f_stats) {
      stats.record_unmap ();
   }

   if ((!r_only) && (block_dirty)) {
      if ((lseek_retval = lseek (fd, curr_block_file_offset, SEEK_SET)) !=
	  curr_block_file_offset) {
	 status_ = BTE_STREAM_STATUS_INVALID;
	 os_errno = errno;
	 LOG_FATAL_ID ("lseek() failed while unmapping current block.");
	 LOG_FATAL_ID (strerror (os_errno));
	 return BTE_ERROR_OS_ERROR;
      }
      if (lseek_retval == f_filelen)
	 f_filelen += logical_blocksize;

      if (write (fd, (char *) curr_block, logical_blocksize) !=
	  (ssize_t) logical_blocksize) {
	 status_ = BTE_STREAM_STATUS_INVALID;
	 os_errno = errno;
	 LOG_FATAL_ID ("write() failed to unmap current block.");
	 LOG_FATAL_ID (strerror (os_errno));
	 return BTE_ERROR_OS_ERROR;
      }
   }

   block_dirty = 0;
   block_valid = 0;
   curr_block_file_offset = 0;

   return BTE_ERROR_NO_ERROR;
};

// A uniform method for advancing the current pointer.  No mapping,
// unmapping, or anything like that is done here.
template < class T >
    inline BTE_err BTE_single_disk < T >::advance_current (void)
{

   // Advance the current pointer and the file offset of the current
   // item.
   current++;
   f_offset += sizeof (T);

   return BTE_ERROR_NO_ERROR;
};

// Register memory usage with the memory manager.
template < class T >
    inline BTE_err BTE_single_disk <
    T >::register_memory_allocation (size_t sz)
{
   MM_err mme;

   if ((mme = MM_manager.register_allocation (sz)) != MM_ERROR_NO_ERROR) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL_ID ("Memory manager error in allocation.");
      return BTE_ERROR_MEMORY_ERROR;
   }

   return BTE_ERROR_NO_ERROR;
};

template < class T >
    inline BTE_err BTE_single_disk <
    T >::register_memory_deallocation (size_t sz)
{
   MM_err mme;

   if ((mme = MM_manager.register_deallocation (sz)) != MM_ERROR_NO_ERROR) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL_ID ("Memory manager error in deallocation.");
      return BTE_ERROR_MEMORY_ERROR;
   }

   return BTE_ERROR_NO_ERROR;
};

template < class T >
    inline off_t BTE_single_disk <
    T >::item_off_to_file_off (off_t item_off)
{
   off_t file_off;

   if (!itemsize_div_blocksize) {

      // Move past the header.  
      file_off = os_blocksize;

      // Add logical_blocksize for each full block.
      file_off += logical_blocksize * (item_off / blocksize_items);

      // Add sizeof(T) for each item in the partially full block.
      file_off += sizeof (T) * (item_off % blocksize_items);

      return file_off;

   } else {

      return (os_blocksize + item_off * sizeof (T));

   }
};

template < class T >
    inline off_t BTE_single_disk <
    T >::file_off_to_item_off (off_t file_off)
{
   off_t item_off;

   if (!itemsize_div_blocksize) {

      // Subtract off the header.
      file_off -= os_blocksize;

      // Account for the full blocks.
      item_off = blocksize_items * (file_off / logical_blocksize);

      // Add in the number of items in the last block.
      item_off += (file_off % logical_blocksize) / sizeof (T);

      return item_off;

   } else {

      return (file_off - os_blocksize) / sizeof (T);

   }
};

template < class T > int BTE_single_disk < T >::available_streams (void)
{
   return remaining_streams;
};

template < class T > off_t BTE_single_disk < T >::chunk_size (void)
{
   return blocksize_items;
};

template < class T > void BTE_single_disk < T >::persist (persistence p)
{
   per = p;
};

#if BTE_UFS_READ_AHEAD

template < class T > void BTE_single_disk < T >::read_ahead (void)
{
   off_t f_curr_block;

   // The current block had better already be valid or we made a
   // mistake in being here.
   tp_assert (block_valid,
	      "Trying to read ahead when current block is invalid.");

   // Check whether there is a next block.  If we are already in the
   // last block of the file then it makes no sense to read ahead.
   f_curr_block = ((f_offset - os_blocksize) / logical_blocksize) *
       logical_blocksize + os_blocksize;

   if (f_eos < f_curr_block + logical_blocksize) {
      return;
   }

   f_next_block = f_curr_block + logical_blocksize;

#if USE_LIBAIO
   // Asyncronously read the first word of each os block in the next
   // logical block.

   for (unsigned int ii = 0; ii < BTE_UFS_LOGICAL_BLOCKSIZE_FACTOR; ii++) {

      // Make sure there is not a pending request for this block
      // before requesting it.  
      if (aio_results[ii].aio_return != AIO_INPROGRESS) {
	 aio_results[ii].aio_return = AIO_INPROGRESS;

	 // We have to cancel the last one, even though it completed,
	 // in order to allow another one with the same result.
	 aiocancel (aio_results + ii);

	 // Start the async I/O.
	 if (aioread (fd, (char *) (read_ahead_buffer + ii), sizeof (int),
		      f_next_block + ii * os_blocksize, SEEK_SET,
		      aio_results + ii)) {

	    os_errno = errno;
	    LOG_FATAL_ID ("aioread() failed to read ahead");
	    LOG_FATAL_ID (strerror (os_errno));
	 }
      }
   }

#endif

#if UFS_DOUBLE_BUFFER
#error Explicit double buffering not supported using read/write BTE
#endif

};

#endif	/* BTE_UFS_READ_AHEAD */

#undef BTE_UFS_MM_BUFFERS

#endif // _BTE_STREAM_UFS_H
