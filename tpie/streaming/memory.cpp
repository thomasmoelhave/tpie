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
#include <cassert>
#include <cstring>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <tpie/streaming/memory.h>
namespace tpie {
namespace streaming {

priority_memory_manager * memory_base::memory_manager() {
	return mm;
}

void memory_base::set_memory_manager(priority_memory_manager * man) {
	mm = man;
}

std::string memory_base::memory_name() {
	return name;
}

void memory_base::set_memory_name(const char * na, bool addr) {
	std::ostringstream n;
	n << na;
	if (addr)
		n << " " << std::hex << this;
	strncpy(name, n.str().c_str(), 127);
	name[127] = '\0';
   
}

memory_base::memory_base() {
	name[0] = 0;
	set_memory_name("MO",true);
}

memory_size_type memory_base::base_memory() {
	throw std::runtime_error("base_memory() not implemented");
}

memory_single::memory_single(): 
	priority(1.0f),
	allocatedMemory(0) {}

double memory_single::memory_priority() {
	return priority;
}

void memory_single::set_memory_priority(double p) {
	priority = p;
}

memory_size_type memory_single::memory() {
	return allocatedMemory;
}

void memory_single::set_memory(double f) {
	throw std::runtime_error("Not implemented");
}

void memory_single::set_memory(memory_size_type memory) {
	allocatedMemory = memory;
}

memory_base::memory_type memory_single::get_memory_type() {
	return memory_base::SINGLE;
}

memory_size_type memory_single::minimum_memory() {
	return base_memory();
}

memory_split::memory_split(): 
	priorityIn(1.0f), priorityOut(1.0f), 
	allocatedMemoryIn(0), allocatedMemoryOut(0) 
{}
	
double memory_split::memory_in_priority() {
	return priorityIn;
}

void memory_split::set_memory_in_priority(double p) {
	priorityIn = p;
}

double memory_split::memory_out_priority() {
	return priorityOut;
}

void memory_split::set_memory_out_priority(double p) {
	priorityOut = p;
}

memory_size_type memory_split::memory_in() {
	return allocatedMemoryIn;
}

memory_size_type memory_split::memory_out() {
	return allocatedMemoryOut;
}

void memory_split::set_memory_in(double f) {
	throw std::runtime_error("Not implemented");
}

void memory_split::set_memory_out(double f) {
	throw std::runtime_error("Not implemented");
}

void memory_split::set_memory_in(memory_size_type m) {
	allocatedMemoryIn = m;
}

void memory_split::set_memory_out(memory_size_type m) {
	allocatedMemoryOut = m;
}

memory_size_type memory_split::minimum_memory_in() {
	throw std::runtime_error("minimumMemory_in() not implemented");
}

memory_size_type memory_split::minimum_memory_out() {
	throw std::runtime_error("minimumMemory_out() not implemented");
}

memory_base::memory_type memory_split::get_memory_type() {
	return memory_base::SPLIT;
}

memory_base::memory_type memory_wrapper::get_memory_type() {
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
		memory_size_type mmin;
		memory_size_type mem;
 	};

 	std::map<memory_base *, node *> in;
 	std::map<memory_base *, node *> out;
 	std::set<memory_base *> visited;
	std::vector<node *> nodes;
	
	memory_size_type base;
	std::map<memory_size_type, memory_size_type> dsm;

	priority_memory_manager_private(priority_memory_manager & o): outer(o) {
		base = sizeof(priority_memory_manager_private);
	}
	
 	void add(memory_base * object) {
		if (visited.count(object) == 1) return;
 		object->set_memory_manager(&outer);
		visited.insert(object);
 		std::vector<memory_base *> i;
 		std::vector<memory_base *> o;
 		object->memory_next(o);
 		object->memory_prev(i);
		
		base += object->base_memory();
		
 		node * a;
 		node * b;
		switch(object->get_memory_type()) {
		case memory_base::SINGLE:
		{
			memory_single* o = static_cast<memory_single*>(object);
			a = b = new node();
 			a->twin = NULL;
			a->prio = o->memory_priority();
			a->mmin = o->minimum_memory() - object->base_memory();
			break;
		}
		case memory_base::SPLIT:
		{
			memory_split* o = static_cast<memory_split*>(object);
			a = new node();
 			b = new node();
 			a->twin = b;
 			b->twin = a;
			a->prio = o->memory_in_priority();
			b->prio = o->memory_out_priority();
			a->mmin = o->minimum_memory_in() - object->base_memory();
			b->mmin = o->minimum_memory_out() - object->base_memory();
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

	void allocate(memory_size_type mem, bool verbose) {
		if(verbose) std::cerr << "Distributing " << mem << " memory" << std::endl;
		mem -= base;

		//For each connected component destribute acording to priorities.
		std::set<node*> handledNodes;
		std::vector< std::vector<node*> > components;
		std::vector< std::map<memory_size_type, std::pair<memory_size_type, double> > > dss;
		
		for(std::vector<node*>::const_iterator i=nodes.begin(); 
			i != nodes.end(); ++i) {
			if (handledNodes.count(*i)) continue;
			components.push_back( std::vector<node *>() );
			dss.push_back( std::map<memory_size_type, std::pair<memory_size_type, double> >() );
			std::vector<node *> stack;
			stack.push_back(*i);
			
			while(!stack.empty()) {
				node * n = stack.back();
				stack.pop_back();
				if (handledNodes.count(n)) continue;
				handledNodes.insert(n);
				components.back().push_back(n);
				n->obj->data_structures(dss.back());
				for(std::set<node *>::const_iterator i=n->in.begin();
					i != n->in.end(); ++i) 
					stack.push_back(*i);
				for(std::set<node *>::const_iterator i=n->out.begin();
					i != n->out.end(); ++i) 
					stack.push_back(*i);
			}
		}
		
		//Do initial fair memory assignment
		for(memory_size_type i=0; i < components.size(); ++i) {
			std::vector<node*> & comp = components[i];
			double p=0;
			for(memory_size_type j=0; j != comp.size(); ++j)
				p += comp[j]->prio;

			for(std::map<memory_size_type, std::pair<memory_size_type, double> >::const_iterator j=	dss[i].begin();
				j != dss[i].end(); ++j) {
				p += j->second.second;
			}

			for(memory_size_type j = 0; j != comp.size(); ++j) 
				comp[j]->mem = std::max(comp[j]->mmin, memory_size_type(comp[j]->prio * mem / p));

			for(std::map<memory_size_type, std::pair<memory_size_type, double> >::const_iterator j=	dss[i].begin();
				j != dss[i].end(); ++j) {
				memory_size_type m=std::max(j->second.first, memory_size_type(j->second.second * mem / p));
				if( dsm.count(j->first) == 0 || dsm[j->first] > m) 
					dsm[j->first] = m;
			}
		}

		//Now we might have distributed to much and to litle memory
		//so we redistribute
 		for(memory_size_type i=0; i < components.size(); ++i) {
 			std::vector<node*> & comp = components[i];
			std::vector<bool> skip(comp.size(), false);;
 			memory_size_type am=mem;
			
 			double p=0;
 			for(memory_size_type j=0; j != comp.size(); ++j)
				p += comp[j]->prio;
			
 			for(std::map<memory_size_type, std::pair<memory_size_type, double> >::const_iterator j=	dss[i].begin();
 				j != dss[i].end(); ++j) 
 				am -= dsm[j->first];
			
 			bool go = true;
 			while(go) {
 				go=false;
 				for(memory_size_type j = 0; j != comp.size(); ++j) {
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
			for(memory_size_type i=0; i < components.size(); ++i) {
				std::vector<node*> & comp = components[i];
				memory_size_type s=0;
				std::cerr << "Phase " << i << std::endl;
				for(memory_size_type j = 0; j != comp.size(); ++j) {
					memory_size_type b=comp[j]->obj->base_memory();
					s += comp[j]->mem;
					std::cerr << "  " << comp[j]->obj->memory_name() << " " << (comp[j]->mmin+b) << " " << (comp[j]->mem+b) ;
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
				
				for(std::map<memory_size_type, std::pair<memory_size_type, double> >::iterator j=dss[i].begin();
					j != dss[i].end(); j++) {
					s += dsm[j->first];
					std::cerr << "  " << j->first << " " << dsm[j->first] << std::endl;
					
				}
				std::cerr << "  Allocated " << s+base << " of " << mem+base << std::endl;
			}
		}

		for(memory_size_type i=0; i < components.size(); ++i) {
			std::vector<node*> & comp = components[i];
			for(memory_size_type j = 0; j != components[i].size(); ++j) {
				memory_size_type m = comp[j]->mem+comp[j]->obj->base_memory();
				switch(comp[j]->obj->get_memory_type()) {
				case memory_base::SINGLE:
				{
					memory_single* o = static_cast<memory_single*>(comp[j]->obj);
					o->set_memory(comp[j]->mem+comp[j]->obj->base_memory());
					break;
				}
				case memory_base::SPLIT:
				{
					memory_split* o = static_cast<memory_split*>(comp[j]->obj);
					if (comp[j]->in.size() == 0) {
						o->set_memory_out(m);
					} else {
						o->set_memory_in(m);
					}
					break;
				}
				case memory_base::WRAPPER:
					break;
				}
			}
		}
	}

	memory_size_type data_structure_memory(memory_size_type ds) {
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
	throw std::runtime_error("Not implemented");		
}

void priority_memory_manager::allocate(memory_size_type mem, bool verbose) {
	p->allocate(mem, verbose);
}

memory_size_type priority_memory_manager::data_structure_memory(memory_size_type ds) {
	return p->data_structure_memory(ds);
}


}
}
