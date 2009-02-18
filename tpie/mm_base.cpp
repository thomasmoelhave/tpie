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

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <errno.h>

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

void *operator new (TPIE_OS_SIZE_T sz)
{
   void *p;
#ifdef USE_DMALLOC
	char	*file;
	GET_RET_ADDR(file);
#endif

	if ((MM_manager.register_new != mem::IGNORE_MEMORY_EXCEEDED)
			&& (MM_manager.register_allocation (sz + SIZE_SPACE) !=
				mem::NO_ERROR)) {
		switch(MM_manager.register_new) {
			case mem::ABORT_ON_MEMORY_EXCEEDED: 
			{
				TP_LOG_FATAL_ID ("In operator new() - allocation request \"");
				TP_LOG_FATAL (static_cast<TPIE_OS_LONG>(sz + SIZE_SPACE));
				TP_LOG_FATAL ("\" plus previous allocation \"");
				TP_LOG_FATAL (static_cast<TPIE_OS_LONG>(MM_manager.memory_used () - (sz + SIZE_SPACE)));
				TP_LOG_FATAL ("\" exceeds user-defined limit \"");
				TP_LOG_FATAL (static_cast<TPIE_OS_LONG>(MM_manager.memory_limit ()));
				TP_LOG_FATAL ("\" \n");
				TP_LOG_FLUSH_LOG;
				std::cerr << "memory manager: memory allocation limit " 
					<< static_cast<TPIE_OS_LONG>(MM_manager.memory_limit ()) 
					<< " exceeded while allocating " 
					<< static_cast<TPIE_OS_LONG>(sz)
					<< " bytes" << "\n";
#ifdef USE_DMALLOC
				dmalloc_shutdown();
#endif
				assert (0);		// core dump if debugging
#ifdef TPIE_USE_EXCEPTIONS
				std::stringstream ss;
				ss << "Could not allocate " 
					<< (sz+SIZE_SPACE)/1024/1024 << " megabytes (" 
					<< sz+SIZE_SPACE << " bytes) from the heap."
					<<" The TPIE memory limit was exceeded "
					<< " Available memory accoding to TPIE is "
					<< MM_manager.memory_available()/1024/1024 << " megabytes ("
					<< MM_manager.memory_available() << " bytes)\n";
				throw out_of_memory_error(ss.str());
#else
				exit (1);
#endif
			} break;
			case mem::WARN_ON_MEMORY_EXCEEDED:
			{
				TP_LOG_WARNING_ID ("In operator new() - allocation request \"");
				TP_LOG_WARNING (static_cast<TPIE_OS_LONG>(sz + SIZE_SPACE));
				TP_LOG_WARNING ("\" plus previous allocation \"");
				TP_LOG_WARNING (static_cast<TPIE_OS_LONG>(MM_manager.memory_used () - (sz + SIZE_SPACE)));
				TP_LOG_WARNING ("\" exceeds user-defined limit \"");
				TP_LOG_WARNING (static_cast<TPIE_OS_LONG>(MM_manager.memory_limit ()));
				TP_LOG_WARNING ("\" \n");
				TP_LOG_FLUSH_LOG;
				std::cerr << "memory manager: memory allocation limit " 
					<< static_cast<TPIE_OS_LONG>(MM_manager.memory_limit ()) 
					<< " exceeded while allocating " 
					<< static_cast<TPIE_OS_LONG>(sz)
					<< " bytes" << "\n";
			} break;
			case mem::IGNORE_MEMORY_EXCEEDED:
			{ 
			} break;
	    }
	}
	
#ifdef USE_DMALLOC
	p = _malloc_leap(file, 0, sz + SIZE_SPACE);
#else
	p = malloc(sz + SIZE_SPACE);
#endif
	if (!p) {
		//deregister the allocation again
		//it is VITAL that this happens here since the stringstream below needs
		//memory to function :)
		if (MM_manager.register_new != mem::IGNORE_MEMORY_EXCEEDED) {
			if (MM_manager.register_deallocation (sz + SIZE_SPACE) != mem::NO_ERROR) {
				TP_LOG_WARNING_ID("In operator delete [] - MM_manager.register_deallocation failed");
			}
		}
	    
		TP_LOG_FATAL_ID ("Out of memory. Cannot continue.");
	    TP_LOG_FLUSH_LOG;
		const char* err = strerror(errno);
		std::stringstream ss;
		ss << "Could not allocate " 
			<< (sz+SIZE_SPACE)/1024/1024 << " megabytes (" 
			<< sz+SIZE_SPACE << " bytes) from the heap."
			<<" malloc returned a null pointer, errno is "
			<< errno << " (" << err << "). Available memory accoding to TPIE is "
			<< MM_manager.memory_available()/1024/1024 << " megabytes ("
			<< MM_manager.memory_available() << " bytes)\n";

#ifdef TPIE_USE_EXCEPTIONS
		throw out_of_memory_error(ss.str());
#else
		std::cerr << ss.str() << std::endl;
		assert(0);
		exit (1);
#endif
	}
	*(reinterpret_cast<size_t *>(p)) = (MM_manager.allocation_count_factor() ? sz : 0);
	return (reinterpret_cast<char *>(p)) + SIZE_SPACE;
}

void *operator new[] (TPIE_OS_SIZE_T sz)
{
    void *p;
#ifdef USE_DMALLOC
    char	*file;
    GET_RET_ADDR(file);
#endif
    
	if ((MM_manager.register_new != mem::IGNORE_MEMORY_EXCEEDED)
			&& (MM_manager.register_allocation (sz + SIZE_SPACE) !=
				mem::NO_ERROR)) {
		switch(MM_manager.register_new) {
			case mem::ABORT_ON_MEMORY_EXCEEDED:
			{
				TP_LOG_FATAL_ID ("In operator new() - allocation request \"");
				TP_LOG_FATAL (static_cast<TPIE_OS_LONG>(sz + SIZE_SPACE));
				TP_LOG_FATAL ("\" plus previous allocation \"");
				TP_LOG_FATAL (static_cast<TPIE_OS_LONG>(MM_manager.memory_used () - (sz + SIZE_SPACE)));
				TP_LOG_FATAL ("\" exceeds user-defined limit \"");
				TP_LOG_FATAL (static_cast<TPIE_OS_LONG>(MM_manager.memory_limit ()));
				TP_LOG_FATAL ("\" \n");
				TP_LOG_FLUSH_LOG;
				std::cerr << "memory manager: memory allocation limit " 
					<< static_cast<TPIE_OS_LONG>(MM_manager.memory_limit ()) 
					<< " exceeded while allocating " 
					<< static_cast<TPIE_OS_LONG>(sz)
					<< " bytes" << "\n";
#ifdef USE_DMALLOC
				dmalloc_shutdown();
#endif
				assert (0);		// core dump if debugging
				exit (1);
			} break;
		case mem::WARN_ON_MEMORY_EXCEEDED:
		{
			TP_LOG_WARNING_ID ("In operator new() - allocation request \"");
			TP_LOG_WARNING (static_cast<TPIE_OS_LONG>(sz + SIZE_SPACE));
			TP_LOG_WARNING ("\" plus previous allocation \"");
			TP_LOG_WARNING (static_cast<TPIE_OS_LONG>(MM_manager.memory_used () - (sz + SIZE_SPACE)));
			TP_LOG_WARNING ("\" exceeds user-defined limit \"");
			TP_LOG_WARNING (static_cast<TPIE_OS_LONG>(MM_manager.memory_limit ()));
			TP_LOG_WARNING ("\" \n");
			TP_LOG_FLUSH_LOG;
			std::cerr << "memory manager: memory allocation limit " 
				<< static_cast<TPIE_OS_LONG>(MM_manager.memory_limit ()) 
				<< " exceeded while allocating " 
				<< static_cast<TPIE_OS_LONG>(sz)
				<< " bytes" << "\n";
		} break;
		case mem::IGNORE_MEMORY_EXCEEDED:
		{	
		} break;
		}
	}
    
#ifdef USE_DMALLOC
    p = _malloc_leap(file, 0, sz + SIZE_SPACE);
#else
    p = malloc(sz + SIZE_SPACE);
#endif
    if (!p) {
	//deregister the allocation again
	//it is VITAL that this happens here since the stringstream below needs
	//memory to function :)
	if (MM_manager.register_new != mem::IGNORE_MEMORY_EXCEEDED) {
		if (MM_manager.register_deallocation (sz + SIZE_SPACE) != mem::NO_ERROR) {
			TP_LOG_WARNING_ID("In operator delete [] - MM_manager.register_deallocation failed");
		}
	}
	TP_LOG_FATAL_ID ("Out of memory. Cannot continue.");
	TP_LOG_FLUSH_LOG;
	std::stringstream ss;
	const char* err = strerror(errno);
	ss << "Could not allocate " 
			<< sz/1024/1024 << " megabytes (" 
			<< sz << " bytes) from the heap."
			<<" malloc returned a null pointer, errno is "
			<< errno << " (" << err << "). Available memory accoding to TPIE is "
			<< MM_manager.memory_available()/1024/1024 << " megabytes ("
			<< MM_manager.memory_available() << " bytes)\n";

#ifdef TPIE_USE_EXCEPTIONS
	throw out_of_memory_error(ss.str());
#else
	std::cerr << ss.str() << std::endl;
	assert(0);
	exit (1);
#endif
    }
    *(reinterpret_cast<size_t *>(p)) = (MM_manager.allocation_count_factor() ? sz : 0);
	return (reinterpret_cast<char *>(p)) + SIZE_SPACE;
}

void operator delete (void *ptr)
{
    if (!ptr) {
	TP_LOG_WARNING_ID ("operator delete was given a NULL pointer");
	return;
    }
    
    const TPIE_OS_SIZE_T dealloc_size =  
	static_cast<TPIE_OS_SIZE_T>(*(reinterpret_cast<size_t*>((reinterpret_cast<char*>(ptr)) - SIZE_SPACE)));
    
    if (MM_manager.register_new != mem::IGNORE_MEMORY_EXCEEDED) {
	if (MM_manager.register_deallocation (
		dealloc_size ? dealloc_size + SIZE_SPACE : 0) != mem::NO_ERROR) {
	    TP_LOG_WARNING_ID("In operator delete - MM_manager.register_deallocation failed");
	}
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

void operator delete[] (void *ptr) {
    if (!ptr) {
	TP_LOG_WARNING_ID ("operator delete [] was given a NULL pointer");
	return;
    }
    
    const TPIE_OS_SIZE_T dealloc_size =  
	static_cast<TPIE_OS_SIZE_T>(*(reinterpret_cast<size_t*>((reinterpret_cast<char*>(ptr)) - SIZE_SPACE)));
    
    if (MM_manager.register_new != mem::IGNORE_MEMORY_EXCEEDED) {
	if (MM_manager.register_deallocation (
		dealloc_size ? dealloc_size + SIZE_SPACE : 0) != mem::NO_ERROR) {
	    TP_LOG_WARNING_ID("In operator delete [] - MM_manager.register_deallocation failed");
	}
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

#ifndef NDEBUG
TPIE_OS_SPACE_OVERHEAD_BODY
#endif

