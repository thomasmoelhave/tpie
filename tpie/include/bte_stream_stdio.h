//
// File: bte_stream_stdio.h (formerly bte_stdio.h)
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/11/94
//
// $Id: bte_stream_stdio.h,v 1.1 2002-01-06 18:46:31 tavi Exp $
//
#ifndef _BTE_STREAM_STDIO_H
#define _BTE_STREAM_STDIO_H

#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <tpie_log.h>
#include <tpie_assert.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>

// Include the registration based memory manager.
#define MM_IMP_REGISTER
#include <mm.h>

#include <bte_stream_base.h>

// File system streams are streams in a special format that is designed 
// to be stored in an ordinary file in a UN*X file system.  They are 
// predominatly designed to be used to store streams in a persistent way.
// They may disappear in later versions as persistence becomes an integral
// part of TPIE.
//
// For simplicity, we work through the standard C I/O library (stdio).
//

// A base class for all BTE_stream_stdio<T> classes.
class BTE_stream_stdio_base {
 protected:
   static int remaining_streams;
};

// A BTE_STDIO file contains a header in a block by itself and then
// some number of items.

#define BTE_STDIO_HEADER_MAGIC_NUMBER	0x666777

// The header of a file system stream.
typedef struct BTE_stdio_header_v1 {
   unsigned int magic_number;	// Set to BTE_STDIO_HEADER_MAGIC_NUMBER
   unsigned int version;	// Should be 1 for current version.
   unsigned int length;		// # of bytes in this structure.
   unsigned int block_length;	// # of bytes in a block.
   size_t item_size;		// The size of each item in the stream.
} BTE_stdio_header;

#define BTE_STDIO_PATH_NAME_LEN 128

//
// A class of BTE streams implimented using ordinary stdio
// semantics.
//
template < class T > class BTE_stream_stdio:
public BTE_stream_base < T >, public BTE_stream_stdio_base {
 private:
   FILE * file;
   BTE_stdio_header header;

   unsigned int substream_level;

   int r_only;			// Non-zero if this stream was opened for reading 

   // only. 

   int os_errno;   // A place to cache OS error values.  It is normally
                   // set after each call to the OS.

   char path[BTE_STDIO_PATH_NAME_LEN];

   // If this stream is actually a substream, these will be set to
   // indicate the portion of the file that is part of this stream.
   // If the stream is the whole file, they will be set to -1.
   off_t logical_bos;
   off_t logical_eos;

   inline BTE_err register_memory_allocation (size_t sz);
   inline BTE_err register_memory_deallocation (size_t sz);

   persistence per; // The persistence status of this stream.

  // Read and check the header; used by constructors
   int readcheck_header ();

 public:
   T read_tmp;

   // Constructors
   BTE_stream_stdio (const char *dev_path, const BTE_stream_type st);
   BTE_stream_stdio (const BTE_stream_type st);
   BTE_stream_stdio (const BTE_stream_stdio < T > &s);

   // A psuedo-constructor for substreams.
   BTE_err new_substream (BTE_stream_type st, off_t sub_begin,
			  off_t sub_end,

			  BTE_stream_base < T > **sub_stream);

   ~BTE_stream_stdio (void);

   BTE_err read_item (T ** elt);
   BTE_err write_item (const T & elt);
   int read_only (void) {
      return r_only;
   };

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

   int available_streams (void);

   off_t chunk_size (void);

   // Tell the stream whether to leave its data on the disk or not
   // when it is destructed.
   void persist (persistence);

   //return the os block size 
   unsigned int get_block_length ();

};

template < class T >
    BTE_stream_stdio < T >::BTE_stream_stdio (const char *dev_path,
					      const BTE_stream_type st)
{
   BTE_err berr;

   // Reduce the number of streams avaialble.
   if (remaining_streams == 0) {
      status_ = BTE_STREAM_STATUS_INVALID;
      return;
   }
   if (remaining_streams > 0) {
      remaining_streams--;
   }
   // Cache the path name
   if (strlen (dev_path) > BTE_STDIO_PATH_NAME_LEN - 1) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL_ID("Path name too long:");
      LOG_FATAL_ID(dev_path);
      return;
   }
   strncpy (path, dev_path, BTE_STDIO_PATH_NAME_LEN);
   status_ = BTE_STREAM_STATUS_NO_STATUS;

   // Not a substream.
   substream_level = 0;
   logical_bos = logical_eos = -1;

   // By default, all streams are deleted at destruction time.
   per = PERSIST_DELETE;

   switch (st) {
   case BTE_READ_STREAM:
      // Open the file for reading.
      r_only = 1;
      if ((file = fopen (dev_path, "rb")) == NULL) {
	 status_ = BTE_STREAM_STATUS_INVALID;
	 LOG_FATAL_ID("Failed to open file:");
	 LOG_FATAL_ID(dev_path);
	 return;
      }
      //read and check header
      if (readcheck_header () == -1) {
	 LOG_FATAL_ID("Bad header.");
	 return;
      }
      // Seek past the end of the first block.
      if (fseek (file, header.block_length, 0) == -1) {
	 LOG_FATAL_ID("fseek failed.");
	 return;
      }
      break;

#if(0)
   case BTE_WRITE_STREAM:
      // Open the file for writing
      r_only = 0;
      if ((file = fopen (dev_path, "rb+")) == NULL) {
	 status_ = BTE_STREAM_STATUS_INVALID;
	 LOG_FATAL_ID("Failed to open file:");
	 LOG_FATAL_ID(dev_path);
	 return;
      }
      // Create and write the header
      header.magic_number = BTE_STDIO_HEADER_MAGIC_NUMBER;
      header.version = 1;
      header.length = sizeof (header);
      header.block_length = get_block_length ();
      header.item_size = sizeof (T);
      if (fwrite ((char *) &header, sizeof (header), 1, file) != 1) {
	 status_ = BTE_STREAM_STATUS_INVALID;
	 LOG_FATAL_ID("Failed to write header to file:");
	 LOG_FATAL_ID(dev_path);
	 return;
      }
      //truncate the file to header block
      BTE_err er;

      if ((er = this->truncate (0)) != BTE_ERROR_NO_ERROR) {
	 LOG_FATAL_ID("Cannot truncate in file:");
	 LOG_FATAL_ID(dev_path);
	 return;
      }
      if ((er = this->seek (0)) != BTE_ERROR_NO_ERROR) {
	 LOG_FATAL_ID("Cannot seek in file:");
	 LOG_FATAL_ID(dev_path);
	 return;
      }
      break;
#endif

   case BTE_WRITE_STREAM:
   case BTE_WRITEONLY_STREAM:
   case BTE_APPEND_STREAM:
      // Open the file for appending.
      r_only = 0;
      if ((file = fopen (dev_path, "rb+")) == NULL) {
	 //file does not  exist - create it
	 if ((file = fopen (dev_path, "wb+")) == NULL) {
	    status_ = BTE_STREAM_STATUS_INVALID;
	    LOG_FATAL_ID("Failed to open file:");
	    LOG_FATAL_ID(dev_path);
	    return;
	 }
	 // Create and write the header
	 header.magic_number = BTE_STDIO_HEADER_MAGIC_NUMBER;
	 header.version = 1;
	 header.length = sizeof (header);
	 header.block_length = get_block_length ();
	 header.item_size = sizeof (T);
	 if (fwrite ((char *) &header, sizeof (header), 1, file) != 1) {
	    status_ = BTE_STREAM_STATUS_INVALID;
	    LOG_FATAL_ID("Failed to write header to file:");
	    LOG_FATAL_ID(dev_path);
	    return;
	 }
	 //truncate the file to header block
	 BTE_err er;

	 if ((er = this->truncate (0)) != BTE_ERROR_NO_ERROR) {
	    LOG_FATAL_ID("Cannot truncate in file:");
	    LOG_FATAL_ID(dev_path);
	    return;
	 }
	 if ((er = this->seek (0)) != BTE_ERROR_NO_ERROR) {
	    LOG_FATAL_ID("Cannot seek in file:");
	    LOG_FATAL_ID(dev_path);
	    return;
	 }

      } else {
	 //file exists - read and check header
	 if (readcheck_header () == -1) {
	    LOG_FATAL_ID("Bad header.");
	    return;
	 }
	 // Seek to the end of the stream  if BTE_APPEND STREAM
	 if (st == BTE_APPEND_STREAM) {
	    if (fseek (file, 0, SEEK_END)) {
	       status_ = BTE_STREAM_STATUS_INVALID;
	       LOG_FATAL_ID("Failed to go to EOF of file:");
	       LOG_FATAL_ID(dev_path);
	       return;
	    }
	    // Make sure there was at least a full block there to pass.
	    if ((unsigned) ftell (file) < (unsigned) header.block_length) {
	       status_ = BTE_STREAM_STATUS_INVALID;
	    }
	 } else {
	    // seek to 0 if  BTE_WRITE_STREAM
	    if ((berr = this->seek (0)) != BTE_ERROR_NO_ERROR) {
	       LOG_FATAL_ID("Cannot seek in file:");
	       LOG_FATAL_ID(dev_path);
	       return;
	    }
	 }
      }
      break;

   default:
      // Either a bad value or a case that has not been implemented
      // yet.
      LOG_WARNING_ID("Bad or unimplemented case.");
      status_ = BTE_STREAM_STATUS_INVALID;
      break;
   }

   // Register memory usage before returning.
   register_memory_allocation (sizeof (BTE_stream_stdio < T >));
   // A quick and dirty guess.  One block in the buffer cache, one in
   // user space.
   register_memory_allocation (header.block_length * 2);

}

// A psuedo-constructor for substreams.  This allows us to get around
// the fact that one cannot have virtual constructors.
template < class T >
    BTE_err BTE_stream_stdio < T >::new_substream (BTE_stream_type st,
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

   if (substream_level) {
      if (
	  (sub_begin * sizeof (T) >=
	   (unsigned) (logical_eos - logical_bos))
	  || (sub_end * sizeof (T) >
	      (unsigned) (logical_eos - logical_bos))) {
	 *sub_stream = NULL;
	 return BTE_ERROR_OFFSET_OUT_OF_RANGE;
      }
   }
   // We actually have to completely reopen the file in order to get
   // another seek pointer into it.  We'll do this by constructing
   // the stream that will end up being the substream.
   BTE_stream_stdio *sub = new BTE_stream_stdio (path, st);

   // Set up the beginning and end positions.
   if (substream_level) {
      sub->logical_bos = logical_bos + sub_begin * sizeof (T);
      sub->logical_eos = logical_bos + (sub_end + 1) * sizeof (T);
   } else {
      sub->logical_bos = sub->header.block_length + sub_begin * sizeof (T);
      sub->logical_eos =
	  sub->header.block_length + (sub_end + 1) * sizeof (T);
   }

   // Set the current position.
   fseek (sub->file, sub->logical_bos, 0);

   sub->substream_level = substream_level + 1;
   sub->per =
       (per == PERSIST_READ_ONCE) ? PERSIST_READ_ONCE : PERSIST_PERSISTENT;
   *sub_stream = (BTE_stream_base < T > *)sub;

   return BTE_ERROR_NO_ERROR;
}

template < class T > BTE_stream_stdio < T >::BTE_stream_stdio (BTE_stream_type	/*st */
    )
{

   tp_assert (0, "This constructor is under construction");
   status_ = BTE_STREAM_STATUS_INVALID;
   exit (1);
   // This function has not yet been implemented.
   // Generate a unique name for the file to be created.
   // Create the file with the appropriate type.
}

template < class T > BTE_stream_stdio < T >::~BTE_stream_stdio (void)
{

   fclose (file);
   // Get rid of the file if not persistent and if not substream.
   if ((per != PERSIST_PERSISTENT) && (substream_level == 0)) {
      if (unlink (path)) {
	 os_errno = errno;
	 LOG_WARNING_ID("Failed to unlink() file:");
	 LOG_WARNING_ID(path);
	 LOG_WARNING_ID(strerror(os_errno));
      }
   }
   // Register memory deallocation before returning.
   register_memory_deallocation (sizeof (BTE_stream_stdio < T >));

   // A quick and dirty guess.  One block in the buffer cache, one in
   // user space.
   register_memory_deallocation (header.block_length * 2);

   if (remaining_streams >= 0) {
      remaining_streams++;
   }

}

template < class T > BTE_err BTE_stream_stdio < T >::read_item (T ** elt)
{
   int stdio_ret;
   BTE_err ret;

   if ((logical_eos >= 0) && (ftell (file) >= logical_eos)) {
      tp_assert ((logical_bos >= 0), "eos set but bos not.");
      status_ = BTE_STREAM_STATUS_END_OF_STREAM;
      ret = BTE_ERROR_END_OF_STREAM;
   } else {

      stdio_ret = fread ((char *) (&read_tmp), sizeof (T), 1, file);

      if (stdio_ret == 1) {
	 *elt = &read_tmp;
	 ret = BTE_ERROR_NO_ERROR;
      } else {
	 // Assume EOF.  Fix this later.
	 status_ = BTE_STREAM_STATUS_END_OF_STREAM;
	 ret = BTE_ERROR_END_OF_STREAM;
      }
   }

   return ret;
}

template < class T >
    BTE_err BTE_stream_stdio < T >::write_item (const T & elt)
{
   int stdio_ret;
   BTE_err ret;

   if ((logical_eos >= 0) && (ftell (file) > logical_eos)) {
      tp_assert ((logical_bos >= 0), "eos set but bos not.");
      status_ = BTE_STREAM_STATUS_END_OF_STREAM;
      ret = BTE_ERROR_END_OF_STREAM;
   } else {
      //printf("write_item: stream_len is %d\n", this->stream_len());
      stdio_ret = fwrite ((char *) &elt, sizeof (T), 1, file);
      if (stdio_ret == 1) {
	 ret = BTE_ERROR_NO_ERROR;
      } else {
	 LOG_FATAL_ID("write_item failed.");
	 //assert(0);
	 status_ = BTE_STREAM_STATUS_INVALID;
	 ret = BTE_ERROR_IO_ERROR;
      }
   }
   return ret;
}

// Register memory usage with the memory manager.
template < class T >
    inline BTE_err BTE_stream_stdio <
    T >::register_memory_allocation (size_t sz)
{
   MM_err mme;

   if ((mme = MM_manager.register_allocation (sz)) != MM_ERROR_NO_ERROR) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL_ID("Memory manager error in allocation.");
      return BTE_ERROR_MEMORY_ERROR;
   }

   return BTE_ERROR_NO_ERROR;
}

template < class T >
    inline BTE_err BTE_stream_stdio <
    T >::register_memory_deallocation (size_t sz)
{
   MM_err mme;

   if ((mme = MM_manager.register_deallocation (sz)) != MM_ERROR_NO_ERROR) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL_ID("Memory manager error in deallocation.");
      return BTE_ERROR_MEMORY_ERROR;
   }

   return BTE_ERROR_NO_ERROR;
}

template < class T >
    BTE_err BTE_stream_stdio < T >::main_memory_usage (size_t * usage,
						       MM_stream_usage
						       usage_type)
{
   switch (usage_type) {
   case MM_STREAM_USAGE_OVERHEAD:
      *usage = sizeof (*this);
      break;
   case MM_STREAM_USAGE_BUFFER:
      *usage = 2 * header.block_length;
      break;
   case MM_STREAM_USAGE_CURRENT:
   case MM_STREAM_USAGE_MAXIMUM:
   case MM_STREAM_USAGE_SUBSTREAM:
      *usage = sizeof (*this) + 2 * header.block_length;
      break;
   }

   return BTE_ERROR_NO_ERROR;
}

// Return the number of items in the stream.
template < class T > off_t BTE_stream_stdio < T >::stream_len (void)
{

   if (substream_level) {	// We are in a substream.
      return (logical_eos - logical_bos) / sizeof (T);
   } else {
      // There must be a way to get this information directly,
      // instead of fseeking around.

      // Where are we now?
      off_t current = ftell (file);

      // Go to the end and see where we are.
      fseek (file, 0, SEEK_END);
      off_t end = ftell (file);

      // Go back.
      fseek (file, current, SEEK_SET);

      // Lars May 22, 1997: This is a quick hack to fix a problem
      // with headers of length less than a block. That shouldn't 
      // be possible but there is a bug somewhere
      // - Look at it later (seems to have something to do with
      //   substreams. Header block is not truncated).
      if (end < (int) header.block_length) {
	 //printf("shouldnt be here! possible bug\n);
	 return 0;
      } else {
	 return (end - header.block_length) / sizeof (T);
      };
   }
}

// Return the path name in newly allocated space.
template < class T >
    BTE_err BTE_stream_stdio < T >::name (char **stream_name)
{

   int len = strlen (path);

   tp_assert (len < BTE_STDIO_PATH_NAME_LEN, "Path length is too long.");

   // Return the path name in newly allocated space.
   char *new_path = new char[len + 1];

   strncpy (new_path, path, len + 1);
   *stream_name = new_path;
   return BTE_ERROR_NO_ERROR;
};

// Move to a specific position.
template < class T > BTE_err BTE_stream_stdio < T >::seek (off_t offset)
{
   off_t file_position;

   if (substream_level) {
      if (offset * sizeof (T) > (unsigned) (logical_eos - logical_bos)) {
	 return BTE_ERROR_OFFSET_OUT_OF_RANGE;
      } else {
	 file_position = offset * sizeof (T) + logical_bos;
      }
   } else {
      file_position = offset * sizeof (T) + header.block_length;
   }

   if (fseek (file, file_position, SEEK_SET)) {
      LOG_FATAL("fseek failed to go to position " << file_position << 
                   " of \"" << "\"\n");
      LOG_FLUSH_LOG;
      return BTE_ERROR_OS_ERROR;
   }
   return BTE_ERROR_NO_ERROR;
}

// Truncate the stream.
template < class T >
    BTE_err BTE_stream_stdio < T >::truncate (off_t offset)
{
   off_t file_position;

   // Can't truncate a substream.
   if (substream_level) {
      return BTE_ERROR_STREAM_IS_SUBSTREAM;
   }

   if (offset < 0) {
      return BTE_ERROR_OFFSET_OUT_OF_RANGE;
   }

   file_position = offset * sizeof (T) + header.block_length;

   // Truncate the file.
   if (::truncate (path, file_position)) {
      os_errno = errno;

      LOG_FATAL_ID("Failed to truncate() to the new end of file:");
      LOG_FATAL_ID(path);
      LOG_FATAL_ID(strerror (os_errno));
      return BTE_ERROR_OS_ERROR;
   }
   // Go to the end.
   if (fseek (file, file_position, SEEK_SET)) {
      LOG_FATAL ("fseek failed to go to position " << file_position <<
		 " of \"" << "\"\n");
      LOG_FLUSH_LOG;
      return BTE_ERROR_OS_ERROR;
   }
   return BTE_ERROR_NO_ERROR;
}

template < class T > int BTE_stream_stdio < T >::available_streams (void)
{
   return remaining_streams;
}

template < class T > off_t BTE_stream_stdio < T >::chunk_size (void)
{
   // Quick and dirty guess.
   return (header.block_length * 2) / sizeof (T);
}

template < class T > void BTE_stream_stdio < T >::persist (persistence p)
{
   per = p;
}

template < class T >
    unsigned int BTE_stream_stdio < T >::get_block_length ()
{
   unsigned int block_length;

#ifdef _SC_PAGE_SIZE
   block_length = sysconf (_SC_PAGE_SIZE);
#else
   block_length = getpagesize ();
#endif
   return block_length;
}

//return -1 if error, 0 otherwise
template<class T> int BTE_stream_stdio < T >::readcheck_header ()
{
   // Read the header.
   if ((fread ((char *) &header, sizeof (header), 1, file)) != 1) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL_ID("Failed to read header from file:");
      LOG_FATAL_ID(path);
      return -1;
   }
   //
   // Do some error checking on the header.  A better error 
   // handling mechanism will appear later.  For now this is
   // just a skeleton that invalidates the stream whenever
   // a problem is detected.
   //
   if (header.magic_number != BTE_STDIO_HEADER_MAGIC_NUMBER) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL_ID("Bad magic number in file:");
      LOG_FATAL_ID(path);
      return -1;
   }
   if (header.version != 1) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL ("Bad header version (");
      LOG_FATAL (header.version);
      LOG_FATAL (") in \"");
      LOG_FATAL (path);
      LOG_FATAL ("\"\n");
      LOG_FLUSH_LOG;
      return -1;
   }
   if (header.length != sizeof (BTE_stdio_header)) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL ("Bad header length (");
      LOG_FATAL (header.length);
      LOG_FATAL (") in \"");
      LOG_FATAL (path);
      LOG_FATAL ("\"\n");
      LOG_FLUSH_LOG;
      return -1;
   }
   if (header.item_size != sizeof (T)) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL ("Bad item size (");
      LOG_FATAL (header.item_size);
      LOG_FATAL (" when ");
      LOG_FATAL (sizeof (T));
      LOG_FATAL (" expected) in \"");
      LOG_FATAL (path);
      LOG_FATAL ("\"\n");
      LOG_FLUSH_LOG;
      return -1;
   }
   //check the header block size
   if (header.block_length != get_block_length ()) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL ("Bad block length (");
      LOG_FATAL (get_block_length ());
      LOG_FATAL (" when ");
      LOG_FATAL (header.block_length);
      LOG_FATAL (" expected) in \"");
      LOG_FATAL (path);
      LOG_FATAL ("\"\n");
      LOG_FLUSH_LOG;
      return -1;
   }
   //everything's fine
   return 0;
}

#endif // _BTE_STREAM_STDIO_H
