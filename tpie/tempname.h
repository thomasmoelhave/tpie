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

#ifndef _TPIE_TEMPNAM_H
#define _TPIE_TEMPNAM_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
#include <tpie/util.h>

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

	class temp_file {
	private:
		std::string m_path;
	public:
		temp_file();
		~temp_file();
		const std::string & path();
	};
	
}

#endif // _TPIE_TEMPNAM_H 
