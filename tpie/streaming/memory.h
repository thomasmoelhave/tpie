// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, The TPIE development team
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

#ifndef _TPIE_STREAMING_MEMORY_H
#define _TPIE_STREAMING_MEMORY_H

#include <vector>
#include <map>
#include <tpie/types.h>
#include <string>
namespace tpie {
namespace streaming {

class priority_memory_manager;

template <typename T>
struct memory_fits {
	static memory_size_type fits(memory_size_type mem) {
		memory_size_type high=1;
		while(T::memory(high) <= mem)
			high *= 2;
		memory_size_type low=high/2;
		memory_size_type best=low;
		while(low <= high) {
			memory_size_type c = (high+low)/2;
			memory_size_type m = T::memory(c);
			if(m <= mem) {
				best = c;
				low = c+1;
			} else
				high = c-1;
		}
		return best;
	}
};
 
class memory_base {
private:
	char name[128];
	priority_memory_manager * mm;
	memory_base(const memory_base &) {}
public:
	enum memory_type {
		SINGLE, SPLIT, WRAPPER
	};
	memory_base();
	virtual void memory_next(std::vector<memory_base *> &) {}
	virtual void memory_prev(std::vector<memory_base *> &) {}
	virtual void data_structures(std::map<memory_size_type, std::pair<memory_size_type, double> > &) {}
	std::string memory_name();
	void set_memory_name(const char * name, bool useAddr=false);
	virtual memory_size_type base_memory() ;
	virtual memory_type get_memory_type() = 0;
	priority_memory_manager * memory_manager();
	void set_memory_manager(priority_memory_manager * memoryManager);
};

class memory_single: public memory_base {
private:
	double priority;
	memory_size_type allocatedMemory;
public:
	memory_single();
	double memory_priority();
	void set_memory_priority(double priority);
	memory_size_type memory();
	void set_memory(double f);
	void set_memory(memory_size_type m);
	memory_base::memory_type get_memory_type();
	virtual memory_size_type minimum_memory();
};

class memory_split: public memory_base {
private:
	double priorityIn;
	double priorityOut;
	double allocatedMemoryIn;
	double allocatedMemoryOut;
public:
	memory_split();
	double memory_in_priority();
	void set_memory_in_priority(double p);
	double memory_out_priority();
	void set_memory_out_priority(double p);
	memory_size_type memory_in();
	memory_size_type memory_out();
	void set_memory_in(double f);
	void set_memory_out(double f);
	void set_memory_in(memory_size_type m);
	void set_memory_out(memory_size_type m);
	virtual memory_size_type minimum_memory_in();
	virtual memory_size_type minimum_memory_out();
	memory_base::memory_type get_memory_type();
};


class memory_wrapper: public memory_base {
public:
	memory_base::memory_type get_memory_type();
	virtual memory_base * first() = 0;
	virtual memory_base * last() = 0;
};

// class common_wrapper: public memory_base {
// public:
// };

class priority_memory_manager_private;

class priority_memory_manager {
private:
	priority_memory_manager_private * p;
public:
	priority_memory_manager();
	~priority_memory_manager();
	void add(memory_base * object);
	void allocate(double f=1.0, bool verbose=false);
	void allocate(memory_size_type mem, bool verbose=false);
	memory_size_type data_structure_memory(memory_size_type ds);
};

}
}

#endif //_TPIE_STREAMING_MEMORY_H
