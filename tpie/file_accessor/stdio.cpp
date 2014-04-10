// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, 2010, The TPIE development team
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
#include <tpie/exception.h>
#include <tpie/file_count.h>
#include <tpie/file_accessor/stdio.h>
#include <tpie/stats.h>
#include <cstring> // strerror
#include <cstdio>

#ifdef TPIE_HAS_POSIX_FADVISE
#include <fcntl.h> // posix_fadvise
#endif // TPIE_HAS_POSIX_FADVISE

namespace tpie {
namespace file_accessor {

stdio::stdio()
	: m_fd(0)
	, m_cacheHint(access_normal)
{
}

void stdio::give_advice() {
#ifdef TPIE_HAS_POSIX_FADVISE
	int advice;
	switch (m_cacheHint) {
		case access_normal:
			advice = POSIX_FADV_NORMAL;
			break;
		case access_sequential:
			advice = POSIX_FADV_SEQUENTIAL;
			break;
		case access_random:
			advice = POSIX_FADV_RANDOM;
			break;
		default:
			advice = POSIX_FADV_NORMAL;
			break;
	}
	::posix_fadvise(fileno((FILE *) m_fd), 0, 0, advice);
#endif // TPIE_HAS_POSIX_FADVISE
}

void stdio::throw_errno(int e) {
	if (e == ENOSPC) throw out_of_space_exception(strerror(e));
	else throw io_exception(strerror(e));
}

void stdio::throw_ferror() {
	throw_errno(ferror((FILE *) m_fd));
}

void stdio::read_i(void * data, memory_size_type size) {
	if (::fread(data, 1, size, (FILE *) m_fd) != size) throw_ferror();
	increment_bytes_read(size);
}

void stdio::write_i(const void * data, memory_size_type size) {
	if (::fwrite(data, 1, size, (FILE *) m_fd) != size) throw_ferror();
	increment_bytes_written(size);
}

void stdio::seek_i(stream_size_type offset) {
#ifdef _WIN32
	if (::_fseeki64((FILE *) m_fd, offset, SEEK_SET) != 0) throw_ferror();
#else
	if (::fseeko((FILE *) m_fd, offset, SEEK_SET) != 0) throw_ferror();
#endif
}

void stdio::open_wo(const std::string & path) {
	close_i();
	m_fd = ::fopen(path.c_str(), "wb");
	if (m_fd == NULL) throw_errno(errno);
	setvbuf((FILE *) m_fd, NULL, _IONBF, 0);
	give_advice();
}

void stdio::open_ro(const std::string & path) {
	close_i();
	m_fd = ::fopen(path.c_str(), "rb");
	if (m_fd == NULL) throw_errno(errno);
	setvbuf((FILE *) m_fd, NULL, _IONBF, 0);
	give_advice();
}

bool stdio::try_open_rw(const std::string & path) {
	close_i();
	m_fd = ::fopen(path.c_str(), "r+b");
	if (m_fd == NULL) {
		if (errno == ENOENT) return false;
		else throw_errno(errno);
	}
	setvbuf((FILE *) m_fd, NULL, _IONBF, 0);
	give_advice();
	return true;
}

void stdio::open_rw_new(const std::string & path) {
	close_i();
	m_fd = ::fopen(path.c_str(), "w+b");
	if (m_fd == NULL) throw_errno(errno);
	setvbuf((FILE *) m_fd, NULL, _IONBF, 0);
	give_advice();
}

bool stdio::is_open() const {
	return m_fd != NULL;
}

void stdio::close_i() {
	if (m_fd == NULL) return;
	::fclose((FILE *) m_fd);
	m_fd = NULL;
}

void stdio::truncate_i(stream_size_type /*size*/) {
	throw tpie::io_exception("Truncate not supported by stdio file_accessor");
}

}
}
