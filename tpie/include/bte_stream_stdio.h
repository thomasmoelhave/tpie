//
// File: bte_stream_stdio.h (formerly bte_stdio.h)
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/11/94
//
// $Id: bte_stream_stdio.h,v 1.6 2003-04-17 15:36:48 jan Exp $
//
#ifndef _BTE_STREAM_STDIO_H
#define _BTE_STREAM_STDIO_H

// For header's type field (83 == 'S').
#define BTE_STREAM_STDIO 83

// Get definitions for working with Unix and Windows
#include <portability.h>

#include <string.h>
#include <tpie_log.h>
#include <tpie_assert.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>

#include <bte_stream_base.h>

// File system streams are streams in a special format that is designed 
// to be stored in an ordinary file in a UN*X file system.  They are 
// predominatly designed to be used to store streams in a persistent way.
// They may disappear in later versions as persistence becomes an integral
// part of TPIE.
//
// For simplicity, we work through the standard C I/O library (stdio).
//


// A class of BTE streams implemented using ordinary stdio
// semantics.
template < class T > 
class BTE_stream_stdio: public BTE_stream_base < T > {
private:

   FILE * file;
   BTE_stream_header header;

   size_t os_block_size_;

   int os_errno;   // A place to cache OS error values.  It is normally
                   // set after each call to the OS.

   char path[BTE_STREAM_PATH_NAME_LEN];

   // If this stream is actually a substream, these will be set to
   // indicate the portion of the file that is part of this stream.
   // If the stream is the whole file, they will be set to -1.
   TPIE_OS_OFFSET logical_bos;
   TPIE_OS_OFFSET logical_eos;

   // Offset of the current item in the file.
   TPIE_OS_OFFSET f_offset;
  
   // Offset past the last item in the file.
   TPIE_OS_OFFSET f_eof;

   // Read and check the header; used by constructors
   int readcheck_header ();

   inline TPIE_OS_OFFSET file_off_to_item_off (TPIE_OS_OFFSET file_off);
   inline TPIE_OS_OFFSET item_off_to_file_off (TPIE_OS_OFFSET item_off);

 public:
   T read_tmp;

   // Constructors
   BTE_stream_stdio (const char *dev_path, const BTE_stream_type st, 
		     size_t lbf = 1);

   // A psuedo-constructor for substreams.
   BTE_err new_substream (BTE_stream_type st, TPIE_OS_OFFSET sub_begin,
			  TPIE_OS_OFFSET sub_end,
			  BTE_stream_base < T > **sub_stream);

   ~BTE_stream_stdio (void);

   BTE_err read_item (T ** elt);
   BTE_err write_item (const T & elt);

   // Query memory usage
   BTE_err main_memory_usage (size_t * usage, MM_stream_usage usage_type);

   // Return the number of items in the stream.
   TPIE_OS_OFFSET stream_len (void);

   // Return the path name in newly allocated space.
   BTE_err name (char **stream_name);

   // Move to a specific position in the stream.
   BTE_err seek (TPIE_OS_OFFSET offset);

   // Truncate the stream.
   BTE_err truncate (TPIE_OS_OFFSET offset);

   TPIE_OS_OFFSET chunk_size (void);
};

template < class T >
BTE_stream_stdio < T >::BTE_stream_stdio (const char *dev_path,
					  const BTE_stream_type st,
					  size_t lbf) {
   BTE_err berr;

   // Reduce the number of streams avaialble.
   if (remaining_streams <= 0) {
      status_ = BTE_STREAM_STATUS_INVALID;
      return;
   }

   // Cache the path name
   if (strlen (dev_path) > BTE_STREAM_PATH_NAME_LEN - 1) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL_ID("Path name too long:");
      LOG_FATAL_ID(dev_path);
      return;
   }
   strncpy (path, dev_path, BTE_STREAM_PATH_NAME_LEN);
   status_ = BTE_STREAM_STATUS_NO_STATUS;

   os_block_size_ = os_block_size ();

   // Not a substream.
   substream_level = 0;

   logical_bos = logical_eos = -1;

   // By default, all streams are deleted at destruction time.  (the
   // comment above is misleading. the AMI level stream is controlling
   // the persistency of this stream)
   per = PERSIST_DELETE;

   remaining_streams--;
   
   switch (st) {
   case BTE_READ_STREAM:
      // Open the file for reading.
      r_only = 1;
      if ((file = TPIE_OS_FOPEN(dev_path, "rb")) == NULL) {
	 status_ = BTE_STREAM_STATUS_INVALID;
	 LOG_FATAL_ID("Failed to open file:");
	 LOG_FATAL_ID(dev_path);
	 return;
      }
      // Read and check header
      if (readcheck_header () == -1) {
	 LOG_FATAL_ID("Bad header.");
	 return;
      }
      // Seek past the end of the first block.
      //if (TPIE_OS_FSEEK (file, os_block_size_, 0) == -1) {
      //status_ = BTE_STREAM_STATUS_INVALID;
      //LOG_FATAL_ID("fseek failed.");
      //return;
      //}
      if ((berr = this->seek (0)) != BTE_ERROR_NO_ERROR) {
	LOG_FATAL_ID("Cannot seek in file:");
	LOG_FATAL_ID(dev_path);
	return;
      }
      break;

   case BTE_WRITE_STREAM:
   case BTE_WRITEONLY_STREAM:
   case BTE_APPEND_STREAM:
      // Open the file for appending.
      r_only = 0;
      if ((file = TPIE_OS_FOPEN(dev_path, "rb+")) == NULL) {
	 //file does not  exist - create it
	 if ((file = TPIE_OS_FOPEN (dev_path, "wb+")) == NULL) {
	    status_ = BTE_STREAM_STATUS_INVALID;
	    LOG_FATAL_ID("Failed to open file:");
	    LOG_FATAL_ID(dev_path);
	    return;
	 }
	 // Create and write the header
	 init_header(&header);
	 header.type = BTE_STREAM_STDIO;

	 if (fwrite ((char *) &header, sizeof (header), 1, file) != 1) {
	    status_ = BTE_STREAM_STATUS_INVALID;
	    LOG_FATAL_ID("Failed to write header to file:");
	    LOG_FATAL_ID(dev_path);
	    return;
	 }

	 // Truncate the file to header block
	 if ((berr = this->truncate (0)) != BTE_ERROR_NO_ERROR) {
	    LOG_FATAL_ID("Cannot truncate in file:");
	    LOG_FATAL_ID(dev_path);
	    return;
	 }
	 if ((berr = this->seek (0)) != BTE_ERROR_NO_ERROR) {
	    LOG_FATAL_ID("Cannot seek in file:");
	    LOG_FATAL_ID(dev_path);
	    return;
	 }

	 gstats_.record(STREAM_CREATE);
	 stats_.record(STREAM_CREATE);

      } else {
	 // File exists - read and check header
	 if (readcheck_header () == -1) {
	    LOG_FATAL_ID("Bad header in file:");
	    LOG_FATAL_ID(dev_path);
	    return;
	 }
	 // Seek to the end of the stream  if BTE_APPEND_STREAM
	 if (st == BTE_APPEND_STREAM) {
	    if (TPIE_OS_FSEEK (file, 0, TPIE_OS_FLAG_SEEK_END)) {
	       status_ = BTE_STREAM_STATUS_INVALID;
	       LOG_FATAL_ID("Failed to go to EOF of file:");
	       LOG_FATAL_ID(dev_path);
	       return;
	    }
	    // Make sure there was at least a full block there to pass.
	    if (TPIE_OS_FTELL (file) < (long)os_block_size_) {
	      LOG_FATAL_ID("File too short:");
	      LOG_FATAL_ID(dev_path);
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

   f_eof = item_off_to_file_off(header.item_logical_eof);

   // Register memory usage before returning.
   register_memory_allocation (sizeof (BTE_stream_stdio < T >));
   // A quick and dirty guess.  One block in the buffer cache, one in
   // user space. TODO.
   register_memory_allocation (os_block_size_ * 2);
   gstats_.record(STREAM_OPEN);
   stats_.record(STREAM_OPEN);
}

// A psuedo-constructor for substreams.  This allows us to get around
// the fact that one cannot have virtual constructors.
template < class T >
BTE_err BTE_stream_stdio < T >::new_substream (BTE_stream_type st,
					       TPIE_OS_OFFSET sub_begin,
					       TPIE_OS_OFFSET sub_end,
					       BTE_stream_base < T >
					       **sub_stream)
{
  // Check permissions.
   if ((st != BTE_READ_STREAM) && ((st != BTE_WRITE_STREAM) || r_only)) {
      *sub_stream = NULL;
      return BTE_ERROR_PERMISSION_DENIED;
   }

    tp_assert (((st == BTE_WRITE_STREAM) && !r_only) ||
	       (st == BTE_READ_STREAM),
	       "Bad things got through the permisssion checks.");

   if (substream_level) {
      if ((sub_begin * (TPIE_OS_OFFSET)sizeof (T) >=
	   (logical_eos - logical_bos))
	  || (sub_end * (TPIE_OS_OFFSET)sizeof (T) >
	      (logical_eos - logical_bos))) {
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
      sub->logical_bos = sub->os_block_size_ + sub_begin * sizeof (T);
      sub->logical_eos =
	  sub->os_block_size_ + (sub_end + 1) * sizeof (T);
   }

   // Set the current position.
   TPIE_OS_FSEEK (sub->file, sub->logical_bos, 0);

   sub->substream_level = substream_level + 1;
   sub->per =
       (per == PERSIST_READ_ONCE) ? PERSIST_READ_ONCE : PERSIST_PERSISTENT;
   *sub_stream = (BTE_stream_base < T > *)sub;

   gstats_.record(SUBSTREAM_CREATE);
   stats_.record(SUBSTREAM_CREATE);
   return BTE_ERROR_NO_ERROR;
}


template < class T > BTE_stream_stdio < T >::~BTE_stream_stdio (void) {

  if (!r_only) {
    header.item_logical_eof = file_off_to_item_off(f_eof);
    if (TPIE_OS_FSEEK (file, 0, TPIE_OS_FLAG_SEEK_SET) == -1) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_WARNING_ID("Failed to seek in file:")
      LOG_WARNING_ID(path)
    } else if (TPIE_OS_FWRITE ((char *) &header, sizeof (header), 1, file) != 1) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_WARNING_ID("Failed to write header to file:")
      LOG_WARNING_ID(path)
      //      return;
    }
  }

  if (TPIE_OS_FCLOSE (file) != 0) {
    status_ = BTE_STREAM_STATUS_INVALID;
    LOG_WARNING_ID("Failed to close file:");
    LOG_WARNING_ID(path);
  }

  // Get rid of the file if not persistent and if not substream.
  if (!substream_level) {
    if (per == PERSIST_DELETE) {
      if (r_only) {
	LOG_WARNING_ID("Read only stream is PERSIST_DELETE:");
	LOG_WARNING_ID(path);
	LOG_WARNING_ID("Ignoring persistency request.");
      } else if (unlink (path)) {
	os_errno = errno;
	LOG_WARNING_ID("Failed to unlink() file:");
	LOG_WARNING_ID(path);
	LOG_WARNING_ID(strerror(os_errno));
      } else {
	gstats_.record(STREAM_DELETE);
	stats_.record(STREAM_DELETE);
      }
    }
  } else {
    gstats_.record(SUBSTREAM_DELETE);
    stats_.record(SUBSTREAM_DELETE);
  }
  // Register memory deallocation before returning.
  register_memory_deallocation (sizeof (BTE_stream_stdio < T >));
  
  // A quick and dirty guess.  One block in the buffer cache, one in
  // user space. TODO.
  register_memory_deallocation (os_block_size_ * 2);
  
  if (remaining_streams >= 0) {
    remaining_streams++;
  }
  gstats_.record(STREAM_CLOSE);
  stats_.record(STREAM_CLOSE);
}

template < class T > BTE_err BTE_stream_stdio < T >::read_item (T ** elt)
{
   int stdio_ret;
   BTE_err ret;

   if ((logical_eos >= 0) && (TPIE_OS_FTELL (file) >= logical_eos)) {
      tp_assert ((logical_bos >= 0), "eos set but bos not.");
      status_ = BTE_STREAM_STATUS_END_OF_STREAM;
      ret = BTE_ERROR_END_OF_STREAM;
   } else {

      stdio_ret = TPIE_OS_FREAD ((char *) (&read_tmp), sizeof (T), 1, file);

      if (stdio_ret == 1) {
 	 f_offset += sizeof(T);
	 *elt = &read_tmp;
	 ret = BTE_ERROR_NO_ERROR;
      } else {
	 // Assume EOF.  Fix this later.
	 status_ = BTE_STREAM_STATUS_END_OF_STREAM;
	 ret = BTE_ERROR_END_OF_STREAM;
      }
   }

   gstats_.record(ITEM_READ);
   stats_.record(ITEM_READ);
   return ret;
}

template < class T > 
BTE_err BTE_stream_stdio < T >::write_item (const T & elt) {

   int stdio_ret;
   BTE_err ret;

   if ((logical_eos >= 0) && (TPIE_OS_FTELL (file) > logical_eos)) {
      tp_assert ((logical_bos >= 0), "eos set but bos not.");
      status_ = BTE_STREAM_STATUS_END_OF_STREAM;
      ret = BTE_ERROR_END_OF_STREAM;
   } else {
      stdio_ret = TPIE_OS_FWRITE ((char *) &elt, sizeof (T), 1, file);
      if (stdio_ret == 1) {
	 if (f_eof == f_offset)
	   f_eof += sizeof(T);
         f_offset += sizeof(T);
	 ret = BTE_ERROR_NO_ERROR;
      } else {
	 LOG_FATAL_ID("write_item failed.");
	 status_ = BTE_STREAM_STATUS_INVALID;
	 ret = BTE_ERROR_IO_ERROR;
      }
   }
   gstats_.record(ITEM_WRITE);
   stats_.record(ITEM_WRITE);
   return ret;
}


template < class T >
BTE_err BTE_stream_stdio < T >::main_memory_usage (size_t * usage,
						   MM_stream_usage
						   usage_type) {

   switch (usage_type) {
   case MM_STREAM_USAGE_OVERHEAD:
      *usage = sizeof (*this);
      break;
   case MM_STREAM_USAGE_BUFFER:
      *usage = 2 * os_block_size_;
      break;
   case MM_STREAM_USAGE_CURRENT:
   case MM_STREAM_USAGE_MAXIMUM:
   case MM_STREAM_USAGE_SUBSTREAM:
      *usage = sizeof (*this) + 2 * os_block_size_;
      break;
   }

   return BTE_ERROR_NO_ERROR;
}

// Return the number of items in the stream.
template < class T > 
TPIE_OS_OFFSET BTE_stream_stdio < T >::stream_len (void) {

   if (substream_level) {	// We are in a substream.
     ///      return (logical_eos - logical_bos) / sizeof (T);
     // [tavi 01/25/02] Commented out the above and replaced it with the following:
     return file_off_to_item_off(logical_eos) - file_off_to_item_off(logical_bos);
   } else {
      // There must be a way to get this information directly,
      // instead of fseeking around.

      // Where are we now?
     ///      TPIE_OS_OFFSET current = ftell (file);

      // Go to the end and see where we are.
     ///      TPIE_OS_FSEEK (file, 0, TPIE_OS_FLAG_SEEK_END);
     ///      TPIE_OS_OFFSET end = ftell (file);

      // Go back.
     ///      TPIE_OS_FSEEK (file, current, TPIE_OS_FLAG_SEEK_SET);

      // Lars May 22, 1997: This is a quick hack to fix a problem
      // with headers of length less than a block. That shouldn't 
      // be possible but there is a bug somewhere
      // - Look at it later (seems to have something to do with
      //   substreams. Header block is not truncated).
     ///      if (end < (int) os_block_size_) {
	 //printf("shouldnt be here! possible bug\n);
     ///	 return 0;
     ///      } else {
     ///	 return (end - os_block_size_) / sizeof (T);
     ///      }
     // [tavi 01/25/02] Commented out the above and replaced it with the following:
     return file_off_to_item_off(f_eof);
   }
}

// Return the path name in newly allocated space.
template < class T >
BTE_err BTE_stream_stdio < T >::name (char **stream_name) {
  
  int len = strlen (path);

  tp_assert (len < BTE_STREAM_PATH_NAME_LEN, "Path length is too long.");
  
  // Return the path name in newly allocated space.
  char *new_path = new char[len + 1];
  
  strncpy (new_path, path, len + 1);
  *stream_name = new_path;
  return BTE_ERROR_NO_ERROR;
}

// Move to a specific position.
template < class T > BTE_err BTE_stream_stdio < T >::seek (TPIE_OS_OFFSET offset) {

   TPIE_OS_OFFSET file_position;

   if (substream_level) {
      if (offset * (TPIE_OS_OFFSET)sizeof (T) > (TPIE_OS_OFFSET)(logical_eos - logical_bos)) {
	 return BTE_ERROR_OFFSET_OUT_OF_RANGE;
      } else {
	 file_position = offset * sizeof (T) + logical_bos;
      }
   } else {
      file_position = offset * sizeof (T) + os_block_size_;
   }

   if (TPIE_OS_FSEEK (file, file_position, TPIE_OS_FLAG_SEEK_SET)) {
      LOG_FATAL("fseek failed to go to position " << file_position << 
                   " of \"" << "\"\n");
      LOG_FLUSH_LOG;
      return BTE_ERROR_OS_ERROR;
   }

   f_offset = file_position;
   gstats_.record(ITEM_SEEK);
   stats_.record(ITEM_SEEK);
   return BTE_ERROR_NO_ERROR;
}

// Truncate the stream.
template < class T >
BTE_err BTE_stream_stdio < T >::truncate (TPIE_OS_OFFSET offset) {
	TPIE_OS_TRUNCATE_STREAM_TEMPLATE_CLASS_BODY;
}

template < class T > TPIE_OS_OFFSET BTE_stream_stdio < T >::chunk_size (void)
{
   // Quick and dirty guess.
   return (os_block_size_ * 2) / sizeof (T);
}


// Return -1 if error, 0 otherwise.
template<class T> int BTE_stream_stdio < T >::readcheck_header ()
{
   // Read the header.
   if ((TPIE_OS_FREAD ((char *) &header, sizeof (header), 1, file)) != 1) {
      status_ = BTE_STREAM_STATUS_INVALID;
      LOG_FATAL_ID("Failed to read header from file:");
      LOG_FATAL_ID(path);
      return -1;
   }
   if (check_header(&header) < 0) {
     return -1;
   }

   //everything's fine
   return 0;
}

template<class T> 
TPIE_OS_OFFSET BTE_stream_stdio < T >::file_off_to_item_off (TPIE_OS_OFFSET file_off) {
  return (file_off - os_block_size_) / sizeof (T);
}

template<class T> 
TPIE_OS_OFFSET BTE_stream_stdio < T >::item_off_to_file_off (TPIE_OS_OFFSET item_off) {
  return (os_block_size_ + item_off * sizeof (T));
}

#endif // _BTE_STREAM_STDIO_H
