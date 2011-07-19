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
#include <tpie/backtrace.h>
#include <tpie/tpie_log.h>
#if defined(WIN32) || defined(__CYGWIN__)

namespace tpie {
void backtrace(std::ostream & out, int depth){}
}

#else // WIN32
#include <cxxabi.h>
#include <execinfo.h>
#include <cstdlib>
#include <iostream>

namespace tpie {
void backtrace(std::ostream & out, int depth) {
	out << "=====================> Backtrace <=====================" << std::endl;
	void * array[depth+1];
	int nSize = ::backtrace(array, depth+1);
	char ** symbols = backtrace_symbols(array, nSize);
	for(int i=1; i < nSize; ++i) {
		if (!symbols[i]) continue;
		std::string sym = symbols[i];
		
		std::string method, index;
		size_t r = sym.rfind(")");
		size_t m = sym.rfind("+",r);
		size_t l = sym.rfind("(",m);
		size_t s = sym.rfind("/", l);
		
		if (l == std::string::npos || 
			r == std::string::npos ||
			m == std::string::npos) {
			l = sym.rfind("[");
			method="Unknown";
			index="";
		} else {
			method=sym.substr(l+1,m-l-1);
			index=sym.substr(m,r-m);
		}
		if (s == std::string::npos) s=0;
		std::string exe = sym.substr(s+1,l-s-1);
		{
			int x; 
			char * buff=abi::__cxa_demangle(method.c_str(), NULL, NULL, &x);
			if (x == 0) method = buff;
			std::free(buff);
		}
		out << exe << ": " << method << index << std::endl;
	}
	out << "=======================================================" << std::endl;
	std::free(symbols);
}
}
#endif //WIN32

namespace tpie {
void __softassert(const char * expr, const char * file, int line) {
	log_error() << "Soft assertion error: " << expr << std::endl
				<< file << ":" << line << std::endl;
	backtrace(log_error());
}


}


