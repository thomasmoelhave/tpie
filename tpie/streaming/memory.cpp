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
#include <iostream>
#include <set>
#include <cstring>
namespace tpie {
namespace streaming {

priority_memory_manager * memory_base::memoryManager() {
	return mm;
}

void memory_base::setMemoryManager(priority_memory_manager * man) {
	mm = man;
}

std::string memory_base::memoryName() {
	return name;
}

void memory_base::setMemoryName(const char * na, bool addr) {
	std::ostringstream n;
	n << na;
	if (addr)
		n << " " << std::hex << this;
	strncpy(name, n.str().c_str(), 127);
	name[127] = '\0';
   
}

memory_base::memory_base() {
	name[0] = 0;
	setMemoryName("MO",true);
}

TPIE_OS_SIZE_T memory_base::memoryBase() {
	throw std::runtime_error("memoryBase() not implemented");
}


memory_single::memory_single(): 
	priority(1.0f),
	allocatedMemory(0) {}

double memory_single::memoryPriority() {
	return priority;
}

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
	return memoryBase();
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

memory_base::memory_type memory_wrapper::memoryType() {
	return memory_base::WRAPPER;
}

class priority_memory_manager_private {
public:
	priority_memory_manager & outer;
 	struct node {
 		node * twin;
 		std::set<node *> in;
 		std::set<node *> out;
 		memory_base * obj;
		double prio;
		TPIE_OS_SIZE_T mmin;
		TPIE_OS_SIZE_T mem;
 	};

 	std::map<memory_base *, node *> in;
 	std::map<memory_base *, node *> out;
 	std::set<memory_base *> visited;
	std::vector<node *> nodes;
	
	TPIE_OS_SIZE_T base;
	std::map<TPIE_OS_SIZE_T, TPIE_OS_SIZE_T> dsm;

 	void allocate(TPIE_OS_SIZE_T mem) {
		
 	}

	priority_memory_manager_private(priority_memory_manager & o): outer(o) {
		base = sizeof(priority_memory_manager_private);
	}
	
 	void add(memory_base * object) {
		if (visited.count(object) == 1) return;
 		object->setMemoryManager(&outer);
		visited.insert(object);
 		std::vector<memory_base *> i;
 		std::vector<memory_base *> o;
 		object->memoryNext(o);
 		object->memoryPrev(i);
		
		base += object->memoryBase();
		
 		node * a;
 		node * b;
		switch(object->memoryType()) {
		case memory_base::SINGLE:
		{
			memory_single* o = static_cast<memory_single*>(object);
			a = b = new node();
 			a->twin = NULL;
			a->prio = o->memoryPriority();
			a->mmin = o->minimumMemory() - object->memoryBase();
			break;
		}
		case memory_base::SPLIT:
		{
			memory_split* o = static_cast<memory_split*>(object);
			a = new node();
 			b = new node();
 			a->twin = b;
 			b->twin = a;
			a->prio = o->memoryInPriority();
			b->prio = o->memoryOutPriority();
			a->mmin = o->minimumMemoryIn() - object->memoryBase();
			b->mmin = o->minimumMemoryOut() - object->memoryBase();
			nodes.push_back(b);
			break;
		}
		case memory_base::WRAPPER:
		{
			a = b = new node();
 			a->twin = b->twin = NULL;
			assert(o.size() == 0);
			o.push_back( static_cast<memory_wrapper*>(object)->first() );				   
			a->mmin = 0;
			a->prio = 0.0f;
			break;
		}}
		nodes.push_back(a);
 		a->obj = b->obj = object;
 		in[object] = a;
 		out[object] = b;
		
 		for(size_t _=0; _ < i.size(); ++_) {
 			add(i[_]);
 			a->in.insert( out[ i[_] ] );
			out[i[_]]->out.insert(a);
 		}

 		for(size_t _=0; _ < o.size(); ++_) {
 			add(o[_]);
 			b->out.insert( in[ o[_] ] );
			in[ o[_] ]->in.insert(b);
 		}
 	}

	void allocate(TPIE_OS_SIZE_T mem, bool verbose) {
		if(verbose) std::cerr << "Distributing " << mem << " memory" << std::endl;
		mem -= base;

		//For each connected component destribute acording to priorities.
		std::set<node*> handledNodes;
		std::vector< std::vector<node*> > components;
		std::vector< std::map<TPIE_OS_SIZE_T, std::pair<TPIE_OS_SIZE_T, double> > > dss;
		
		for(std::vector<node*>::const_iterator i=nodes.begin(); 
			i != nodes.end(); ++i) {
			if (handledNodes.count(*i)) continue;
			components.push_back( std::vector<node *>() );
			dss.push_back( std::map<TPIE_OS_SIZE_T, std::pair<TPIE_OS_SIZE_T, double> >() );
			std::vector<node *> stack;
			stack.push_back(*i);
			
			while(!stack.empty()) {
				node * n = stack.back();
				stack.pop_back();
				if (handledNodes.count(n)) continue;
				handledNodes.insert(n);
				components.back().push_back(n);
				n->obj->dataStructures(dss.back());
				for(std::set<node *>::const_iterator i=n->in.begin();
					i != n->in.end(); ++i) 
					stack.push_back(*i);
				for(std::set<node *>::const_iterator i=n->out.begin();
					i != n->out.end(); ++i) 
					stack.push_back(*i);
			}
		}
		
		//Do initial fair memory assignment
		for(int i=0; i < components.size(); ++i) {
			std::vector<node*> & comp = components[i];
			double p=0;
			for(int j=0; j != comp.size(); ++j)
				p += comp[j]->prio;

			for(std::map<TPIE_OS_SIZE_T, std::pair<TPIE_OS_SIZE_T, double> >::const_iterator j=	dss[i].begin();
				j != dss[i].end(); ++j) {
				p += j->second.second;
			}

			for(int j = 0; j != comp.size(); ++j) 
				comp[j]->mem = std::max(comp[j]->mmin, TPIE_OS_SIZE_T(comp[j]->prio * mem / p));

			for(std::map<TPIE_OS_SIZE_T, std::pair<TPIE_OS_SIZE_T, double> >::const_iterator j=	dss[i].begin();
				j != dss[i].end(); ++j) {
				TPIE_OS_SIZE_T m=std::max(j->second.first, TPIE_OS_SIZE_T(j->second.second * mem / p));
				if( dsm.count(j->first) == 0 || dsm[j->first] > m) 
					dsm[j->first] = m;
			}
		}

		//Now we might have distributed to much and to litle memory
		//so we redistribute
 		for(int i=0; i < components.size(); ++i) {
 			std::vector<node*> & comp = components[i];
			std::vector<bool> skip(comp.size(), false);;
 			TPIE_OS_SIZE_T am=mem;
			
 			double p=0;
 			for(int j=0; j != comp.size(); ++j)
				p += comp[j]->prio;
			
 			for(std::map<TPIE_OS_SIZE_T, std::pair<TPIE_OS_SIZE_T, double> >::const_iterator j=	dss[i].begin();
 				j != dss[i].end(); ++j) 
 				am -= dsm[j->first];
			
 			bool go = true;
 			while(go) {
 				go=false;
 				for(int j = 0; j != comp.size(); ++j) {
					if(skip[j]) continue;
 					comp[j]->mem = comp[j]->prio * am / p;
 					if(comp[j]->mem <= comp[j]->mmin) {
						comp[j]->mem = comp[j]->mmin;
 						go=true;
 						p -= comp[j]->prio;
 						am -= comp[j]->mmin;
						skip[j]=true;
 					}
 				}
 			}
 		}

		if (verbose) {
			for(int i=0; i < components.size(); ++i) {
				std::vector<node*> & comp = components[i];
				TPIE_OS_SIZE_T s=0;
				std::cerr << "Phase " << i << std::endl;
				for(int j = 0; j != comp.size(); ++j) {
					TPIE_OS_SIZE_T b=comp[j]->obj->memoryBase();
					s += comp[j]->mem;
					std::cerr << "  " << comp[j]->obj->memoryName() << " " << (comp[j]->mmin+b) << " " << (comp[j]->mem+b) ;
// 					if( comp[j]->obj->memoryType() ==  memory_base::SPLIT) {
// 						if (comp[j]->in.size() == 0) std::cerr << " out";
// 						else std::cerr << " in";
// 					}
					std::cerr << std::endl;
				// 	for(std::set<node *>::iterator k=comp[j]->in.begin();
// 						k != comp[j]->in.end(); ++k) 
// 						std::cerr << "    i " << (*k)->obj->memoryName() << std::endl; 
// 					for(std::set<node *>::iterator k=comp[j]->out.begin();
// 						k != comp[j]->out.end(); ++k) 
// 						std::cerr << "    o " << (*k)->obj->memoryName() << std::endl; 
				}
				
				for(std::map<TPIE_OS_SIZE_T, std::pair<TPIE_OS_SIZE_T, double> >::iterator j=dss[i].begin();
					j != dss[i].end(); j++) {
					s += dsm[j->first];
					std::cerr << "  " << j->first << " " << dsm[j->first] << std::endl;
					
				}
				std::cerr << "  Allocated " << s+base << " of " << mem+base << std::endl;
			}
		}

		for(int i=0; i < components.size(); ++i) {
			std::vector<node*> & comp = components[i];
			for(int j = 0; j != components[i].size(); ++j) {
				std::vector<node*> & comp = components[i];
				TPIE_OS_SIZE_T m = comp[j]->mem+comp[j]->obj->memoryBase();
				switch(comp[j]->obj->memoryType()) {
				case memory_base::SINGLE:
				{
					memory_single* o = static_cast<memory_single*>(comp[j]->obj);
					o->setMemory(comp[j]->mem+comp[j]->obj->memoryBase());
					break;
				}
				case memory_base::SPLIT:
				{
					memory_split* o = static_cast<memory_split*>(comp[j]->obj);
					if (comp[j]->in.size() == 0) {
						o->setMemoryOut(m);
					} else {
						o->setMemoryIn(m);
					}
					break;
				}}
			}
		}
	}

	TPIE_OS_SIZE_T dataStructureMemory(TPIE_OS_SIZE_T ds) {
		return dsm[ds];
	}
	
};

priority_memory_manager::priority_memory_manager() {
	p = new priority_memory_manager_private(*this);
}

priority_memory_manager::~priority_memory_manager() {
	delete p;
}

void priority_memory_manager::add(memory_base * object) {
	p->add(object);
}

void priority_memory_manager::allocate(double f, bool verbose) {
	
}

void priority_memory_manager::allocate(TPIE_OS_SIZE_T mem, bool verbose) {
	p->allocate(mem, verbose);
}

TPIE_OS_SIZE_T priority_memory_manager::dataStructureMemory(TPIE_OS_SIZE_T ds) {
	p->dataStructureMemory(ds);
}


}
}
