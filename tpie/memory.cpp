// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
//
// Copyright 2011, 2014, The TPIE development team
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

#include "memory.h"
#include <iostream>
#include <sstream>
#include "tpie_log.h"
#include <cstring>
#include <cstdlib>
#include <algorithm>
#ifndef WIN32
#include <cxxabi.h>
#endif
#include <tpie/spin_lock.h>
#include <tpie/exception.h>

namespace tpie {

memory_manager * mm = 0;

memory_manager::memory_manager(): resource_manager(MEMORY), m_mutex(0) {}

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Buffers messages to the debug log.
/// TPIE logging might use the memory manager. We don't allow memory
/// allocation/deallocation while doing an allocation/deallocation, so messages
/// stored in the log_flusher aren't sent to the TPIE log until we are done
/// allocating/deallocating.
///////////////////////////////////////////////////////////////////////////////
struct log_flusher {
	std::stringstream buf;
	~log_flusher() {
		std::string msg = buf.str();
		if(!msg.empty()) {
			tpie::log_debug() << msg;
			tpie::log_debug().flush();
		}
	}
};

std::pair<uint8_t *, size_t> memory_manager::__allocate_consecutive(size_t upper_bound, size_t granularity) {
	log_flusher lf;

	size_t high=available()/granularity;
	if (upper_bound != 0) 
		high=std::min(upper_bound/granularity, high);
	size_t low=1;
	uint8_t * res;

	//first check quickly if we can get "high" bytes of memory
	//directly.
	try {
		res = new uint8_t[high*granularity];
		register_allocation(high*granularity, typeid(uint8_t));
		return std::make_pair(res, high*granularity);
	} catch (std::bad_alloc &) {
		lf.buf << "Failed to get " << (high*granularity)/(1024*1024) << " megabytes of memory. "
		 	   << "Performing binary search to find largest amount "
			   << "of memory available. This might take a few moments.\n";
	}

	//perform a binary search in [low,high] for highest possible 
	//memory allocation
	size_t best=0;
	do {
		size_t mid = static_cast<size_t>((static_cast<uint64_t>(low)+high)/2);

		lf.buf << "Search area is  [" << low*granularity << "," << high*granularity << "]"
			   << " query amount is: " << mid*granularity << ":\n";

		//try to allocate "mid" bytes of memory
		//std::new throws an exception if memory allocation fails
		try {
			uint8_t* mem = new uint8_t[mid*granularity];
			low = mid+1;
			best=mid*granularity;
			delete[] mem;
		} catch (std::bad_alloc &) {
			high = mid-1;
			lf.buf << "   failed.\n";
		}
	} while (high >= low);
	lf.buf << "- - - - - - - END MEMORY SEARCH - - - - - -\n";	

	res = new uint8_t[best];
	register_allocation(best, typeid(uint8_t));
	return std::make_pair(res, best);
}

void memory_manager::throw_out_of_resource_error(const std::string & s) {
	throw out_of_memory_error(s);
}

void memory_manager::complain_about_unfreed_memory() {
	shared_spin_lock lock(m_mutex);

	bool first = true;
	
	for(const auto & p: m_allocations) {
		if (p.second.count == 0) continue;
		if (first) {
			log_error() << "The following types were either leaked or deleted with delete instead of tpie_delete" << std::endl << std::endl;
			first = false;
		}
		log_error() << p.first.name() << ": " << p.second.count << " of " << p.second.bytes << " bytes" << std::endl;
	}
}

void memory_manager::register_typed_allocation(size_t bytes, const std::type_info & t) {
	shared_spin_lock l(m_mutex);
	auto it = m_allocations.find(std::type_index(t));
	if (it == m_allocations.end()) {
		l.release();
		{
			unique_spin_lock l2(m_mutex);
			it = m_allocations.find(std::type_index(t));
			if (it == m_allocations.end())
				it = m_allocations.emplace(std::type_index(t), type_allocations()).first;
		}
		l.acquire();
	}
	it->second.count++;
	it->second.bytes += bytes;
}

void memory_manager::register_typed_deallocation(size_t bytes, const std::type_info & t) {
	shared_spin_lock l(m_mutex);
	auto it = m_allocations.find(std::type_index(t));
	if (it == m_allocations.end()) {
		log_error() << "Tried to unregister unknown type " << t.name() << std::endl;
	} else {
		it->second.count--;
		it->second.bytes -= bytes;
	}
}


std::unordered_map<std::type_index, memory_digest_item> memory_manager::memory_digest() {
	std::unordered_map<std::type_index, memory_digest_item> res;
	shared_spin_lock lock(m_mutex);
	for(const auto & p: m_allocations) {
		if (p.second.bytes < 1024*512) continue;
		memory_digest_item mdi;
		mdi.name = p.first.name();
#ifndef WIN32
		{
			int x;
			char * buff=abi::__cxa_demangle(mdi.name.c_str(), NULL, NULL, &x);
			if (x == 0) mdi.name = buff;
			std::free(buff);
		}
#endif
		mdi.count = p.second.count;
		mdi.bytes = p.second.bytes;
		res.emplace(p.first, std::move(mdi)).first;
	}
	return res;
}

void init_memory_manager() {
	mm = new memory_manager();
}

void finish_memory_manager() {
	mm->complain_about_unfreed_memory();
	delete mm;
	mm = 0;
}

memory_manager & get_memory_manager() {
#ifndef TPIE_NDEBUG
	if (mm == 0) throw std::runtime_error("Memory management not initialized");
#endif
	return * mm;
}

size_t consecutive_memory_available(size_t granularity) {
	std::pair<uint8_t *, size_t> r = get_memory_manager().__allocate_consecutive(0, granularity);
	tpie_delete_array(r.first, r.second);
	return r.second;
}


} //namespace tpieg
