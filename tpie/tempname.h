// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2012, The TPIE development team
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

///////////////////////////////////////////////////////////////////////////////
/// \file tempname.h  Temporary file names
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_TEMPNAM_H
#define _TPIE_TEMPNAM_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
#include <tpie/stats.h>
#include <stdexcept>
#include <boost/utility.hpp>
// The name of the environment variable pointing to a tmp directory.
#define TMPDIR_ENV "TMPDIR"

// The name of the environment variable to consult for default device
// descriptions.
#define AMI_SINGLE_DEVICE_ENV "AMI_SINGLE_DEVICE"

namespace tpie {

	struct tempfile_error: public std::runtime_error {
		explicit tempfile_error(const std::string & what): std::runtime_error(what) {}
	};

	///////////////////////////////////////////////////////////////////////////
	/// \brief Static methods for generating temporary file names and finding
	/// temporary file directories.
	///////////////////////////////////////////////////////////////////////////
	class tempname {
	public:
		///////////////////////////////////////////////////////////////////////
		/// \brief Generate path for a new temporary file.
		///
		/// The temporary file name consists of a base name, set using
		/// \ref set_default_base_name and defaulting to "TPIE"; an optional
		/// post base name, given as parameter to tpie_name; and a random
		/// string of characters.  The parts are joined with underscores, and a
		/// file extension, either given as parameter to tpie_name or to \ref
		/// set_default_extension.  If neither is used, "tpie" will be used as
		/// the extension.
		///
		/// This file name is suffixed a temporary directory passed as a
		/// parameter. If no temporary directory is passed, the directory
		/// reported by \ref get_actual_path is used instead.
		///
		/// The path returned does not already exist on the filesystem.
		///////////////////////////////////////////////////////////////////////
		static std::string tpie_name(const std::string& post_base = "",
									 const std::string& dir = "",
									 const std::string& ext = "");

		///////////////////////////////////////////////////////////////////////
		/// \brief Generate path for a new temporary directory.
		///
		/// The rules for the generated path are the same as \ref tpie_name,
		/// except no file extension is added.
		///////////////////////////////////////////////////////////////////////
		static std::string tpie_dir_name(const std::string& post_base = "",
										 const std::string& dir = "");

		///////////////////////////////////////////////////////////////////////
		/// \brief Get the default path for temporary files on the system.
		///////////////////////////////////////////////////////////////////////
		static std::string get_system_path();

		///////////////////////////////////////////////////////////////////////
		/// \brief Sets the default temporary path.
		///
		/// \param path The default path to use; this path must exist in the system.
		/// \param subdir Subdirectory of the temporary path, will be created if it does not exist.
		///////////////////////////////////////////////////////////////////////
		static void set_default_path(const std::string& path, const std::string& subdir="");

		///////////////////////////////////////////////////////////////////////
		/// \brief Set default base name for temporary files.
		/// \sa tpie_name
		///////////////////////////////////////////////////////////////////////
		static void set_default_base_name(const std::string& name);

		///////////////////////////////////////////////////////////////////////
		/// \brief Set default extension for temporary files.
		/// \sa tpie_name
		///////////////////////////////////////////////////////////////////////
		static void set_default_extension(const std::string& ext);

		///////////////////////////////////////////////////////////////////////
		/// \brief Get default path for directory containing temporary files if
		/// one is set using \ref set_default_path.
		/// \sa get_actual_path
		///////////////////////////////////////////////////////////////////////
		static const std::string& get_default_path();

		///////////////////////////////////////////////////////////////////////
		/// \brief Get default base name for temporary files if one is set
		/// using \ref set_default_base_name.
		/// \sa set_default_base_name
		///////////////////////////////////////////////////////////////////////
		static const std::string& get_default_base_name();

		///////////////////////////////////////////////////////////////////////
		/// \brief Get default extension for temporary files if one is set
		/// using \ref set_default_extension.
		/// \sa set_default_extension
		///////////////////////////////////////////////////////////////////////
		static const std::string& get_default_extension();


		///////////////////////////////////////////////////////////////////////
		/// Return The actual path used for temporary files taking environment
		/// variables into account. The path is the found by querying the
		/// following in order:
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
		/// \return A string containing the path.
		///////////////////////////////////////////////////////////////////////
		static std::string get_actual_path();

	private:
		static std::string default_path;
		static std::string default_base_name;
		static std::string default_extension;

		static std::string tpie_mktemp();
	};

	///////////////////////////////////////////////////////////////////////////
	/// \brief Class representing the existence of a temporary file.
	/// When a temp_file object goes out of scope and is not set to persistent,
	/// the associated temporary file will be deleted.
	///////////////////////////////////////////////////////////////////////////
	class temp_file : boost::noncopyable {
	private:
		std::string m_path;
		bool m_persist;
		stream_size_type m_recordedSize;
	public:
		///////////////////////////////////////////////////////////////////////
		/// \returns Whether this file should not be deleted when this object
		/// goes out of scope.
		///////////////////////////////////////////////////////////////////////
		inline bool is_persistent() const {return m_persist;}

		///////////////////////////////////////////////////////////////////////
		/// \brief Set persistence. When true, the file will not be deleted
		/// when this goes out of scope.
		///////////////////////////////////////////////////////////////////////
		inline void set_persistent(bool p) {m_persist=p;}

		///////////////////////////////////////////////////////////////////////
		/// \brief Create a temp_file associated with a specific file.
		///////////////////////////////////////////////////////////////////////
		temp_file(const std::string & path, bool persist=false);

		///////////////////////////////////////////////////////////////////////
		/// \brief Associate with a specific file.
		///////////////////////////////////////////////////////////////////////
		void set_path(const std::string & path, bool persist=false);

		void update_recorded_size(stream_size_type size) {
			increment_temp_file_usage(static_cast<stream_offset_type>(size)-
									  static_cast<stream_offset_type>(m_recordedSize));
			m_recordedSize=size;
			
		}

		///////////////////////////////////////////////////////////////////////
		/// \brief Create a temp_file and generate a random temporary file
		/// name.
		///////////////////////////////////////////////////////////////////////
		temp_file();

		///////////////////////////////////////////////////////////////////////
		/// \brief temp_file destructor. If not persistent, unlink the
		/// associated file.
		///////////////////////////////////////////////////////////////////////
		~temp_file();

		///////////////////////////////////////////////////////////////////////
		/// \brief Get the path of the associated file.
		///////////////////////////////////////////////////////////////////////
		const std::string & path();

		///////////////////////////////////////////////////////////////////////
		/// \brief If not persistent, unlink the associated file.
		///////////////////////////////////////////////////////////////////////
		void free();
	};
	
}

#endif // _TPIE_TEMPNAM_H 
