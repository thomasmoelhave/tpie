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
#include <tpie/mm_base.h>
#include <tpie/tpie_log.h>
#include <tpie/mm_manager.h>
#include <tpie/util.h>
#include <tpie/static_string_stream.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cerrno>

// support for dmalloc (for tracking memory leaks)
#ifdef USE_DMALLOC
#define DMALLOC_DISABLE
#include <dmalloc.h>
#include <return.h>
#endif

using namespace tpie;

#ifdef MM_BACKWARD_COMPATIBLE
int   register_new = IGNORE_MEMORY_EXCEEDED;
#endif


/// Ensures alignment on quad word boundaries.  It may be
/// possible to check whether a machine needs this at configuration
/// time or if dword alignment is ok.  On the HP 9000, bus errors occur
/// when loading doubles that are not qword aligned.
/// It is also used to keep the allocation size counter.
static const TPIE_OS_SIZE_T SIZE_SPACE=(sizeof(TPIE_OS_SIZE_T) > 8 ? sizeof(TPIE_OS_SIZE_T) : 8);

#ifdef TPIE_USE_EXCEPTIONS
#define EXCEPTIONS_PARAM(x) x
#else
#define EXCEPTIONS_PARAM(x)
#endif

tpie::static_string_stream sss;

static void *do_new (TPIE_OS_SIZE_T sz, bool EXCEPTIONS_PARAM(allow_throw))
{
	void *p;
	if ((MM_manager.register_new != mem::IGNORE_MEMORY_EXCEEDED)
			&& (MM_manager.register_allocation (sz + SIZE_SPACE) !=
					mem::NO_ERROR)) {
		switch(MM_manager.register_new) {
			case mem::ABORT_ON_MEMORY_EXCEEDED: 
			{
				MM_manager.register_deallocation(sz + SIZE_SPACE); //Deallocate the space again
				sss.clear();
				sss << "Memory allocation error, memory limit exceeded. "
					<<"Allocation request \""
					<< (sz + SIZE_SPACE)
					<< "\" plus previous allocation \""
					<< MM_manager.memory_used ()
					<< "\" exceeds user-defined limit \""
					<< MM_manager.memory_limit ()
					<< "\"";
#ifdef TPIE_USE_EXCEPTIONS
				if (allow_throw) throw out_of_memory_error(sss.c_str());
#endif
				log_error() << sss.c_str() << std::endl;
				exit (1);
			} break;
			case mem::WARN_ON_MEMORY_EXCEEDED:
			{
				//// THIL **WILL** CAUSE A STACK OVERFLOW IF THE LOG ALLOCATES (WHICH IT DOES)
				log_warning() << "Memory allocation error, memory limit exceeded. "
							  << "In operator new() - allocation request \""
							  << static_cast<TPIE_OS_LONG>(sz + SIZE_SPACE)
							  << "\" plus previous allocation \""
							  << static_cast<TPIE_OS_LONG>(MM_manager.memory_used () - (sz + SIZE_SPACE))
							  << "\" exceeds user-defined limit \""
							  << static_cast<TPIE_OS_LONG>(MM_manager.memory_limit ())
							  << "\"" << std::endl;
			} break;
			case mem::IGNORE_MEMORY_EXCEEDED:
			{ 
			} break;
	    }
	}
	
	p = malloc(sz + SIZE_SPACE);

	if (!p) {
		//deregister the allocation again
		//it is VITAL that this happens here since the stringstream below needs
		//memory to function :)
		if (MM_manager.register_new != mem::IGNORE_MEMORY_EXCEEDED) {
			if (MM_manager.register_deallocation (sz + SIZE_SPACE) != mem::NO_ERROR) {
				log_mem_debug() << "In operator delete [] - MM_manager.register_deallocation failed" << std::endl;
			}
		}
	    
		const char* err = strerror(errno);
		sss.clear();
		sss << "Memory allocation error, likely due to heap fragmentation. "
			<< "Could not allocate " 
			<< (sz+SIZE_SPACE)/1024/1024 << " megabytes (" 
			<< sz+SIZE_SPACE << " bytes) from the heap."
			<<" malloc returned a null pointer, errno is "
			<< errno << " (" << err << "). Available memory according to TPIE is "
			<< MM_manager.memory_available()/1024/1024 << " megabytes ("
			<< MM_manager.memory_available() << " bytes)";		
			
#ifdef TPIE_USE_EXCEPTIONS
		if (allow_throw) {
			throw out_of_memory_error(sss.c_str());
		}
#endif
		log_error() << sss.c_str() << std::endl;
		exit (1);
	}
	*(reinterpret_cast<size_t *>(p)) = (MM_manager.allocation_count_factor() ? sz : 0);
	return (reinterpret_cast<char *>(p)) + SIZE_SPACE;
}

void *operator new (TPIE_OS_SIZE_T sz) throw(std::bad_alloc)
{
    return do_new(sz, true);
}

void *operator new (TPIE_OS_SIZE_T sz, const std::nothrow_t &) throw()
{
    return do_new(sz, false);
}

void *operator new[] (TPIE_OS_SIZE_T sz) throw(std::bad_alloc)
{
    return do_new(sz, true);
}

void *operator new[] (TPIE_OS_SIZE_T sz, const std::nothrow_t &) throw()
{
    return do_new(sz, false);
}

void operator delete (void *ptr) throw()
{
    if (!ptr) return;
    
    const TPIE_OS_SIZE_T dealloc_size =  
	static_cast<TPIE_OS_SIZE_T>(*(reinterpret_cast<size_t*>((reinterpret_cast<char*>(ptr)) - SIZE_SPACE)));
    
	if (MM_manager.register_deallocation (
		dealloc_size ? dealloc_size + SIZE_SPACE : 0) != mem::NO_ERROR) {
	    TP_LOG_WARNING_ID("In operator delete - MM_manager.register_deallocation failed");
	}
    void *p = (reinterpret_cast<char *>(ptr)) - SIZE_SPACE;
    
#ifdef USE_DMALLOC
    char	*file;
    GET_RET_ADDR(file);
    _free_leap(file, 0, p);    
#else
    free(p);
#endif
}

void operator delete[] (void *ptr) throw() {
    if (!ptr) return;
    
    const TPIE_OS_SIZE_T dealloc_size =  
	static_cast<TPIE_OS_SIZE_T>(*(reinterpret_cast<size_t*>((reinterpret_cast<char*>(ptr)) - SIZE_SPACE)));
    
	if (MM_manager.register_deallocation (
			dealloc_size ? dealloc_size + SIZE_SPACE : 0) != mem::NO_ERROR) {
	    TP_LOG_WARNING_ID("In operator delete [] - MM_manager.register_deallocation failed");
	}
	void *p = (reinterpret_cast<char *>(ptr)) - SIZE_SPACE;
#ifdef USE_DMALLOC
    char	*file;
    GET_RET_ADDR(file);
    _free_leap(file, 0, p);    
#else
    free(p);
#endif
}

// return the overhead on each memory allocation request 
int mem::manager::space_overhead () {
    return SIZE_SPACE;
}

#undef EXCEPTIONS_PARAM

#ifndef TPIE_NDEBUG
TPIE_OS_SPACE_OVERHEAD_BODY
#endif

