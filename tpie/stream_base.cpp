//
// File: ami_stream_base.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 8/24/93
//

#include <tpie/stream_base.h>

#include "lib_config.h"

#include <tpie/tempname.h>

using namespace tpie::ami;

// The default device description for AMI single streams.
device stream_base::default_device;

// The device index of the most recently created stream.
unsigned int stream_base::device_index;

// Initializer
unsigned int stream_base_device_initializer::count;

// Trick to declare at least one initializer
// The constructor for this will set up the default device properly
stream_base_device_initializer one_sbd_initializer_per_source_file;

stream_base_device_initializer::stream_base_device_initializer(){

    err ae;
    
    if (!count++) {
        // Try to initialize from the environment.
	ae = stream_base::default_device.read_environment(AMI_SINGLE_DEVICE_ENV);
	
        if (ae == NO_ERROR) {        
            return;
        }
	
        // Try to initialize from TMP_DIR
        ae = stream_base::default_device.read_environment(TMPDIR_ENV);
	
        if (ae == NO_ERROR) {
            return;
        }
	
        // Try to initialize to a default path
        ae = stream_base::default_device.set_to_path(TMP_DIR "|" TMP_DIR);
	
        if (ae != NO_ERROR) {
            TP_LOG_WARNING_ID("Unable to initialize the default device description for AMI single streams.");
        }
	
        TP_LOG_DEBUG_ID("Default device description for AMI single streams:");

//  CHECK THIS: How do we output ami::devide (the operator<< is gone)?
//        TP_LOG_DEBUG_ID(stream_base::default_device);
	
        // Set the last device index used to the last device index, so
        // that the first stream will wrap around to go on device 0.
        stream_base::device_index = stream_base::default_device.arity() - 1;
    }
}
