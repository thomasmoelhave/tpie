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

#include <tpie/file_accessor/win32.h>
#include <tpie/exception.h>
#include <windows.h>

namespace {

DWORD get_creation_flag(tpie::cache_hint cacheHint) {
	switch (cacheHint) {
		case tpie::access_normal:
			return 0;
		case tpie::access_sequential:
			return FILE_FLAG_SEQUENTIAL_SCAN;
		case tpie::access_random:
			return FILE_FLAG_RANDOM_ACCESS;
	}
}

void throw_getlasterror() {
	char buffer[1024];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, buffer, 1023, 0);
	switch (GetLastError()) {
		case ERROR_HANDLE_DISK_FULL:
		case ERROR_DISK_FULL:
		case ERROR_DISK_TOO_FRAGMENTED:
		case ERROR_DISK_QUOTA_EXCEEDED:
		case ERROR_VOLMGR_DISK_NOT_ENOUGH_SPACE:
			throw tpie::out_of_space_exception(buffer);
		default:
			throw tpie::io_exception(buffer);
	}
}

} // unnamed namespace

namespace tpie {
namespace file_accessor {

win32::win32()
	: m_fd(INVALID_HANDLE_VALUE)
	, m_cacheHint(access_normal)
{
}

void win32::read_i(void * data, memory_size_type size) {
	DWORD bytesRead = 0;
	OVERLAPPED * const overlapped = 0;
	if (!ReadFile((HANDLE) m_fd, data, (DWORD) size, &bytesRead, overlapped)
			|| bytesRead != size)
		throw_getlasterror();
	increment_bytes_read(size);
}

void win32::write_i(const void * data, memory_size_type size) {
	DWORD bytesWritten = 0;
	OVERLAPPED * const overlapped = 0;
	if (!WriteFile((HANDLE) m_fd, data, (DWORD) size, &bytesWritten, overlapped)
			|| bytesWritten != size)
		throw_getlasterror();
	increment_bytes_written(size);
}

void win32::seek_i(stream_size_type size) {
	LARGE_INTEGER distanceToMove;
	distanceToMove.QuadPart = size;
	const HANDLE fd = (HANDLE) m_fd;
	LARGE_INTEGER * const newFilePointer = NULL;
	int moveMethod = 0; // FILE_BEGIN
	if (!SetFilePointerEx(fd, distanceToMove, newFilePointer, moveMethod))
		throw_getlasterror();
}

void * win32::create_file(const std::string & fileName, int desiredAccess,
						  int creationDisposition)
{
	const int shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
	const int securityAttributes = 0;
	const int flagsAndAttributes = get_creation_flag(m_cacheHint);
	void * const templateFile = 0;

	return ::CreateFile(fileName.c_str(), desiredAccess,
						shareMode, securityAttributes,
						creationDisposition, flagsAndAttributes,
						templateFile);
}

void win32::open_wo(const std::string & path) {
	m_fd = create_file(path, GENERIC_WRITE, CREATE_ALWAYS);
	if ((HANDLE) m_fd == INVALID_HANDLE_VALUE)
		throw_getlasterror();
}

void win32::open_ro(const std::string & path) {
	m_fd = create_file(path, GENERIC_READ, OPEN_EXISTING);
	if ((HANDLE) m_fd == INVALID_HANDLE_VALUE)
		throw_getlasterror();
}

bool win32::try_open_rw(const std::string & path) {
	m_fd = create_file(path.c_str(), GENERIC_READ | GENERIC_WRITE, OPEN_EXISTING);
	if ((HANDLE) m_fd == INVALID_HANDLE_VALUE) {
		if (GetLastError() != ERROR_FILE_NOT_FOUND)
			throw_getlasterror();
		return false;
	}
	return true;
}

void win32::open_rw_new(const std::string & path) {
	m_fd = create_file(path.c_str(), GENERIC_READ | GENERIC_WRITE, CREATE_NEW);
	if ((HANDLE) m_fd == INVALID_HANDLE_VALUE)
		throw_getlasterror();
}

bool win32::is_open() const {
	return (HANDLE) m_fd != INVALID_HANDLE_VALUE;
}

void win32::close_i() {
	if (is_open()) CloseHandle((HANDLE) m_fd);

	m_fd = INVALID_HANDLE_VALUE;
}

void win32::truncate_i(stream_size_type size) {
	LARGE_INTEGER distanceToMove;
	distanceToMove.QuadPart = size;
	LARGE_INTEGER * const newFilePointer = NULL;
	int moveMethod = 0; // FILE_BEGIN
	SetFilePointerEx((HANDLE) m_fd, distanceToMove, newFilePointer, moveMethod);
	if (!SetEndOfFile((HANDLE) m_fd)) throw_getlasterror();
}

} // namespace tpie
} // namespace file_accessor
