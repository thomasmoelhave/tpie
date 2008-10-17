#ifndef _TPIE_AMI_STREAM_BASE_H
#define _TPIE_AMI_STREAM_BASE_H

#include <tpie/portability.h>

#include <tpie/device.h>
#include <tpie/bte/stream_base_generic.h>

namespace tpie {

    namespace ami {

// An initializer class to set the default device for the
// AMI_stream_single_base class.
	class stream_base_device_initializer {

	private:
	    static unsigned int count;

	public:
	    stream_base_device_initializer();
	    
	    ~stream_base_device_initializer() {
		// Do nothing.
	    }
	};

    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {

// A base class for AMI single streams that is used to hold the
// default device description for AMI single streams regardless of the
// particular type of object in the stream.
	
	class stream_base {

	    friend stream_base_device_initializer::stream_base_device_initializer();
	    
	public:
	    // The default device description for AMI streams.
	    static device default_device;
	    
	    // The index into the device list for the next stream.    
	    static unsigned int device_index;    
	    
	    static const stats_stream& gstats() { 
		return bte::stream_base_generic::gstats(); 
	    }
	};
	
	
// This is a trick to make sure that at least one initializer is declared.
// The constructor for this initializer will make sure that the default
// device is set up properly.
	extern stream_base_device_initializer one_sbd_initializer_per_source_file;

    }  //  ami namespace

}  //  tpie namespace

#endif // _TPIE_AMI_STREAM_BASE_H
