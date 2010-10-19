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

#include <tpie/portability.h>
#include <tpie/util.h>
#include <tpie/progress_indicator_base.h>

namespace tpie {
  
class progress_indicator_subindicator : public progress_indicator_base {
private:
	progress_indicator_base * m_parent;
	TPIE_OS_OFFSET m_range;
	TPIE_OS_OFFSET m_oldValue;
public:
	progress_indicator_subindicator(TPIE_OS_OFFSET minRange, 
									TPIE_OS_OFFSET maxRange, 
									TPIE_OS_OFFSET stepValue,
									progress_indicator_base * parent, 
									TPIE_OS_OFFSET range) : 
	    progress_indicator_base("", "", minRange, maxRange, stepValue), 
		m_parent(parent), m_range(range), m_oldValue(-1) {};
	
	void refresh() {
		TPIE_OS_OFFSET value= get_current() * m_range / (get_max_range() - get_min_range());
		if (value > m_oldValue && m_parent) {
			m_parent->step(value - m_oldValue);
			m_oldValue = value;
		}
	}

	void set_title(const std::string&) {}
	void set_description(const std::string&) {}

	void done(const std::string& text = std::string()) {
		unused(text);
		m_current = m_maxRange; 
		refresh();
	}
};
	
	
}; //namespace tpie
