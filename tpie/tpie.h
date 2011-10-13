// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, The TPIE development team
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
#ifndef __TPIE_TPIE_H__
#define __TPIE_TPIE_H__

#include <tpie/config.h>
namespace tpie {

enum subsystem {
    MEMORY_MANAGER=1,
    DEFAULT_LOGGING=2,
    PROGRESS=4,
	PRIMEDB=8,
	JOB_MANAGER=16,
    ALL=MEMORY_MANAGER | DEFAULT_LOGGING | PROGRESS | PRIMEDB | JOB_MANAGER
};

void tpie_init(int subsystems=ALL);
void tpie_finish(int subsystems=ALL);

} //namespace tpie

#endif //__TPIE_TPIE_H__
