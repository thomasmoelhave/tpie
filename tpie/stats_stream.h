//
// File:    tpie_stats_stream.h
// Authors: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: tpie_stats_stream.h,v 1.3 2004-08-17 16:48:25 jan Exp $
//
// Statistics for streams.

////////////////////////////////////////////////////////////////////////////////
/// \file tpie_stats_stream.h Declares status information tags for TPIE Streams
/// \sa tpie_stats, AMI_stream#stats()
////////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_STATS_STREAM_H
#define _TPIE_STATS_STREAM_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <tpie/tpie_stats.h>

#define TPIE_STATS_STREAM_COUNT 11

/**Status information about  a TPIE stream */
enum TPIE_STATS_STREAM {
  /** Number of block reads */
  BLOCK_READ = 0,
  /** Number of block writes */
  BLOCK_WRITE,
  /** Number of item reads */
  ITEM_READ,
  /** Number of item writes */ 
  ITEM_WRITE,
  /** Number of item seek operations */ 
  ITEM_SEEK,
  /** Number of stream open operations */ 
  STREAM_OPEN,
  /** Number of stream close operations */ 
  STREAM_CLOSE,
  /** Number of stream create operations */ 
  STREAM_CREATE,
  /** Number of stream delete operations */ 
  STREAM_DELETE,
  /** Number of substream create operations */ 
  SUBSTREAM_CREATE,
  /** Number of substream delete operations */ 
  SUBSTREAM_DELETE
};

///////////////////////////////////////////////////////////////////////////
/// Encapsulates statistics about a TPIE stream.
///////////////////////////////////////////////////////////////////////////
typedef tpie_stats<TPIE_STATS_STREAM_COUNT> tpie_stats_stream;

#endif //_TPIE_STATS_STREAM_H
