// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
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

#ifndef _TPIE_PROGRESS_INDICATOR_NULL_H
#define _TPIE_PROGRESS_INDICATOR_NULL_H

#include <tpie/portability.h>
#include <tpie/progress_indicator_base.h>

namespace tpie {

///////////////////////////////////////////////
///
/// \class progress_indicator_null 
/// \brief a dummy progress indicator that produces no output
///
///////////////////////////////////////////////
class progress_indicator_null : public progress_indicator_base {

public:
    progress_indicator_null (TPIE_OS_OFFSET range) :progress_indicator_base(range) {}

    virtual ~progress_indicator_null() { /*Do nothing*/ }

	virtual void init(TPIE_OS_OFFSET range) {}
	virtual void done() {}
	virtual void set_range(TPIE_OS_OFFSET range) {}
	virtual void refresh() {}

};

} //tpie namespace

#endif //_TPIE_PROGRESS_INDICATOR_NULL_H
