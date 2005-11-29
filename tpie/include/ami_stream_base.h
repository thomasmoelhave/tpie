#ifndef _AMI_STREAM_BASE_H
#define _AMI_STREAM_BASE_H

#include <portability.h>

#include <ami_device.h>
#include <bte_stream_base_generic.h>
#include <tpie_stats_stream.h>

// An initializer class to set the default device for the
// AMI_stream_single_base class.
class AMI_stream_base_device_initializer {
private:
    static unsigned int count;
public:
    AMI_stream_base_device_initializer();

    ~AMI_stream_base_device_initializer() {
	// Do nothing.
    }
};

// A base class for AMI single streams that is used to hold the
// default device description for AMI single streams regardless of the
// particular type of object in the stream.

class AMI_stream_base {
    friend AMI_stream_base_device_initializer::
    AMI_stream_base_device_initializer();
public:
    // The default device description for AMI streams.
    static AMI_device default_device;
    
    // The index into the device list for the next stream.    
    static unsigned int device_index;    
    
    static const tpie_stats_stream& gstats() { 
	return BTE_stream_base_generic::gstats(); 
    }
};


// This is a trick to make sure that at least one initializer is declared.
// The constructor for this initializer will make sure that the default
// device is set up properly.
extern AMI_stream_base_device_initializer one_sbd_initializer_per_source_file;

#endif // _AMI_STREAM_BASE_H
