// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
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

///////////////////////////////////////////////////////////////////////////////
/// \file execution.h  Methods related to the execution of a pipeline.
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_PIPELINING_EXECUTION_H__
#define __TPIE_PIPELINING_EXECUTION_H__

namespace tpie {

namespace pipelining {

template <typename fact_t>
inline void pipeline_impl<fact_t>::operator()(stream_size_type items, progress_indicator_base & pi, const memory_size_type mem) {
	typedef std::vector<phase> phases_t;
	typedef phases_t::const_iterator it;

	segment_map::ptr map = r.get_segment_map()->find_authority();
	graph_traits g(*map);
	const phases_t & phases = g.phases();
	if (mem == 0) log_warning() << "No memory for pipelining" << std::endl;
	for (it i = phases.begin(); i != phases.end(); ++i) {
		i->assign_memory(mem);
	}
	g.go_all(items, pi);
}

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_EXECUTION_H__
