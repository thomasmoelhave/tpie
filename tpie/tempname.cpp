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

#include <tpie/config.h>
#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <cstring>
#include <tpie/tempname.h>
#include <tpie/tpie_log.h>
#include <string>
#include <tpie/portability.h>
#include <boost/filesystem.hpp>
#include <boost/random.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <stdexcept>
#include <tpie/util.h>
#include <tpie/err.h>

#ifdef _WIN32
#include <Windows.h>
#undef NO_ERROR
#endif

using namespace tpie;

boost::rand48 prng(42);

namespace {

std::string default_path;
std::string default_base_name;
std::string default_extension;
std::string tpie_mktemp();

}

std::string tempname::get_system_path() {
#ifdef WIN32
	//set temporary path
	CHAR temp_path[MAX_PATH];
		
	if (GetTempPath(MAX_PATH,temp_path) != 0) {
		return std::string(temp_path);
	} else {
		TP_LOG_WARNING_ID("Could not get default system path, using current working dir.\n");
		return ".";
	}
#else
	return "/var/tmp";
#endif
}

namespace {
std::string gen_temp(const std::string& post_base, const std::string& dir, const std::string& suffix) {
	std::string base_name;
	boost::filesystem::path base_dir;

	base_name = default_base_name;
	if(base_name.empty())
		base_name = "TPIE";

	if(!dir.empty())
		base_dir = dir;
	else
		base_dir = tempname::get_actual_path();

	boost::filesystem::path p;
	for(int i=0; i < 42; ++i) {
		if(post_base.empty())
			p = base_dir / (base_name + "_" + tpie_mktemp() + suffix);
		else
			p = base_dir / (base_name + "_" + post_base + "_" + tpie_mktemp() + suffix);
		if ( !boost::filesystem::exists(p) )
#if BOOST_FILESYSTEM_VERSION == 3
			return p.string();
#else
			return p.file_string();
#endif
		
	}
	throw tempfile_error("Unable to find free name for temporary file");
}
}

std::string tempname::tpie_name(const std::string& post_base, const std::string& dir, const std::string& ext) {
	if (ext.empty()) return gen_temp(post_base, dir, ".tpie");
	else return gen_temp(post_base, dir, "." + ext);
}

std::string tempname::tpie_dir_name(const std::string& post_base, const std::string& dir) {
	return gen_temp(post_base, dir, "");
}

std::string tempname::get_actual_path() {
	//information about the search order is in the header
	std::string dir;
	if(!default_path.empty()) 
		dir = default_path; //user specified path
	else if(getenv(AMI_SINGLE_DEVICE_ENV) != NULL)  //TPIE env variable
		dir = getenv(AMI_SINGLE_DEVICE_ENV);
	else if(getenv(TMPDIR_ENV) != NULL)  
		dir = getenv(TMPDIR_ENV); //OS env variable (from portability.h)
	else  
		dir = get_system_path(); //OS path

	return dir;
}

namespace {
std::string tpie_mktemp()
{
	const std::string chars[] = 
	{ "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", 
	"N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", 
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", 
	"n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", 
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
	const int chars_count = 62;
	static int counter = boost::posix_time::second_clock::local_time().time_of_day().total_seconds() % (chars_count * chars_count); 

	std::string result = "";
	result +=
		chars[counter/chars_count] +
		chars[counter%chars_count] +
		chars[prng() % chars_count] +
		chars[prng() % chars_count] +
		chars[prng() % chars_count] +
		chars[prng() % chars_count] +
		chars[prng() % chars_count] +
		chars[prng() % chars_count];

	counter = (counter + 1) % (chars_count * chars_count);

	return result;
}
}


void tempname::set_default_path(const std::string&  path, const std::string& subdir) {
	if (subdir=="") {
		default_path = path;
		return;
	}
	boost::filesystem::path p = path;
	p = p / subdir;
	try {
		if (!boost::filesystem::exists(p)) {
			boost::filesystem::create_directory(p);
		}
		if (!boost::filesystem::is_directory(p)) {	
			default_path = path;
			TP_LOG_WARNING_ID("Could not use " << p << " as directory for temporary files, trying " << path);
		}

#if BOOST_FILESYSTEM_VERSION == 3
		default_path = p.string();
#else
		default_path = p.directory_string();
#endif
	} catch (boost::filesystem::filesystem_error) { 
		TP_LOG_WARNING_ID("Could not use " << p << " as directory for temporary files, trying " << path);
		default_path = path; 
	}	
}

void tempname::set_default_base_name(const std::string& name) {
	default_base_name = name;
}

void tempname::set_default_extension(const std::string& ext) {
	default_extension = ext;
}


const std::string& tempname::get_default_path() {
	return default_path;
}

const std::string& tempname::get_default_base_name() {
	return default_base_name;
}

const std::string& tempname::get_default_extension() {
	return default_extension;
}

namespace tpie {
namespace bits {

temp_file_inner::~temp_file_inner() {
	if (m_path.empty() || m_persist || !boost::filesystem::exists(m_path)) 
		return;

	boost::filesystem::remove(m_path);
	update_recorded_size(0);
}

temp_file_inner::temp_file_inner() : m_persist(false), m_recordedSize(0), m_count(0) {}

temp_file_inner::temp_file_inner(const std::string & path, bool persist): m_path(path), m_persist(persist), m_recordedSize(0), m_count(0) {}

const std::string & temp_file_inner::path() {
	if(m_path.empty())
		m_path = tempname::tpie_name();
	return m_path;
}

void temp_file_inner::update_recorded_size(stream_size_type size) {
	increment_temp_file_usage(static_cast<stream_offset_type>(size) - static_cast<stream_offset_type>(m_recordedSize));
	m_recordedSize=size;
}

void intrusive_ptr_add_ref(temp_file_inner * p) {
	++p->m_count;
}

void intrusive_ptr_release(temp_file_inner * p) {
	if(--p->m_count == 0)
		delete p;
}

} // namespace bits
} // namespace tpie