// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2012, The TPIE development team
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

#include <tpie/pipelining/pipeline.h>
#include <tpie/pipelining/node.h>
#include <tpie/pipelining/graph.h>
#include <boost/unordered_map.hpp>
#include <iostream>
#include <boost/unordered_set.hpp>

namespace {
	typedef tpie::pipelining::bits::node_map S;

	class name {
	public:
		inline name(S::ptr segmap, S::id_t id) : segmap(segmap), id(id) {}
		S::ptr segmap;
		S::id_t id;
	};

	inline std::ostream & operator<<(std::ostream & out, const name & n) {
		S::val_t p = n.segmap->get(n.id);
		std::string name = p->get_name();
		if (name.size())
			return out << name << " (" << n.id << ')';
		else
			return out << typeid(*p).name() << " (" << n.id << ')';
	}
} // default namespace

namespace tpie {

namespace pipelining {

namespace bits {

typedef boost::unordered_map<const node *, size_t> nodes_t;

void pipeline_base::plot(std::ostream & out) {
	out << "digraph {\n";
	node_map::ptr segmap = m_segmap->find_authority();
	for (node_map::mapit i = segmap->begin(); i != segmap->end(); ++i) {
		out << '"' << name(segmap, i->first) << "\";\n";
	}
	const node_map::relmap_t & relations = segmap->get_relations();
	for (node_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
		switch (i->second.second) {
			case pushes:
				out << '"' << name(segmap, i->first) << "\" -> \"" << name(segmap, i->second.first) << "\";\n";
				break;
			case pulls:
				out << '"' << name(segmap, i->second.first) << "\" -> \"" << name(segmap, i->first) << "\" [arrowhead=none,arrowtail=normal,dir=both];\n";
				break;
			case depends:
				out << '"' << name(segmap, i->second.first) << "\" -> \"" << name(segmap, i->first) << "\" [arrowhead=none,arrowtail=normal,dir=both,style=dashed];\n";
				break;
		}
	}
	out << '}' << std::endl;
}

void pipeline_base::operator()(stream_size_type items, progress_indicator_base & pi, const memory_size_type initialMemory) {
	typedef std::vector<phase> phases_t;
	typedef phases_t::const_iterator it;

	node_map::ptr map = m_segmap->find_authority();
	graph_traits g(*map);
	const phases_t & phases = g.phases();
	if (initialMemory == 0) log_warning() << "No memory for pipelining" << std::endl;

	memory_size_type mem = initialMemory;
	mem -= graph_traits::memory_usage(phases.size());

	if (mem > initialMemory) { // overflow
		log_warning() << "Not enough memory for pipelining framework overhead" << std::endl;
		mem = 0;
	}

	for (it i = phases.begin(); i != phases.end(); ++i) {
		i->assign_memory(mem);
	}
	g.go_all(items, pi);
}

} // namespace bits

void pipeline::output_memory(std::ostream & o) const {
	bits::node_map::ptr segmap = p->get_node_map()->find_authority();
	for (bits::node_map::mapit i = segmap->begin(); i != segmap->end(); ++i) {
		bits::node_map::val_t p = segmap->get(i->first);
		o << p->get_name() << ": min=" << p->get_minimum_memory() << "; max=" << p->get_available_memory() << "; prio=" << p->get_memory_fraction() << ";" << std::endl;

	}
}


} // namespace pipelining

} // namespace tpie
