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

#ifndef __TPIE_PIPELINING_PAIR_FACTORY_H__
#define __TPIE_PIPELINING_PAIR_FACTORY_H__

#include <tpie/types.h>
#include <tpie/pipelining/priority_type.h>

namespace tpie {

namespace pipelining {

namespace bits {

template <typename child_t>
struct pair_factory_base {
	pair_factory_base()
		: m_maps(new segment_map::ptr[2])
		, m_final(false)
	{
	}

	pair_factory_base(const pair_factory_base & other)
		: m_maps(new segment_map::ptr[2])
		, m_final(other.m_final)
	{
		m_maps[0] = other.m_maps[0];
		m_maps[1] = other.m_maps[1];
	}

	inline double memory() const {
		return self().fact1.memory() + self().fact2.memory();
	}

	inline void name(const std::string & n, priority_type) {
		push_breadcrumb(n);
	}

	void push_breadcrumb(const std::string & n) {
		self().fact1.push_breadcrumb(n);
		self().fact2.push_breadcrumb(n);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Internal - used by subclasses to record references to
	/// segment_maps for a later connectivity check.
	///////////////////////////////////////////////////////////////////////////
	template <typename pipe_t>
	pipe_t record(size_t idx, const pipe_t & pipe) const {
		m_maps[idx] = pipe.get_segment_map();
		if (idx == 0 && m_final) {
			// Now is the opportunity to check that the constructed pipeline is
			// connected.
			assert_connected();
			self().recursive_connected_check();
		}
		return pipe;
	}

	void assert_connected() const {
		if (m_maps[0]->find_authority() != m_maps[1]->find_authority()) {
			log_error() << "Segment map disconnected - more information in debug log." << std::endl;
			log_debug()
				<< "Note about pipe_segment implementations.\n\n"
				   "In a pipe_segment constructor that accepts a destination pipe_segment,\n"
				   "a relation should always be established between the current pipe_segment\n"
				   "and the destination using one of the member functions add_push_destination,\n"
				   "add_pull_destination and add_dependency.\n\n"
				   "If this relational graph is not connected, some pipe_segments will not\n"
				   "be initialized: prepare(), begin(), end() and other methods will never\n"
				   "be called, and memory will not be assigned.\n"
				   "---------------------------------------------------------------------------" << std::endl;
			throw tpie::exception("Segment map disconnected - did you forget to add_push_destination?");
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Signal that this factory is used to instantiate a pipeline_impl,
	/// i.e. that it was made by piping together a pipe_begin and a pipe_end.
	///////////////////////////////////////////////////////////////////////////
	child_t & final() {
		m_final = true;
		return self();
	}

private:
	inline child_t & self() {return *static_cast<child_t*>(this);}
	inline const child_t & self() const {return *static_cast<const child_t*>(this);}

	boost::scoped_array<segment_map::ptr> m_maps;
	bool m_final;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Contains a method `check` that calls recursive_connected_check when
/// fact_t is a pair factory, and otherwise does nothing.
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
struct maybe_check_connected;

template <class fact1_t, class fact2_t>
struct pair_factory : public pair_factory_base<pair_factory<fact1_t, fact2_t> > {
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
		return this->record(0, fact1.construct(this->record(1, fact2.construct(dest))));
	}

	void recursive_connected_check() const {
		maybe_check_connected<fact1_t>::check(fact1);
		maybe_check_connected<fact2_t>::check(fact2);
	}

	fact1_t fact1;
	fact2_t fact2;
};

template <class fact1_t, class termfact2_t>
struct termpair_factory : public pair_factory_base<termpair_factory<fact1_t, termfact2_t> > {
	typedef typename fact1_t::template generated<typename termfact2_t::generated_type>::type generated_type;

	inline termpair_factory(const fact1_t & fact1, const termfact2_t & fact2)
		: fact1(fact1), fact2(fact2) {
		}

	fact1_t fact1;
	termfact2_t fact2;

	inline generated_type construct() const {
		return this->record(0, fact1.construct(this->record(1, fact2.construct())));
	}

	void recursive_connected_check() const {
		maybe_check_connected<fact1_t>::check(fact1);
		maybe_check_connected<termfact2_t>::check(fact2);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief For pair factories, recursively check that the pipe_segments created
/// share their segment_map.
///////////////////////////////////////////////////////////////////////////////
template <typename fact1_t, typename fact2_t>
struct maybe_check_connected<pair_factory<fact1_t, fact2_t> > {
	static void check(const pair_factory<fact1_t, fact2_t> & fact) {
		fact.assert_connected();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief See pair_factory specialization.
///////////////////////////////////////////////////////////////////////////////
template <typename fact1_t, typename termfact2_t>
struct maybe_check_connected<termpair_factory<fact1_t, termfact2_t> > {
	static void check(const termpair_factory<fact1_t, termfact2_t> & fact) {
		fact.assert_connected();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief For user-defined factories in the general case, we cannot do
/// anything to ensure that segment maps are joined together.
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
struct maybe_check_connected {
	static void check(const fact_t & /*fact*/) {
	}
};

} // namespace bits

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_PAIR_FACTORY_H__
