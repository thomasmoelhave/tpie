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

#include <tpie/portability.h>
#include <stdio.h>
#include <errno.h>

//Needed for windows only

#ifdef _WIN32
std::ostream& operator<<(std::ostream& s, const TPIE_OS_OFFSET x){
  char buf[30];
  sprintf(buf,"%I64d",x);
  return s << buf;
}
#endif

namespace tpie {
	
#ifdef _WIN32
//On windows there do not seem to be any limit on the number of handels we can create
size_t get_os_available_fds() {return 1024*128;}  
#else
size_t get_os_available_fds() {
	size_t f=0;
	for(int fd=0; fd < getdtablesize(); ++fd) {
		int flags = fcntl(fd, F_GETFD, 0);
		if (flags == -1 && errno == EBADF) ++f;
	}
	return f-5; //-5 to prevent race conditions
}
#endif


void atomic_rename(const std::string & src, const std::string & dst) {
	//Note according to posix rename is atomic..
	//On windows it is probably not
	int r = rename(src.c_str(), dst.c_str());
	if (r == 0) return;
	if (errno != EEXIST) throw std::runtime_error("Atomic rename failed");
	remove(dst.c_str());
	r = rename(src.c_str(), dst.c_str());
	if (r != 0) throw std::runtime_error("Atomic rename failed");
}



}
