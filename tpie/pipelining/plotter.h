// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
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

#ifndef __TPIE_PIPELINING_PLOTTER_H__
#define __TPIE_PIPELINING_PLOTTER_H__

#include <tpie/pipelining/core.h>
#include <boost/unordered_map.hpp>
#include <iostream>
#include <boost/unordered_set.hpp>

namespace tpie {

namespace pipelining {

namespace plotter {
	struct name {
		typedef segment_map S;
		inline name(S::ptr segmap, S::id_t id) : segmap(segmap), id(id) {}
		S::ptr segmap;
		S::id_t id;
	};
	inline std::ostream & operator<<(std::ostream & out, const name & n) {
		segment_map::val_t p = n.segmap->get(n.id);
		std::string name = p->get_name();
		if (name.size())
			return out << name << " (" << n.id << ')';
		else
			return out << typeid(*p).name() << " (" << n.id << ')';
	}
}

typedef boost::unordered_map<const pipe_segment *, size_t> nodes_t;

inline void pipeline_base::plot(std::ostream & out) {
	out << "digraph {\n";
	segment_map::ptr segmap = m_segmap->find_authority();
	for (segment_map::mapit i = segmap->begin(); i != segmap->end(); ++i) {
		out << '"' << plotter::name(segmap, i->first) << "\";\n";
	}
	const segment_map::relmap_t & relations = segmap->get_relations();
	for (segment_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
		switch (i->second.second) {
			case pushes:
				out << '"' << plotter::name(segmap, i->first) << "\" -> \"" << plotter::name(segmap, i->second.first) << "\";\n";
				break;
			case pulls:
				out << '"' << plotter::name(segmap, i->second.first) << "\" -> \"" << plotter::name(segmap, i->first) << "\" [arrowhead=none,arrowtail=normal,dir=both];\n";
				break;
			case depends:
				out << '"' << plotter::name(segmap, i->second.first) << "\" -> \"" << plotter::name(segmap, i->first) << "\" [arrowhead=none,arrowtail=normal,dir=both,style=dashed];\n";
				break;
		}
	}
	out << '}' << std::endl;
}

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_PLOTTER_H__
