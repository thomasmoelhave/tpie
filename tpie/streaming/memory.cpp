// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
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


class priority_memory_manager_private {
public:
	struct node {
		node * twin;
		std::set<node *> in;
		std::est<node *> out;
		memory_base * obj;
	};

	std::map<memory_base *, node *> in;
	std::map<memory_base *, node *> out;
	std::set<memory_base *> visited;
	//std::map<TPIE_OS_SIZE_T, > foo;
	
	void allocate(TPIE_OS_SIZE_T mem) {
				
	}

	void add(memory_base * object) {
		if (visited.count(object) == 1) return;
		visited.insert(object);

		std::vector<memory_base *> i;
		std::vector<memory_base *> o;
		object->memoryNext(o);
		object->memoryPrev(i);
		
		node * a;
		node * b;
		if (object->isSplit()) {
			a = new node();
			b = new node();
			a->twin = b;
			b->twin = a;
		} else {
			a = b = new node();
			a->twin = b->twin = NULL;
		}
		a->obj = b->ojb = object;
		in[object] = a;
		out[object] = b;
		
		for(size_t _=0; _ < i.size(); ++_) {
			add(i[_]);
			a->in->insert( in[ i[_] ] );
		}

		for(size_t _=0; _ < o.size(); ++_) {
			add(o[_]);
			b->out->insert( in[ o[_] ] );
		}
	}
};


class priority_memory_manager {
private:
	priority_memory_manager_private * p;
public:
	priority_memory_manager();
	~priority_memory_manager();
	void add(memory_base * object);
	void allocate();
	void allocate(double f);
	void allocate(TPIE_OS_SIZE_T mem);
};




