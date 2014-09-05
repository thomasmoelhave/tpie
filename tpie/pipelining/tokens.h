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
/// \file tokens.h  Pipeline tokens.
///
/// \section sec_pipegraphs  The two pipeline graphs
///
/// A pipeline consists of several nodes. Each node either produces,
/// transforms or consumes items. One node may push items to another
/// node, and it may pull items from another node, and it may depend
/// implicitly on the execution of another node. For instance, to reverse an
/// item stream using two nodes, one node will write items to a
/// file_stream, and the other will read them in backwards. Thus, the second
/// node depends on the first, but it does not directly push to or pull from
/// it.
///
/// To a pipeline graph we associate two different directed edge sets.
///
/// The <i>item flow graph</i> is a directed acyclic graph, and edges go from
/// producer towards consumer, regardless of push/pull kind.
///
/// The <i>actor graph</i> is a directed graph where edges go from actors, so a
/// node has an edge to another node if the corresponding node either pushes
/// to or pulls from the other corresponding node.
///
/// The item flow graph is useful for transitive dependency resolution and
/// execution order decision. The actor graph is useful for presenting the
/// pipeline flow to the user graphically.
///
/// \subsection sub_pipegraphimpl  Implementation
///
/// Since nodes are copyable, we cannot store node pointers
/// limitlessly, as pointers will change while the pipeline is being
/// constructed. Instead, we associate to each node a token
/// (numeric id) that is copied with the node. The node_token class
/// signals the mapping from numeric ids to node pointers to a
/// node_map.
///
/// However, we do not want a global map from ids to node pointers, as
/// an application may construct many pipelines throughout its lifetime. To
/// mitigate this problem, each node_map is local to a pipeline, and each
/// node_token knows (directly or indirectly) which node_map currently
/// holds the mapping of its id to its node.
///
/// When we need to connect one node to another in the pipeline graphs,
/// we need the two corresponding node_tokens to share the same node_map.
/// When we merge two node_maps, the mappings in one are copied to the
/// other, and one node_map remembers that it has been usurped by another
/// node_map. This corresponds to the set representative in a union-find
/// data structure, and we implement union-find merge by rank. We use Boost
/// smart pointers to deallocate node_maps when they are no longer needed.
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_PIPELINING_TOKENS_H__
#define __TPIE_PIPELINING_TOKENS_H__

#include <tpie/exception.h>
#include <tpie/pipelining/exception.h>
#include <tpie/pipelining/predeclare.h>
#include <tpie/types.h>
#include <tpie/tpie_assert.h>
#include <map>
#include <vector>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/any.hpp>

namespace tpie {

namespace pipelining {

namespace bits {

enum node_relation {
	pushes,
	pulls,
	depends
};

class node_map {
public:
	typedef uint64_t id_t;
	typedef node * val_t;

	typedef std::map<id_t, val_t> map_t;
	typedef map_t::const_iterator mapit;

	typedef std::multimap<id_t, std::pair<id_t, node_relation> > relmap_t;
	typedef relmap_t::const_iterator relmapit;

	typedef std::map<std::string, std::pair<memory_size_type, boost::any> > datastructuremap_t;

	typedef boost::shared_ptr<node_map> ptr;
	typedef boost::weak_ptr<node_map> wptr;

	static inline ptr create() {
		ptr result(new node_map);
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

	size_t size() const {
		return m_tokens.size();
	}

	// union-find
	ptr find_authority();

	void add_relation(id_t from, id_t to, node_relation rel);

	inline const relmap_t & get_relations() const {
		return m_relations;
	}

	const datastructuremap_t & get_datastructures() const {
		return m_datastructures;
	}

	datastructuremap_t & get_datastructures() {
		return m_datastructures;
	}

	inline size_t in_degree(id_t from, node_relation rel) const {
		return out_degree(m_relationsInv, from, rel);
	}

	inline size_t out_degree(id_t from, node_relation rel) const {
		return out_degree(m_relations, from, rel);
	}

	size_t out_degree(id_t from) const {
		return out_degree(m_relations, from);
	}

	void assert_authoritative() const {
		if (m_authority) throw non_authoritative_node_map();
	}

	void dump(std::ostream & os = std::cout) const;

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compute the transitive closure of the given node in the item
	/// flow graph.
	///////////////////////////////////////////////////////////////////////////
	void get_successors(id_t from, std::vector<id_t> & successors);

private:
	map_t m_tokens;
	relmap_t m_relations;
	relmap_t m_relationsInv;
	datastructuremap_t m_datastructures;

	size_t out_degree(const relmap_t & map, id_t from, node_relation rel) const;
	size_t out_degree(const relmap_t & map, id_t from) const;

	wptr self;

	// union rank structure
	ptr m_authority;
	size_t m_rank;

	inline node_map()
		: m_rank(0)
	{
	}

	inline node_map(const node_map &);
	inline node_map & operator=(const node_map &);

	static id_t nextId;
};

} // namespace bits

class node_token {
public:
	typedef bits::node_map::id_t id_t;
	typedef bits::node_map::val_t val_t;

	// Use for the simple case in which a node owns its own token
	inline node_token(val_t owner)
		: m_tokens(bits::node_map::create())
		, m_id(m_tokens->add_token(owner))
		, m_free(false)
	{
	}

	// This copy constructor has two uses:
	// 1. Simple case when a node is copied (freshToken = false)
	// 2. Advanced case when a node is being constructed with a specific token (freshToken = true)
	inline node_token(const node_token & other, val_t newOwner, bool freshToken = false)
		: m_tokens(other.m_tokens->find_authority())
		, m_id(other.id())
		, m_free(false)
	{
		if (freshToken) {
			if (!other.m_free)
				throw exception("Trying to take ownership of a non-free token");
			if (m_tokens->get(m_id) != 0)
				throw exception("A token already has an owner, but m_free is true - contradiction");
		} else {
			if (other.m_free)
				throw exception("Trying to copy a free token");
		}
		m_tokens->set_token(m_id, newOwner);
	}

	// Use for the advanced case when a node_token is allocated before the node
	inline node_token()
		: m_tokens(bits::node_map::create())
		, m_id(m_tokens->add_token(0))
		, m_free(true)
	{
	}

	inline id_t id() const { return m_id; }

	inline bits::node_map::ptr map_union(const node_token & with) {
		if (m_tokens == with.m_tokens) return m_tokens;
		m_tokens->union_set(with.m_tokens);
		return m_tokens = m_tokens->find_authority();
	}

	inline bits::node_map::ptr get_map() const {
		return m_tokens;
	}

	inline val_t get() const {
		return m_tokens->get(m_id);
	}

private:
	bits::node_map::ptr m_tokens;
	id_t m_id;
	bool m_free;
};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_TOKENS_H__
