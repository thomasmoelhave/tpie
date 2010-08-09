// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2010, The TPIE development team
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
#ifndef _TPIE_PORTABILITY_H
#define _TPIE_PORTABILITY_H

// This header-file offers macros for independent use on Win and Unix systems.
#ifndef _PORTABILITY_H
#define _PORTABILITY_H

#include <tpie/config.h>
#include <tpie/types.h>

//  The following wil cause tpie::memory_size_type to be a 32-bit integer!
#define _TPIE_SMALL_MAIN_MEMORY

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX  //ensures that the windows min/max macros are not defined
#endif
#endif

// overview of this file:				//
//////////////////////////////////////////
// includes // typedef, enum, etc.  // functions // non tpie specific
// // tpie specific // open functions // working with open files //
// close file functions // warnings // others //



//////////////////////////////////////////////
// includes								 //
//////////////////////////////////////////////

#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
// For time()
#include <time.h>

//for UINT_MAX
#include <limits.h>
#include <limits>

// Get random functions //
#include <cassert>
#include <cstdlib>

#ifdef _WIN32
#if (_MSC_VER < 1300) && !defined(__MINGW32__)
#include <fstream.h>
#else
#include <fstream>
#endif
#else
#include <fstream>
#endif

// for reading command line parameter //
#if defined(_WIN32) && !defined(__MINGW32__)
#include <sstream>
#else
#if (__GNUC__ == 2)
#include <strstream>
#else
#include <sstream>
#endif
#endif

#include <algorithm>
#include <functional>
#include <list>
#include <queue>
#include <stack>
#include <vector>

#ifdef _WIN32
#include <io.h>
#include <time.h>
#include <time.h>
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#endif

//////////////////////////////////////////////
// typedefs, enum etc.						//
//////////////////////////////////////////////

#ifdef _WIN32
#ifdef _WIN64
typedef __time64_t TPIE_OS_TIME_T;
inline TPIE_OS_TIME_T TPIE_OS_TIME(TPIE_OS_TIME_T* timep) {
	return _time64(timep);
}
#else
typedef time_t TPIE_OS_TIME_T;
inline TPIE_OS_TIME_T TPIE_OS_TIME(TPIE_OS_TIME_T* timep) {
	return time(timep);
}
#endif
typedef time_t TPIE_OS_TMS;
#else
typedef time_t TPIE_OS_TIME_T;
typedef tms TPIE_OS_TMS;
inline TPIE_OS_TIME_T TPIE_OS_TIME(TPIE_OS_TIME_T* timep) {
	return time(timep);
}
#endif


#ifdef _WIN32
//windows doesn't have a default way
//of printing 64 bit integers
//printf doesn't work either with %d, use %I64d in Win32
#if (_MSC_VER < 1300) && !defined(__MINGW32__)
extern std::ostream& operator<<(std::ostream& s, const tpie::stream_size_type x);
#endif
#endif

#if defined (_WIN32) && !defined(__MINGW32__)
// typedef long TPIE_OS_LONG;
// typedef __int64 TPIE_OS_LONGLONG;
// typedef unsigned __int64 TPIE_OS_ULONGLONG;
#else
// typedef long TPIE_OS_LONG;
// typedef long long int TPIE_OS_LONGLONG;
// typedef unsigned long long int TPIE_OS_ULONGLONG;
#endif

#if defined (_WIN32) && !defined(__MINGW32__)
// typedef SSIZE_T tpie::memory_offset_type;
#ifdef _TPIE_SMALL_MAIN_MEMORY
#if (_MSC_VER < 1400)
// typedef unsigned __int32 tpie::memory_size_type;
#else
// typedef size_t tpie::memory_size_type;
#endif
#else
// typedef size_t tpie::memory_size_type;
#endif
#else
// typedef ssize_t tpie::memory_offset_type;
// typedef size_t tpie::memory_size_type;
#endif

#ifdef _WIN32
enum TPIE_OS_FLAG {
	TPIE_OS_FLAG_SEEK_SET = FILE_BEGIN,
	TPIE_OS_FLAG_SEEK_CUR = FILE_CURRENT,
	TPIE_OS_FLAG_SEEK_END = FILE_END,
	TPIE_OS_FLAG_PROT_READ= FILE_MAP_READ,
	TPIE_OS_FLAG_PROT_WRITE=FILE_MAP_WRITE,
	TPIE_OS_FLAG_MAP_SHARED,
	TPIE_OS_FLAG_MS_SYNC,
	TPIE_OS_FLAG_MS_ASYNC,
	TPIE_OS_FLAG_MAP_FIXED = 0
};
#else
enum TPIE_OS_FLAG {
	TPIE_OS_FLAG_SEEK_SET = SEEK_SET,
	TPIE_OS_FLAG_SEEK_CUR = SEEK_CUR,
	TPIE_OS_FLAG_SEEK_END = SEEK_END,
	TPIE_OS_FLAG_PROT_READ= PROT_READ,
	TPIE_OS_FLAG_PROT_WRITE=PROT_WRITE,
	TPIE_OS_FLAG_MAP_SHARED = MAP_SHARED,
	TPIE_OS_FLAG_MS_SYNC = MS_SYNC,
	TPIE_OS_FLAG_MS_ASYNC = MS_ASYNC,
	TPIE_OS_FLAG_MAP_FIXED = MAP_FIXED
};
#endif

#ifdef _WIN32
const int TPIE_OS_PERSIST_READ_ONCE = 0;
#else
const int TPIE_OS_PERSIST_READ_ONCE = 0;
#endif

enum TPIE_OS_MAPPING_FLAG {
	TPIE_OS_FLAG_USE_MAPPING_FALSE,
	TPIE_OS_FLAG_USE_MAPPING_TRUE
};

#ifdef _WIN32
typedef struct {
	HANDLE FileHandle,
	mapFileHandle;
	BOOL RDWR;
	TPIE_OS_MAPPING_FLAG useFileMapping;
} TPIE_OS_FILE_DESCRIPTOR;
#else
typedef int TPIE_OS_FILE_DESCRIPTOR;
#endif


// Default block id type
typedef tpie::stream_size_type block_id_type;


//////////////////////////////////////////////
// macros								   //
//////////////////////////////////////////////

#ifdef _WIN32
#define TPIE_OS_SET_LIMITS_BODY	\
	return 512;
#else
#define TPIE_OS_SET_LIMITS_BODY						\
	struct rlimit limits;						\
	if (getrlimit(RLIMIT_NOFILE,&limits) == -1) {	\
		limits.rlim_cur = 255;						   \
	}									   \
	return limits.rlim_cur;
#endif


#ifdef _WIN32
#define TPIE_OS_SET_CLOCK_TICK				\
		clock_tick_ = CLOCKS_PER_SEC
#else
#define TPIE_OS_SET_CLOCK_TICK clock_tick_ = sysconf(_SC_CLK_TCK); elapsed_.tms_utime = 0; elapsed_.tms_stime = 0; elapsed_.tms_cutime = 0; elapsed_.tms_cstime = 0;
#endif


#ifdef _WIN32
#define TPIE_OS_UNIX_ONLY_SET_ELAPSED_TIME(current)
#else
#define TPIE_OS_UNIX_ONLY_SET_ELAPSED_TIME(current) elapsed_.tms_utime += (current).tms_utime - last_sync_.tms_utime; elapsed_.tms_stime += (current).tms_stime - last_sync_.tms_stime; elapsed_.tms_cutime += (current).tms_cutime - last_sync_.tms_cutime; elapsed_.tms_cstime += (current).tms_cstime - last_sync_.tms_cstime;
#endif


#ifdef _WIN32
#define TPIE_OS_SET_CURRENT_TIME(current) time(& current ); current_real_ = clock();
#else
#define TPIE_OS_SET_CURRENT_TIME(current) current_real_ = times(& current);
#endif


#ifdef _WIN32
#define TPIE_OS_LAST_SYNC_REAL_DECLARATION last_sync_real_ = clock();
#else
#define TPIE_OS_LAST_SYNC_REAL_DECLARATION last_sync_real_ = times(&last_sync_);
#endif


#ifdef _WIN32
#define TPIE_OS_USER_TIME_BODY return double(elapsed_real()) / double(clock_tick())
#else
#define TPIE_OS_USER_TIME_BODY return double(elapsed().tms_utime) / double(clock_tick())
#endif


#ifdef _WIN32
#define TPIE_OS_OPERATOR_OVERLOAD \
	return s << double(wt.elapsed_real()) / double(wt.clock_tick());
#else
#define TPIE_OS_OPERATOR_OVERLOAD return s << double(wt.elapsed().tms_utime) / double(wt.clock_tick()) << "u " << double(wt.elapsed().tms_stime) / double(wt.clock_tick()) << "s " << double(wt.elapsed_real()) / double(wt.clock_tick());
#endif

//////////////////////////////////////////////
// functions					//
//////////////////////////////////////////////


//////////////////////////////////////////////
//	non-tpie specific functions		//

#ifdef _WIN32
// Win32 File Seeks use high/low order offsets
// Getting Highorder 32 Bit OFFSET
inline LONG getHighOrderOff(tpie::stream_offset_type off) {
	//Be careful with sign bits.
	return (LONG)((ULONGLONG)(off)>>32);
}


// Getting Loworder 32 Bit OFFSET
inline LONG getLowOrderOff(tpie::stream_offset_type off) {
	return (LONG)((ULONGLONG)(off) % 0x000100000000ULL);
}
#endif



//////////////////////////////////////////////
//	   tpie specific functions			//

#ifdef _WIN32
inline tpie::memory_size_type  TPIE_OS_PAGESIZE() {
	SYSTEM_INFO systemInfos;
	GetSystemInfo(&systemInfos);
	return (tpie::memory_size_type )systemInfos.dwPageSize;
}
#else
#ifdef _SC_PAGE_SIZE
inline tpie::memory_size_type  TPIE_OS_PAGESIZE() {
	return sysconf (_SC_PAGE_SIZE);
}
#else
inline tpie::memory_size_type  TPIE_OS_PAGESIZE() {
	return getpagesize();
}
#endif
#endif


#ifdef _WIN32
inline tpie::memory_size_type  TPIE_OS_BLOCKSIZE() {
	SYSTEM_INFO systemInfos;
	GetSystemInfo(&systemInfos);
	return systemInfos.dwAllocationGranularity;
}
#else
#ifdef _SC_PAGE_SIZE
inline tpie::memory_size_type TPIE_OS_BLOCKSIZE() {
	return sysconf (_SC_PAGE_SIZE);
}
#else
inline tpie::memory_size_type  TPIE_OS_BLOCKSIZE() {
	return getpagesize();
}
#endif
#endif



//////////////////////////////////////////////
//	   open functions			//

//there is no difference between the systemcalls
//but for later adaptation to other systems it maybe useful
#ifdef _WIN32
inline FILE* TPIE_OS_FOPEN(const std::string& filename,
			   const std::string& mode) {
	return fopen(filename.c_str(),mode.c_str());
}
#else
inline FILE* TPIE_OS_FOPEN(const std::string& filename,
			   const std::string& mode) {
	return fopen(filename.c_str(),mode.c_str());
}
#endif

//there is no difference between the systemcalls
//but for later adaptation to other systems it maybe useful
#ifdef _WIN32
#ifdef __MINGW32__
inline int TPIE_OS_FSEEK(FILE* file, tpie::stream_offset_type offset, int whence) {
	return fseeko64(file, static_cast<tpie::stream_offset_type>(offset), whence);
}
#else
inline int TPIE_OS_FSEEK(FILE* file, tpie::stream_offset_type offset, int whence) {
	//  Please note that the second parameter should be tpie::stream_offset_type
	//  instead of int. This is due to the fact that VS2003 does not
	//  support large files with fopen/fseek etc.
	return _fseeki64(file, static_cast<tpie::stream_offset_type>(offset), whence);
}
#endif
#else
inline int TPIE_OS_FSEEK(FILE* file, tpie::stream_offset_type offset, int whence) {
	return fseek(file, offset, whence);
}
#endif

//there is no difference between the systemcalls
//but for later adaptation to other systems it maybe useful
#ifdef _WIN32
#ifdef __MINGW32__
inline tpie::stream_offset_type TPIE_OS_FTELL(FILE* file) {
	return ftello64(file);
}
#else
inline tpie::stream_offset_type TPIE_OS_FTELL(FILE* file) {
	return _ftelli64(file);
}
#endif
#else
inline tpie::stream_offset_type TPIE_OS_FTELL(FILE* file) {
	return ftell(file);
}
#endif

//there is no difference between the systemcalls
//but for later adaptation to other systems it maybe useful
#ifdef _WIN32
inline size_t TPIE_OS_FREAD(void* buffer, size_t size, size_t nitems, FILE* stream) {
	return fread(buffer, size, nitems, stream);
}
#else
inline size_t TPIE_OS_FREAD(void* buffer, size_t size, size_t nitems, FILE* stream) {
	return fread(buffer, size, nitems, stream);
}
#endif

//there is no difference between the systemcalls
//but for later adaptation to other systems it maybe useful
#ifdef _WIN32
inline size_t TPIE_OS_FWRITE(const void* buffer, size_t size, size_t nitems, FILE* stream) {
	return fwrite(buffer, size, nitems, stream);
}
#else
inline size_t TPIE_OS_FWRITE(const void* buffer, size_t size, size_t nitems, FILE* stream) {
	return fwrite(buffer, size, nitems, stream);
}
#endif

//there is no difference between the systemcalls
//but for later adaptation to other systems it maybe useful
#ifdef _WIN32
inline int TPIE_OS_FCLOSE(FILE* file) {
	return fclose(file);
}
#else
inline int TPIE_OS_FCLOSE(FILE* file) {
	return fclose(file);
}
#endif


// internal help-function for translating Unix open into a Windows CreateFile + CreateFileMapping
#ifdef _WIN32
#include <FCNTL.h>
inline TPIE_OS_FILE_DESCRIPTOR portabilityInternalOpen(LPCTSTR name, int flag, TPIE_OS_MAPPING_FLAG mappingFlag) {
	DWORD creation_flag = FILE_FLAG_SEQUENTIAL_SCAN;
	TPIE_OS_FILE_DESCRIPTOR internalHandle;
	switch(flag) {
	case _O_RDONLY:
	internalHandle.RDWR = false;
	internalHandle.FileHandle =	CreateFile(
		name,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		0,
		OPEN_EXISTING,
		creation_flag,
		0);
	break;
	case _O_EXCL:
	internalHandle.RDWR = true;
	internalHandle.FileHandle =	CreateFile(
		name,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		0,
		CREATE_NEW,
		creation_flag,
		0);
	break;
	case _O_RDWR:
	internalHandle.RDWR = true;
	internalHandle.FileHandle =	CreateFile(
		name,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		0,
		OPEN_EXISTING,
		creation_flag,
		0);
	break;
	default :
	internalHandle.RDWR = false;
	internalHandle.FileHandle =	CreateFile(
		name,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		0,
		OPEN_EXISTING,
		creation_flag,
		0);
	};
	internalHandle.useFileMapping = mappingFlag;
	DWORD dwFileSize = GetFileSize(internalHandle.FileHandle,NULL);
	if (dwFileSize == 0) {
	SetFilePointer(internalHandle.FileHandle,
			   static_cast<LONG>(TPIE_OS_BLOCKSIZE()),0,FILE_BEGIN);
	SetEndOfFile(internalHandle.FileHandle);
	};
	if (internalHandle.useFileMapping == TPIE_OS_FLAG_USE_MAPPING_TRUE) {
	internalHandle.mapFileHandle =
		CreateFileMapping(
		internalHandle.FileHandle,
		0,
		(internalHandle.RDWR ? PAGE_READWRITE : PAGE_READONLY),
		0, 0,
		NULL);
	}
	else {
	internalHandle.mapFileHandle = (void*)1;
	}
	return internalHandle;
}
#endif


#ifdef _WIN32
inline TPIE_OS_FILE_DESCRIPTOR TPIE_OS_OPEN_ORDONLY(const std::string& name,TPIE_OS_MAPPING_FLAG mappingFlag = TPIE_OS_FLAG_USE_MAPPING_FALSE) {
	return portabilityInternalOpen(name.c_str(), _O_RDONLY,mappingFlag);
}
#else
inline TPIE_OS_FILE_DESCRIPTOR TPIE_OS_OPEN_ORDONLY(const std::string& name,TPIE_OS_MAPPING_FLAG = TPIE_OS_FLAG_USE_MAPPING_FALSE /* mappingFlag */ ) {
	return ::open(name.c_str(), O_RDONLY);
}
#endif


#ifdef _WIN32
inline TPIE_OS_FILE_DESCRIPTOR TPIE_OS_OPEN_OEXCL(const std::string& name, TPIE_OS_MAPPING_FLAG mappingFlag = TPIE_OS_FLAG_USE_MAPPING_FALSE) {
	return portabilityInternalOpen(name.c_str(), _O_EXCL, mappingFlag);
}
#else
inline TPIE_OS_FILE_DESCRIPTOR TPIE_OS_OPEN_OEXCL(const std::string& name, TPIE_OS_MAPPING_FLAG = TPIE_OS_FLAG_USE_MAPPING_FALSE /* mappingFlag */) {
	return ::open(name.c_str(), O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
}
#endif


#ifdef _WIN32
inline TPIE_OS_FILE_DESCRIPTOR TPIE_OS_OPEN_ORDWR(const std::string& name, TPIE_OS_MAPPING_FLAG mappingFlag = TPIE_OS_FLAG_USE_MAPPING_FALSE) {
	return portabilityInternalOpen(name.c_str(), _O_RDWR, mappingFlag);
}
#else
inline TPIE_OS_FILE_DESCRIPTOR TPIE_OS_OPEN_ORDWR(const std::string&  name, TPIE_OS_MAPPING_FLAG = TPIE_OS_FLAG_USE_MAPPING_FALSE /* mappingFlag */ ) {
	return ::open(name.c_str(), O_RDWR);
}
#endif


//////////////////////////////////////////////
//		working with open files		 //

#ifdef _WIN32
inline BOOL TPIE_OS_IS_VALID_FILE_DESCRIPTOR(TPIE_OS_FILE_DESCRIPTOR& fd) {
	BOOL x;
	if (fd.FileHandle == INVALID_HANDLE_VALUE || fd.mapFileHandle == NULL) {
	x = false;
	} else {
	x = true;
	};
	return x;
}
#else
inline bool TPIE_OS_IS_VALID_FILE_DESCRIPTOR(TPIE_OS_FILE_DESCRIPTOR& fd) {
	return (fd == -1 ? false : true);
}
#endif


// for working with HANDLEs under Windows we have to use SetFilePointer instead of _lseek //
#ifdef _WIN32
inline tpie::stream_offset_type TPIE_OS_LSEEK(TPIE_OS_FILE_DESCRIPTOR &fd,tpie::stream_offset_type offset,TPIE_OS_FLAG origin) {
	LONG highOrderOff = getHighOrderOff(offset);
	DWORD x = SetFilePointer(fd.FileHandle,getLowOrderOff(offset),&highOrderOff,origin);

	if ( x==0xFFFFFFFF && (GetLastError() != NO_ERROR) ) {
	  //Error
	  return -1;
	}
	else{
	  return tpie::stream_offset_type((((ULONGLONG) highOrderOff)<<32)+(ULONGLONG) x);
	}
}
#else
inline tpie::stream_offset_type TPIE_OS_LSEEK(TPIE_OS_FILE_DESCRIPTOR &fd,tpie::stream_offset_type offset,TPIE_OS_FLAG origin) {
	return ::lseek(fd, offset, origin);
}
#endif


#ifdef _WIN32
inline tpie::memory_offset_type TPIE_OS_WRITE(TPIE_OS_FILE_DESCRIPTOR fd, const void* buffer, tpie::memory_size_type count) {
	DWORD bytesWritten = 0;
	::WriteFile(fd.FileHandle, buffer, (DWORD)count, &bytesWritten, 0);
	return (tpie::memory_offset_type)(bytesWritten > 0 ? bytesWritten : -1);
}
#else
inline tpie::memory_offset_type TPIE_OS_WRITE(TPIE_OS_FILE_DESCRIPTOR fd, const void* buffer, size_t count) {
	return ::write(fd,buffer,count);
}
#endif

#ifdef _WIN32
inline tpie::memory_offset_type TPIE_OS_READ(TPIE_OS_FILE_DESCRIPTOR fd, void* buffer, tpie::memory_size_type count) {
	DWORD bytesRead = 0;
	ReadFile(fd.FileHandle, buffer, (DWORD)count, &bytesRead, 0);
	return (bytesRead > 0 ? bytesRead : -1);
}
#else
inline tpie::memory_offset_type TPIE_OS_READ(TPIE_OS_FILE_DESCRIPTOR fd, void* buffer, size_t count) {
	return ::read(fd,buffer,count);
}
#endif

#ifdef _WIN32
// The suggested starting address of the mmap call has to be
// a multiple of the systems granularity (else the mapping fails)
// Hence, the parameter addr is not used at present.
inline LPVOID TPIE_OS_MMAP(LPVOID /* addr */,
			   tpie::memory_size_type  len,
			   int	 prot,
			   int	 /* flags */ ,
			   TPIE_OS_FILE_DESCRIPTOR fildes,
			   tpie::stream_offset_type off) {
	return MapViewOfFileEx( fildes.mapFileHandle,
				prot,
				getHighOrderOff(off),
				getLowOrderOff(off),
				len, NULL);
}
#else
inline void* TPIE_OS_MMAP(void* addr, size_t len, int prot, int flags, TPIE_OS_FILE_DESCRIPTOR fildes, tpie::stream_offset_type off) {
	return mmap(static_cast<caddr_t>(addr), len, prot, flags, fildes, off);
}
#endif


#ifdef _WIN32
inline int TPIE_OS_MUNMAP(LPVOID addr, size_t /* len */) {
	return (UnmapViewOfFile(addr) == 0 ? -1 : 0);
}
#else
inline int TPIE_OS_MUNMAP(void* addr, size_t len) {
	return munmap(static_cast<caddr_t>(addr), len);
}
#endif


#ifdef _WIN32
inline int TPIE_OS_MSYNC(LPVOID addr, size_t len, int /* flags */) {
	return (FlushViewOfFile(addr,len) ? 0 : -1);
}
#else
inline int TPIE_OS_MSYNC(char* addr, size_t len,int flags) {
	return msync(addr, len, flags);
}
#endif



#ifdef _WIN32
// Force the use of truncate to lengthen a collection under WIN32, due
// to mapping issues.
#ifdef BTE_COLLECTION_USE_FTRUNCATE
#undef BTE_COLLECTION_USE_FTRUNCATE
#endif
#define BTE_COLLECTION_USE_FTRUNCATE 1

inline int TPIE_OS_FTRUNCATE(TPIE_OS_FILE_DESCRIPTOR& fd, tpie::stream_offset_type length) {
  // Save the offset
  tpie::stream_offset_type so = TPIE_OS_LSEEK(fd, 0, TPIE_OS_FLAG_SEEK_CUR);
  if (fd.useFileMapping == TPIE_OS_FLAG_USE_MAPPING_TRUE) {
	CloseHandle(fd.mapFileHandle);
  }
  LONG highOrderOff = getHighOrderOff(length);
  int x = ((((fd).RDWR == false) 	||
		(SetFilePointer((fd).FileHandle,getLowOrderOff(length),&highOrderOff,FILE_BEGIN) == 0xFFFFFFFF)	||
		(SetEndOfFile((fd).FileHandle) == 0)) ? -1 : 0);

  if (fd.useFileMapping == TPIE_OS_FLAG_USE_MAPPING_TRUE) {
	fd.mapFileHandle= CreateFileMapping( (fd).FileHandle,
					 0,
					 ((fd).RDWR ? PAGE_READWRITE : PAGE_READONLY),
					 0, 0,
					 NULL);
  }
  // Restore the offset, mimicking the ftruncate() behavior.
  TPIE_OS_LSEEK(fd, (so < length ? so : length), TPIE_OS_FLAG_SEEK_SET);
  return x;
}
#else
inline int TPIE_OS_FTRUNCATE(TPIE_OS_FILE_DESCRIPTOR& fd, tpie::stream_offset_type length) {
	return ftruncate(fd, length);
}
#endif



//////////////////////////////////////////////
//				tpie close file functions	//


#ifdef _WIN32
inline int TPIE_OS_CLOSE(TPIE_OS_FILE_DESCRIPTOR fd) {
	if (fd.useFileMapping == TPIE_OS_FLAG_USE_MAPPING_TRUE) {
		return ((  (CloseHandle(fd.mapFileHandle) != 0) &&
			   (CloseHandle(fd.FileHandle) != 0)) ? 0 : -1);
	} else {
		return ((CloseHandle(fd.FileHandle) != 0) ? 0 : -1);
	}
}
#else
inline int TPIE_OS_CLOSE(TPIE_OS_FILE_DESCRIPTOR fd) {
	return ::close(fd);
}
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#define TPIE_OS_SNPRINTF _snprintf
#else
#define TPIE_OS_SNPRINTF ::snprintf
#endif



//////////////////////////////////////////////
// warnings									//
//////////////////////////////////////////////

#ifdef _WIN32
#define TPIE_OS_UNIX_ONLY_WARNING_AMI_IMP_SINGLE
#else
#define TPIE_OS_UNIX_ONLY_WARNING_AMI_IMP_SINGLE \
#warning The AMI_IMP_SINGLE flag is obsolete. \
#warning Please use AMI_STREAM_IMP_SINGLE.\
#warning Implicitly defining AMI_STREAM_IMP_SINGLE.
#endif


#ifdef _WIN32
#define TPIE_OS_UNIX_ONLY_WARNING_BTE_COLLECTION_IMP_MMB
#else
#define TPIE_OS_UNIX_ONLY_WARNING_BTE_COLLECTION_IMP_MMB \
#warning The BTE_COLLECTION_IMP_MMB flag is obsolete.\
#warning Please use BTE_COLLECTION_IMP_MMAP. \
#warning Implicitly defining BTE_COLLECTION_IMP_MMAP.
#endif


#ifdef _WIN32
#define TPIE_OS_UNIX_ONLY_WARNING_MULTIPLE_BTE_COLLECTION_IMP_DEFINED
#else
#define TPIE_OS_UNIX_ONLY_WARNING_MULTIPLE_BTE_COLLECTION_IMP_DEFINED \
#warning Multiple BTE_COLLECTION_IMP_* defined.	\
#warning Undetermined default implementation. \
#warning Implicitly defining BTE_COLLECTION_IMP_MMAP.
#endif


// To avoid this warning, define one of: BTE_COLLECTION_IMP_MMAP.	//
// BTE_COLLECTION_IMP_UFS, BTE_COLLECTION_USER_DEFINED			//
// in your app_config.h file.								   //
#ifdef _WIN32
#define TPIE_OS_UNIX_ONLY_WARNING_NO_DEFAULT_BTE_COLLECTION
#else
#define	TPIE_OS_UNIX_ONLY_WARNING_NO_DEFAULT_BTE_COLLECTION \
#warning No default BTE_COLLECTION implementation defined, using BTE_COLLECTION_IMP_MMAP by default.
#endif


#ifdef _WIN32
#define TPIE_OS_UNIX_ONLY_WARNING_USE_BTE_STREAM_IMP_UFS
#else
#define TPIE_OS_UNIX_ONLY_WARNING_USE_BTE_STREAM_IMP_UFS \
#warning The BTE_IMP_UFS flag is obsolete. Please use BTE_STREAM_IMP_UFS. \
#warning Implicitly defining BTE_STREAM_IMP_UFS.
#endif


#ifdef _WIN32
#define TPIE_OS_UNIX_ONLY_WARNING_USE_BTE_STREAM_IMP_MMAP
#else
#define TPIE_OS_UNIX_ONLY_WARNING_USE_BTE_STREAM_IMP_MMAP \
#warning The BTE_IMP_MMB flag is obsolete. Please use BTE_STREAM_IMP_MMAP. \
#warning Implicitly defining BTE_STREAM_IMP_MMAP.
#endif


#ifdef _WIN32
#define TPIE_OS_UNIX_ONLY_WARNING_USE_BTE_STREAM_IMP_STDIO
#else
#define TPIE_OS_UNIX_ONLY_WARNING_USE_BTE_STREAM_IMP_STDIO  \
#warning The BTE_IMP_STDIO flag is obsolete. Please use BTE_STREAM_IMP_STDIO.\
#warning Implicitly defining BTE_STREAM_IMP_STDIO.
#endif


#ifdef _WIN32
#define TPIE_OS_UNIX_ONLY_WARNING_USE_BTE_STREAM_IMP_USER_DEFINED
#else
#define TPIE_OS_UNIX_ONLY_WARNING_USE_BTE_STREAM_IMP_USER_DEFINED  \
#warning The BTE_IMP_USER_DEFINED flag is obsolete. Please use BTE_STREAM_IMP_USER_DEFINED.\
#warning Implicitly defining BTE_STREAM_IMP_USER_DEFINED.
#endif


#ifdef _WIN32
#define TPIE_OS_UNIX_ONLY_WARNING_MULTIPLE_BTE_STREAM_IMP_DEFINED
#else
#define TPIE_OS_UNIX_ONLY_WARNING_MULTIPLE_BTE_STREAM_IMP_DEFINED \
#warning Multiple BTE_STREAM_IMP_* defined, but BTE_STREAM_IMP_MULTI_IMP undefined.\
#warning Implicitly defining BTE_STREAM_IMP_MULTI_IMP.
#endif


#ifdef _WIN32
#define	TPIE_OS_UNIX_ONLY_WARNING_NO_IMPLEMENTATION_USING_BTE_STREAM_IMP_UFS
#else
#define	TPIE_OS_UNIX_ONLY_WARNING_NO_IMPLEMENTATION_USING_BTE_STREAM_IMP_UFS \
#warning No implementation defined.  Using BTE_STREAM_IMP_UFS by default.
#endif

#ifdef _WIN32
inline int TPIE_OS_TRUNCATE(FILE* file, const std::string& /* path */, tpie::stream_offset_type offset) {
//	TPIE_OS_LONG highOrderOff = getHighOrderOff(offset);
//	DWORD x = SetFilePointer(fd.FileHandle,getLowOrderOff(offset),&highOrderOff, FILE_BEGIN);
//	return SetEndOfFile(fd.FileHandle);
	long loffset = static_cast<long>(offset);
	if (offset > TPIE_OS_OFFSET(std::numeric_limits<long>::max()) || loffset < 0) {
		std::stringstream ss;
		ss << "Truncat offset " << offset << " too big (or negative.)";
		throw std::runtime_error(ss.str());
	}
	return _chsize(file->_file, loffset);
}
#else
inline int TPIE_OS_TRUNCATE(FILE* /* file */, const std::string& path, tpie::stream_offset_type offset) {
	return ::truncate(path.c_str(), offset);
}
#endif

#ifdef _WIN32
#define TPIE_OS_TRUNCATE_STREAM_TEMPLATE_CLASS_BODY \
LOG_FATAL_ID("_WIN32 does not support truncate() for "); \
LOG_FATAL_ID(path); \
return BTE_ERROR_OS_ERROR
#else
#define TPIE_OS_TRUNCATE_STREAM_TEMPLATE_CLASS_BODY off_t file_position; if (substream_level) { return BTE_ERROR_STREAM_IS_SUBSTREAM; } if (offset < 0) {   return BTE_ERROR_OFFSET_OUT_OF_RANGE; } file_position = offset * sizeof (T) + os_block_size_; if (::truncate (path, file_position)) {   os_errno = errno;  TP_LOG_FATAL_ID("Failed to truncate() to the new end of file:");  TP_LOG_FATAL_ID(path);  TP_LOG_FATAL_ID(strerror (os_errno));   return BTE_ERROR_OS_ERROR; } if (fseek (file, file_position, SEEK_SET)) { LOG_FATAL("fseek failed to go to position " << file_position << " of \"" << "\"\n"); LOG_FLUSH_LOG;  return BTE_ERROR_OS_ERROR; } f_offset = file_position; f_eof = file_position; return BTE_ERROR_NO_ERROR
#endif

/*
#ifdef _WIN32
#define TPIE_OS_WIN_ONLY_TEMPLATE_MERGE_HEAP_ELEMENT_COMPILER_FOOLER template<> class merge_heap_element<int>{};
#else
#define TPIE_OS_WIN_ONLY_TEMPLATE_MERGE_HEAP_ELEMENT_COMPILER_FOOLER
#endif
*/




//////////////////////////////////////////////
// others									//
//////////////////////////////////////////////

// config.h needs in line 35:				 //
// For WIN32, do not include <unistd.h>		 //
// Where is unistd.h?						 //


	//		//**
	//		*void * operator new() - Get a block of memory from the debug heap
	//		*
	//		*Purpose:
	//		*	   Allocate of block of memory of at least size bytes from the heap and
	//		*	   return a pointer to it.
	//		*
	//		*	   Allocates any type of supported memory block.
	//		*
	//		*Entry:
	//		*	   unsigned int	cb		  - count of bytes requested
	//		*	   int			 nBlockUse   - block type
	//		*	   char *		  szFileName  - file name
	//		*	   int			 nLine	   - line number
	//		*
	//		*Exit:
	//		*	   Success:  Pointer to memory block
	//		*	   Failure:  NULL (or some error value)
	//		*
	//		*Exceptions:
	//  	*
	//		******************************************************************************* /

#ifdef _WIN32
#ifndef NDEBUG
#define TPIE_OS_SPACE_OVERHEAD_BODY \
void * __cdecl _nh_malloc_dbg (tpie::memory_size_type, int, int, const char *,	int );\
void * operator new(\
			tpie::memory_size_type cb,\
			int /* nBlockUse */,\
			const char * /* szFileName */,\
			int /* nLine */ \
			)\
{\
  void *p;\
  if ((MM_manager.register_new != mem::IGNORE_MEMORY_EXCEEDED)\
	  && (MM_manager.register_allocation (cb + SIZE_SPACE) !=	mem::NO_ERROR)) {\
	switch(MM_manager.register_new) {\
	case mem::ABORT_ON_MEMORY_EXCEEDED:\
	   TP_LOG_FATAL_ID("In operator new() - allocation request ");\
	   TP_LOG_FATAL((TPIE_OS_LONG)(cb + SIZE_SPACE));\
	   TP_LOG_FATAL(" plus previous allocation ");\
	   TP_LOG_FATAL((TPIE_OS_LONG)(MM_manager.memory_used() - (cb + SIZE_SPACE)));\
	   TP_LOG_FATAL(" exceeds user-defined limit ");\
	   TP_LOG_FATAL((TPIE_OS_LONG)(MM_manager.memory_limit()));\
	   TP_LOG_FATAL(" ");\
	   std::cerr << "memory manager: memory allocation limit " << (TPIE_OS_LONG)MM_manager.memory_limit() << " exceeded while allocating " << (TPIE_OS_LONG)cb << " bytes" << std::endl;\
		exit(1);\
		break;\
	case mem::WARN_ON_MEMORY_EXCEEDED: \
	  TP_LOG_WARNING_ID("In operator new() - allocation request \"");\
	  TP_LOG_WARNING((TPIE_OS_LONG)(cb + SIZE_SPACE));\
	  TP_LOG_WARNING("\" plus previous allocation \"");\
	  TP_LOG_WARNING((TPIE_OS_LONG)(MM_manager.memory_used () - (cb + SIZE_SPACE)));\
	  TP_LOG_WARNING("\" exceeds user-defined limit \"");\
	  TP_LOG_WARNING((TPIE_OS_LONG)(MM_manager.memory_limit ()));\
	  TP_LOG_WARNING("\" \n");\
	  TP_LOG_FLUSH_LOG;\
	  std::cerr << "memory manager: memory allocation limit " << (TPIE_OS_LONG)MM_manager.memory_limit () << " exceeded " << "while allocating " << (TPIE_OS_LONG)cb << " bytes" << std::endl;\
	   break;\
   case mem::IGNORE_MEMORY_EXCEEDED:\
	   break;\
   }\
   }\
   p = malloc (cb+SIZE_SPACE);\
   if (!p) {\
	  TP_LOG_FATAL_ID("Out of memory. Cannot continue.");\
	  TP_LOG_FLUSH_LOG;\
	  std::cerr << "out of memory while allocating " << (TPIE_OS_LONG)cb << " bytes" << std::endl;\
	   perror ("mm_base::new malloc");\
	   assert(0);\
	   exit (1);\
   }\
   *((tpie::memory_size_type *) p) = cb;\
   return ((char *) p) + SIZE_SPACE;\
};
#endif
#else
#define TPIE_OS_SPACE_OVERHEAD_BODY //
#endif

#ifdef _WIN32
#define TPIE_OS_SET_GLIBCPP_FORCE_NEW //
#define TPIE_OS_UNSET_GLIBCPP_FORCE_NEW //
#else
#define TPIE_OS_SET_GLIBCPP_FORCE_NEW setenv("GLIBCPP_FORCE_NEW", "1", 1);
#define TPIE_OS_UNSET_GLIBCPP_FORCE_NEW unsetenv("GLIBCPP_FORCE_NEW");
#endif

#ifdef _WIN32
#undef NO_ERROR //ensures that the NO_ERROR macro of windows is not defined
#endif

#include <tpie/util.h>

#ifdef TPIE_DEPRECATED_WARNINGS
#ifdef __GNUC__
#define DEPRECATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED(func) __declspec(deprecated) func
#else
#define DEPRECATED(func) func
#endif
#else
#define DEPRECATED(func) func
#endif

DEPRECATED(bool TPIE_OS_EXISTS(const std::string & path));
inline bool TPIE_OS_EXISTS(const std::string & path) {
	return tpie::file_exists(path);
}

DEPRECATED(bool TPIE_OS_UNLINK(const std::string & path));
inline bool TPIE_OS_UNLINK(const std::string & path) {
	return tpie::remove(path),true;
}

DEPRECATED(void TPIE_OS_SRANDOM(unsigned int seed));
inline void TPIE_OS_SRANDOM(unsigned int seed) {
	tpie::seed_random(seed);
}

DEPRECATED(tpie::uint32_t TPIE_OS_RANDOM());
inline tpie::uint32_t TPIE_OS_RANDOM() {
	return tpie::random();
}

DEPRECATED(typedef tpie::stream_offset_type TPIE_OS_OFFSET);
DEPRECATED(typedef tpie::memory_offset_type TPIE_OS_SSIZE_T);
DEPRECATED(typedef tpie::memory_size_type TPIE_OS_SIZE_T);
DEPRECATED(typedef tpie::stream_size_type TPIE_OS_OUTPUT_SIZE_T);
DEPRECATED(typedef tpie::int64_t TPIE_OS_LONGLONG);
DEPRECATED(typedef tpie::int32_t TPIE_OS_LONG);

#undef DEPRECATED

#endif // _portability_H
#endif //_TPIE_PORTABILITY_H
