//
// File: ami_stream.h (formerly part of ami.h and ami_imps.h)
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
//
// $Id: ami_stream.h,v 1.10 2005-11-17 17:11:25 jan Exp $
//
#ifndef _AMI_STREAM_H
#define _AMI_STREAM_H

// Get definitions for working with Unix and Windows
#include <portability.h>

// include definition of VERSION macro
#include <versions.h>

// Include the configuration header.
#include <config.h>

// Get the error codes.
#include <err.h>

// Get the device description class
#include <device.h>

// Get an appropriate BTE.  Flags may have been set to determine
// exactly what BTE implementaion will be used, but that should be of
// little concern to us.  bte/stream.h and recursively included files
// will worry about parsing the appropriate flags and getting us an
// implementation.
#include <bte/stream.h>

#include <tempname.h>

// AMI stream types passed to constructors
enum AMI_stream_type {
    AMI_READ_STREAM = 1,	// Open existing stream for reading
    AMI_WRITE_STREAM,		// Open for writing.  Create if non-existent
    AMI_APPEND_STREAM,		// Open for writing at end.  Create if needed.
    AMI_READ_WRITE_STREAM	// Open to read and write.
};

// AMI stream status.
enum AMI_stream_status {
    AMI_STREAM_STATUS_VALID = 0,
    AMI_STREAM_STATUS_INVALID
};

#include <stream_base.h>

template<class T> 
class AMI_stream : public AMI_stream_base {
    
public:

//    AMI_stream() { 
//	m_status = AMI_STREAM_STATUS_INVALID;
//    }
    
    // We have a variety of constructors for different uses.
    
    // A temporary AMI_stream using the default BTE stream type and
    // a temporary space on the disk or in some file system.
    AMI_stream(unsigned int device = UINT_MAX);
    
    // An AMI stream based on a specific path name.
    AMI_stream(const char *path_name, 
	       AMI_stream_type st = AMI_READ_WRITE_STREAM);
    
    // An AMI stream based on a specific existing BTE stream.  Note
    // that in this case the BTE stream will not be detroyed when the
    // destructor is called.
    AMI_stream(BTE_STREAM<T> *bs);

    // A psuedo-constructor for substreams.
    AMI_err new_substream(AMI_stream_type st, 
			  TPIE_OS_OFFSET  sub_begin, 
			  TPIE_OS_OFFSET  sub_end,
			  AMI_stream<T> **sub_stream);
  
    ~AMI_stream();
    
    // Inquire the status.
    AMI_stream_status status() const { 
	return m_status; 
    }
    
    bool is_valid() const { 
	return m_status == AMI_STREAM_STATUS_VALID; 
    }
    
    bool operator!() const { 
	return !is_valid(); 
    }
    
    AMI_err read_item(T **elt);
    AMI_err write_item(const T &elt);
  
    AMI_err read_array(T *mm_space, TPIE_OS_OFFSET *len);
    AMI_err write_array(const T *mm_space, TPIE_OS_OFFSET len);
  
  
    // Return the number of items in the stream.
    TPIE_OS_OFFSET stream_len(void) const { 
	return m_bteStream->stream_len(); 
    }
  
    // Return the path name of this stream in newly allocated space.
    AMI_err name(char **stream_name);
  
    // Move to a specific position in the stream.
    AMI_err seek(TPIE_OS_OFFSET offset);
    
    // Return the current position in the stream.
    TPIE_OS_OFFSET tell() const { 
	return m_bteStream->tell(); 
    }

    // Truncate
    AMI_err truncate(TPIE_OS_OFFSET offset);
    
    // Query memory usage
    AMI_err main_memory_usage(TPIE_OS_SIZE_T *usage,
			      MM_stream_usage usage_type);
  
    const tpie_stats_stream& stats() const { 
	return m_bteStream->stats(); 
    }

    int available_streams(void) {
	return m_bteStream->available_streams();
    }
    
    TPIE_OS_OFFSET chunk_size(void) const { 
	return m_bteStream->chunk_size(); 
    }
    
    void persist(persistence p) {
	m_bteStream->persist(p);
    }
    
    persistence persist() const { 
	return m_bteStream->persist(); 
    }

    char *sprint();
    
private:

    // Prohibit these two.
    AMI_stream(const  AMI_stream<T>& other);
    AMI_stream<T>& operator=(const AMI_stream<T>& other);

    // Point to a base stream, since the particular type of BTE
    // stream we are using may vary.
    BTE_STREAM<T> * m_bteStream;
    
    bool m_readOnly;
    
    // Non-zero if we should destroy the bte stream when we the
    // AMI stream is destroyed.
    bool m_destructBTEStream;

    AMI_stream_status m_status;
};

// Create a temporary AMI stream on one of the devices in the default
// device description. Persistence is PERSIST_DELETE by default. We
// are given the index of the string describing the desired device.
template<class T>
AMI_stream<T>::AMI_stream(unsigned int device) : m_bteStream(NULL),
						 m_readOnly(false),
						 m_destructBTEStream(true),
						 m_status(AMI_STREAM_STATUS_INVALID)
{

    // [tavi] Hack to fix an error that appears in gcc 2.8.1
    if (device == UINT_MAX) {
	device = (device_index = ((device_index + 1) % default_device.arity())); 
    }
    
    // Get a unique name.
    char *path = tpie_tempnam("AMI", default_device[device]);
    
    TP_LOG_DEBUG_ID("Temporary stream in file: ");
    TP_LOG_DEBUG_ID(path);
    
    // Create the BTE stream.
    m_bteStream = new BTE_STREAM<T>(path, BTE_WRITE_STREAM);
    
    // (Short circuit evaluation...)
    if (m_bteStream == NULL || 
	m_bteStream->status() == BTE_STREAM_STATUS_INVALID) {
	TP_LOG_FATAL_ID("BTE returned invalid or NULL stream.");
	return;
    }
    
    m_bteStream->persist(PERSIST_DELETE);
    
    if (seek(0) != AMI_ERROR_NO_ERROR) {
	TP_LOG_FATAL_ID("seek(0) returned error.");
	return;
    }

    //  Set status to VALID.
    m_status = AMI_STREAM_STATUS_VALID;
};


// A stream created with this constructor will persist on disk at the
// location specified by the path name.
template<class T>
AMI_stream<T>::AMI_stream(const char *path_name,
			  AMI_stream_type st) :
    m_bteStream(NULL),
    m_readOnly(false),
    m_destructBTEStream(true),
    m_status(AMI_STREAM_STATUS_INVALID) {
    
  // Decide BTE stream type
    BTE_stream_type bst;
    switch (st) {
    case AMI_READ_STREAM: 
	bst = BTE_READ_STREAM;
	break;
    case AMI_APPEND_STREAM:
	bst = BTE_APPEND_STREAM;
	break;
    case AMI_WRITE_STREAM:
    case AMI_READ_WRITE_STREAM:
	bst = BTE_WRITE_STREAM; //BTE_WRITE_STREAM means both read and
	//write; this is inconsistent and should be modified..
	break;
    default:
	TP_LOG_WARNING_ID("Unknown stream type passed to constructor;");
	TP_LOG_WARNING_ID("Defaulting to AMI_READ_WRITE_STREAM.");
	bst = BTE_WRITE_STREAM;
	break;
    }
    
    m_readOnly          = (st == AMI_READ_STREAM);
    m_destructBTEStream = true;
    
    // Create the BTE stream.
    m_bteStream = new BTE_STREAM<T>(path_name, bst);
    // (Short circuit evaluation...)
    if (m_bteStream == NULL || m_bteStream->status() == BTE_STREAM_STATUS_INVALID) {
	TP_LOG_FATAL_ID("BTE returned invalid or NULL stream.");
	return;
    }
    
    m_bteStream->persist(PERSIST_PERSISTENT);
    
    // If an APPEND stream, the BTE constructor seeks to its end;
    if (st != AMI_APPEND_STREAM) {
	if (seek(0) != AMI_ERROR_NO_ERROR) {
	    TP_LOG_FATAL_ID("seek(0) returned error.");
	    return;
	}
    }
    
    m_status = AMI_STREAM_STATUS_VALID;
};


template<class T>
AMI_stream<T>::AMI_stream(BTE_STREAM<T> *bs) :
    m_bteStream(bs),
    m_readOnly(false),
    m_destructBTEStream(false),
    m_status(AMI_STREAM_STATUS_INVALID) {
    
    
    if (m_bteStream == NULL || m_bteStream->status() == BTE_STREAM_STATUS_INVALID) {
	TP_LOG_FATAL_ID("BTE returned invalid or NULL stream.");
	return;
    }
    
    m_readOnly = bs->read_only();
    m_status = AMI_STREAM_STATUS_VALID;
};


template<class T>
AMI_err AMI_stream<T>::new_substream(AMI_stream_type st,
				     TPIE_OS_OFFSET  sub_begin,
				     TPIE_OS_OFFSET  sub_end,
				     AMI_stream<T> **sub_stream)
{
    AMI_err ae = AMI_ERROR_NO_ERROR;
    // Check permissions. Only READ and WRITE are allowed, and only READ is
    // allowed if m_readOnly is set.
    if ((st != AMI_READ_STREAM) && ((st != AMI_WRITE_STREAM) || m_readOnly)) {
        *sub_stream = NULL;
	TP_LOG_DEBUG_ID("permission denied");		
        return AMI_ERROR_PERMISSION_DENIED;
    }
    
    BTE_stream_base<T> *bte_ss;
    
    if (m_bteStream->new_substream(((st == AMI_READ_STREAM) ? BTE_READ_STREAM :
				    BTE_WRITE_STREAM),
				   sub_begin, sub_end,
				   &bte_ss) != BTE_ERROR_NO_ERROR) {
	TP_LOG_DEBUG_ID("new_substream failed");		
	*sub_stream = NULL;
	return AMI_ERROR_BTE_ERROR;
    }
    
    AMI_stream<T> *ami_ss;
    
    // This is a potentially dangerous downcast.  It is being done for
    // the sake of efficiency, so that calls to the BTE can be
    // inlined.  If multiple implementations of BTE streams are
    // present it could be very dangerous.
    
    ami_ss = new AMI_stream<T>(static_cast<BTE_STREAM<T>*>(bte_ss));
    
    ami_ss->m_destructBTEStream = true;
    ae = ami_ss->seek(0);
    assert(ae == AMI_ERROR_NO_ERROR); // sanity check
    
    *sub_stream = ami_ss;
    
    return ae;
}


template<class T>
AMI_err AMI_stream<T>::name(char **stream_name) {
    BTE_err be = m_bteStream->name(stream_name);
    if (be != BTE_ERROR_NO_ERROR) {
	TP_LOG_WARNING_ID("bte error");
	return AMI_ERROR_BTE_ERROR;
    } else {
	return AMI_ERROR_NO_ERROR;
    }
}

// Move to a specific offset.
template<class T>
AMI_err AMI_stream<T>::seek(TPIE_OS_OFFSET offset) {
    if (m_bteStream->seek(offset) != BTE_ERROR_NO_ERROR) {
	TP_LOG_WARNING_ID("bte error");		
	return AMI_ERROR_BTE_ERROR;
    }
    
    return AMI_ERROR_NO_ERROR;
}

// Truncate
template<class T>
AMI_err AMI_stream<T>::truncate(TPIE_OS_OFFSET offset) {
    if (m_bteStream->truncate(offset) != BTE_ERROR_NO_ERROR) {
	TP_LOG_WARNING_ID("bte error");
	return AMI_ERROR_BTE_ERROR;
    }
    
    return AMI_ERROR_NO_ERROR;
}

// Query memory usage
template<class T>
AMI_err AMI_stream<T>::main_memory_usage(TPIE_OS_SIZE_T *usage,
                                                MM_stream_usage usage_type) {
    if (m_bteStream->main_memory_usage(usage, usage_type) != BTE_ERROR_NO_ERROR) {
	TP_LOG_WARNING_ID("bte error");		
	return AMI_ERROR_BTE_ERROR;
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
	break;
    }
    
    return AMI_ERROR_NO_ERROR;
}

template<class T>
AMI_stream<T>::~AMI_stream() {
    if (m_destructBTEStream) {
        delete m_bteStream;
    }
}

template<class T>
AMI_err AMI_stream<T>::read_item(T **elt) {
    BTE_err bte_err;
    AMI_err ae;
    
    bte_err = m_bteStream->read_item(elt);
    switch(bte_err) {
    case BTE_ERROR_NO_ERROR:
	ae = AMI_ERROR_NO_ERROR;
	break;
    case BTE_ERROR_END_OF_STREAM:
	TP_LOG_DEBUG_ID("eos in read_item");
	ae = AMI_ERROR_END_OF_STREAM;
	break;
    default:
	TP_LOG_DEBUG_ID("bte error in read_item");
	ae = AMI_ERROR_BTE_ERROR;
	break;
    }
    return ae;
}

template<class T>
AMI_err AMI_stream<T>::write_item(const T &elt) {
    if (m_bteStream->write_item(elt) != BTE_ERROR_NO_ERROR) {
	TP_LOG_WARNING_ID("bte error");
	return AMI_ERROR_BTE_ERROR;
    }
    return AMI_ERROR_NO_ERROR;
}


template<class T>
AMI_err AMI_stream<T>::read_array(T *mm_space, TPIE_OS_OFFSET *len) {
    BTE_err        be;
    T *            read;
    TPIE_OS_OFFSET ii;
    
    // How long is it.
    TPIE_OS_OFFSET str_len = *len;
            
    // Read them all.
    for (ii = str_len; ii--; ) {
        if ((be = m_bteStream->read_item(&read)) != BTE_ERROR_NO_ERROR) {
            if (be == BTE_ERROR_END_OF_STREAM) {
                return AMI_ERROR_END_OF_STREAM;
            } else { 
                return AMI_ERROR_BTE_ERROR;
            }
        }
        *mm_space++ = *read;
    }
    
    *len = str_len;
    
    return AMI_ERROR_NO_ERROR;
}

template<class T>
AMI_err AMI_stream<T>::write_array(const T *mm_space, TPIE_OS_OFFSET len) {
    BTE_err        be;
    TPIE_OS_OFFSET ii;
    
    for (ii = len; ii--; ) {
        if ((be = m_bteStream->write_item(*mm_space++)) != BTE_ERROR_NO_ERROR) {
            if (be == BTE_ERROR_END_OF_STREAM) {
                return AMI_ERROR_END_OF_STREAM;
            } else { 
                return AMI_ERROR_BTE_ERROR;
            }
        }
    }
    return AMI_ERROR_NO_ERROR;
}        


// sprint()
// Return a string describing the stream
//
// This function gives easy access to the file name, length.
// It is not reentrant, but this should not be too much of a problem 
// if you are careful.
template<class T>
char *AMI_stream<T>::sprint() {
    static char buf[BUFSIZ];
    char *s;
    name(&s);
    sprintf(buf, "[AMI_STREAM %s %ld]", s, static_cast<long>(stream_len()));
    delete s;
    return buf;
}


#include <stream_compatibility.h>

#endif // _AMI_STREAM_H
