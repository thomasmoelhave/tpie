// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, The TPIE development team
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

#ifndef __TPIE_PIPELINING_FACTORY_BASE_H__
#define __TPIE_PIPELINING_FACTORY_BASE_H__

// XXX remove when init_segment is removed
#include <tpie/backtrace.h>

namespace tpie {

namespace pipelining {

class factory_init_hook {
public:
	virtual void init_node(node & r) = 0;
	virtual ~factory_init_hook() {
	}
};

class factory_base {
public:
	factory_base() : m_amount(0), m_set(false) {
	}

	inline void memory(double amount) {
		m_amount = amount;
		m_set = true;
	}

	inline double memory() const {
		return m_amount;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Add a node initialization hook. When a node is
	/// instantiated in construct(), the given hook will get a chance to do
	/// some additional initialization.
	///////////////////////////////////////////////////////////////////////////
	void hook_initialization(factory_init_hook * hook) {
		m_hooks.push_back(hook);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Copy the hooks that have been added to this factory to another.
	///////////////////////////////////////////////////////////////////////////
	void copy_hooks_to(factory_base & other) const {
		for (size_t i = 0; i < m_hooks.size(); ++i) {
			other.m_hooks.push_back(m_hooks[i]);
		}
	}

	inline void init_segment(node & r) const {
		log_fatal() << "init_segment has been renamed to init_node" << std::endl;
		backtrace(log_fatal());
		init_node(r);
	}

	inline void init_node(node & r) const {
		if (m_set) r.set_memory_fraction(memory());
		if (!m_name.empty()) {
			r.set_name(m_name, m_namePriority);
		}
		if (!m_breadcrumbs.empty()) {
			r.set_breadcrumb(m_breadcrumbs);
		}
		for (size_t i = 0; i < m_hooks.size(); ++i) {
			m_hooks[i]->init_node(r);
		}
	}

	void init_sub_node(node & r) const {
		if (m_set) r.set_memory_fraction(memory());
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
		for (size_t i = 0; i < m_hooks.size(); ++i) {
			m_hooks[i]->init_node(r);
		}
	}

	inline void name(const std::string & n, priority_type p) {
		m_name = n;
		m_namePriority = p;
	}

	inline void push_breadcrumb(const std::string & n) {
		if (m_breadcrumbs.empty()) m_breadcrumbs = n;
		else m_breadcrumbs = n + " | " + m_breadcrumbs;
	}

private:
	double m_amount;
	bool m_set;
	std::string m_name;
	std::string m_breadcrumbs;
	priority_type m_namePriority;
	std::vector<factory_init_hook *> m_hooks;
};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_FACTORY_BASE_H__
