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

#include <tpie/pipelining.h>
#include <boost/random.hpp>
#include <tpie/file_stream.h>
#include <iostream>

using namespace tpie;
using namespace tpie::pipelining;

/* This file should solve the following problem:
 * Given a graph consisting of (nodeid, parentid), find the number of children.
 * We solve this by sorting the input twice. Once by id, once by parent.
 * We then scan through both sorted streams at the same time, annotating each
 * (nodeid, parentid) with the number of nodes whose parentid is nodeid.
 */

struct node {
	size_t id;
	size_t parent;
};

struct sort_by_id {
	inline operator()(const node & lhs, const node & rhs) {
		return lhs.id < rhs.id;
	}
};

struct sort_by_parent {
	inline operator()(const node & lhs, const node & rhs) {
		return lhs.parent < rhs.parent;
	}
};

struct node_output {
	size_t id;
	size_t parent;
	size_t children;
};

struct input_nodes_t : public pipe_segment {
	typedef node item_type;

	inline input_nodes_t(const dest_t & dest, size_t nodes)
		: dest(dest)
		, nodes(nodes)
	{
	}

	inline operator()() {
		dest.begin();
		for (size_t i = 0; i < nodes; ++i) {
		}
		dest.end();
	}

private:
	dest_t dest;
	size_t nodes;
};

inline pipe_begin<factory_1<input_nodes_t, size_t> >
input_nodes(size_t nodes) {
	return nodes;
}

template <typename byid_t, typename byparent_t>
struct count_t {
	template <typename dest_t>
	struct type : public pipe_segment {
		type(const dest_t & dest, byid_t & byid, byparent_t & byparent) {
		}
	};
};

template <typename byid, typename byparent>
inline pipe_begin<factory_2<typename count_t<byid_t, byparent_t>::type, byid_t &, byparent_t &> >
count(byid_t & byid, byparent_t & byparent) {
	return factory_2<typename count_t<byid_t, byparent_t>::type, byid_t &, byparent_t &>(byid, byparent);
}

template <typename dest_t>
struct output_count_t : public pipe_segment {
	inline void begin() {
		std::cout << "Begin output" << std::endl;
	}
	inline void end() {
		std::cout << "End output" << std::endl;
	}

	inline void push(const node_output & node) {
		std::cout << node.id << ", " << node.parent << ", " << node.children << std::endl;
	}
};

inline pipe_end<termfactory_0<output_count_t> >
output_count() {
	return termfactory_0<output_count_t>();
}

template <typename pred_t>
struct passive_sorter {
	passive_sorter(const pred_t & pred)
		: pred(pred)
	{
	}

	struct input_t {
		input_t()
		{
		}
	};

	template <typename dest_t>
	struct output_t {
		output_t(const dest_t & dest)
			: dest(dest)
		{
		}

	private:
		dest_t dest;
	};

private:
	pred_t pred;
};

int main() {
	size_t nodes = 1 << 20;
	passive_sorter<sort_by_id> byid(sort_by_id());
	passive_sorter<sort_by_parent> byparent(sort_by_parent());
	pipeline p1 = input_nodes(nodes) | fork(byid.input()) | byparent.input();
	pipeline p2 = count(byid.output(), byparent.output()) | output_count();
}
