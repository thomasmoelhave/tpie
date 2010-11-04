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
#ifndef __TPIE_PROGRESS_INDICATOR_SUBINDICATOR_H__
#define __TPIE_PROGRESS_INDICATOR_SUBINDICATOR_H__
#include <tpie/portability.h>
#include <tpie/util.h>
#include <tpie/progress_indicator_base.h>

namespace tpie {

class progress_indicator_subindicator: public progress_indicator_base {
public:
	void refresh();
	virtual void push_breadcrumb(const char * crumb);
	virtual void pop_breadcrumb();
	virtual void init(TPIE_OS_OFFSET range, TPIE_OS_OFFSET step=1);
	virtual void done();
	progress_indicator_subindicator(progress_indicator_base * parent,
									TPIE_OS_OFFSET outerRange,
									const char * crumb=0,
									bool display_subcrumbs=true);
protected:
	progress_indicator_base * m_parent;
	TPIE_OS_OFFSET m_outerRange;
	TPIE_OS_OFFSET m_oldValue;
	char m_crumb[40];
	bool m_display_subcrumbs;
};
	
} //namespace tpie
#endif //__TPIE_PROGRESS_INDICATOR_SUBINDICATOR_H__
