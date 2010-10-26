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

namespace tpie {

void progress_indicator_subindicator::set_description(const std::string& description) {
    if (m_parent) 
		m_parent->set_description(description);
}

std::string progress_indicator_subindicator::get_description() {
	if (m_parent)
		return m_parent->get_description();
	else 
		return "";
}

progress_indicator_subindicator::progress_indicator_subindicator(
	progress_indicator_base * parent, TPIE_OS_OFFSET range,	TPIE_OS_OFFSET minRange,
	TPIE_OS_OFFSET maxRange, TPIE_OS_OFFSET stepValue):
	progress_indicator_base("", "", minRange, maxRange, stepValue), 
	m_parent(parent), m_range(range), m_oldValue(0), m_dpl(0) {
}

void progress_indicator_subindicator::refresh() {
	TPIE_OS_OFFSET val = get_current();
	if (val > get_max_range()) val = get_max_range();
	TPIE_OS_OFFSET value= (val - get_min_range() )* m_range / (get_max_range() - get_min_range());
	if (value > m_oldValue && m_parent) {
		m_parent->step(value - m_oldValue);
		m_oldValue = value;
	}
}

void progress_indicator_subindicator::set_description_part(const std::string& text) {
	if (!m_parent) return;
	std::string desc = m_parent->get_description();
	if (desc.size() >= m_dpl) desc.resize(desc.size() - m_dpl);
	if (text != "") {
		desc += " > "  + text;
		m_dpl = text.size() + 3;
	} else 
		m_dpl = 3;
	m_parent->set_description(desc);
}

void progress_indicator_subindicator::init(const std::string& text) {
	set_description_part(text);
	progress_indicator_base::init("");
}

void progress_indicator_subindicator::done(const std::string&) {
	set_description_part("");
	m_current = m_maxRange; 
	refresh();
	progress_indicator_base::done();
}

} //namespace tpie
