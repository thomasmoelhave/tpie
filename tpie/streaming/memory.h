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
#include <tpie/portability.h>

namespace tpie {
namespace streaming {

class priority_memory_manager;

template <typename T>
struct memory_fits {
	static TPIE_OS_SIZE_T fits(TPIE_OS_SIZE_T mem) {
		TPIE_OS_SIZE_T high=1;
		while(T::memory(high) <= mem)
			high *= 2;
		TPIE_OS_SIZE_T low=high/2;
		TPIE_OS_SIZE_T best=low;
		while(low <= high) {
			TPIE_OS_SIZE_T c = (high+low)/2;
			TPIE_OS_SIZE_T m = T::memory(c);
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
	virtual void memoryNext(std::vector<memory_base *> &) {}
	virtual void memoryPrev(std::vector<memory_base *> &) {}
	virtual void dataStructures(std::map<TPIE_OS_SIZE_T, std::pair<TPIE_OS_SIZE_T, double> > &) {}
	std::string memoryName();
	void setMemoryName(const char * name, bool useAddr=false);
	virtual TPIE_OS_SIZE_T memoryBase();
	virtual memory_type memoryType() = 0;
	priority_memory_manager * memoryManager();
	void setMemoryManager(priority_memory_manager * man);
};

class memory_single: public memory_base {
private:
	double priority;
	TPIE_OS_SIZE_T allocatedMemory;
public:
	memory_single();
	double memoryPriority();
	void setMemoryPriority(double priority);
	TPIE_OS_SIZE_T memory();
	void setMemory(double f);
	void setMemory(TPIE_OS_SIZE_T m);
	memory_base::memory_type memoryType();
	virtual TPIE_OS_SIZE_T minimumMemory();
};

class memory_split: public memory_base {
private:
	double priorityIn;
	double priorityOut;
	double allocatedMemoryIn;
	double allocatedMemoryOut;
public:
	memory_split();
	double memoryInPriority();
	void setMemoryInPriority(double p);
	double memoryOutPriority();
	void setMemoryOutPriority(double p);
	TPIE_OS_SIZE_T memoryIn();
	TPIE_OS_SIZE_T memoryOut();
	void setMemoryIn(double f);
	void setMemoryOut(double f);
	void setMemoryIn(TPIE_OS_SIZE_T m);
	void setMemoryOut(TPIE_OS_SIZE_T m);
	virtual TPIE_OS_SIZE_T minimumMemoryIn();
	virtual TPIE_OS_SIZE_T minimumMemoryOut();
	memory_base::memory_type memoryType();
};


class memory_wrapper: public memory_base {
public:
	memory_base::memory_type memoryType();
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
	void allocate(TPIE_OS_SIZE_T mem, bool verbose=false);
	TPIE_OS_SIZE_T dataStructureMemory(TPIE_OS_SIZE_T ds);
};

}
}

#endif //_TPIE_STREAMING_MEMORY_H
