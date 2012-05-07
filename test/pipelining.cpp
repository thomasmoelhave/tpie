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

#include <tpie/tpie.h>
#include <tpie/pipelining.h>
#include <boost/random.hpp>
#include <tpie/file_stream.h>
#include <iostream>
#include <sstream>

using namespace tpie;
using namespace tpie::pipelining;

/* This file should solve the following problem:
 * Given a graph consisting of (nodeid, parentid), find the number of children.
 * We solve this by sorting the input twice. Once by id, once by parent.
 * We then scan through both sorted streams at the same time, annotating each
 * (nodeid, parentid) with the number of nodes whose parentid is nodeid.
 */

struct node {
	inline node(size_t id, size_t parent) : id(id), parent(parent) {
	}
	inline node() {
	}
	size_t id;
	size_t parent;
};

struct sort_by_id {
	inline bool operator()(const node & lhs, const node & rhs) {
		return lhs.id < rhs.id;
	}
};

struct sort_by_parent {
	inline bool operator()(const node & lhs, const node & rhs) {
		return lhs.parent < rhs.parent;
	}
};

struct node_output {
	node_output(const node & from) : id(from.id), parent(from.parent), children(0) {
	}
	size_t id;
	size_t parent;
	size_t children;
};

inline std::ostream & operator<<(std::ostream & stream, const node & n) {
	return stream << '(' << n.id << ", " << n.parent << ')';
}

inline std::ostream & operator<<(std::ostream & stream, const node_output & n) {
	return stream << '(' << n.id << ", " << n.parent << ", " << n.children << ')';
}

template <typename dest_t>
struct input_nodes_t : public pipe_segment {
	typedef node item_type;

	inline input_nodes_t(const dest_t & dest, size_t nodes)
		: dest(dest)
		, nodes(nodes)
	{
	}

	inline void operator()() {
		static boost::mt19937 mt;
		static boost::uniform_int<> dist(0, nodes-1);
		dest.begin();
		for (size_t i = 0; i < nodes; ++i) {
			dest.push(node(i, dist(mt)));
		}
		dest.end();
	}

	void push_successors(std::deque<const pipe_segment *> & q) const {
		q.push_back(&dest);
	}

private:
	dest_t dest;
	size_t nodes;
};

inline pipe_begin<factory_1<input_nodes_t, size_t> >
input_nodes(size_t nodes) {
	return factory_1<input_nodes_t, size_t>(nodes);
}

template <typename byid_t, typename byparent_t>
struct count_t {
	template <typename dest_t>
	struct type : public pipe_segment {
		type(const dest_t & dest, const byid_t & byid, const byparent_t & byparent)
			: dest(dest), byid(byid), byparent(byparent)
		{
		}

		inline void operator()() {
			dest.begin();
			byid.begin();
			byparent.begin();
			tpie::auto_ptr<node> buf(0);
			while (byid.can_pull()) {
				node_output cur = byid.pull();
				if (buf.get()) {
					if (buf->parent != cur.id) {
						goto seen_children;
					} else {
						++cur.children;
					}
				}
				while (byparent.can_pull()) {
					node child = byparent.pull();
					if (child.parent != cur.id) {
						if (!buf.get()) {
							buf.reset(tpie_new<node>(child));
						} else {
							*buf = child;
						}
						break;
					} else {
						++cur.children;
					}
				}
seen_children:
				dest.push(cur);
			}
			byparent.end();
			byid.end();
			dest.end();
		}

		void push_successors(std::deque<const pipe_segment *> & q) const {
			q.push_back(&dest);
			q.push_back(&byid);
			q.push_back(&byparent);
		}

		dest_t dest;
		byid_t byid;
		byparent_t byparent;
	};
};

template <typename byid_t, typename byparent_t>
inline pipe_begin<factory_2<count_t<byid_t, byparent_t>::template type, const byid_t &, const byparent_t &> >
count(const byid_t & byid, const byparent_t & byparent) {
	return factory_2<count_t<byid_t, byparent_t>::template type, const byid_t &, const byparent_t &>(byid, byparent);
}

struct output_count_t : public pipe_segment {
	size_t children;
	size_t nodes;

	inline output_count_t()
		: children(0)
		, nodes(0)
	{
	}

	inline void begin() {
		std::cout << "Begin output" << std::endl;
	}

	inline void end() {
		std::cout << "End output" << std::endl;
		std::cout << "We saw " << nodes << " nodes and " << children << " children" << std::endl;
	}

	inline void push(const node_output & node) {
		if (nodes < 32) std::cout << node << std::endl;
		else if (nodes == 32) std::cout << "..." << std::endl;
		children += node.children;
		++nodes;
	}

	void push_successors(std::deque<const pipe_segment *> &) const { }
};

inline pipe_end<termfactory_0<output_count_t> >
output_count() {
	return termfactory_0<output_count_t>();
}

int main(int argc, char ** argv) {
	tpie_init();
	size_t nodes = 1 << 20;
	if (argc > 1) std::stringstream(argv[1]) >> nodes;
	passive_sorter<node, sort_by_id> byid;
	passive_sorter<node, sort_by_parent> byparent;
	pipeline p1 = input_nodes(nodes) | fork(byid.input()) | byparent.input();
	pipeline p2 = count(byid.output(), byparent.output()) | output_count();
	p1.plot();
	p1.plot_phases();
	p2.plot();
	p2.plot_phases();
	p1();
	p2();
	tpie_finish();
	return 0;
}
