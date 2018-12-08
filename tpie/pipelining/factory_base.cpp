// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, 2013, The TPIE development team
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

#include <tpie/pipelining/factory_base.h>

namespace tpie::pipelining {

void factory_base::hook_initialization(factory_init_hook * hook) {
	m_hooks.push_back(hook);
}

void factory_base::copy_hooks_to(factory_base & other) const {
	for (const auto & hook: m_hooks) {
		other.m_hooks.push_back(hook);
	}
}


void factory_base::init_node(node & r) {
	if (!m_name.empty()) {
		r.set_name(m_name, m_namePriority);
	}
	if (!m_phaseName.empty()) {
		r.set_phase_name(m_phaseName, m_phaseNamePriority);
	}
	if (!m_breadcrumbs.empty()) {
		r.set_breadcrumb(m_breadcrumbs);
	}
	init_common(r);
}

void factory_base::init_sub_node(node & r) {
	if (m_breadcrumbs.empty()) {
		if (m_name.empty()) {
			// no op
		} else {
			r.set_breadcrumb(m_name);
		}
	} else {
		if (m_name.empty()) {
			r.set_breadcrumb(m_breadcrumbs);
		} else {
			r.set_breadcrumb(m_breadcrumbs + " | " + m_name);
		}
	}
	init_common(r);
}


void factory_base::add_default_edge(node & r, const node_token & dest) const {
	if (r.get_node_map()->find_authority()->out_degree(r.get_id()) > 0) return;
	switch (m_destinationKind) {
	case destination_kind::none:
		break;
	case destination_kind::push:
		r.add_push_destination(dest);
		break;
	case destination_kind::pull:
		r.add_pull_source(dest);
		break;
	}
}

void factory_base::add_node_set_edges(node & r) const {
	bits::node_map::ptr m=r.get_node_map();
	for (size_t i=0; i < m_add_to_set.size(); ++i) {
		bits::node_map::ptr m2=m_add_to_set[i]->m_map;
		if (m2 && m2 != m)
			m2->union_set(m);
	}
	for (size_t i=0; i < m_add_relations.size(); ++i) {
		bits::node_map::ptr m2=m_add_relations[i].first->m_map;
		if (m2 && m2 != m)
			m2->union_set(m);
	}
	m = m->find_authority();
	for (size_t i=0; i < m_add_to_set.size(); ++i)
		m_add_to_set[i]->m_map = m;
	for (size_t i=0; i < m_add_relations.size(); ++i)
		m_add_relations[i].first->m_map = m;
	
	for (size_t i=0; i < m_add_to_set.size(); ++i) {
		node_set s=m_add_to_set[i];
		for (size_t j=0; j < s->m_relations.size(); ++j)
			m->add_relation(s->m_relations[j].first, r.get_id(), s->m_relations[j].second);
		s->m_nodes.push_back(r.get_id());
	}
	
	for (size_t i=0; i < m_add_relations.size(); ++i) {
		node_set s=m_add_relations[i].first;
		bits::node_relation relation = m_add_relations[i].second;
		for (size_t j=0; j < s->m_nodes.size(); ++j)
			m->add_relation(r.get_id(), s->m_nodes[j], relation);
		s->m_relations.push_back(std::make_pair(r.get_id(), relation));
	}
}

void factory_base::add_to_set(node_set s) {
	if (s)
		m_add_to_set.push_back(s);
}

void factory_base::add_dependencies(node_set s) {
	if (s)
		m_add_relations.push_back(std::make_pair(s,bits::no_forward_depends));
}

void factory_base::add_forwarding_dependencies(node_set s) {
	if (s)
		m_add_relations.push_back(std::make_pair(s,bits::depends));
}

void factory_base::forward(const std::string & key, any_noncopyable value) {
	m_forwards.push_back({key, std::move(value)});
}


void factory_base::init_common(node & r) {
	if (m_set) r.set_memory_fraction(memory());
	
	for (size_t i = 0; i < m_hooks.size(); ++i) {
		m_hooks[i]->init_node(r);
	}
	
	auto nodeMap = r.get_node_map()->find_authority();

	for (auto &p : m_forwards) {
		nodeMap->forward_from_pipe_base(r.get_id(), p.first, std::move(p.second));
	}
}

}
