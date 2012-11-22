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

///////////////////////////////////////////////////////////////////////////////
/// \file tokens.h  Pipeline segment tokens.
///
/// \section sec_pipegraphs  The two pipeline graphs
///
/// A pipeline consists of several segments. Each segment either produces,
/// transforms or consumes items. One segment may push items to another
/// segment, and it may pull items from another segment, and it may depend
/// implicitly on the execution of another segment. For instance, to reverse an
/// item stream using two segments, one segment will write items to a
/// file_stream, and the other will read them in backwards. Thus, the second
/// segment depends on the first, but it does not directly push to or pull from
/// it.
///
/// To a pipeline we associate two different graphs. In both graphs, each
/// segment is a node and each relationship is a directed edge.
///
/// The <i>item flow graph</i> is a directed acyclic graph, and edges go from
/// producer towards consumer, regardless of push/pull kind.
///
/// The <i>actor graph</i> is a directed graph where edges go from actors, so a
/// node has an edge to another node if the corresponding segment either pushes
/// to or pulls from the other corresponding segment.
///
/// The item flow graph is useful for transitive dependency resolution and
/// execution order decision. The actor graph is useful for presenting the
/// pipeline flow to the user graphically.
///
/// \subsection sub_pipegraphimpl  Implementation
///
/// Since pipe_segments are copyable, we cannot store pipe_segment pointers
/// limitlessly, as pointers will change while the pipeline is being
/// constructed. Instead, we associate to each pipe_segment a segment token
/// (numeric id) that is copied with the pipe_segment. The segment_token class
/// signals the mapping from numeric ids to pipe_segment pointers to a
/// segment_map.
///
/// However, we do not want a global map from ids to pipe_segment pointers, as
/// an application may construct many pipelines throughout its lifetime. To
/// mitigate this problem, each segment_map is local to a pipeline, and each
/// segment_token knows (directly or indirectly) which segment_map currently
/// holds the mapping of its id to its pipe_segment.
///
/// When we need to connect one pipe segment to another in the pipeline graphs,
/// we need the two corresponding segment_tokens to share the same segment_map.
/// When we merge two segment_maps, the mappings in one are copied to the
/// other, and one segment_map remembers that it has been usurped by another
/// segment_map. This corresponds to the set representative in a union-find
/// data structure, and we implement union-find merge by rank. We use Boost
/// smart pointers to deallocate segment_maps when they are no longer needed.
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_PIPELINING_TOKENS_H__
#define __TPIE_PIPELINING_TOKENS_H__

#include <tpie/exception.h>
#include <tpie/pipelining/exception.h>
#include <tpie/types.h>
#include <tpie/tpie_assert.h>
#include <map>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace tpie {

namespace pipelining {

struct pipe_segment;

struct segment_token;

namespace bits {

enum segment_relation {
	pushes,
	pulls,
	depends
};

struct segment_map {
	typedef uint64_t id_t;
	typedef pipe_segment * val_t;

	typedef std::map<id_t, val_t> map_t;
	typedef map_t::const_iterator mapit;

	typedef std::multimap<id_t, std::pair<id_t, segment_relation> > relmap_t;
	typedef relmap_t::const_iterator relmapit;

	typedef boost::shared_ptr<segment_map> ptr;
	typedef boost::weak_ptr<segment_map> wptr;

	static inline ptr create() {
		ptr result(new segment_map);
		result->self = wptr(result);
		return result;
	}

	inline id_t add_token(val_t token) {
		id_t id = nextId++;
		set_token(id, token);
		return id;
	}

	inline void set_token(id_t id, val_t token) {
		assert_authoritative();
		m_tokens[id] = token;
	}

	// union-find link
	void link(ptr target);

	inline void union_set(ptr target) {
		find_authority()->link(target->find_authority());
	}

	inline val_t get(id_t id) const {
		mapit i = m_tokens.find(id);
		if (i == m_tokens.end()) return 0;
		return i->second;
	}

	inline mapit begin() const {
		return m_tokens.begin();
	}

	inline mapit end() const {
		return m_tokens.end();
	}

	// union-find
	ptr find_authority();

	inline void add_relation(id_t from, id_t to, segment_relation rel) {
		m_relations.insert(std::make_pair(from, std::make_pair(to, rel)));
		m_relationsInv.insert(std::make_pair(to, std::make_pair(from, rel)));
	}

	inline const relmap_t & get_relations() const {
		return m_relations;
	}

	inline size_t in_degree(id_t from, segment_relation rel) const {
		return out_degree(m_relationsInv, from, rel);
	}

	inline size_t out_degree(id_t from, segment_relation rel) const {
		return out_degree(m_relations, from, rel);
	}

	void assert_authoritative() const {
		if (m_authority) throw non_authoritative_segment_map();
	}

	void dump(std::ostream & os = std::cout) const;

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Internal method called by graph_traits. Iterates through the
	/// edge lists and calls add_successor on each pipe_segment in the phase.
	///////////////////////////////////////////////////////////////////////////
	void send_successors() const;

private:
	map_t m_tokens;
	relmap_t m_relations;
	relmap_t m_relationsInv;

	size_t out_degree(const relmap_t & map, id_t from, segment_relation rel) const;

	wptr self;

	// union rank structure
	ptr m_authority;
	size_t m_rank;

	inline segment_map()
		: m_rank(0)
	{
	}

	inline segment_map(const segment_map &);
	inline segment_map & operator=(const segment_map &);

	static id_t nextId;
};

} // namespace bits

struct segment_token {
	typedef bits::segment_map::id_t id_t;
	typedef bits::segment_map::val_t val_t;

	// Use for the simple case in which a pipe_segment owns its own token
	inline segment_token(val_t owner)
		: m_tokens(bits::segment_map::create())
		, m_id(m_tokens->add_token(owner))
		, m_free(false)
	{
	}

	// This copy constructor has two uses:
	// 1. Simple case when a pipe_segment is copied (freshToken = false)
	// 2. Advanced case when a pipe_segment is being constructed with a specific token (freshToken = true)
	inline segment_token(const segment_token & other, val_t newOwner, bool freshToken = false)
		: m_tokens(other.m_tokens->find_authority())
		, m_id(other.id())
		, m_free(false)
	{
		if (freshToken) {
			tp_assert(other.m_free, "Trying to take ownership of a non-free token");
			tp_assert(m_tokens->get(m_id) == 0, "A token already has an owner, but m_free is true - contradiction");
		} else {
			tp_assert(!other.m_free, "Trying to copy a free token");
		}
		m_tokens->set_token(m_id, newOwner);
	}

	// Use for the advanced case when a segment_token is allocated before the pipe_segment
	inline segment_token()
		: m_tokens(bits::segment_map::create())
		, m_id(m_tokens->add_token(0))
		, m_free(true)
	{
	}

	inline size_t id() const { return m_id; }

	inline bits::segment_map::ptr map_union(const segment_token & with) {
		if (m_tokens == with.m_tokens) return m_tokens;
		m_tokens->union_set(with.m_tokens);
		return m_tokens = m_tokens->find_authority();
	}

	inline bits::segment_map::ptr get_map() const {
		return m_tokens;
	}

	inline val_t get() const {
		return m_tokens->get(m_id);
	}

private:
	bits::segment_map::ptr m_tokens;
	size_t m_id;
	bool m_free;
};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_TOKENS_H__
