// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2016, The TPIE development team
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

#include <ostream>
#include <vector>
#include <tpie/jsonprint.h>

namespace tpie {


struct item {
	size_t indent;
	bool first;
	bool array;
	size_t size;
};

class JSONReflectorP {
public:
	std::ostream & o;
	bool pretty;
	std::vector<item> stack;

	JSONReflectorP(std::ostream & o, bool pretty): o(o), pretty(pretty) {
		this->pretty = true;
	}
	
	void next(bool name) {
		if (!name && !stack.back().array) return;
		
		if (stack.back().first)
			stack.back().first = false;
		else {
			if (pretty)
				o << ",\n";
			else
				o << ", ";
		}
		if (pretty)
			o << std::string(stack.back().indent, ' ');
	}
	
	void start(bool array) {
		next(false);
		stack.back().first = false;
		o << (array?'[':'{');
		if (pretty) o << '\n';
		stack.push_back({stack.back().indent + 2, true, array, stack.back().indent + 2});
	}
	
	void end(bool array) {
		stack.pop_back();
		if (pretty)
			o << '\n' << std::string(stack.back().indent, ' ');
		o << (array?']':'}');
	}

	void name(const char * name) {
		next(true);
		o << name << ": ";
	}

	template <typename T>
	void value(const T & item) {
		next(false);
		o << item;
	}
	
	void value(const std::string & v) {
		next(false);
		o << '"' << v << '"';
	}

};

JSONReflector::JSONReflector(std::ostream & o, bool pretty) {
	p = new JSONReflectorP(o, pretty);
	p->stack.push_back({0, true, false});
}

JSONReflector::~JSONReflector() {
	delete p;
}

void JSONReflector::begin(const char *) {
	p->start(false);
}

void JSONReflector::end() {
	p->end(false);
}

void JSONReflector::beginArray(size_t) {
	p->start(true);
}

void JSONReflector::endArray() {
	p->end(true);
}

void JSONReflector::name(const char * name) {
	p->name(name);
}

void JSONReflector::writeUint(uint64_t v) {
	p->value(v);
}

void JSONReflector::writeInt(int64_t v) {
	p->value(v);
}

void JSONReflector::writeDouble(double v) {
	p->value(v);
}

void JSONReflector::writeString(const std::string & v) {
	p->value(v);
}

} //namespace tpie
