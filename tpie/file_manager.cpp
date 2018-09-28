// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
	// vi:set ts=4 sts=4 sw=4 noet :
//
// Copyright 2011, 2014, The TPIE development team
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

#include "file_manager.h"
#include <tpie/exception.h>
#include <iostream>
#include <sstream>
#include "tpie_log.h"
#include <cstring>
#include <cstdlib>
#ifndef _WIN32
#include <sys/time.h>
#include <sys/resource.h>
#endif

namespace tpie {

size_t get_maximum_open_files() {
#ifdef _WIN32
	return _getmaxstdio();
#else
	struct rlimit limits;
	if (getrlimit(RLIMIT_NOFILE, &limits) == -1) {
		return 256;
	}
	return limits.rlim_cur;
#endif
}

file_manager * fm = 0;

file_manager::file_manager(): resource_manager(FILES) {}

std::string file_manager::amount_with_unit(size_t amount) const  {
	std::ostringstream os;
	if (amount == 1) {
		os << "a file";
	} else {
		os << amount << " files";
	}
	return os.str();
}

void file_manager::throw_out_of_resource_error(const std::string & s) {
	throw out_of_files_error(s);
}

void init_file_manager() {
	const size_t reserved_files = 42;

	fm = new file_manager();
	fm->set_limit(get_maximum_open_files() - reserved_files);
}

void finish_file_manager() {
	delete fm;
	fm = 0;
}

file_manager & get_file_manager() {
#ifndef TPIE_NDEBUG
	if (fm == 0) throw std::runtime_error("File management not initialized");
#endif
	return * fm;
}

} //namespace tpieg
