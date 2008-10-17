//
// File: stream.h (formerly part of ami.h and ami_imps.h)
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
//
// $Id: ami_stream.h,v 1.10 2005-11-17 17:11:25 jan Exp $
//

///////////////////////////////////////////////////////////////////////////
/// \file tpie/stream.h Declares TPIE streams.
///////////////////////////////////////////////////////////////////////////


#ifndef _TPIE_AMI_STREAM_H
#define _TPIE_AMI_STREAM_H


// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// Include the configuration header.
#include <tpie/config.h>

// Get the error codes.
#include <tpie/err.h>

// Get the device description class
#include <tpie/device.h>

// Get an appropriate BTE.  Flags may have been set to determine
// exactly what BTE implementaion will be used, but that should be of
// little concern to us.  bte/stream.h and recursively included files
// will worry about parsing the appropriate flags and getting us an
// implementation.
#include <tpie/bte/stream.h>

#include <tpie/tempname.h>

namespace tpie {

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

#include <tpie/stream_base.h>

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
template<class T> 
class stream : public stream_base {
    
public:
    
    // We have a variety of constructors for different uses.

    ////////////////////////////////////////////////////////////////////////////
    /// A new stream of type \ref READ_WRITE_STREAM is constructed on
    /// the given device as a file with a randomly generated name, 
    /// prefixed by "".  
    ////////////////////////////////////////////////////////////////////////////
    stream(unsigned int device = UINT_MAX);
    
    ////////////////////////////////////////////////////////////////////////////
    /// A new stream is constructed and 
    /// named and placed according to the given parameter pathname.
    /// Its type is given by bst which defaults to \ref READ_WRITE_STREAM.
    ////////////////////////////////////////////////////////////////////////////
    stream(const std::string& path_name, 
	   stream_type        st = READ_WRITE_STREAM);
    
    ////////////////////////////////////////////////////////////////////////////
    // A new stream based on a specific existing BTE stream.  Note
    // that in this case the BTE stream will not be destroyed when the
    // destructor for the constructed stream is called.
    ////////////////////////////////////////////////////////////////////////////
    stream(BTE_STREAM<T> *bs);


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
		      TPIE_OS_OFFSET  sub_begin, 
		      TPIE_OS_OFFSET  sub_end,
		      stream<T>       **sub_stream);
  
    ////////////////////////////////////////////////////////////////////////////
    /// Destructor that frees the memory buffer and closes the file;
    /// if the persistence flag is set to PERSIST_DELETE, also removes the file.
    ////////////////////////////////////////////////////////////////////////////
    ~stream();
    
    ////////////////////////////////////////////////////////////////////////////
    /// Returns the status of the stream instance; the result is either
    /// STREAM_STATUS_VALID or STREAM_STATUS_INVALID. 
    /// The only operation that can leave the stream invalid is the constructor
    /// (if that happens, the log file contains more information). No items 
    /// should be read from or written to an invalid stream.
    ////////////////////////////////////////////////////////////////////////////
    stream_status status() const { 
	return m_status; 
    }
    
    ////////////////////////////////////////////////////////////////////////////
    /// Returns wether the status of the stream is STREAM_STATUS_VALID.
    /// \sa status()
    ////////////////////////////////////////////////////////////////////////////
    bool is_valid() const { 
	return m_status == STREAM_STATUS_VALID; 
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Returns true if the block's status is not BLOCK_STATUS_VALID. 
    /// \sa is_valid(), status()
    ////////////////////////////////////////////////////////////////////////////
    bool operator!() const { 
	return !is_valid(); 
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Reads the current item from the stream and advance the "current item"
    /// pointer to the next item. The item read is pointed to by 
    /// *elt. If no error has occurred, return ERROR_NO_ERROR.
    /// If the ``current item'' pointer is beyond the last item in the stream,
    /// ERROR_END_OF_STREAM is returned
    ////////////////////////////////////////////////////////////////////////////
    err read_item(T **elt);
    
    ////////////////////////////////////////////////////////////////////////////
    /// Writes elt to the stream in the current position. Advance the 
    /// "current item" pointer to the next item. If no error has occurred
    /// ERROR_NO_ERROR is returned.
    ////////////////////////////////////////////////////////////////////////////
    err write_item(const T &elt);
  
    ////////////////////////////////////////////////////////////////////////////
    /// Reads *len items from the current position of the stream into
    /// the array mm_array. The "current position" pointer is increased
    /// accordingly.
    ////////////////////////////////////////////////////////////////////////////
    err read_array(T *mm_space, TPIE_OS_OFFSET *len);
    
    ////////////////////////////////////////////////////////////////////////////
    /// Writes len items from array |mm_array to the
    /// stream, starting in the current position. The "current item"
    /// pointer is increased accordingly.
    ////////////////////////////////////////////////////////////////////////////
    err write_array(const T *mm_space, TPIE_OS_OFFSET len);
  
  
    ////////////////////////////////////////////////////////////////////////////
    /// Returns the number of items in the stream.
    ////////////////////////////////////////////////////////////////////////////
    TPIE_OS_OFFSET stream_len(void) const { 
	return m_bteStream->stream_len(); 
    }
  
    ////////////////////////////////////////////////////////////////////////////
    /// Returns the path name of this stream in newly allocated space.
    ////////////////////////////////////////////////////////////////////////////
    std::string name() const;
  
    ////////////////////////////////////////////////////////////////////////////
    /// Move the current position to off (measured in terms of items.
    ////////////////////////////////////////////////////////////////////////////
    err seek(TPIE_OS_OFFSET offset);
    
    ////////////////////////////////////////////////////////////////////////////
    /// Returns the current position in the stream measured of items from the
    /// beginning of the stream.
    ////////////////////////////////////////////////////////////////////////////
    TPIE_OS_OFFSET tell() const { 
	return m_bteStream->tell(); 
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
    err truncate(TPIE_OS_OFFSET offset);
    
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
    err main_memory_usage(TPIE_OS_SIZE_T *usage,
			  MM_stream_usage usage_type);
  
    ////////////////////////////////////////////////////////////////////////////
    /// Returns a \ref tpie_stats_stream object containing  statistics of 
    /// the stream. 
    ////////////////////////////////////////////////////////////////////////////
    const stats_stream& stats() const { 
	return m_bteStream->stats(); 
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Returns the number of globally available streams.
    /// The number should resemble the the maximum
    /// number of streams allowed (which is OS-dependent) minus the number
    /// of streams currently opened by TPIE.
    ////////////////////////////////////////////////////////////////////////////
    int available_streams(void) {
	return m_bteStream->available_streams();
    }
    
    ////////////////////////////////////////////////////////////////////////////
    /// Returns the maximum number of items (of type T) 
    /// that can be stored in one block.
    ////////////////////////////////////////////////////////////////////////////
    TPIE_OS_OFFSET chunk_size(void) const { 
	return m_bteStream->chunk_size(); 
    }
    
    ////////////////////////////////////////////////////////////////////////////
    /// Set the stream's \ref persistence flag to p, which can have one of two values:
    /// \ref PERSIST_DELETE or \ref PERSIST_PERSISTENT.}
    ////////////////////////////////////////////////////////////////////////////
    void persist(persistence p) {
	m_bteStream->persist(p);
    }
    
    ////////////////////////////////////////////////////////////////////////////
    /// Set the stram's \ref persistence flag to \ref PERSIST_PERSISTENT, thereby
    /// ensuring it is not deleted when destructed.
    ////////////////////////////////////////////////////////////////////////////
    persistence persist() const { 
	return m_bteStream->persist(); 
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Return a string describing the stream.
    // This function gives easy access to the stream's file name and its length.
    ////////////////////////////////////////////////////////////////////////////
    char *sprint();
    
private:

    /** Restricted copy constructor */
    stream(const  stream<T>& other);
    /** Restricted assignment operator*/
    stream<T>& operator=(const stream<T>& other);

    /** Pointer to a base stream, since the particular type of BTE
     * stream we are using may vary. */
    BTE_STREAM<T> * m_bteStream;
    
    /** True if stream is not writable*/
    bool m_readOnly;
    
    /** Non-zero if we should destroy the bte stream when we the
     * AMI stream is destroyed. */
    bool m_destructBTEStream;

    stream_status m_status;
};

// Create a temporary AMI stream on one of the devices in the default
// device description. Persistence is PERSIST_DELETE by default. We
// are given the index of the string describing the desired device.
	template<class T>
	stream<T>::stream(unsigned int device) : m_bteStream(NULL),
						 m_readOnly(false),
						 m_destructBTEStream(true),
						 m_status(STREAM_STATUS_INVALID)
	{

	    // [tavi] Hack to fix an error that appears in gcc 2.8.1
	    if (device == UINT_MAX) {
		device = (device_index = ((device_index + 1) % default_device.arity())); 
	    }
    
	    // Get a unique name.
		std::string path = tpie_tempnam("");
    
	    TP_LOG_DEBUG_ID("Temporary stream in file: ");
	    TP_LOG_DEBUG_ID(path);
    
	    // Create the BTE stream.
	    m_bteStream = new BTE_STREAM<T>(path, bte::WRITE_STREAM);
    
	    // (Short circuit evaluation...)
	    if (m_bteStream == NULL || 
		m_bteStream->status() == bte::STREAM_STATUS_INVALID) {

		TP_LOG_FATAL_ID("BTE returned invalid or NULL stream.");

		return;
	    }
    
	    m_bteStream->persist(PERSIST_DELETE);
    
	    if (seek(0) != NO_ERROR) {

		TP_LOG_FATAL_ID("seek(0) returned error.");

		return;
	    }

	    //  Set status to VALID.
	    m_status = STREAM_STATUS_VALID;

	};


// A stream created with this constructor will persist on disk at the
// location specified by the path name.
	template<class T>
	stream<T>::stream(const std::string& path_name, stream_type st) :
	    m_bteStream(NULL),
	    m_readOnly(false),
	    m_destructBTEStream(true),
	    m_status(STREAM_STATUS_INVALID) {
    
	    // Decide BTE stream type
	    bte::stream_type bst;

	    switch (st) {
	    case READ_STREAM: 
		bst = bte::READ_STREAM;

		break;

	    case APPEND_STREAM:
		bst = bte::APPEND_STREAM;

		break;

	    case WRITE_STREAM:
	    case READ_WRITE_STREAM:
		bst = bte::WRITE_STREAM; //WRITE_STREAM means both read and
		//write; this is inconsistent and should be modified..

		break;

	    default:

		TP_LOG_WARNING_ID("Unknown stream type passed to constructor;");
		TP_LOG_WARNING_ID("Defaulting to READ_WRITE_STREAM.");

		bst = bte::WRITE_STREAM;

		break;
	    }
    
	    m_readOnly          = (st == READ_STREAM);
	    m_destructBTEStream = true;
    
	    // Create the BTE stream.
	    m_bteStream = new BTE_STREAM<T>(path_name, bst);
	    // (Short circuit evaluation...)
	    if (m_bteStream == NULL || m_bteStream->status() == bte::STREAM_STATUS_INVALID) {

		TP_LOG_FATAL_ID("BTE returned invalid or NULL stream.");

		return;
	    }
    
	    m_bteStream->persist(PERSIST_PERSISTENT);
    
	    // If an APPEND stream, the BTE constructor seeks to its end;
	    if (st != APPEND_STREAM) {
		if (seek(0) != NO_ERROR) {

		    TP_LOG_FATAL_ID("seek(0) returned error.");

		    return;
		}
	    }
    
	    m_status = STREAM_STATUS_VALID;
	};


	template<class T>
	stream<T>::stream(BTE_STREAM<T> *bs) :
	    m_bteStream(bs),
	    m_readOnly(false),
	    m_destructBTEStream(false),
	    m_status(STREAM_STATUS_INVALID) {
    
    
	    if (m_bteStream == NULL || m_bteStream->status() == bte::STREAM_STATUS_INVALID) {

		TP_LOG_FATAL_ID("BTE returned invalid or NULL stream.");

		return;
	    }
    
	    m_readOnly = bs->read_only();
	    m_status = STREAM_STATUS_VALID;
	};


	template<class T>
	err stream<T>::new_substream(stream_type     st,
				     TPIE_OS_OFFSET  sub_begin,
				     TPIE_OS_OFFSET  sub_end,
				     stream<T>       **sub_stream)
	{
	    err retval = NO_ERROR;

	    // Check permissions. Only READ and WRITE are allowed, and only READ is
	    // allowed if m_readOnly is set.
	    if ((st != READ_STREAM) && ((st != WRITE_STREAM) || m_readOnly)) {

		*sub_stream = NULL;

		TP_LOG_DEBUG_ID("permission denied");		

		return PERMISSION_DENIED;

	    }
    
	    bte::stream_base<T> *bte_ss;
    
	    if (m_bteStream->new_substream(((st == READ_STREAM) ? bte::READ_STREAM :
					    bte::WRITE_STREAM),
					   sub_begin, sub_end,
					   &bte_ss) != bte::NO_ERROR) {

		*sub_stream = NULL;

		TP_LOG_DEBUG_ID("new_substream failed");		

		return BTE_ERROR;
	    }
    
	    stream<T> *ami_ss;
    
	    // This is a potentially dangerous downcast.  It is being done for
	    // the sake of efficiency, so that calls to the BTE can be
	    // inlined.  If multiple implementations of BTE streams are
	    // present it could be very dangerous.
    
	    ami_ss = new stream<T>(static_cast<BTE_STREAM<T>*>(bte_ss));
    
	    ami_ss->m_destructBTEStream = true;

	    retval = ami_ss->seek(0);
	    assert(retval == NO_ERROR); // sanity check
    
	    *sub_stream = ami_ss;
    
	    return retval;
	}


	template<class T>
	std::string stream<T>::name() const {
	    return m_bteStream->name();
	}

// Move to a specific offset.
	template<class T>
	err stream<T>::seek(TPIE_OS_OFFSET offset) {

	    if (m_bteStream->seek(offset) != bte::NO_ERROR) {

		TP_LOG_WARNING_ID("bte error");		

		return BTE_ERROR;
	    }
    
	    return NO_ERROR;
	}

// Truncate
	template<class T>
	err stream<T>::truncate(TPIE_OS_OFFSET offset) {

	    if (m_bteStream->truncate(offset) != bte::NO_ERROR) {

		TP_LOG_WARNING_ID("bte error");

		return BTE_ERROR;
	    }
    
	    return NO_ERROR;
	}

// Query memory usage
	template<class T>
	err stream<T>::main_memory_usage(TPIE_OS_SIZE_T *usage,
					 MM_stream_usage usage_type) {

	    if (m_bteStream->main_memory_usage(usage, usage_type) != bte::NO_ERROR) {

		TP_LOG_WARNING_ID("bte error");		

		return BTE_ERROR;
	    }
    
	    switch (usage_type) {
	    case MM_STREAM_USAGE_OVERHEAD:
	    case MM_STREAM_USAGE_CURRENT:
	    case MM_STREAM_USAGE_MAXIMUM:
	    case MM_STREAM_USAGE_SUBSTREAM:
		*usage += sizeof(*this);

		break;

	    case MM_STREAM_USAGE_BUFFER:

		break;

	    default:

		tp_assert(0, "Unknown MM_stream_usage type added.");

	    }
    
	    return NO_ERROR;
	}

	template<class T>
	stream<T>::~stream() {
	    if (m_destructBTEStream) {
		delete m_bteStream;
	    }
	}

	template<class T>
	err stream<T>::read_item(T **elt) {

	    err retval = NO_ERROR;
    
	    bte::err bte_err = m_bteStream->read_item(elt);

	    switch(bte_err) {
	    case bte::NO_ERROR:
		retval = NO_ERROR;

		break;

	    case bte::END_OF_STREAM:

		retval = END_OF_STREAM;

		TP_LOG_DEBUG_ID("eos in read_item");

		break;

	    default:
		retval = BTE_ERROR;

		TP_LOG_DEBUG_ID("bte error in read_item");

		break;
	    }

	    return retval;
	}

	template<class T>
	err stream<T>::write_item(const T &elt) {

	    if (m_bteStream->write_item(elt) != bte::NO_ERROR) {

		TP_LOG_WARNING_ID("bte error");

		return BTE_ERROR;

	    }

	    return NO_ERROR;
	}


	template<class T>
	err stream<T>::read_array(T *mm_space, TPIE_OS_OFFSET *len) {
	    bte::err       be = bte::NO_ERROR;
	    T *            read;
	    TPIE_OS_OFFSET ii;
    
	    // How long is it.
	    TPIE_OS_OFFSET str_len = *len;
            
	    // Read them all.
	    for (ii = str_len; ii--; ) {
		if ((be = m_bteStream->read_item(&read)) != bte::NO_ERROR) {

		    if (be == bte::END_OF_STREAM) {
			return END_OF_STREAM;
		    }
		    else { 
			return BTE_ERROR;
		    }
		}
		*mm_space++ = *read;
	    }
    
	    *len = str_len;
    
	    return NO_ERROR;
	}

	template<class T>
	err stream<T>::write_array(const T *mm_space, TPIE_OS_OFFSET len) {
	    bte::err       be = bte::NO_ERROR;
	    TPIE_OS_OFFSET ii;
    
	    for (ii = len; ii--; ) {
		if ((be = m_bteStream->write_item(*mm_space++)) != bte::NO_ERROR) {

		    if (be == bte::END_OF_STREAM) {
			return END_OF_STREAM;
		    } 
		    else { 
			return BTE_ERROR;
		    }
		}
	    }
	    return NO_ERROR;
	}        

	template<class T>
	char *stream<T>::sprint() {
	    static char buf[BUFSIZ];
	    std::string s(name());
	    sprintf(buf, "[STREAM %s %ld]", s.c_str(), static_cast<long>(stream_len()));
	    return buf;
	}

    }  //  ami namespace

}  //  tpie namespace

#include <tpie/stream_compatibility.h>

#endif // _TPIE_AMI_STREAM_H
