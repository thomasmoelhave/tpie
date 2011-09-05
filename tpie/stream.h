// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

///////////////////////////////////////////////////////////////////////////
/// \file tpie/stream.h Declares TPIE streams.
///////////////////////////////////////////////////////////////////////////
#ifndef _TPIE_AMI_STREAM_H
#define _TPIE_AMI_STREAM_H


// Include the configuration header.
#include <tpie/config.h>

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// Get the error codes.
#include <tpie/err.h>
#include <tpie/persist.h>
#include <tpie/stats_stream.h>
// Get the device description class

// Get an appropriate BTE.  Flags may have been set to determine
// exactly what BTE implementaion will be used, but that should be of
// little concern to us.  bte/stream.h and recursively included files
// will worry about parsing the appropriate flags and getting us an
// implementation.
#include <tpie/tempname.h>
#include <tpie/file_stream.h>
#include <tpie/file_count.h>

#include <tpie/tpie_log.h>

namespace tpie {

	enum stream_usage {
	    /** Overhead of the object without the buffer */
	    STREAM_USAGE_OVERHEAD = 1,
	    /** Max amount ever used by a buffer */
	    STREAM_USAGE_BUFFER,
	    /** Amount currently in use. */
	    STREAM_USAGE_CURRENT,
	    /** Max amount that will ever be used. */
	    STREAM_USAGE_MAXIMUM,
	    /** Maximum additional amount used by each substream created. */
	    STREAM_USAGE_SUBSTREAM
	};

    namespace ami {

/** AMI stream types passed to constructors */
	enum stream_type {
	    READ_STREAM = 1,	// Open existing stream for reading
	    WRITE_STREAM,		// Open for writing.  Create if non-existent
	    APPEND_STREAM,		// Open for writing at end.  Create if needed.
	    READ_WRITE_STREAM	// Open to read and write.
	};
	
/**  AMI stream status. */
	enum stream_status {
	    /** Stream is valid */
	    STREAM_STATUS_VALID = 0,
	    /** Stream is invalid */
	    STREAM_STATUS_INVALID
	};

    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {

////////////////////////////////////////////////////////////////////////////////
/// A Stream<T> object stores an ordered collection of objects of
/// type T on external memory. 
/// \anchor stream_types The  type of a Stream indicates what
/// operations are permitted on the stream. 
/// Stream types provided in TPIE are the following:
///
/// \anchor READ_STREAM \par READ_STREAM: 
/// Input operations on the stream are permitted, but output is not permitted.
/// 
/// \anchor WRITE_STREAM \par WRITE_STREAM: 
/// Output operations are permitted, but input operations are not permitted. 
/// 
/// \anchor APPEND_STREAM \par APPEND_STREAM: 
/// Output is appended to the end of the stream. Input operations are not
/// permitted. This is similar to WRITE_STREAM except that if the stream is
/// constructed on a file containing an existing stream,
/// objects written to the stream will be appended at the end of the stream.
///
/// \anchor READ_WRITE_STREAM \par READ_WRITE_STREAM:
/// Both input and output operations are permitted.
////////////////////////////////////////////////////////////////////////////////
template<class T > 
class stream {
    
public:
	typedef T item_type;
    
    // We have a variety of constructors for different uses.

    ////////////////////////////////////////////////////////////////////////////
    /// A new stream of type \ref READ_WRITE_STREAM is constructed on
    /// the given device as a file with a randomly generated name, 
    /// prefixed by "".  
    ////////////////////////////////////////////////////////////////////////////
    stream();
    
    ////////////////////////////////////////////////////////////////////////////
    /// A new stream is constructed and 
    /// named and placed according to the given parameter pathname.
    /// Its type is given by st which defaults to \ref READ_WRITE_STREAM.
    ////////////////////////////////////////////////////////////////////////////
    stream(const std::string& path_name, 
	   stream_type st = READ_WRITE_STREAM);

    ////////////////////////////////////////////////////////////////////////////
    /// A substream is a TPIE stream that is part of another TPIE stream.
    /// More precisely, a substream B of a stream A is defined as a
    /// contiguous range of objects from the ordered collection of objects
    /// that make up the stream A.  If desired, one can construct
    /// substreams of substreams of substreams ad infinitum. Since a
    /// substream is a stream in its own right, many of the stream member
    /// functions can be applied to a substream. A substream can be
    /// created via this pseudo-constructor. The reason we do not
    /// use a real constructor is to get around the fact that
    /// constructors can not be virtual.
    /// \param[in] st  specifies the type of the substream
    /// \param[in] sub_begin offset, that defines the begin of the substream
    /// within the original stream
    /// \param[in] sub_end offset, that defines the end of the substream
    /// within the original stream
    /// \param[out] sub_stream  upon completion points to the newly created
    /// substream.
    ////////////////////////////////////////////////////////////////////////////
    err new_substream(stream_type     st, 
		      stream_offset_type  sub_begin, 
		      stream_offset_type  sub_end,
			  stream<T>       **sub_stream);
  
    ////////////////////////////////////////////////////////////////////////////
    /// Returns the status of the stream instance; the result is either
    /// STREAM_STATUS_VALID or STREAM_STATUS_INVALID. 
    /// The only operation that can leave the stream invalid is the constructor
    /// (if that happens, the log file contains more information). No items 
    /// should be read from or written to an invalid stream.
    ////////////////////////////////////////////////////////////////////////////
    inline stream_status status() const { 
		return m_status; 
    }
    
    ////////////////////////////////////////////////////////////////////////////
    /// Returns wether the status of the stream is STREAM_STATUS_VALID.
    /// \sa status()
    ////////////////////////////////////////////////////////////////////////////
    inline bool is_valid() const { 
		return m_status == STREAM_STATUS_VALID; 
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Returns true if the block's status is not BLOCK_STATUS_VALID. 
    /// \sa is_valid(), status()
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator!() const { 
		return !is_valid(); 
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Reads the current item from the stream and advance the "current item"
    /// pointer to the next item. The item read is pointed to by 
    /// *elt. If no error has occurred, return \ref NO_ERROR.
    /// If the ``current item'' pointer is beyond the last item in the stream,
    /// ERROR_END_OF_STREAM is returned
    ////////////////////////////////////////////////////////////////////////////
    inline err read_item(T **elt);
    
    ////////////////////////////////////////////////////////////////////////////
    /// Writes elt to the stream in the current position. Advance the 
    /// "current item" pointer to the next item. If no error has occurred
    /// \ref NO_ERROR is returned.
    ////////////////////////////////////////////////////////////////////////////
    inline err write_item(const T &elt);
  
    ////////////////////////////////////////////////////////////////////////////
    /// Reads *len items from the current position of the stream into
    /// the array mm_array. The "current position" pointer is increased
    /// accordingly.
	/// \deprecated
    ////////////////////////////////////////////////////////////////////////////
    err read_array(T *mm_space, stream_offset_type *len);

    ////////////////////////////////////////////////////////////////////////////
    /// Reads len items from the current position of the stream into
    /// the array mm_array. The "current position" pointer is increased
    /// accordingly.
    ////////////////////////////////////////////////////////////////////////////
    err read_array(T *mm_space, memory_size_type & len);
    
    ////////////////////////////////////////////////////////////////////////////
    /// Writes len items from array |mm_array to the
    /// stream, starting in the current position. The "current item"
    /// pointer is increased accordingly.
    ////////////////////////////////////////////////////////////////////////////
    err write_array(const T *mm_space, memory_size_type len);
    
    ////////////////////////////////////////////////////////////////////////////
    /// Returns the number of items in the stream.
    ////////////////////////////////////////////////////////////////////////////
    inline stream_offset_type stream_len(void) const { 
		return m_stream.size();
    }
  
    ////////////////////////////////////////////////////////////////////////////
    /// Returns the path name of this stream in newly allocated space.
    ////////////////////////////////////////////////////////////////////////////
    inline std::string name() const;
  
    ////////////////////////////////////////////////////////////////////////////
    /// Move the current position to off (measured in terms of items.
    ////////////////////////////////////////////////////////////////////////////
    inline err seek(stream_offset_type offset);
    
    ////////////////////////////////////////////////////////////////////////////
    /// Returns the current position in the stream measured of items from the
    /// beginning of the stream.
    ////////////////////////////////////////////////////////////////////////////
    inline stream_offset_type tell() const { 
		return m_stream.offset(); 
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Resize the stream to off items. If off is less than the
    /// number of objects in the stream, truncate()
    /// truncates the stream to off objects. If
    /// off is more than the number of objects in the
    /// stream, truncate() extends the stream to the
    /// specified number of objects. In either case, the "current
    /// item" pointer will be moved to the new end of the stream.
    ////////////////////////////////////////////////////////////////////////////
    inline err truncate(stream_offset_type offset);
    
    ////////////////////////////////////////////////////////////////////////////
    /// This function is used for obtaining the amount of main memory used by an
    /// Stream<T> object (in bytes).
    /// \param[in] usage_type of type \ref MM_stream_usage and is 
    /// one of the following:
    /// \par MM_STREAM_USAGE_CURRENT
    /// Total amount of memory currently used by the stream.
    /// \par MM_STREAM_USAGE_MAXIMUM
    /// Max amount of memory that will ever be used by the stream.
    /// \par MM_STREAM_USAGE_OVERHEAD
    /// The amount of memory used by the object itself, without the data buffer.
    /// \par MM_STREAM_USAGE_BUFFER
    /// The amount of memory used by the data buffer.
    /// \par MM_STREAM_USAGE_SUBSTREAM
    /// The additional amount of memory that will be used by each substream created.
    /// \param[out] usage amount of memory in bytes used by the stream
    ////////////////////////////////////////////////////////////////////////////
    err main_memory_usage(size_type *usage,
			  stream_usage usage_type) const;



	////////////////////////////////////////////////////////////////////////////
    /// Returns the number of bytes that count streams will maximaly consume
    ////////////////////////////////////////////////////////////////////////////
	static memory_size_type memory_usage(memory_size_type count);
  
    ////////////////////////////////////////////////////////////////////////////
    /// Returns a \ref tpie_stats_stream object containing  statistics of 
    /// the stream. 
    ////////////////////////////////////////////////////////////////////////////
    stats_stream stats() const { 
		return stats_stream();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Returns a \ref tpie_stats_stream object containing  statistics of 
    /// the entire tpie system. 
    ////////////////////////////////////////////////////////////////////////////
	static stats_stream gstats() {
		return stats_stream();
	}

    ////////////////////////////////////////////////////////////////////////////
    /// Returns the number of globally available streams.
    /// The number should resemble the the maximum
    /// number of streams allowed (which is OS-dependent) minus the number
    /// of streams currently opened by TPIE.
    ////////////////////////////////////////////////////////////////////////////
    size_t available_streams(void) {
		return available_files();
    }
    
    ////////////////////////////////////////////////////////////////////////////
    /// Returns the maximum number of items (of type T) 
    /// that can be stored in one block.
    ////////////////////////////////////////////////////////////////////////////
    memory_size_type chunk_size(void) const { 
		return file_base::block_size(1.0) / sizeof(T);
    }
    
    ////////////////////////////////////////////////////////////////////////////
    /// Set the stream's \ref persistence flag to p, which can have one of two values:
    /// \ref PERSIST_DELETE or \ref PERSIST_PERSISTENT.}
    ////////////////////////////////////////////////////////////////////////////
    void persist(persistence p) {
		m_temp.set_persistent(p == PERSIST_PERSISTENT);
    }
    
    ////////////////////////////////////////////////////////////////////////////
    /// Set the stram's \ref persistence flag to \ref PERSIST_PERSISTENT, thereby
    /// ensuring it is not deleted when destructed.
    ////////////////////////////////////////////////////////////////////////////
    persistence persist() const { 
		return m_temp.is_persistent() ? PERSIST_PERSISTENT : PERSIST_DELETE;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Return a string describing the stream.
    // This function gives easy access to the stream's file name and its length.
    ////////////////////////////////////////////////////////////////////////////
	std::string& sprint();
    
private:

    /** Restricted copy constructor */
    stream(const stream<T>& other);
    /** Restricted assignment operator*/
    stream<T>& operator=(const stream<T>& other);
	
	temp_file m_temp;
	file_stream<T> m_stream;
    
    /** Non-zero if we should destroy the bte stream when we the
     * AMI stream is destroyed. */
    //bool m_destructBTEStream;
    stream_status m_status;
};

// Create a temporary AMI stream on one of the devices in the default
// device description. Persistence is PERSIST_DELETE by default. We
// are given the index of the string describing the desired device.
	template<class T>
	stream<T>::stream(): m_status(STREAM_STATUS_INVALID)
	{
		TP_LOG_DEBUG_ID("Temporary stream in file: ");
	    TP_LOG_DEBUG_ID( m_temp.path() );
		try {
			m_stream.open( m_temp.path() );
		} catch(const stream_exception &) {
			TP_LOG_FATAL_ID("Open failed");
			return;
		}
	    //  Set status to VALID.
	    m_status = STREAM_STATUS_VALID;
	};


// A stream created with this constructor will persist on disk at the
// location specified by the path name.
	template<class T>
	stream<T>::stream(const std::string& path_name, stream_type st) :
		m_temp(path_name, true), m_status(STREAM_STATUS_INVALID) {
		try {
			m_stream.open( path_name, st==READ_STREAM ? file_base::read: file_base::read_write);
			if (st == APPEND_STREAM) m_stream.seek(0, file_base::end);
		} catch(const stream_exception &) {
			TP_LOG_FATAL_ID("Open failed");
			return;
		}
	    m_status = STREAM_STATUS_VALID;
	};

	
	// *stream::new_substream* //
	template<class T>
	err stream<T>::new_substream(stream_type     st,
				     stream_offset_type  sub_begin,
				     stream_offset_type  sub_end,
								 stream<T>       **sub_stream)
	{
	    // err retval = NO_ERROR;

	    // // Check permissions. Only READ and WRITE are allowed, and only READ is
	    // // allowed if m_readOnly is set.
	    // if ((st != READ_STREAM) && ((st != WRITE_STREAM) || m_readOnly)) {

		// *sub_stream = NULL;

		// TP_LOG_DEBUG_ID("permission denied");		

		// return PERMISSION_DENIED;

	    // }
    
		// typename bte_t::base_t *bte_ss;
    
	    // if (m_bteStream->new_substream(((st == READ_STREAM) ? bte::READ_STREAM :
		// 			    bte::WRITE_STREAM),
		// 			   sub_begin, sub_end,
		// 			   &bte_ss) != bte::NO_ERROR) {

		// *sub_stream = NULL;

		// TP_LOG_DEBUG_ID("new_substream failed");		

		// return BTE_ERROR;
	    // }
    
	    // stream<T,bte_t> *ami_ss;
    
	    // // This is a potentially dangerous downcast.  It is being done for
	    // // the sake of efficiency, so that calls to the BTE can be
	    // // inlined.  If multiple implementations of BTE streams are
	    // // present it could be very dangerous.
    
	    // ami_ss = new stream<T,bte_t>(static_cast<bte_t*>(bte_ss));
    
	    // ami_ss->m_destructBTEStream = true;

	    // retval = ami_ss->seek(0);
	    // assert(retval == NO_ERROR); // sanity check
    
	    // *sub_stream = ami_ss;
    
	    // return retval;
		return BTE_ERROR;
	}


	template<class T>
	inline std::string stream<T>::name() const {
		return m_stream.path();
	}

// Move to a specific offset.
	template<class T>
	inline err stream<T>::seek(stream_offset_type offset) {
		try {
			m_stream.seek(offset);
		} catch(const stream_exception &) {
			TP_LOG_WARNING_ID("BTE error - seek failed");		
			return BTE_ERROR;
		}
	    return NO_ERROR;
	}

// Truncate
	template<class T>
	inline err stream<T>::truncate(stream_offset_type offset) {
		try {
			m_stream.truncate(offset);
		} catch(const stream_exception & e) {
			TP_LOG_WARNING_ID("BTE error - truncate failed" << e.what());
			return BTE_ERROR;
		}
	    return NO_ERROR;
	}


template<class T>
size_type stream<T>::memory_usage(size_type count) {
	return file_stream<T>::memory_usage(count) + sizeof(stream<T>)*count;
}
	

// Query memory usage
	template<class T>
	err stream<T>::main_memory_usage(memory_size_type *usage,
					 stream_usage usage_type) const {

	    switch (usage_type) {
	    case STREAM_USAGE_OVERHEAD:
			*usage = sizeof(*this) + file_stream<T>::memory_usage(0.0);
			return NO_ERROR;
	    case STREAM_USAGE_CURRENT:
	    case STREAM_USAGE_MAXIMUM:
	    case STREAM_USAGE_SUBSTREAM:
			*usage =  memory_usage(1);
			return NO_ERROR;
	    case STREAM_USAGE_BUFFER:
			*usage = file_stream<T>::memory_usage(1.0) - file_stream<T>::memory_usage(0.0); 
			return NO_ERROR;
		}
		return BTE_ERROR;
	}

	template<class T>
	inline err stream<T>::read_item(T **elt) {
		try {
			*elt = &m_stream.read_mutable();
		} catch(const end_of_stream_exception &) {
			//TP_LOG_DEBUG_ID("eos in read_item");
			return END_OF_STREAM;
		} catch(const stream_exception &) {
			TP_LOG_DEBUG_ID("bte error in read_item");
			return BTE_ERROR;
		}
		return NO_ERROR;
	}

	template<class T>
	inline err stream<T>::write_item(const T &elt) {
		try {
			m_stream.write(elt); 
		} catch(const stream_exception &) {
			TP_LOG_WARNING_ID("BTE error - write item failed");
			return BTE_ERROR;
	    }
	    return NO_ERROR;
	}

	template<class T>
	err stream<T>::read_array(T *mm_space, stream_offset_type *len) {
		size_type l=*len;
		err e = read_array(mm_space, l);
		*len = l;
		return e;
	}

	template<class T>
	err stream<T>::read_array(T *mm_space, size_type & len) {
		size_type l = static_cast<size_type>(std::min(
			static_cast<stream_size_type>(len), 
			static_cast<stream_size_type>(m_stream.size() - m_stream.offset())));
		try {
			m_stream.read(mm_space, mm_space+l);
		} catch(const stream_exception &) {
			return BTE_ERROR;
		}
		return (l == len)?NO_ERROR:END_OF_STREAM;
	}

	template<class T>
	err stream<T>::write_array(const T *mm_space, size_type len) {
		try {
			m_stream.write(mm_space, mm_space+len);
		} catch(const stream_exception &) {
			return BTE_ERROR;
		}
		return NO_ERROR;
	}

	template<class T>
	std::string& stream<T>::sprint() {
	    static std::string buf;
		std::stringstream ss;
		ss << "STREAM " << name() <<  " " << static_cast<long>(stream_len());
		ss >> buf;
	    return buf;
	}

    }  //  ami namespace

}  //  tpie namespace

#include <tpie/stream_compatibility.h>

#endif // _TPIE_AMI_STREAM_H
