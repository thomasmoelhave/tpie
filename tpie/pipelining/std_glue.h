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

#ifndef __TPIE_PIPELINING_STD_GLUE_H__
#define __TPIE_PIPELINING_STD_GLUE_H__

#include <vector>

#include <tpie/pipelining/core.h>
#include <tpie/pipelining/factory_helpers.h>

namespace tpie {

template <typename dest_t>
struct input_vector_t {
	typedef typename dest_t::item_type item_type;

	input_vector_t(const dest_t & dest, const std::vector<item_type> & input) : dest(dest), input(input) {
	}

	void operator()() {
		typedef typename std::vector<item_type>::const_iterator IT;
		for (IT i = input.begin(); i != input.end(); ++i) {
			dest.push(*i);
		}
	}

private:
	dest_t dest;
	const std::vector<item_type> & input;
};

template <typename T>
struct output_vector_t {
	typedef T item_type;

	output_vector_t(std::vector<T> & output) : output(output) {
	}

	void push(const T & item) {
		output.push_back(item);
	}

private:
	std::vector<item_type> & output;
};

template<typename T>
generate<factory_1<input_vector_t, const std::vector<T> &> > input_vector(const std::vector<T> & input) {
	return factory_1<input_vector_t, const std::vector<T> &>(input);
}

template <typename T>
terminator<termfactory_1<output_vector_t<T>, std::vector<T> &> > output_vector(std::vector<T> & output) {
	return termfactory_1<output_vector_t<T>, std::vector<T> &>(output);
}

}

#endif
