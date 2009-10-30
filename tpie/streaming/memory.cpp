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

#include <stdexcept>
#include <tpie/streaming/memory.h>
namespace tpie {
namespace streaming {

memory_single::memory_single(): 
	priority(1.0f),
	allocatedMemory(0) {}

void memory_single::setMemoryPriority(double p) {
	priority = p;
}

TPIE_OS_SIZE_T memory_single::memory() {
	return allocatedMemory;
}

void memory_single::setMemory(double f) {
	throw std::runtime_error("Not implemented");
}

void memory_single::setMemory(TPIE_OS_SIZE_T m) {
	allocatedMemory = m;
}

memory_base::memory_type memory_single::memoryType() {
	return memory_base::SINGLE;
}

TPIE_OS_SIZE_T memory_single::minimumMemory() {
	throw std::runtime_error("minimumMemory() not implemented");
}

memory_split::memory_split(): 
	priorityIn(1.0f), priorityOut(1.0f), 
	allocatedMemoryIn(0), allocatedMemoryOut(0) 
{}
	
double memory_split::memoryInPriority() {
	return priorityIn;
}

void memory_split::setMemoryInPriority(double p) {
	priorityIn = p;
}

double memory_split::memoryOutPriority() {
	return priorityOut;
}

void memory_split::setMemoryOutPriority(double p) {
	priorityOut = p;
}

TPIE_OS_SIZE_T memory_split::memoryIn() {
	return allocatedMemoryIn;
}

TPIE_OS_SIZE_T memory_split::memoryOut() {
	return allocatedMemoryOut;
}

void memory_split::setMemoryIn(double f) {
	
}

void memory_split::setMemoryOut(double f) {
}

void memory_split::setMemoryIn(TPIE_OS_SIZE_T m) {
	allocatedMemoryIn = m;
}
void memory_split::setMemoryOut(TPIE_OS_SIZE_T m) {
	allocatedMemoryOut = m;
}

TPIE_OS_SIZE_T memory_split::minimumMemoryIn() {
	throw std::runtime_error("minimumMemoryIn() not implemented");
}

TPIE_OS_SIZE_T memory_split::minimumMemoryOut() {
	throw std::runtime_error("minimumMemoryOut() not implemented");
}

memory_base::memory_type memory_split::memoryType() {
	return memory_base::SPLIT;
}




// class priority_memory_manager_private {
// public:
// 	struct node {
// 		node * twin;
// 		std::set<node *> in;
// 		std::est<node *> out;
// 		memory_base * obj;
// 	};

// 	std::map<memory_base *, node *> in;
// 	std::map<memory_base *, node *> out;
// 	std::set<memory_base *> visited;
// 	//std::map<TPIE_OS_SIZE_T, > foo;
	
// 	void allocate(TPIE_OS_SIZE_T mem) {
				
// 	}

// 	void add(memory_base * object) {
// 		if (visited.count(object) == 1) return;
// 		visited.insert(object);

// 		std::vector<memory_base *> i;
// 		std::vector<memory_base *> o;
// 		object->memoryNext(o);
// 		object->memoryPrev(i);
		
// 		node * a;
// 		node * b;
// 		if (object->isSplit()) {
// 			a = new node();
// 			b = new node();
// 			a->twin = b;
// 			b->twin = a;
// 		} else {
// 			a = b = new node();
// 			a->twin = b->twin = NULL;
// 		}
// 		a->obj = b->ojb = object;
// 		in[object] = a;
// 		out[object] = b;
		
// 		for(size_t _=0; _ < i.size(); ++_) {
// 			add(i[_]);
// 			a->in->insert( in[ i[_] ] );
// 		}

// 		for(size_t _=0; _ < o.size(); ++_) {
// 			add(o[_]);
// 			b->out->insert( in[ o[_] ] );
// 		}
// 	}
// };


// class priority_memory_manager {
// private:
// 	priority_memory_manager_private * p;
// public:
// 	priority_memory_manager();
// 	~priority_memory_manager();
// 	void add(memory_base * object);
// 	void allocate();
// 	void allocate(double f);
// 	void allocate(TPIE_OS_SIZE_T mem);
// };

}
}
