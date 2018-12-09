// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2017 The TPIE development team
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

#ifndef __TPIE_PIPELINING_ORDERED_MERGE_H__
#define __TPIE_PIPELINING_ORDERED_MERGE_H__

#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/node.h>

namespace tpie::pipelining {
namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \class merge_t
/// \brief Merge a pull pipeline into a push pipeline
///
/// Currently, it is not very well defined what constitutes a merge.
///////////////////////////////////////////////////////////////////////////////

template <typename dest_t, typename fact_t, typename comp_t>
class ordered_merge_t : public node {
public:
	typedef typename push_type<dest_t>::type item_type;
	comp_t comp;
	typename fact_t::constructed_type with;
	dest_t dest;

	ordered_merge_t(dest_t dest, fact_t fact, comp_t comp):
		comp(comp), with(fact.construct()), dest(std::move(dest)) {
		add_push_destination(dest);
		add_pull_source(with);
	}
	
	void push(const item_type & item) {
		while (with.can_pull() && comp(with.peek(), item))
			dest.push(with.pull());
		dest.push(item);
	}
	
	void end() override {
		while (with.can_pull())
			dest.push(with.pull());
	}
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// A node that merges a pull pipeline into a push pipeline. It pulls an items
/// for each item pushed to it.
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t, typename comp_t=std::less<> >
inline pipe_middle<tfactory<bits::ordered_merge_t, Args<fact_t, comp_t>, fact_t, comp_t>>
	ordered_merge(fact_t fact, comp_t comp=std::less<>()) {
	return {std::move(fact), std::move(comp)};
}

} // namespace tpie::pipelining

#endif //__TPIE_PIPELINING_ORDERED_MERGE_H__
