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
#include <tpie/disjoint_sets.h>

namespace tpie {

namespace pipelining {

typedef boost::unordered_map<const pipe_segment *, size_t> nodes_t;

nodes_t nodes(const pipe_segment & r) {
	nodes_t numbers;
	size_t next_number = 0;
	const pipe_segment * c = &r;
	while (c != 0) {
		if (!numbers.count(c)) {
			//out << '"' << next_number << "\";\n";
			numbers.insert(std::make_pair(c, next_number));
			++next_number;
		}
		c = c->get_next();
	}
	return numbers;
}

tpie::disjoint_sets<size_t> phases(const nodes_t & n) {
	tpie::disjoint_sets<size_t> res(n.size());
	for (nodes_t::const_iterator i = n.begin(); i != n.end(); ++i) {
		res.make_set(i->second);
	}
	for (nodes_t::const_iterator i = n.begin(); i != n.end(); ++i) {
		const pipe_segment * next = i->first->get_next();
		if (next && !i->first->buffering())
			res.union_set(i->second, n.find(next)->second);
	}
	return res;
}

template <typename fact_t>
void pipeline_impl<fact_t>::actual_plot(std::ostream & out) {
	out << "digraph {\nrankdir=LR;\n";
	nodes_t n = nodes(r);
	for (nodes_t::iterator i = n.begin(); i != n.end(); ++i) {
		out << '"' << i->first << "\";\n";
	}
	{
		const pipe_segment * c = &r;
		while (c != 0) {
			//size_t number = n.find(c)->second;
			const pipe_segment * next = c->get_next();
			if (next) {
				//size_t next_number = n.find(next)->second;
				out << '"' << c << "\" -> \"" << next;
				if (c->buffering())
					out << "\" [style=dashed];\n";
				else
					out << "\";\n";
			}
			c = next;
		}
	}
	out << '}' << std::endl;
}

template <typename fact_t>
void pipeline_impl<fact_t>::actual_plot_phases(std::ostream & out) {
	nodes_t n = nodes(r);
	tpie::disjoint_sets<size_t> p = phases(n);
	out << "digraph {\n";
	for (nodes_t::iterator i = n.begin(); i != n.end(); ++i) {
		out << '"' << i->second << "\";\n";
	}
	for (nodes_t::iterator i = n.begin(); i != n.end(); ++i) {
		size_t cur = i->second;
		size_t rep = p.find_set(cur);
		if (rep != cur) {
			out << '"' << cur << "\" -> \"" << rep << "\";\n";
		}
	}
	out << '}' << std::endl;
}


} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_PLOTTER_H__
