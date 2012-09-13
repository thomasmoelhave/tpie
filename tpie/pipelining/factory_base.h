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

namespace tpie {

namespace pipelining {

struct factory_base {
	factory_base() : m_amount(0), m_set(false) {
	}

	inline void memory(double amount) {
		m_amount = amount;
		m_set = true;
	}

	inline double memory() const {
		return m_amount;
	}

	inline void init_segment(pipe_segment & r) const {
		if (m_set) r.set_memory_fraction(memory());
		if (!m_name.empty()) {
			r.set_name(m_name, m_namePriority);
		}
		if (!m_breadcrumbs.empty()) {
			r.set_breadcrumb(m_breadcrumbs);
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
};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_FACTORY_BASE_H__
