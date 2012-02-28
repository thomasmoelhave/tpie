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
#include <tpie/disjoint_sets.h>

namespace tpie {

namespace pipelining {

///////////////////////////////////////////////////////////////////////////////
/// Base class of all segments. A segment should inherit from pipe_segment,
/// have a single template parameter dest_t if it is not a terminus segment,
/// and implement methods begin(), push() and end(), if it is not a source
/// segment.
///////////////////////////////////////////////////////////////////////////////
struct pipe_segment {
	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the pipe_segment that receives items from this segment
	/// (directly or indirectly), or NULL if we are a terminus segment.
	///////////////////////////////////////////////////////////////////////////
	virtual const pipe_segment * get_next() const = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief True if our successor segment does not begin processing before
	/// we are finished receiving elements.
	/// As an example, consider reversing the items of a stream. You cannot
	/// push items further down the pipeline before all items are received and
	/// end() is called. Thus we have to store items in a buffer which may grow
	/// arbitrarily large and thus may be stored on disk.
	///////////////////////////////////////////////////////////////////////////
	virtual bool buffering() const {
		return false;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \class pipeline_virtual
/// Virtual superclass for pipelines implementing the function call operator.
///////////////////////////////////////////////////////////////////////////////
struct pipeline_virtual {
	///////////////////////////////////////////////////////////////////////////
	/// \brief Invoke the pipeline.
	///////////////////////////////////////////////////////////////////////////
	virtual void operator()() = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Generate a GraphViz graph documenting the pipeline flow.
	///////////////////////////////////////////////////////////////////////////
	virtual void plot(std::ostream & out) = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Generate a GraphViz graph documenting the pipeline flow.
	///////////////////////////////////////////////////////////////////////////
	virtual void plot_phases(std::ostream & out) = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Virtual dtor.
	///////////////////////////////////////////////////////////////////////////
	virtual ~pipeline_virtual() {}
};

///////////////////////////////////////////////////////////////////////////////
/// \class pipeline_impl
/// \tparam fact_t Factory type
/// Templated subclass of pipeline_virtual for push pipelines.
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
struct pipeline_impl : public pipeline_virtual {
	typedef typename fact_t::generated_type gen_t;
	typedef boost::unordered_map<const pipe_segment *, size_t> nodes_t;

	inline pipeline_impl(const fact_t & factory) : r(factory.construct()) {}
	virtual ~pipeline_impl() {
	}
	void operator()() {
		r();
	}
	inline operator gen_t() {
		return r;
	}
	void plot(std::ostream & out) {
		out << "digraph {\nrankdir=LR;\n";
		nodes_t n = nodes();
		for (nodes_t::iterator i = n.begin(); i != n.end(); ++i) {
			out << '"' << i->first << "\";\n";
		}
		{
			const pipe_segment * c = &r;
			while (c != 0) {
				//size_t number = n.find(c)->second;
				const pipe_segment * next = c->get_next();
				if (next) {
					//size_t next_number = n.find(next)->second;
					out << '"' << c << "\" -> \"" << next;
					if (c->buffering())
						out << "\" [style=dashed];\n";
					else
						out << "\";\n";
				}
				c = next;
			}
		}
		out << '}' << std::endl;
	}

	void plot_phases(std::ostream & out) {
		nodes_t n = nodes();
		tpie::disjoint_sets<size_t> p = phases(n);
		out << "digraph {\n";
		for (nodes_t::iterator i = n.begin(); i != n.end(); ++i) {
			out << '"' << i->second << "\";\n";
		}
		for (nodes_t::iterator i = n.begin(); i != n.end(); ++i) {
			size_t cur = i->second;
			size_t rep = p.find_set(cur);
			if (rep != cur) {
				out << '"' << cur << "\" -> \"" << rep << "\";\n";
			}
		}
		out << '}' << std::endl;
	}

private:
	gen_t r;

	nodes_t nodes() {
		boost::unordered_map<const pipe_segment *, size_t> numbers;
		size_t next_number = 0;
		const pipe_segment * c = &r;
		while (c != 0) {
			if (!numbers.count(c)) {
				//out << '"' << next_number << "\";\n";
				numbers.insert(std::make_pair(c, next_number));
				++next_number;
			}
			c = c->get_next();
		}
		return numbers;
	}

	tpie::disjoint_sets<size_t> phases(const nodes_t & n) {
		tpie::disjoint_sets<size_t> res(n.size());
		for (nodes_t::const_iterator i = n.begin(); i != n.end(); ++i) {
			res.make_set(i->second);
		}
		for (nodes_t::const_iterator i = n.begin(); i != n.end(); ++i) {
			const pipe_segment * next = i->first->get_next();
			if (next && !i->first->buffering())
				res.union_set(i->second, n.find(next)->second);
		}
		return res;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \class datasource_wrapper
/// \tparam gen_t Generator type
/// Templated subclass of pipeline_virtual for pull pipelines.
///////////////////////////////////////////////////////////////////////////////
template <typename gen_t>
struct datasource_wrapper : public pipeline_virtual {
	gen_t generator;
	inline datasource_wrapper(const gen_t & generator) : generator(generator) {
	}
	inline void operator()() {
		generator();
	}
	void plot(std::ostream & out) {
		out << "datasource" << std::endl;
	}
	void plot_phases(std::ostream & out) {
		out << "datasource" << std::endl;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \class pipeline
///
/// This class is used to avoid writing the template argument in the
/// pipeline_impl type.
///////////////////////////////////////////////////////////////////////////////
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
		p->plot(std::cout);
	}
	inline void plot_phases() {
		p->plot_phases(std::cout);
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

///////////////////////////////////////////////////////////////////////////////
/// \class pipe_middle
///
/// A pipe_middle class pushes input down the pipeline.
///
/// \tparam fact_t A factory with a construct() method like the factory_0,
///                factory_1, etc. helpers.
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
struct pipe_middle {
	typedef fact_t factory_type;

	inline pipe_middle() {
	}

	inline pipe_middle(const fact_t & factory) : factory(factory) {
	}

	///////////////////////////////////////////////////////////////////////////
	/// The pipe operator combines this generator/filter with another filter.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact2_t>
	inline pipe_middle<bits::pair_factory<fact_t, fact2_t> >
	operator|(const pipe_middle<fact2_t> & r) {
		return bits::pair_factory<fact_t, fact2_t>(factory, r.factory);
	}

	///////////////////////////////////////////////////////////////////////////
	/// This pipe operator combines this generator/filter with a terminator to
	/// make a pipeline.
	///////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
/// \class datasource
///
/// A data source pulls data up the pipeline.
///
/// \tparam gen_t Generator type
///////////////////////////////////////////////////////////////////////////////
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
