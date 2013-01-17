// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino=(0 :
// Copyright 2013, The TPIE development team
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

#include <iostream>

#include <tpie/tpie.h>
#include <tpie/serialization_sort.h>

int main() {
	tpie::tpie_init(tpie::ALL & ~tpie::JOB_MANAGER);
	const tpie::memory_size_type memory = 10*1024*1024;
	tpie::get_memory_manager().set_limit(memory);
	{
	tpie::serialization_sort<std::string, std::less<std::string> >
		sorter(memory - tpie::get_memory_manager().used());
	std::string line;
	sorter.begin();
	while (std::getline(std::cin, line)) {
		sorter.push(line);
	}
	tpie::log_info() << "Temp file usage: " << tpie::get_temp_file_usage() << std::endl;
	sorter.end();
	tpie::log_info() << "Temp file usage: " << tpie::get_temp_file_usage() << std::endl;
	while (sorter.can_pull()) {
		std::cout << sorter.pull() << '\n';
	}
	tpie::log_info() << "Temp file usage: " << tpie::get_temp_file_usage() << std::endl;
	}
	tpie::log_info() << "Temp file usage: " << tpie::get_temp_file_usage() << std::endl;
	tpie::tpie_finish(tpie::ALL & ~tpie::JOB_MANAGER);
	return 0;
}
