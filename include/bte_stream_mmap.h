//
// File: bte_stream_mmap.h (formerly bte_mmb.h)
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/13/94
//
// $Id: bte_stream_mmap.h,v 1.1 2002-01-06 18:46:50 tavi Exp $
//
// Memory mapped streams.  This particular implementation explicitly manages
// blocks, and only ever maps in one block at a time.
//
#ifndef _BTE_STREAM_MMAP_H
#define _BTE_STREAM_MMAP_H

#include <assert.h>

// define to collect stats on prefetch performance
// #define COLLECT_STATS

#if USE_LIBAIO
#if !HAVE_LIBAIO
#error USE_LIBAIO requested, but aio library not in configuration.
#endif
#include <sys/asynch.h>
#endif

#ifdef BTE_MMB_READ_AHEAD
#define BTE_MMB_MM_BUFFERS 2
#else
#define BTE_MMB_MM_BUFFERS 1
#endif

// Include the registration based memory manager.
#define MM_IMP_REGISTER
#include <mm.h>

// Get the BTE_stream_base class and other definitions.
#include <bte_stream_base.h>

// We need a variety of OS constants for the mmap() implementation.
extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
}
#if !HAVE_PROTOTYPE_MMAP
extern "C" mmap (caddr_t addr, size_t len, int prot, int flags,
		 int filedes, off_t off);
#endif

#if !HAVE_PROTOTYPE_MUNMAP
extern "C" int munmap (addr_t addr, int len);
#endif

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

// Stats code.
#include <bte_mmb_stats.h>

#ifndef BTE_MMB_LOGICAL_BLOCKSIZE_FACTOR
#define BTE_MMB_LOGICAL_BLOCKSIZE_FACTOR 1
#endif

#define MMB_HEADER_MAGIC_NUMBER	0xABCDEF

// A base class for all BTE_stream_mmb<T> classes.
class BTE_stream_mmb_base {
protected:
  static bool f_stats;
  static bte_mmb_stats stats;
  static int remaining_streams;
public:
  // Gathering and providing statisitics.T> classes.) implementation.y manages
  static void reset_stats (void);
  static void stats_on ();
  static void stats_off ();
  static const bte_stats & statistics (void);
};

//
// A header structure that will appear in the first block of a BTE stream.
// This structure will generally not occupy the entire first block, but 
// to preserve block boundaries, nothing else will be added to the block.
//
// If more information is needed in the header, it should be added to a
// newer version of this structure.
//
struct mmap_stream_header {
 public:
   unsigned int magic_number;	// Set to MMB_HEADER_MAGIC_NUMBER
   unsigned int version;	// Should be 1 for current version.
   unsigned int length;		// # of bytes in this structure.
   off_t item_logical_eof;
   // The number of items in the stream.
   size_t item_size;		// The size of each item in the stream.
   size_t block_size;		// The size of a physical block on the device
   // where this stream resides.
   unsigned int items_per_block;
};


#define BTE_MMB_PATH_NAME_LEN 128

// figure out the block offset for an offset (pos) in file
// os_blocksize is assumed to be the header size
#define BLOCK_OFFSET(pos) \
 ((((pos) - os_blocksize) / logical_blocksize) \
 * logical_blocksize + os_blocksize)

//
// BTE_stream_mmb<T>
//
// This is a class template for the mmap() based implementation of a 
// BTE stream of objects of type T.  This version maps in only one
// block of the file at a time. 
//
template < class T > class BTE_stream_mmb:
public BTE_stream_base < T >, public BTE_stream_mmb_base {
 private:
   unsigned int mmap_status;

   int fd;			// descriptor of the mapped file.

   size_t os_blocksize;

   size_t logical_blocksize;

   // How deeply is this stream nested.
   unsigned int substream_level;

   // Offset of the current item in the file.  This is the logical
   // offset of the item within the file, that is, the place we would
   // have to lseek() to in order to read() or write() the item if we
   // were using ordinary (i.e. non-mmap()) file access methods.
   off_t f_offset;

   // Offset just past the end of the last item in the stream.  If this
   // is a substream, we can't write here or anywhere beyond.
   off_t f_eos;

   // Length of the file in the file system.
   // this is the first offset that would be not part of the file.
   // different from f_eos since we can grow the file independently
   // of the actual writes.
   off_t f_filelen;

   // Beginning of the file.  Can't write before here.
   off_t f_bos;

   // A pointer to the mapped in header block for the stream.
   mmap_stream_header *header;

   T *current;			// The current item (mapped in)
   T *curr_block;		// pointer to beginning of the currently mapped block.
   int block_valid;		// Non-zero if current points to a valid, mapped block.
   int block_mapped;		// true if the curr_block is mapped

   // for use in double buffering
   T *next_block;		// ptr to next block
   off_t f_next_block;		// position of next block
   int have_next_block;		// is next block mapped

   int r_only;			// Non-zero if this stream was opened for reading only. 
   int w_only;			// stream is write-only

   // A place to cache OS error values.  It is normally set after each
   // call to the OS.
   int os_errno;

   char path[BTE_MMB_PATH_NAME_LEN];

#if USE_LIBAIO
   // A buffer to read the first word of each OS block in the next logical
   // block for read ahead.
   int read_ahead_buffer[BTE_MMB_LOGICAL_BLOCKSIZE_FACTOR];

   // Results of asyncronous I/O.
   aio_result_t aio_results[BTE_MMB_LOGICAL_BLOCKSIZE_FACTOR];
#endif

#ifdef BTE_MMB_READ_AHEAD
   // Read ahead into the next logical block.
   void read_ahead (void);
#endif

   void initialize ();
   int check_header (mmap_stream_header * header);

   mmap_stream_header *map_header (void);
   void unmap_header (void);

   inline BTE_err validate_current (void);
   BTE_err map_current (void);
   inline BTE_err invalidate_current (void);
   BTE_err unmap_current (void);

   inline BTE_err advance_current (void);

   inline BTE_err register_memory_allocation (size_t sz);
   inline BTE_err register_memory_deallocation (size_t sz);

   inline off_t item_off_to_file_off (off_t item_off);
   inline off_t file_off_to_item_off (off_t item_off);

   persistence per;		// The persistence status of this stream.

#ifdef COLLECT_STATS
   long stats_hits;
   long stats_misses;
   long stats_compulsory;
   long stats_eos;
#endif

 public:
   // Constructors
   BTE_stream_mmb (const char *dev_path, BTE_stream_type st);

   BTE_stream_mmb (BTE_stream_type st);
   BTE_stream_mmb (BTE_stream_mmb < T > &s);

   // A substream constructor.
   BTE_stream_mmb (BTE_stream_mmb * super_stream,
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
   ~BTE_stream_mmb (void);

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

   void print (char *pref = "");
   inline BTE_err grow_file (off_t block_offset);

   static void log_fatal (char *a, char *b, char *c, char *d, char *e) {
      LOG_FATAL (a);
      LOG_FATAL (b);
      LOG_FATAL (c);
      LOG_FATAL (d);
      LOG_FATAL (e);
      LOG_FLUSH_LOG;
   };
};

/* ********************************************************************** */

/* definitions start here */

static int call_munmap (void *addr, size_t len)
{
   int rv;

#ifdef MACH_ALPHA
   rv = munmap (addr, len);
#else
   rv = munmap ((caddr_t) addr, len);
#endif
   return rv;
}

static void *call_mmap (void *addr, size_t len, int r_only, int w_only,
			int fd, off_t off, int fixed)
{
   void *ptr;
   int flags = 0;

   assert (!fixed || addr);

#ifdef MACH_ALPHA
// for enhanced mmap calls
#define MAP_OVERWRITE 0x1000	// block will be overwritten
   flags = (MAP_FILE |
	    (fixed ? MAP_FIXED : MAP_VARIABLE) |
	    (w_only ? MAP_OVERWRITE : 0));
#else
   flags = (fixed ? MAP_FIXED : 0);
#endif
   flags |= MAP_SHARED;
#ifdef MACH_ALPHA
   ptr = mmap (addr, len,
	       (r_only ? PROT_READ : PROT_READ | PROT_WRITE),
	       flags, fd, off);
#else
   ptr = mmap ((caddr_t) addr, len,
	       (r_only ? PROT_READ : PROT_READ | PROT_WRITE),
	       flags, fd, off);
#endif
   assert (ptr);
   return ptr;
}

// Do some error checking on the header, such as to make sure that
// it is a stream of objects of the right size and that it has
// the correct header version.
template < class T >
    int BTE_stream_mmb < T >::check_header (mmap_stream_header * header)
{
   if (!header) {
      LOG_FATAL_ID ("Could not map header");
      LOG_FATAL_ID (path);
      return -1;
   }
   if (header->magic_number != MMB_HEADER_MAGIC_NUMBER) {
      LOG_FATAL_ID ("header: magic number mismatch (expected/actual)");
      LOG_FATAL_ID (MMB_HEADER_MAGIC_NUMBER);
      LOG_FATAL_ID (header->magic_number);
      LOG_FATAL_ID (path);
      return -1;
   }
   if ((header->version != 1) ||
       (header->length != sizeof (*header)) ||
       (header->item_size != sizeof (T)) ||
       (header->block_size != logical_blocksize)) {
      LOG_FATAL_ID ("header: incompatible with current BTE");
      LOG_FATAL_ID (path);
      return -1;
   }
   return 0;
}

template < class T > void BTE_stream_mmb < T >::initialize ()
{
#ifdef COLLECT_STATS
   stats_misses = stats_hits = stats_compulsory = stats_eos = 0;
#endif
   have_next_block = 0;
   block_valid = 0;
   block_mapped = 0;
   f_offset = f_bos = os_blocksize;
   next_block = curr_block = current = NULL;
   f_stats = 1;
}

//
// This constructor creates a stream whose contents are taken from the
// file whose path is given.
//

template < class T >
    BTE_stream_mmb < T >::BTE_stream_mmb (const char *dev_path,
					  const BTE_stream_type st)
{
   status_ = BTE_STREAM_STATUS_NO_STATUS;

   if (f_stats) {
      stats.record_create ();
   }
   // Reduce the number of streams avaialble.
   if (remaining_streams > 0) {
      remaining_streams--;
   } else if (remaining_streams == 0) {
      status_ = BTE_STREAM_STATUS_INVALID;
      return;
   }

   // Cache the path name
   if (strlen (dev_path) > BTE_MMB_PATH_NAME_LEN - 1) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL ("Path name \"");
      LOG_FATAL (dev_path);
      LOG_FATAL ("\" too long.\n");
      LOG_FLUSH_LOG;
      return;
   }

   strncpy (path, dev_path, BTE_MMB_PATH_NAME_LEN);
   r_only = (st == BTE_READ_STREAM);
   w_only = (st == BTE_WRITEONLY_STREAM);

   // Find out the block size from the file system and set the
   // logical block size.

#ifdef _SC_PAGE_SIZE
   os_blocksize = sysconf (_SC_PAGE_SIZE);
#else
   os_blocksize = getpagesize ();
#endif

   logical_blocksize = os_blocksize * BTE_MMB_LOGICAL_BLOCKSIZE_FACTOR;

   // We can't handle streams of large objects.
   if (sizeof (T) > logical_blocksize) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL ("Object size is larger than os block size.\n");
      LOG_FLUSH_LOG;
      return;
   }
   // This is a top level stream
   substream_level = 0;

   persist (PERSIST_DELETE);

   switch (st) {
   case BTE_READ_STREAM:
      // Open the file for reading.
      if ((fd =::open (path, O_RDONLY)) == -1) {
	 status_ = BTE_STREAM_STATUS_INVALID;
	 os_errno = errno;
	 log_fatal ("open() failed to open \"", path, "\": ",
		    strerror (os_errno), "\n");
	 assert (0);
	 return;
      }
      // Get ready to read the first item out of the file.
      initialize ();
      header = map_header ();
      if (check_header (header) < 0) {
	 status_ = BTE_STREAM_STATUS_INVALID;
	 assert (0);
	 return;
      }
      // Set the eos marker appropriately.
      // Rajiv - done later
      // f_eos = item_off_to_file_off(header->item_logical_eof);

      break;

   case BTE_WRITE_STREAM:
   case BTE_WRITEONLY_STREAM:
   case BTE_APPEND_STREAM:
      // Open the file for writing.  First we will try to open 
      // is with the O_EXCL flag set.  This will fail if the file
      // already exists.  If this is the case, we will call open()
      // again without it and read in the header block.
      if ((fd =::open (path, O_RDWR | O_CREAT | O_EXCL,
		       S_IRUSR | S_IWUSR |
		       S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) == -1) {

	 // Try again, hoping the file already exists.
	 if ((fd =::open (path, O_RDWR)) == -1) {
	    status_ = BTE_STREAM_STATUS_INVALID;

	    os_errno = errno;

	    LOG_FATAL ("open() failed to open \"");
	    LOG_FATAL (path);
	    LOG_FATAL ("\": ");
	    LOG_FATAL (strerror (os_errno));
	    LOG_FATAL ('\n');

	    LOG_FLUSH_LOG;

	    return;
	 }
	 // the file already exists, so read the header
	 initialize ();
	 header = map_header ();
	 if (check_header (header) < 0) {
	    status_ = BTE_STREAM_STATUS_INVALID;
	    assert (0);
	    return;
	 }

	 // Rajiv - done later
	 // f_eos = item_off_to_file_off(header->item_logical_eof);

      } else {			// The file was just created.
	 f_eos = os_blocksize;
	 // Rajiv
	 assert (lseek (fd, 0, SEEK_END) == 0);
#ifdef VERBOSE
	 if (verbose)
	    cout << "CONS created file: " << path << endl;
#endif

	 // what does this do??? Rajiv
	 // Create and map in the header.
	 if (lseek (fd, os_blocksize - 1, SEEK_SET) != os_blocksize - 1) {
	    status_ = BTE_STREAM_STATUS_INVALID;
	    os_errno = errno;
	    log_fatal ("lseek() failed to move past header of \"",
		       path, "\": ", strerror (os_errno), "\n");
	    assert (0 == 1);
	    return;
	 }
	 initialize ();
	 header = map_header ();
	 if (!header) {
	    status_ = BTE_STREAM_STATUS_INVALID;
	    assert (0 == 1);
	    return;
	 }
	 header->magic_number = MMB_HEADER_MAGIC_NUMBER;
	 header->version = 1;
	 header->length = sizeof (*header);
	 header->item_logical_eof = 0;
	 header->item_size = sizeof (T);
	 header->block_size = logical_blocksize;
	 header->items_per_block = logical_blocksize / sizeof (T);
      }
      break;
   }

   f_filelen = lseek (fd, 0, SEEK_END);
   assert (f_filelen >= 0);
   f_eos = item_off_to_file_off (header->item_logical_eof);
   if (st == BTE_APPEND_STREAM) {
      f_offset = f_eos;
   } else {
      f_offset = os_blocksize;
   }
#ifdef VERBOSE
   // Rajiv
   if (verbose)
      cout << "CONS logical eof=" << header->item_logical_eof << "\n";
#endif

   // By default, all streams are deleted at destruction time.
   per = PERSIST_DELETE;

   // Register memory usage before returning.
   //register_memory_allocation(os_blocksize); // for the header.
   register_memory_allocation (sizeof (mmap_stream_header));	// for the header.
   register_memory_allocation (sizeof (BTE_stream_mmb < T >));
   register_memory_allocation (BTE_MMB_MM_BUFFERS * logical_blocksize);	// for buffers

#ifdef VERBOSE
   // Rajiv
   if (verbose) {
      switch (st) {
      case BTE_READ_STREAM:
	 cout << "CONS read stream\n";
	 break;
      case BTE_WRITE_STREAM:
	 cout << "CONS read/write stream\n";
	 break;
       deafult:
	 cout << "CONS someother stream\n";
	 break;
      }
      print ("CONS ");
   }
#endif
}

// A constructor to create a temporary stream.
template < class T >
    BTE_stream_mmb < T >::BTE_stream_mmb (BTE_stream_type st)
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
    BTE_stream_mmb < T >::BTE_stream_mmb (BTE_stream_mmb * super_stream,
					  BTE_stream_type st,
					  off_t sub_begin, off_t sub_end)
{
   status_ = BTE_STREAM_STATUS_NO_STATUS;

   if (f_stats) {
      stats.record_create ();
   }
   // Reduce the number of streams avaialble.
   if (remaining_streams > 0) {
      remaining_streams--;
   } else if (remaining_streams == 0) {
      status_ = BTE_STREAM_STATUS_INVALID;
      return;
   }

   if (super_stream->status_ == BTE_STREAM_STATUS_INVALID) {
      status_ = BTE_STREAM_STATUS_INVALID;
      return;
   }

   if (super_stream->r_only && (st != BTE_READ_STREAM)) {
      status_ = BTE_STREAM_STATUS_INVALID;
      return;
   }
   // Rajiv
   initialize ();

   // Copy the relevant fields from the super_stream.
   fd = super_stream->fd;
   os_blocksize = super_stream->os_blocksize;
   logical_blocksize = super_stream->logical_blocksize;
   header = super_stream->header;
   f_filelen = super_stream->f_filelen;

   substream_level = super_stream->substream_level + 1;

   per = PERSIST_PERSISTENT;

   // The arguments sub_start and sub_end are logical item positions
   // within the stream.  We need to convert them to offsets within
   // the stream where items are found.

   off_t super_item_begin = file_off_to_item_off (super_stream->f_bos);

   f_bos = item_off_to_file_off (super_item_begin + sub_begin);
   f_eos = item_off_to_file_off (super_item_begin + sub_end + 1);

   if (f_eos > super_stream->f_eos) {
      status_ = BTE_STREAM_STATUS_INVALID;
      return;
   }

   f_offset = f_bos;

   curr_block = NULL;
   block_valid = 0;

   r_only = super_stream->r_only;
   w_only = super_stream->w_only;

   strncpy (path, super_stream->path, BTE_MMB_PATH_NAME_LEN);

   // substreams are considered to have no memory overhead!
}

// A psuedo-constructor for substreams.  This serves as a wrapper for
// the constructor above in order to get around the fact that one
// cannot have virtual constructors.
template < class T >
    BTE_err BTE_stream_mmb < T >::new_substream (BTE_stream_type st,
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

   BTE_stream_mmb < T > *sub =
       new BTE_stream_mmb < T > (this, st, sub_begin, sub_end);

   *sub_stream = (BTE_stream_base < T > *)sub;

   return BTE_ERROR_NO_ERROR;
}

template < class T > BTE_stream_mmb < T >::~BTE_stream_mmb (void)
{
   if (f_stats) {
      stats.record_delete ();
   }
   // If the stream is already invalid for some reason, then don't
   // worry about anything.
   if (status_ == BTE_STREAM_STATUS_INVALID) {
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
#ifdef VERBOSE
   // Rajiv
   if (verbose) {
      cout << "DELE logical eof=" << header->item_logical_eof << "\n";
   }
#endif
   assert (substream_level ||
	   header->item_logical_eof == file_off_to_item_off (f_eos));

   // free the header unless we are in a substream
   if (!substream_level) {
      unmap_header ();
      free (header);
   }
   // Unmap the current block if necessary.
   if (block_mapped) {
      unmap_current ();
   }
   // If this is not a substream then close the file.
   if (!substream_level) {
      // Rajiv
      // make sure the length of the file is correct
      if ((f_filelen > f_eos) &&
	  (ftruncate (fd, BLOCK_OFFSET (f_eos) + logical_blocksize) < 0)) {
	 os_errno = errno;
	 LOG_FATAL ("Failed to ftruncate() to the new end of \"");
	 LOG_FATAL (path);
	 LOG_FATAL ("\": ");
	 LOG_FATAL (strerror (os_errno));
	 LOG_FATAL ('\n');
	 LOG_FLUSH_LOG;
      }

      if (::close (fd)) {
	 os_errno = errno;
	 LOG_WARNING ("Failed to close() \"");
	 LOG_WARNING (path);
	 LOG_WARNING ("\": ");
	 LOG_WARNING (strerror (os_errno));
	 LOG_WARNING ('\n');
	 LOG_FLUSH_LOG;
      }
      // If it should not persist, unlink the file.
      if ((per == PERSIST_DELETE) && unlink (path)) {
	 os_errno = errno;
	 LOG_WARNING ("Failed to unlink() \"");
	 LOG_WARNING (path);
	 LOG_WARNING ("\": ");
	 LOG_WARNING (strerror (os_errno));
	 LOG_WARNING ('\n');
	 LOG_FLUSH_LOG;
      }
   }

   // Register memory deallocation before returning.
   if (!substream_level) {	// not a substream
      //register_memory_deallocation(os_blocksize); // for the header
      register_memory_deallocation (sizeof (mmap_stream_header));	// for the header.
      register_memory_deallocation (sizeof (BTE_stream_mmb < T >));
      register_memory_deallocation (BTE_MMB_MM_BUFFERS *
				    logical_blocksize);
   } else {
      // substreams are considered free!
   }

#ifdef VERBOSE
   if (verbose) {
      if (per == PERSIST_DELETE)
	 cout << "DELE unlinked file\n";
      if (substream_level) {
	 cout << "DELE substream destructor\n";
      }
      print ("DELE ");
   }
#endif

}

// pref = prefix
template < class T > void BTE_stream_mmb < T >::print (char *pref)
{
#ifdef COLLECT_STATS
#ifdef BTE_MMB_READ_AHEAD
   fprintf (stdout, "%sPFSTATS %d %d %d %d (fd=%d)\n",
	    pref, stats_hits, stats_misses,
	    stats_compulsory - stats_misses, stats_eos, fd);
   cout << pref << stats << " RWMUSCD\n";
#endif
#endif
   fprintf (stdout, "%sfile=%s", pref, path);
   fprintf (stdout, ", f_eos=%d, f_filelen=%d", f_eos, f_filelen);
   fprintf (stdout, ", length=%d\n",
	    file_off_to_item_off (f_eos) - file_off_to_item_off (f_bos));
   fprintf (stdout, "\n");
}

// f_eos points just past the last item ever written, so if f_current
// is there we are at the end of the stream and cannot read.

template < class T >
    B_INLINE BTE_err BTE_stream_mmb < T >::read_item (T ** elt)
{
   BTE_err bte_err;

   if (f_stats) {
      stats.record_read ();
   }

   if (w_only) {
#ifdef VERBOSE
      if (verbose)
	 cerr << "ERROR read on a write-only stream\n";
#endif
      return BTE_ERROR_WRITE_ONLY;
   }
   // Make sure we are not currently at the EOS.
   if (f_offset + sizeof (T) > f_eos) {
      //tp_assert(0, "Can't read past eos.");
      //LOG_WARNING("Reading past eos.\n");
      return BTE_ERROR_END_OF_STREAM;
   }
   // Validate the current block.
   if ((bte_err = validate_current ()) != BTE_ERROR_NO_ERROR) {
      return bte_err;
   }
   // Check and make sure that the current pointer points into the current
   // block.
   tp_assert (((char *) current - (char *) curr_block <=
	       logical_blocksize - sizeof (T)),
	      "current is past the end of the current block");
   tp_assert (((char *) current - (char *) curr_block >= 0),
	      "current is before the begining of the current block");

   *elt = current;		// Read
   advance_current ();		// move ptr to next elt

   // If we are in a substream, there should be no way for f_current to
   // pass f_eos.
   tp_assert (!substream_level || (f_offset <= f_eos),
	      "Got past eos in a substream.");

   return BTE_ERROR_NO_ERROR;
}

// f_eos points just past the last item ever written, so if f_current
// is there we are at the end of the stream and can only write if this
// is not a substream.
template < class T >
    B_INLINE BTE_err BTE_stream_mmb < T >::write_item (const T & elt)
{
   BTE_err bte_err;

   if (f_stats) {
      stats.record_write ();
   }
   // This better be a writable stream.
   if (r_only) {
      LOG_WARNING_ID ("write on a read-only stream\n");
      return BTE_ERROR_READ_ONLY;
   }
   // Make sure we are not currently at the EOS of a substream.
   if (substream_level && (f_eos <= f_offset)) {
      tp_assert (f_eos == f_offset, "Went too far in a substream.");
      return BTE_ERROR_END_OF_STREAM;
   }
   // Validate the current block.
   bte_err = validate_current ();
   if (bte_err != BTE_ERROR_NO_ERROR) {
      return bte_err;
   }
   // Check and make sure that the current pointer points into the current
   // block.
   tp_assert (((char *) current - (char *) curr_block <=
	       logical_blocksize - sizeof (T)),
	      "current is past the end of the current block");
   tp_assert (((char *) current - (char *) curr_block >= 0),
	      "current is before the begining of the current block");

   assert (current);
   *current = elt;		// write
   advance_current ();		// Advance the current pointer.

   // If we are in a substream, there should be no way for f_current
   // to pass f_eos.
   tp_assert (!substream_level || (f_offset <= f_eos),
	      "Got past eos in a substream.");

   // If we moved past eos, then update eos unless we are in a
   // substream, in which case EOS will be returned on the next call.
   if ((f_offset > f_eos) && !substream_level) {
      // I dont like this assert Rajiv
      // tp_assert(f_offset == f_eos + sizeof(T), "Advanced too far somehow.");
      tp_assert (f_offset <= f_filelen, "Advanced too far somehow.");
      f_eos = f_offset;
      // this is the only place f_eos is changed excluding 
      // constructors and truncate Rajiv
   }

   return BTE_ERROR_NO_ERROR;
}

// Query memory usage

// Note that in a substream we do not charge for the memory used by
// the header, since it is accounted for in the 0 level superstream.
template < class T >
    BTE_err BTE_stream_mmb < T >::main_memory_usage (size_t * usage,
						     MM_stream_usage
						     usage_type)
{
   switch (usage_type) {
   case MM_STREAM_USAGE_OVERHEAD:
      *usage = (sizeof (*this) +
		(((header == NULL) || substream_level) ? 0 :
		 os_blocksize));
      break;
   case MM_STREAM_USAGE_BUFFER:
      *usage = BTE_MMB_MM_BUFFERS * logical_blocksize;
      break;
   case MM_STREAM_USAGE_CURRENT:
      *usage = (sizeof (*this) +
		(((header == NULL) || substream_level) ? 0 :
		 os_blocksize) +
		((curr_block == NULL) ? 0 :
		 BTE_MMB_MM_BUFFERS * logical_blocksize));
      break;
   case MM_STREAM_USAGE_MAXIMUM:
      *usage = (sizeof (*this) + BTE_MMB_MM_BUFFERS * logical_blocksize +
		(substream_level ? 0 : os_blocksize));
      break;
   case MM_STREAM_USAGE_SUBSTREAM:
      *usage = (sizeof (*this) + BTE_MMB_MM_BUFFERS * logical_blocksize);
      break;
   }

   return BTE_ERROR_NO_ERROR;
};

// Return the number of items in the stream.
template < class T > off_t BTE_stream_mmb < T >::stream_len (void)
{
   return file_off_to_item_off (f_eos) - file_off_to_item_off (f_bos);
};

// Return the path name in newly allocated space.
template < class T >
    BTE_err BTE_stream_mmb < T >::name (char **stream_name)
{
   int len = strlen (path);

   tp_assert (len < BTE_MMB_PATH_NAME_LEN, "Path length is too long.");

   // Return the path name in newly allocated space.

   char *new_path = new char[len + 1];

   strncpy (new_path, path, len + 1);

   *stream_name = new_path;

   return BTE_ERROR_NO_ERROR;
};

// Move to a specific position.
template < class T > BTE_err BTE_stream_mmb < T >::seek (off_t offset)
{
   BTE_err be;
   off_t new_offset;

   if (f_stats) {
      stats.record_seek ();
   }
   // Looks like we can only seek within the file Rajiv
   if ((offset < 0) ||
       (offset >
	file_off_to_item_off (f_eos) - file_off_to_item_off (f_bos))) {
      return BTE_ERROR_OFFSET_OUT_OF_RANGE;
   }
   // Compute the new offset
   new_offset =
       item_off_to_file_off (file_off_to_item_off (f_bos) + offset);
   if (r_only) {
      tp_assert (new_offset <= f_eos, "Advanced too far somehow.");
   }
   // // If it is not in the same block as the current position then
   // // invalidate the current block.
   //if (((new_offset - os_blocksize) / logical_blocksize) !=
   //    ((f_offset - os_blocksize) / logical_blocksize)) {

   // The above was the old code which was wrong: we also need to check that
   // we have the correct block mapped in (f_offset does not always point into
   // the current block!)

   if (((char *) current - (char *) curr_block >= logical_blocksize) ||
       (((new_offset - os_blocksize) / logical_blocksize) !=
	((f_offset - os_blocksize) / logical_blocksize))) {
      if (block_valid) {
	 if ((be = invalidate_current ()) != BTE_ERROR_NO_ERROR)
	    return be;
      }
   } else {
      if (block_valid) {

	 // We have to adjust current.

	 register off_t internal_block_offset;

	 internal_block_offset = file_off_to_item_off (new_offset) %
	     (logical_blocksize / sizeof (T));

	 current = curr_block + internal_block_offset;
      }
   }

   f_offset = new_offset;

   return BTE_ERROR_NO_ERROR;
}

// Truncate the stream.
template < class T > BTE_err BTE_stream_mmb < T >::truncate (off_t offset)
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

   if (((char *) current - (char *) curr_block >= logical_blocksize) ||
       (((new_offset - os_blocksize) / logical_blocksize) !=
	((f_offset - os_blocksize) / logical_blocksize))) {
      if (block_valid) {
	 if ((be = invalidate_current ()) != BTE_ERROR_NO_ERROR)
	    return be;
      }
   }
   // If it is not in the same block as the current end of stream
   // then truncate the file to the end of the new last block.
   if (((new_offset - os_blocksize) / logical_blocksize) !=
       ((f_eos - os_blocksize) / logical_blocksize)) {

      // Determine the offset of the block that new_offset is in.
      block_offset = BLOCK_OFFSET (new_offset);
      // Rajiv 
      // ((new_offset - os_blocksize) / logical_blocksize)
      // * logical_blocksize + os_blocksize;
      f_filelen = block_offset + logical_blocksize;
      if (ftruncate (fd, f_filelen)) {
	 os_errno = errno;
	 LOG_FATAL ("Failed to ftruncate() to the new end of \"");
	 LOG_FATAL (path);
	 LOG_FATAL ("\": ");
	 LOG_FATAL (strerror (os_errno));
	 LOG_FATAL ('\n');
	 LOG_FLUSH_LOG;
	 return BTE_ERROR_OS_ERROR;
      }
   }
   // Reset the current position to the end.    
   f_offset = f_eos = new_offset;

   return BTE_ERROR_NO_ERROR;
}

// Map in the header from the file.  This assumes that the path
// has been cached in path and that the file has been opened and
// fd contains a valid descriptor.
template < class T >
    mmap_stream_header * BTE_stream_mmb < T >::map_header (void)
{
   off_t file_end;
   mmap_stream_header *mmap_hdr;

   // If the underlying file is not at least long enough to contain
   // the header block, then, assuming the stream is writable, we have
   // to create the space on disk by doing an explicit write().
   if ((file_end = lseek (fd, 0, SEEK_END)) < os_blocksize) {
      if (r_only) {
	 status_ = BTE_STREAM_STATUS_INVALID;

	 LOG_FATAL ("No header block in read only stream \"");
	 LOG_FATAL (path);
	 LOG_FATAL ('\n');
	 LOG_FLUSH_LOG;
	 return NULL;

      } else {
	 // A writable stream, so we can ftruncate() space for a
	 // header block.
	 if (ftruncate (fd, os_blocksize)) {
	    os_errno = errno;
	    LOG_FATAL ("Failed to ftruncate() to end of header of \"");
	    LOG_FATAL (path);
	    LOG_FATAL ("\": ");
	    LOG_FATAL (strerror (os_errno));
	    LOG_FATAL ('\n');
	    LOG_FLUSH_LOG;
	    return NULL;
	 }
      }
   }

   // Map in the header block.  If the stream is writable, the header 
   // block should be too.
   // took out the SYSTYPE_BSD ifdef for convenience
   // changed from MAP_FIXED to MAP_VARIABLE because we are using NULL
   mmap_hdr = (mmap_stream_header *)
       (call_mmap ((NULL), sizeof (mmap_stream_header),
		   r_only, w_only, fd, 0, 0));
   if (mmap_hdr == (mmap_stream_header *) (-1)) {
      status_ = BTE_STREAM_STATUS_INVALID;
      os_errno = errno;
      log_fatal ("mmap() failed to map in header from \"",
		 path, "\": ", strerror (os_errno), "\n");
      return NULL;
   }

   header = (mmap_stream_header *) malloc (sizeof (mmap_stream_header));
   if (!header) {
      LOG_FATAL ("out of memory");
      return NULL;
   }
   memcpy (header, mmap_hdr, sizeof (mmap_stream_header));
   call_munmap (mmap_hdr, sizeof (mmap_stream_header));

   return header;
}

// Map in the header from the file.  This assumes that the path
// has been cached in path and that the file has been opened and
// fd contains a valid descriptor.
template < class T > void BTE_stream_mmb < T >::unmap_header ()
{
   mmap_stream_header *mmap_hdr;
   off_t file_end;

   // If the underlying file is not at least long enough to contain
   // the header block, then, assuming the stream is writable, we have
   // to create the space on disk by doing an explicit write().
   if ((file_end = lseek (fd, 0, SEEK_END)) < os_blocksize) {
      if (r_only) {
	 status_ = BTE_STREAM_STATUS_INVALID;

	 LOG_FATAL ("No header block in read only stream \"");
	 LOG_FATAL (path);
	 LOG_FATAL ('\n');
	 LOG_FLUSH_LOG;
	 return;

      } else {
	 // A writable stream, so we can ftruncate() space for a
	 // header block.
	 if (ftruncate (fd, os_blocksize)) {
	    os_errno = errno;
	    LOG_FATAL ("Failed to ftruncate() to end of header of \"");
	    LOG_FATAL (path);
	    LOG_FATAL ("\": ");
	    LOG_FATAL (strerror (os_errno));
	    LOG_FATAL ('\n');
	    LOG_FLUSH_LOG;
	    return;
	 }
      }
   }

   // Map in the header block.  If the stream is writable, the header 
   // block should be too.
   // took out the SYSTYPE_BSD ifdef for convenience
   // changed from MAP_FIXED to MAP_VARIABLE because we are using NULL
   mmap_hdr = (mmap_stream_header *)
       (call_mmap ((NULL), sizeof (mmap_stream_header),
		   r_only, w_only, fd, 0, 0));
   if (mmap_hdr == (mmap_stream_header *) (-1)) {
      status_ = BTE_STREAM_STATUS_INVALID;
      os_errno = errno;
      log_fatal ("mmap() failed to map in header from \"",
		 path, "\": ", strerror (os_errno), "\n");
      return;
   }

   memcpy (mmap_hdr, header, sizeof (mmap_stream_header));
   call_munmap (mmap_hdr, sizeof (mmap_stream_header));
}

//
// Make sure the current block is mapped in and all internal pointers are
// set as appropriate.  
// 
// 

template < class T >
    inline BTE_err BTE_stream_mmb < T >::validate_current (void)
{
   int block_space;		// The space left in the current block.
   BTE_err bte_err;

   // If the current block is valid and current points into it and has
   // enough room in the block for a full item, we are fine.  If it is
   // valid but there is not enough room, invalidate it.
   if (block_valid) {
      assert (current);		// sanity check - rajiv
      if ((block_space = logical_blocksize -
	   ((char *) current - (char *) curr_block)) >= sizeof (T)) {
	 return BTE_ERROR_NO_ERROR;
      } else {			// Not enough room left.
	 // no real need to call invalidate here 
	 // since we call map_current anyway Rajiv
	 if ((bte_err = invalidate_current ()) != BTE_ERROR_NO_ERROR) {
	    return bte_err;
	 }
	 f_offset += block_space;
      }
   }
   // The current block is invalid, since it was either invalid to start
   // with or we just invalidated it because we were out of space.

   tp_assert (!block_valid, "Block is already mapped in.");

   // Now map it the block.
   bte_err = map_current ();
   assert (current);

#ifdef VERBOSE
   // Rajiv
   if (verbose && bte_err != BTE_ERROR_NO_ERROR)
      cerr << "validate_current failed\n";
#endif

   // Rajiv
   tp_assert (f_offset + sizeof (T) <= f_filelen,
	      "Advanced too far somehow.");
   return bte_err;
}

// Map in the current block.
// f_offset is used to determine what block is needed.
template < class T > BTE_err BTE_stream_mmb < T >::map_current (void)
{
   off_t block_offset;
   int do_mmap = 0;
   BTE_err err;

   // We should not currently have a valid block.
   tp_assert (!block_valid, "Block is already mapped in.");

   if (f_stats) {
      stats.record_map ();
   }
   // Determine the offset of the block that the current item is in.
   block_offset = BLOCK_OFFSET (f_offset);
   // Rajiv
   // - os_blocksize) / logical_blocksize)
   // * logical_blocksize + os_blocksize;

   // If the block offset is beyond the logical end of the file, then
   // we either record this fact and return (if the stream is read
   // only) or ftruncate() out to the end of the current block.
   assert (lseek (fd, 0, SEEK_END) == f_filelen);
   // removed -1 from rhs of comparison below Rajiv
   if (f_filelen < block_offset + logical_blocksize) {
      if (r_only) {
	 //LOG_WARNING_ID("hit eof while reading\n");
	 return BTE_ERROR_END_OF_STREAM;
      } else {
	 err = grow_file (block_offset);
	 if (err != BTE_ERROR_NO_ERROR)
	    return err;
      }
   }
   // this is what we just fixed. Rajiv
   tp_assert (f_offset + sizeof (T) <= f_filelen,
	      "Advanced too far somehow.");

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

   // Map it in either r/w or read only.  
#ifdef BTE_MMB_READ_AHEAD
   if (have_next_block && (block_offset == f_next_block)) {
      T *temp;

      temp = curr_block;
      curr_block = next_block;
      next_block = temp;
      have_next_block = 0;
#ifdef COLLECT_STATS
      stats_hits++;
#endif
   } else {
#ifdef COLLECT_STATS
      if (have_next_block) {
	 // not sequential access
	 //munmap((caddr_t)next_block, logical_blocksize);
	 //have_next_block = 0;
	 //next_block = NULL;
	 stats_misses++;
      }
      stats_compulsory++;
#endif
      do_mmap = 1;
   }
#else
   do_mmap = 1;
#endif
   if (do_mmap) {
      // took out the SYSTYPE_BSD ifdef for convenience
      // MAP_VARIABLE the first time round
      // (curr_block ? MAP_FIXED : MAP_VARIABLE) |
      curr_block = (T *) (call_mmap (curr_block, logical_blocksize,
				     r_only, w_only, fd, block_offset,
				     (curr_block != NULL)));
      block_mapped = 1;
   }
   assert ((void *) curr_block != (void *) header);

   if (curr_block == (T *) (-1)) {
      status_ = BTE_STREAM_STATUS_INVALID;
      os_errno = errno;
      LOG_FATAL ("mmap() failed to map in block at ");
      LOG_FATAL (block_offset);
      LOG_FATAL (" from \"");
      LOG_FATAL (path);
      LOG_FATAL ("\": ");
      LOG_FATAL (strerror (os_errno));
      LOG_FATAL ('\n');
      LOG_FLUSH_LOG;
      perror ("mmap failed");	// Rajiv
      return BTE_ERROR_OS_ERROR;
   }

   block_valid = 1;

#ifdef BTE_MMB_READ_AHEAD
   // Start the asyncronous read of the next logical block.
   read_ahead ();
#endif

   // The offset, in terms of number of items, that current should
   // have relative to curr_block.

   register off_t internal_block_offset;

   internal_block_offset = file_off_to_item_off (f_offset) %
       (logical_blocksize / sizeof (T));

   current = curr_block + internal_block_offset;
   assert (current);
   return BTE_ERROR_NO_ERROR;
}

template < class T >
    inline BTE_err BTE_stream_mmb < T >::invalidate_current (void)
{
   // We should currently have a valid block.
   tp_assert (block_valid, "No block is mapped in.");
   block_valid = 0;

   return BTE_ERROR_NO_ERROR;
}

template < class T > BTE_err BTE_stream_mmb < T >::unmap_current (void)
{
   invalidate_current ();	// not really necessary

   if (f_stats) {
      stats.record_unmap ();
   }
   // Unmap it.

   if (call_munmap (curr_block, logical_blocksize)) {
      status_ = BTE_STREAM_STATUS_INVALID;
      os_errno = errno;

      LOG_FATAL ("munmap() failed to unmap current block");
      LOG_FATAL ("\": ");
      LOG_FATAL (strerror (os_errno));
      LOG_FATAL ('\n');
      LOG_FLUSH_LOG;
      return BTE_ERROR_OS_ERROR;
   }
   curr_block = NULL;		// to be safe
   block_mapped = 0;

   block_valid = 0;

#if 0
   fsync (fd);			// Just for fun, fsync it.
#endif

   return BTE_ERROR_NO_ERROR;
}

// A uniform method for advancing the current pointer.  No mapping,
// unmapping, or anything like that is done here.
template < class T >
    inline BTE_err BTE_stream_mmb < T >::advance_current (void)
{

   tp_assert (f_offset <= f_filelen, "Advanced too far somehow.");

   // Advance the current pointer and the file offset of the current
   // item.
   current++;
   f_offset += sizeof (T);

   return BTE_ERROR_NO_ERROR;
}

#define MAX(a,b) ((a)>(b)?(a):(b))
// increase the length of the file, to at least 
// block_offset + logical_blocksize
template < class T > inline BTE_err
    BTE_stream_mmb < T >::grow_file (off_t block_offset)
{
   // can't grow substreams
   assert (!substream_level);

   // Rajiv    
   // make a note of the new file length
   // f_filelen = block_offset + logical_blocksize;
   // XXX
   f_filelen = MAX (BLOCK_OFFSET (f_filelen * 2),
		    block_offset + logical_blocksize);
   if (ftruncate (fd, f_filelen) < 0) {
      os_errno = errno;
      LOG_FATAL ("Failed to ftruncate() out a new block of \"");
      LOG_FATAL (path);
      LOG_FATAL ("\": ");
      LOG_FATAL (strerror (os_errno));
      LOG_FATAL ('\n');
      LOG_FLUSH_LOG;
      // Rajiv
      //cerr << "map_current: ftruncate\n";
      return BTE_ERROR_END_OF_STREAM;	// generate an error Rajiv
   }
   assert (lseek (fd, 0, SEEK_END) == f_filelen);
   return BTE_ERROR_NO_ERROR;
}

// Register memory usage with the memory manager.
template < class T >
    inline BTE_err BTE_stream_mmb <
    T >::register_memory_allocation (size_t sz)
{
   MM_err mme;

   if ((mme = MM_manager.register_allocation (sz)) != MM_ERROR_NO_ERROR) {
      status_ = BTE_STREAM_STATUS_INVALID;

      LOG_FATAL ("Memory manager error in allocation.\n");
      LOG_FLUSH_LOG;
      return BTE_ERROR_MEMORY_ERROR;
   }

   return BTE_ERROR_NO_ERROR;
}

template < class T >
    inline BTE_err BTE_stream_mmb <
    T >::register_memory_deallocation (size_t sz)
{
   MM_err mme;

   if ((mme = MM_manager.register_deallocation (sz)) != MM_ERROR_NO_ERROR) {
      status_ = BTE_STREAM_STATUS_INVALID;

      LOG_FATAL ("Memory manager error in deallocation. (");
      LOG_FATAL (sz);
      LOG_FATAL (" bytes)\n");
      LOG_FLUSH_LOG;
      return BTE_ERROR_MEMORY_ERROR;
   }

   return BTE_ERROR_NO_ERROR;
}

template < class T >
    inline off_t BTE_stream_mmb <
    T >::item_off_to_file_off (off_t item_off)
{
   off_t file_off;

   // Move past the header.

   file_off = os_blocksize;

   // Add logical_blocksize for each full block.

   file_off += logical_blocksize *
       (item_off / (logical_blocksize / sizeof (T)));

   // Add sizeof(T) for each item in the partially full block.

   file_off += sizeof (T) * (item_off % (logical_blocksize / sizeof (T)));

   return file_off;
}

template < class T >
    inline off_t BTE_stream_mmb <
    T >::file_off_to_item_off (off_t file_off)
{
   off_t item_off;

   // Subtract off the header.

   file_off -= os_blocksize;

   // Account for the full blocks.

   item_off = (logical_blocksize / sizeof (T)) *
       (file_off / logical_blocksize);

   // Add in the number of items in the last block.

   item_off += (file_off % logical_blocksize) / sizeof (T);

   return item_off;
}

template < class T > int BTE_stream_mmb < T >::available_streams (void)
{
   return remaining_streams;
}

template < class T > off_t BTE_stream_mmb < T >::chunk_size (void)
{
   return logical_blocksize / sizeof (T);
}

template < class T > void BTE_stream_mmb < T >::persist (persistence p)
{
   per = p;
}

#ifdef BTE_MMB_READ_AHEAD

template < class T > void BTE_stream_mmb < T >::read_ahead (void)
{

   off_t f_curr_block;

   // The current block had better already be valid or we made a
   // mistake in being here.

   tp_assert (block_valid,
	      "Trying to read ahead when current block is invalid.");

   // Check whether there is a next block.  If we are already in the
   // last block of the file then it makes no sense to read ahead.
   // What if we are writing?? Rajiv
   f_curr_block = ((f_offset - os_blocksize) / logical_blocksize) *
       logical_blocksize + os_blocksize;

   if (f_eos < f_curr_block + 2 * logical_blocksize) {
      return;			// XXX
// need to fix this    
      // if not read only, we can extend the file and prefetch.
      // (only if not a substream)
      // Rajiv
      // prefetch only if write only and not substream
      if (!w_only || substream_level) {
#ifdef COLLECT_STATS
	 stats_eos++;
#endif
	 return;
      }
      if (w_only &&
	  !substream_level &&
	  (f_curr_block + 2 * logical_blocksize > f_filelen)) {
#ifdef VERBOSE
	 if (verbose)
	    cout << "growing file (fd" << fd << ") in advance\n";
#endif
	 grow_file (f_curr_block);
      }
   }

   f_next_block = f_curr_block + logical_blocksize;

   // Rajiv
   assert (f_next_block + logical_blocksize <= f_filelen);
   assert (next_block != curr_block);
#if !USE_LIBAIO
   // took out the SYSTYPE_BSD ifdef for readability Rajiv
   next_block = (T *) (call_mmap (next_block, logical_blocksize,
				  r_only, w_only,
				  fd, f_next_block, (next_block != NULL)));
   assert (next_block != (T *) - 1);
   have_next_block = 1;
#endif				// !USE_LIBAIO

#if USE_LIBAIO
   // Asyncronously read the first word of each os block in the next
   // logical block.
   for (unsigned int ii = 0; ii < BTE_MMB_LOGICAL_BLOCKSIZE_FACTOR; ii++) {

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

	    LOG_FATAL ("aioread() failed to read ahead");
	    LOG_FATAL ("\": ");
	    LOG_FATAL (strerror (os_errno));
	    LOG_FATAL ('\n');
	    LOG_FLUSH_LOG;
	 }
      }
   }
#endif				// USE_LIBAIO
}

#endif				// BTE_MMB_READ_AHEAD

#undef BTE_MMB_MM_BUFFERS

#endif	// _BTE_STREAM_MMAP_H
