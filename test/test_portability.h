// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
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

#ifndef __TPIE_TEST_PORTABILITY__
#define __TPIE_TEST_PORTABILITY__

#include <fcntl.h>

//snprintf is different on WIN/Unix platforms
#ifdef _WIN32
#define APP_SNPRINTF _snprintf
#else
#define APP_SNPRINTF snprintf
#endif

#ifdef _WIN32
#define	 TMP_DIR ".\\"
#define TPIE_OS_DIR_DELIMITER "\\"
#else
#define	TMP_DIR	"/var/tmp/"
#define TPIE_OS_DIR_DELIMITER "/"
#endif

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

enum TPIE_OS_MAPPING_FLAG {
	TPIE_OS_FLAG_USE_MAPPING_FALSE,
	TPIE_OS_FLAG_USE_MAPPING_TRUE	
};

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
inline int TPIE_OS_UNLINK(const std::string& filename) {
    return _unlink(filename.c_str());
}
#else
inline int TPIE_OS_UNLINK(const std::string& filename) {
    return ::unlink(filename.c_str());
}
#endif

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

#ifdef _WIN32
// Generate 31 random bits using rand(), which normally generates only
// 15 random bits.
inline int TPIE_OS_RANDOM() {
  return rand() % 0x8000 + (rand() % 0x8000 << 15) + (rand() % 0x2 << 30);
}
#else
inline int TPIE_OS_RANDOM() {
    //adanner: rand and srand are ANSI standards
    //random and srandom are from old BSD systems
    //use the standard unless we run into problems
    //http://www.gnu.org/software/libc/manual/html_node/Pseudo_002dRandom-Numbers.html
    return rand();
}
#endif

#ifdef _WIN32
inline void TPIE_OS_SRANDOM(time_t seed) {
    srand((unsigned int)seed);
}
#else
inline void TPIE_OS_SRANDOM(unsigned int seed) {
    srand(seed);
}
#endif

#endif // __TPIE_TEST_PORTABILITY__
