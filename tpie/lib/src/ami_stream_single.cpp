//
// File: ami_stream_single.cpp (formerly ami_single.cpp)
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 8/24/93
//

#include <versions.h>
VERSION(ami_single_cpp,"$Id: ami_stream_single.cpp,v 1.1 2002-01-14 17:22:26 tavi Exp $");

#include <unistd.h>
#include "lib_config.h"

// Don't bother defining any BTE implementation, since this is library
// code for the AMI.
#define BTE_STREAM_IMP_USER_DEFINED
#define BTE_STREAM BTE_stream_base
#define AMI_STREAM_IMP_SINGLE
#include <ami_stream.h>

// The default device description for AMI single streams.
AMI_device AMI_stream_single_base::default_device;

// The device index of the most recently created stream.
unsigned int AMI_stream_single_base::device_index;

// Initializer

unsigned int AMI_stream_single_base_device_initializer::count;

AMI_stream_single_base_device_initializer::
    AMI_stream_single_base_device_initializer(void)
{
    AMI_err ae;
        
    if (!count++) {
        // Try to initialize from the environment.
        ae = AMI_stream_single_base::
            default_device.read_environment(AMI_SINGLE_DEVICE_ENV);

        if (ae == AMI_ERROR_NO_ERROR) {        
            return;
        }

        // Try to initialize from TMP_DIR
        ae = AMI_stream_single_base::
            default_device.read_environment(TMP_DIR_ENV);

        if (ae == AMI_ERROR_NO_ERROR) {
            return;
        }

        // Try to initialize to a default path
        ae = AMI_stream_single_base::
            default_device.set_to_path(TMP_DIR ":" TMP_DIR);

        if (ae != AMI_ERROR_NO_ERROR) {
            LOG_WARNING_ID("Unable to initialize the default device description for AMI single streams.");
        }

        LOG_DEBUG_ID("Default device description for AMI single streams:");
        LOG_DEBUG_ID(AMI_stream_single_base::default_device);

        // Set the last device index used to the last device index, so
        // that the first stream will wrap around to go on device 0.
        AMI_stream_single_base::device_index =
            AMI_stream_single_base::default_device.arity() - 1;
    }
}

AMI_stream_single_base_device_initializer::
    ~AMI_stream_single_base_device_initializer(void)
{
}


/* like tempnam, but consults environment in an order we like note
 * that the returned pointer is to static storage, so this function is
 * not re-entrant. */
char *
ami_single_temp_name(char *base) {
  char *base_dir;
  static char tmp_path[BUFSIZ];
  char *path;

  // get the dir
  base_dir = getenv(AMI_SINGLE_DEVICE_ENV);
  if (base_dir == NULL) {
	base_dir = getenv(TMP_DIR_ENV);
	if (base_dir == NULL) {
	  base_dir = TMP_DIR;
	}
  }

  sprintf(tmp_path, "%s/%s_XXXXXX", base_dir, base);
  path = mktemp(tmp_path);
  if(!path) {
	LOG_FATAL_ID("could not mktemp");
	LOG_FATAL_ID(tmp_path);
	LOG_FATAL_ID(path);
	tp_assert(path, "No temporary path name returned.");
  }
  return path;
}
