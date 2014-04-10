// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014, The TPIE development team
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

#include <tpie/paths.h>
#include <tpie/tpie_log.h>
#include <tpie/exception.h>
#include <sstream>
#include <cstdlib>

#ifdef WIN32

#	include <windows.h>
#	undef NO_ERROR
#	include <Shlobj.h>

#else // Linux

// For getuid
#	include <unistd.h>

// For getpwuid
#	include <sys/types.h>
#	include <pwd.h>

#endif

namespace tpie {

std::string get_time_estimation_database_path() {
#ifdef WIN32
	TCHAR p[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, p))) {
		return p;
	} else {
		throw tpie::exception("SHGetFolderPath failed");
	}
#else
	std::string dir_name;
	const char * p = getenv("HOME");
	if (p != 0) dir_name=p;
	if (dir_name == "") dir_name = getpwuid(getuid())->pw_dir;
	return dir_name;
#endif
}

} // namespace tpie

namespace {

std::string get_system_path() {
#ifdef WIN32
	CHAR temp_path[MAX_PATH];

	if (GetTempPath(MAX_PATH, temp_path) != 0) {
		return std::string(temp_path);
	} else {
		tpie::log_warning()
			<< "Could not get default system path (GetTempPath), "
			<< "using current working dir." << std::endl;
		return ".";
	}
#else
	return "/var/tmp";
#endif
}

} // unnamed namespace

namespace tpie {

std::string get_temp_path() {
	std::string dir;
	if(getenv(AMI_SINGLE_DEVICE_ENV) != NULL)  //TPIE env variable
		dir = getenv(AMI_SINGLE_DEVICE_ENV);
	else if(getenv(TMPDIR_ENV) != NULL)
		dir = getenv(TMPDIR_ENV); //OS env variable (from portability.h)
	else
		dir = get_system_path(); //OS path

	return dir;
}

} // namespace tpie
