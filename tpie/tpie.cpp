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

#include <tpie/tpie.h>
#include <tpie/fractional_progress.h>
#include <tpie/execution_time_predictor.h>
#include <tpie/tpie_log.h>
#include <tpie/prime.h>
#include <tpie/mm_manager.h>

namespace tpie {

void tpie_init(int subsystems) {
	if (subsystems & MEMORY_MANAGER)	
		init_memory_manager();

	if (subsystems & DEFAULT_LOGGING)
		init_default_log();

	if (subsystems & PRIMEDB)
		init_prime();

	if (subsystems & PROGRESS) {
		init_fraction_db();
		init_execution_time_db();
	}
}

void tpie_finish(int subsystems) {
    if (subsystems & PROGRESS)  {
		finish_execution_time_db();
		finish_fraction_db();
	}

	if (subsystems & PRIMEDB)
		finish_prime();

	if (subsystems & DEFAULT_LOGGING)
		finish_default_log();

	if (subsystems & MEMORY_MANAGER)	
		finish_memory_manager();
}

}
