//
// File: tpie_tempnam.h
// Author: 
// Created: 02/02/02
//
// $Id: tpie_tempnam.h,v 1.4 2004-04-16 21:33:29 adanner Exp $
//
//
#ifndef _TPIE_TEMPNAM_H
#define _TPIE_TEMPNAM_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// The name of the environment variable pointing to a tmp directory.
#define TMPDIR_ENV "TMPDIR"

// The name of the environment variable to consult for default device
// descriptions.
#define AMI_SINGLE_DEVICE_ENV "AMI_SINGLE_DEVICE"

namespace tpie {

	class tempname {
	public:	
		static std::string tpie_name(const std::string& post_base = "", const std::string& dir = "", const std::string& ext = ""); 

		static void set_default_path(const std::string& path);
		static void set_default_base_name(const std::string& name);
		static void set_default_extension(const std::string& ext);

		static const std::string& get_default_path();
		static const std::string& get_default_base_name();
		static const std::string& get_default_extension();


		/////////////////////////////////////////////////////////
		///
		/// Return The actual path used for temporary files
		///	taking environment variables into account. The path is 
		/// the found by querying the following in order:
		///
		/// TPIE2
		///    TPIE global temp dir as set using set_default_path from source:/trunk/tpie/tempname.h 
		/// TPIE1
		///    TPIE environment varables (currently AMI_SINGLE_DEVICE) 
		/// OS2
		///    OS specific environment variables (like TMPDIR/TMP) 
		/// OS1
		///    OS specific standard path (/tmp or /var/tmp on unices and some windows thing for windows) 
		///
		/// \returns a string containing the path
		///
		/////////////////////////////////////////////////////////
		static std::string get_actual_path();
	
	private:
		static std::string default_path;
		static std::string default_base_name; 
		static std::string default_extension;

		static std::string tpie_mktemp();
	};
}

#endif // _TPIE_TEMPNAM_H 
