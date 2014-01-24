// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014, The TPIE development team
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

#include <tpie/pipelining/node_name.h>

namespace tpie {

namespace pipelining {

namespace bits {

std::string extract_pipe_class_name(std::string mangled) {
	if (mangled[0] == '?') {
		// MSVC mangling
		size_t i=1;
		size_t j=mangled.find("@", i);
		if (j== std::string::npos || j== i) return mangled;
		std::string ans=mangled.substr(i, j-i);
		if (ans != "type") return ans;
		i=j+1;
		j=mangled.find("@", i);
		if (j== std::string::npos || j== i) return mangled;
		return mangled.substr(i, j-i);
	} else {
		// Assume GCC mangling
		size_t i=0;
		if (mangled[i] == 'N') ++i; // Skip nested type decleration
		std::string last;
		std::string cur;
		while (true) {
			if (i >= mangled.size() || mangled[i] == 'I') break;
			int v=0;
			while (i < mangled.size()
					&& mangled[i] >= '0'
					&& mangled[i] <= '9') {
				v *= 10;
				v += (mangled[i] - '0');
				i++;
			}
			if (v == 0 || i+v > mangled.size()) return mangled;
			last = cur;
			cur = mangled.substr(i, v);
			i+=v;
		}
		if (cur.empty()) return mangled;
		if (cur != "type") return cur;
		if (last.empty()) return mangled;
		return last;
	}
}

} // namespace bits

} // namespace pipelining

} // namespace tpie
