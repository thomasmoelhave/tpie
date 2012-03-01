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

using namespace tpie;
using namespace tpie::pipelining;

/* This file should solve the following problem:
 * Given a graph consisting of (nodeid, parentid), find the number of children.
 * We solve this by sorting the input twice. Once by id, once by parent.
 * We then scan through both sorted streams at the same time, annotating each
 * (nodeid, parentid) with the number of nodes whose parentid is nodeid.
 */

struct node {
	node(size_t id, size_t parent) : id(id), parent(parent) {
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

	const pipe_segment * get_next() const {
		return &dest;
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
		}

		const pipe_segment * get_next() const {
			return &dest;
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
	inline void begin() {
		std::cout << "Begin output" << std::endl;
	}
	inline void end() {
		std::cout << "End output" << std::endl;
	}

	inline void push(const node_output & node) {
		std::cout << node.id << ", " << node.parent << ", " << node.children << std::endl;
	}

	const pipe_segment * get_next() const {
		return 0;
	}
};

inline pipe_end<termfactory_0<output_count_t> >
output_count() {
	return termfactory_0<output_count_t>();
}

template <typename pred_t>
struct passive_sorter {
	passive_sorter() {
		std::cout << "temp_file in passive_sorter at " << &file << std::endl;
	}

	struct input_t : public pipe_segment {
		typedef node item_type;

		inline input_t(temp_file * file)
			: file(file)
		{
			std::cout << "temp_file in input_t at " << file << std::endl;
		}

		inline input_t(const input_t & other)
			: file(other.file)
			, pbuffer(other.pbuffer)
		{
			std::cout << "temp_file in copied input_t at " << file << std::endl;
		}

		inline void begin() {
			pbuffer = tpie_new<file_stream<node> >();
			pbuffer->open(file->path());
		}

		inline void push(const node & item) {
			pbuffer->write(item);
		}

		inline void end() {
			pbuffer->close();
			tpie_delete(pbuffer);
		}

		const pipe_segment * get_next() const {
			return 0;
		}
	private:
		temp_file * file;
		file_stream<node> * pbuffer;

		input_t();
		input_t & operator=(const input_t &);
	};

	struct output_t {
		typedef node item_type;

		inline output_t(temp_file * file)
			: file(file)
		{
			std::cout << "temp_file in output_t at " << file << std::endl;
		}

		inline output_t(const output_t & other)
			: file(other.file)
		{
			std::cout << "temp_file in copied output_t at " << file << std::endl;
		}

		inline void begin() {
			buffer = tpie_new<file_stream<node> >();
			buffer->open(file->path());
		}

		inline bool can_pull() {
			return buffer->can_read();
		}

		inline node pull() {
			return buffer->read();
		}

		inline void end() {
			buffer->close();
		}

	private:
		temp_file * file;
		file_stream<node> * buffer;

		output_t();
		output_t & operator=(const output_t &);
	};

	inline pipe_end<termfactory_1<input_t, temp_file *> > input() {
		std::cout << "Construct input factory " << typeid(pred_t).name() << " with " << &file << std::endl;
		return termfactory_1<input_t, temp_file *>(&file);
	}

	inline output_t output() {
		return output_t(&file);
	}

private:
	pred_t pred;
	temp_file file;
	passive_sorter(const passive_sorter &);
	passive_sorter & operator=(const passive_sorter &);
};

int main() {
	tpie_init(ALL & ~JOB_MANAGER);
	size_t nodes = 1 << 20;
	std::cout << "Instantiate passive 1" << std::endl;
	passive_sorter<sort_by_id> byid;
	std::cout << "Instantiate passive 2" << std::endl;
	passive_sorter<sort_by_parent> byparent;
	std::cout << "Instantiate pipe 1" << std::endl;
	pipeline p1 = input_nodes(nodes) | fork(byid.input()) | byparent.input();
	std::cout << "Instantiate pipe 2" << std::endl;
	pipeline p2 = count(byid.output(), byparent.output()) | output_count();
	std::cout << "Run pipe 1" << std::endl;
	p1();
	std::cout << "Run pipe 2" << std::endl;
	p2();
	tpie_finish(ALL & ~JOB_MANAGER);
	return 0;
}
