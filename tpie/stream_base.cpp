//
// File: ami_stream_base.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 8/24/93
//

#include <stream_base.h>

#include <versions.h>
VERSION(ami_stream_base_cpp,"$Id:");

#include "lib_config.h"

// For AMI_SINGLE_DEVICE_ENV and TMPDIR_ENV.
#include <tempname.h>

// The default device description for AMI single streams.
AMI_device AMI_stream_base::default_device;

// The device index of the most recently created stream.
unsigned int AMI_stream_base::device_index;

// Initializer
unsigned int AMI_stream_base_device_initializer::count;

// Trick to declare at least one initializer
// The constructor for this will set up the default device properly
AMI_stream_base_device_initializer one_sbd_initializer_per_source_file;

AMI_stream_base_device_initializer::AMI_stream_base_device_initializer(){
    AMI_err ae;
    
    if (!count++) {
        // Try to initialize from the environment.
        ae = AMI_stream_base::
            default_device.read_environment(AMI_SINGLE_DEVICE_ENV);
	
        if (ae == AMI_ERROR_NO_ERROR) {        
            return;
        }
	
        // Try to initialize from TMP_DIR
        ae = AMI_stream_base::
            default_device.read_environment(TMPDIR_ENV);
	
        if (ae == AMI_ERROR_NO_ERROR) {
            return;
        }
	
        // Try to initialize to a default path
        ae = AMI_stream_base::
            default_device.set_to_path(TMP_DIR "|" TMP_DIR);
	
        if (ae != AMI_ERROR_NO_ERROR) {
            TP_LOG_WARNING_ID("Unable to initialize the default device description for AMI single streams.");
        }
	
        TP_LOG_DEBUG_ID("Default device description for AMI single streams:");
        TP_LOG_DEBUG_ID(AMI_stream_base::default_device);
	
        // Set the last device index used to the last device index, so
        // that the first stream will wrap around to go on device 0.
        AMI_stream_base::device_index =
            AMI_stream_base::default_device.arity() - 1;
    }
}
