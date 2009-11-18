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

#ifndef __TPIE_STREAM_EXCEPTION_H__
#define __TPIE_STREAM_EXCEPTION_H__
#include <tpie/exception.h>

namespace tpie {
namespace stream {

struct exception: public tpie::exception {
	exception(const std::string & s): tpie::exception(s) {};
};

struct io_exception: public exception {
	io_exception(const std::string & s): exception(s) {};
};

struct invalid_file_exception: public exception {
	invalid_file_exception(const std::string & s): exception(s) {};
};

struct end_of_stream_exception: public exception {
	end_of_stream_exception(const std::string & s): exception(s) {};
};


}
}
#endif //__TPIE_STREAM_EXCEPTION_H__
