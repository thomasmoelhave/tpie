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
#include <sstream>
#include <tpie/portability.h>
#include <filesystem>
#include <stdexcept>
#include <tpie/util.h>
#include <tpie/exception.h>
#include <tpie/file_accessor/file_accessor.h>
#include <stack>
#include <random>

#ifdef _WIN32
#include <Windows.h>
#undef NO_ERROR
#endif

#ifdef __linux__
#include <sys/vfs.h>
#include <linux/magic.h>
#endif

using namespace tpie;

namespace {

std::string default_path;
std::string default_base_name = "TPIE";
std::string default_extension;
std::stack<std::string> subdirs;

}

std::string _get_system_path() {
#ifdef __linux__
	// If the temporary directory is not specified in an env var,
	// we want to default to /tmp like boost does.
	// However on some linux systems /tmp is a tmpfs filesystem
	// meaning it is stored in RAM.
	// If this is the case we want to use /var/tmp as the default instead.

	static const char * _tmp_envs[] = {
			"TMPDIR", "TMP", "TEMP", "TEMPDIR"
	};
	static const char * _tmp_dirs[] = {
			"/tmp", "/var/tmp"
	};

	// First check environment variables like boost
	for (auto e : _tmp_envs) {
		if (auto p = getenv(e)) {
			return p;
		}
	}

	for (auto d : _tmp_dirs) {
		struct statfs s;
		if (statfs(d, &s) != 0) {
			log_debug() << "Couldn't use statfs on " << d << ": " << strerror(errno) << "\n";
			continue;
		}

		if (s.f_type == TMPFS_MAGIC) {
			log_debug() << d << " is a tmpfs filesystem\n";
			continue;
		}
		return d;
	}
	// If all fails just default to boost
#endif

	auto p = std::filesystem::temp_directory_path();
	return p.string();
}

std::string tempname::get_system_path() {
	static std::string cache;
	if (cache.empty()) {
		cache = _get_system_path();
		log_debug() << "Using " << cache << " as the system temporary directory\n";
	}

	return cache;
}

namespace {

std::string get_timestamp() {
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char buf[64];
	std::strftime(buf, 64, "%Y-%b-%d_%H-%M-%S", std::localtime(&now));
	return buf;
}

std::string construct_name(std::string post_base, std::string timestamp, std::string suffix) {
	std::stringstream ss;
	ss << default_base_name << "_";
	if(!post_base.empty())
		ss << post_base << "_";
	if(!timestamp.empty())
		ss << timestamp << "_";
	char ran[] = "0123456789abcdef";
	std::random_device rd;
	unsigned int r1 = rd();
	unsigned int r2 = rd();
	unsigned int r3 = rd();
	unsigned int r4 = rd();
	ss << ran[(r1 >> 0)&0xF]
		<< ran[(r1 >> 4)&0xF]
		<< ran[(r1 >> 8)&0xF]
		<< ran[(r1 >> 12)&0xF]
		<< '-'
		<< ran[(r2 >> 0)&0xF]
		<< ran[(r2 >> 4)&0xF]
		<< ran[(r2 >> 8)&0xF]
		<< ran[(r2 >> 12)&0xF]
		<< '-'
		<< ran[(r3 >> 0)&0xF]
		<< ran[(r3 >> 4)&0xF]
		<< ran[(r3 >> 8)&0xF]
		<< ran[(r3 >> 12)&0xF]
		<< '-'
		<< ran[(r4 >> 0)&0xF]
		<< ran[(r4 >> 4)&0xF]
		<< ran[(r4 >> 8)&0xF]
		<< ran[(r4 >> 12)&0xF]
		<< suffix;
	return ss.str();
}

void create_subdir() {
	std::filesystem::path base_dir = tempname::get_actual_path();
	std::filesystem::path p;
	p = base_dir / construct_name("", get_timestamp(), "");
	if ( !std::filesystem::exists(p) && std::filesystem::create_directory(p)) {
		std::string path = p.string();
		if (!subdirs.empty() && subdirs.top().empty())
			subdirs.pop();
		subdirs.push(path);
		return;
	}
	throw tempfile_error("Unable to find free name for temporary folder");
}

std::string gen_temp(const std::string& post_base, const std::string& dir, const std::string& suffix) {
	if (!dir.empty()) {
		std::filesystem::path p;
		p = dir; p /= construct_name(post_base, get_timestamp(), suffix);
		if ( !std::filesystem::exists(p) ) {
			return p.string();
		}
		throw tempfile_error("Unable to find free name for temporary file");
	}
	else {
		if (subdirs.empty() || subdirs.top().empty()) create_subdir();

		std::filesystem::path p = subdirs.top();
		p /= construct_name(post_base, "", suffix);

		return p.string();
	}
}

}

namespace tpie {
	void finish_tempfile() {
		while (!subdirs.empty()) {
			if (!subdirs.top().empty()) {
				std::error_code c;
				std::filesystem::remove_all(subdirs.top(), c);
			}
			subdirs.pop();
		}
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

bool tempname::try_directory(const std::string& path) {
	std::filesystem::path p = path;
	if (!std::filesystem::is_directory(p))
		return false;
	std::filesystem::path f = p / construct_name("", get_timestamp(), "");
	if(std::filesystem::exists(f))
		return false;

	std::string file_path = f.string();
	try {
		{
			tpie::file_accessor::raw_file_accessor accessor;
			accessor.open_rw_new(file_path);
			int i = 0xbadf00d;
			accessor.write_i(static_cast<const void*>(&i), sizeof(i));
		}
		std::filesystem::remove(file_path);
		return true;
	}
	catch (tpie::exception &) {}
	catch (std::filesystem::filesystem_error &) {}

	return false;
}

void tempname::set_default_path(const std::string&  path, const std::string& subdir) {
	if (subdir=="") {
		default_path = path;
		subdirs.push(""); // signals that the current global subdirectory has not been created yet
		return;
	}
	std::filesystem::path p = path;
	p = p / subdir;
	try {
		if (!std::filesystem::exists(p)) {
			std::filesystem::create_directory(p);
		}
		if (!std::filesystem::is_directory(p)) {	
			default_path = path;
			subdirs.push(""); // signals that the current global subdirectory has not been created yet
			TP_LOG_WARNING_ID("Could not use " << p << " as directory for temporary files, trying " << path);
		}

		default_path = p.string();
		subdirs.push(""); // signals that the current global subdirectory has not been created yet
	} catch (std::filesystem::filesystem_error &) {
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
	if (m_path.empty() || m_persist || !std::filesystem::exists(m_path))
		return;

	std::filesystem::remove(m_path);
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

TPIE_EXPORT void intrusive_ptr_add_ref(temp_file_inner *p) {
	++p->m_count;
}

TPIE_EXPORT void intrusive_ptr_release(temp_file_inner *p) {
	if(--p->m_count == 0)
		delete p;
}

} // namespace bits
} // namespace tpie
