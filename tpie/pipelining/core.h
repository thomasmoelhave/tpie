// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, The TPIE development team
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

#ifndef __TPIE_PIPELINING_CORE_H__
#define __TPIE_PIPELINING_CORE_H__

#include <boost/unordered_map.hpp>
#include <iostream>

namespace tpie {

namespace pipelining {

struct pipe_segment {
	virtual const pipe_segment * get_next() const = 0;
};

/**
 * \class pipeline_virtual
 * Virtual superclass for pipelines implementing the function call operator.
 */
struct pipeline_virtual {
	/**
	 * Invoke the pipeline. The only virtual function call in this library.
	 */
	virtual void operator()() = 0;
	virtual void plot() = 0;
	virtual ~pipeline_virtual() {}
};

/**
 * \class pipeline_impl
 * \tparam fact_t Factory type
 * Templated subclass of pipeline_virtual for push pipelines.
 */
template <typename fact_t>
struct pipeline_impl : public pipeline_virtual {
	typedef typename fact_t::generated_type gen_t;
	inline pipeline_impl(const fact_t & factory) : r(factory.construct()) {}
	virtual ~pipeline_impl() {
	}
	void operator()() {
		r();
	}
	inline operator gen_t() {
		return r;
	}
	void plot() {
		std::ostream & out = std::cout;
		out << "digraph {\nrankdir=LR;\n";
		boost::unordered_map<const pipe_segment *, size_t> numbers;
		{
			size_t next_number = 0;
			const pipe_segment * c = &r;
			while (c != 0) {
				if (!numbers.count(c)) {
					out << '"' << next_number << "\";\n";
					numbers.insert(std::make_pair(c, next_number));
					++next_number;
				}
				c = c->get_next();
			}
		}
		{
			const pipe_segment * c = &r;
			while (c != 0) {
				size_t number = numbers.find(c)->second;
				const pipe_segment * next = c->get_next();
				if (next) {
					size_t next_number = numbers.find(next)->second;
					out << '"' << number << "\" -> \"" << next_number << "\";\n";
				}
				c = next;
			}
		}
		out << '}' << std::endl;
	}
private:
	gen_t r;
};

/**
 * \class datasource_wrapper
 * \tparam gen_t Generator type
 * Templated subclass of pipeline_virtual for pull pipelines.
 */
template <typename gen_t>
struct datasource_wrapper : public pipeline_virtual {
	gen_t generator;
	inline datasource_wrapper(const gen_t & generator) : generator(generator) {
	}
	inline void operator()() {
		generator();
	}
	void plot() {
		*((char*)0)=42;
	}
};

/**
 * \class pipeline
 *
 * This class is used to avoid writing the template argument in the
 * pipeline_impl type.
 */
struct pipeline {
	template <typename T>
	inline pipeline(const T & from) {
		p = new T(from);
	}
	inline ~pipeline() {
		delete p;
	}
	inline void operator()() {
		(*p)();
	}
	inline void plot() {
		p->plot();
	}
private:
	pipeline_virtual * p;
};

namespace bits {

template <class fact1_t, class fact2_t>
struct pair_factory {
	template <typename dest_t>
	struct generated {
		typedef typename fact1_t::template generated<typename fact2_t::template generated<dest_t>::type>::type type;
	};

	inline pair_factory(const fact1_t & fact1, const fact2_t & fact2)
		: fact1(fact1), fact2(fact2) {
	}

	template <typename dest_t>
	inline typename generated<dest_t>::type
	construct(const dest_t & dest) const {
		return fact1.construct(fact2.construct(dest));
	}

	fact1_t fact1;
	fact2_t fact2;
};

template <class fact1_t, class termfact2_t>
struct termpair_factory {
	typedef typename fact1_t::template generated<typename termfact2_t::generated_type>::type generated_type;

	inline termpair_factory(const fact1_t & fact1, const termfact2_t & fact2)
		: fact1(fact1), fact2(fact2) {
		}

	fact1_t fact1;
	termfact2_t fact2;

	inline generated_type
	construct() const {
		return fact1.construct(fact2.construct());
	}
};

} // namespace bits

template <typename fact_t>
struct pipe_end {
	typedef fact_t factory_type;

	inline pipe_end() {
	}

	inline pipe_end(const fact_t & factory) : factory(factory) {
	}

	fact_t factory;
};

/**
 * \class pipe_middle
 *
 * A pipe_middle class pushes input down the pipeline.
 *
 * \tparam fact_t A factory with a construct() method like the factory_0,
 *                factory_1, etc. helpers.
 */
template <typename fact_t>
struct pipe_middle {
	typedef fact_t factory_type;

	inline pipe_middle() {
	}

	inline pipe_middle(const fact_t & factory) : factory(factory) {
	}

	/**
	 * The pipe operator combines this generator/filter with another filter.
	 */
	template <typename fact2_t>
	inline pipe_middle<bits::pair_factory<fact_t, fact2_t> >
	operator|(const pipe_middle<fact2_t> & r) {
		return bits::pair_factory<fact_t, fact2_t>(factory, r.factory);
	}

	/**
	 * This pipe operator combines this generator/filter with a terminator to
	 * make a pipeline.
	 */
	template <typename fact2_t>
	inline pipe_end<bits::termpair_factory<fact_t, fact2_t> >
	operator|(const pipe_end<fact2_t> & r) {
		return bits::termpair_factory<fact_t, fact2_t>(factory, r.factory);
	}

	fact_t factory;
};

template <typename fact_t>
struct pipe_begin {
	typedef fact_t factory_type;

	inline pipe_begin() {
	}

	inline pipe_begin(const fact_t & factory) : factory(factory) {
	}

	template <typename fact2_t>
	inline pipe_begin<bits::pair_factory<fact_t, fact2_t> >
	operator|(const pipe_middle<fact2_t> & r) {
		return bits::pair_factory<fact_t, fact2_t>(factory, r.factory);
	}

	template <typename fact2_t>
	inline pipeline_impl<bits::termpair_factory<fact_t, fact2_t> >
	operator|(const pipe_end<fact2_t> & r) {
		return bits::termpair_factory<fact_t, fact2_t>(factory, r.factory);
	}

	fact_t factory;
};

/**
 * \class datasource
 *
 * A data source pulls data up the pipeline.
 *
 * \tparam gen_t Generator type
 */
template <typename gen_t>
struct datasource {
	inline datasource(const gen_t & gen) : generator(gen) {
	}

	template <typename fact_t>
	inline datasource<typename fact_t::template generated<gen_t>::type>
	operator|(const fact_t & fact) const {
		return fact.construct(generator);
	}

	inline operator gen_t() const {
		return generator;
	}

	inline operator datasource_wrapper<gen_t>() const {
		return generator;
	}

	inline operator pipeline() const {
		return datasource_wrapper<gen_t>(generator);
	}

	gen_t generator;
};

}

}

#endif
