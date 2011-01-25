// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
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

#include "progress_indicator_subindicator.h"
#include <tpie/backtrace.h>

namespace tpie {

void progress_indicator_subindicator::push_breadcrumb(const char * crumb) {
	if (m_display_subcrumbs && m_parent) m_parent->push_breadcrumb(crumb);
}

void progress_indicator_subindicator::pop_breadcrumb() {
	if (m_display_subcrumbs && m_parent) m_parent->pop_breadcrumb();
}

progress_indicator_subindicator::progress_indicator_subindicator(progress_indicator_base * parent,
																 TPIE_OS_OFFSET outerRange,
																 const char * crumb,
																 bool display_subcrumbs):
	progress_indicator_base("","", 0, 1,1), m_parent(parent), m_outerRange(outerRange), 
	m_oldValue(0), m_display_subcrumbs(display_subcrumbs)
#ifndef NDEBUG
	,m_init_called(false), m_done_called(false)
#endif
{
	if (crumb == 0) 
		m_crumb[0] = 0;
	else {
		strncpy(m_crumb, crumb, 39);
		m_crumb[39] = 0;
	}
}

#ifndef NDEBUG
progress_indicator_subindicator::~progress_indicator_subindicator() {
	if (m_init_called && !m_done_called && !std::uncaught_exception()) {
		std::cerr << "A progress_indicator_subindicator was destructed without done being called" << std::endl;
		tpie::backtrace(std::cerr, 5);
	}
}
#endif

void progress_indicator_subindicator::refresh() {
	TPIE_OS_OFFSET val = get_current();
	if (val > get_max_range()) val = get_max_range();
	if (get_max_range() == get_min_range()) return;
	TPIE_OS_OFFSET value= (val - get_min_range() )* m_outerRange / (get_max_range() - get_min_range());
	if (value > m_oldValue && m_parent) {
		m_parent->step(value - m_oldValue);
		m_oldValue = value;
	}
}

void progress_indicator_subindicator::init(TPIE_OS_OFFSET range, TPIE_OS_OFFSET step) {
#ifndef NDEBUG
	softassert(!m_init_called);
	m_init_called=true;
#endif
	if (m_crumb[0] && m_parent) m_parent->push_breadcrumb(m_crumb);
	progress_indicator_base::init(range, step);
}

void progress_indicator_subindicator::done() {
#ifndef NDEBUG
	softassert(m_init_called);
	softassert(!m_done_called);
	m_done_called=true;
#endif
	if (m_crumb[0] && m_parent) m_parent->pop_breadcrumb();
	progress_indicator_base::done();
	m_current = m_maxRange; 
	refresh();
}

} //namespace tpie
