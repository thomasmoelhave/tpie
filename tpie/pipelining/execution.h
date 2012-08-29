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

inline void phase::assign_memory(memory_size_type m) const {
	if (m < minimum_memory()) {
		TP_LOG_WARNING_ID("Not enough memory for this phase. We have " << m << " but we require " << minimum_memory() << '.');
		assign_minimum_memory();
		return;
	}
	memory_size_type remaining = m;
	double fraction = memory_fraction();
	//std::cout << "Remaining " << m << " fraction " << fraction << " segments " << m_segments.size() << std::endl;
	std::vector<char> assigned(m_segments.size());
	while (true) {
		bool done = true;
		for (size_t i = 0; i < m_segments.size(); ++i) {
			if (assigned[i]) continue;
			pipe_segment * s = m_segments[i];
			memory_size_type min = s->get_minimum_memory();
			double frac = s->get_memory_fraction();
			double to_assign = frac/fraction * remaining;
			if (to_assign < min) {
				done = false;
				s->set_available_memory(min);
				assigned[i] = true;
				remaining -= min;
				fraction -= frac;
			}
		}
		if (!done) continue;
		for (size_t i = 0; i < m_segments.size(); ++i) {
			if (assigned[i]) continue;
			pipe_segment * s = m_segments[i];
			double frac = s->get_memory_fraction();
			double to_assign = frac/fraction * remaining;
			s->set_available_memory(static_cast<memory_size_type>(to_assign));
		}
		break;
	}
}

inline std::vector<size_t> phasegraph::execution_order() {
	dfs(*this);
	std::vector<std::pair<int, size_t> > nodes;
	for (nodemap_t::iterator i = finish_times.begin(); i != finish_times.end(); ++i) {
		nodes.push_back(std::make_pair(-i->second, i->first));
	}
	std::sort(nodes.begin(), nodes.end());
	std::vector<size_t> result(nodes.size());
	size_t j = 0;
	for (size_t i = nodes.size(); i--;) {
		result[j++] = nodes[i].second;
	}
	return result;
}

inline void graph_traits::go_all(stream_size_type n, Progress::base & pi) {
	map.assert_authoritative();
	map.send_successors();
	Progress::fp fp(&pi);
	array<auto_ptr<Progress::sub> > subindicators(m_phases.size());
	for (size_t i = 0; i < m_phases.size(); ++i) {
		phase & curphase = m_phases[i];
		std::string name = curphase.get_name();
		if (0 == name.size()) log_error() << "Phase has no name" << std::endl;
		std::string uid = curphase.get_unique_id();
		subindicators[i].reset(tpie_new<Progress::sub>(fp, uid.c_str(), TPIE_FSI, n, name.c_str()));
	}

	fp.init();
	for (size_t i = 0; i < m_phases.size(); ++i) {
		if (m_evacuatePrevious[i]) m_phases[i-1].evacuate_all();
		m_phases[i].go(*subindicators[i]);
	}
	fp.done();
}

inline void graph_traits::calc_phases() {
	map.assert_authoritative();
	typedef std::map<id_t, size_t> ids_t;
	typedef std::map<size_t, id_t> ids_inv_t;
	ids_t ids;
	ids_inv_t ids_inv;
	size_t nextid = 0;
	for (segment_map::mapit i = map.begin(); i != map.end(); ++i) {
		ids.insert(std::make_pair(i->first, nextid));
		ids_inv.insert(std::make_pair(nextid, i->first));
		++nextid;
	}
	tpie::disjoint_sets<size_t> phases(nextid);
	for (size_t i = 0; i < nextid; ++i) phases.make_set(i);

	const segment_map::relmap_t relations = map.get_relations();
	for (segment_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
		if (i->second.second != depends) phases.union_set(ids[i->first], ids[i->second.first]);
	}
	// `phases` holds a map from segment to phase number

	phasegraph g(phases, nextid);

	// establish phase relationships
	for (segment_map::relmapit i = relations.begin(); i != relations.end(); ++i) {
		if (i->second.second == depends) g.depends(phases.find_set(ids[i->first]), phases.find_set(ids[i->second.first]));
	}

	// toposort the phase graph and find the phase numbers in the execution order
	std::vector<size_t> internalexec = g.execution_order();
	m_phases.resize(internalexec.size());
	m_evacuatePrevious.resize(internalexec.size(), false);

	std::vector<bool>::iterator j = m_evacuatePrevious.begin();
	for (size_t i = 0; i < internalexec.size(); ++i, ++j) {
		// all segments with phase number internalexec[i] should be executed in phase i

		// first, insert phase representatives
		m_phases[i].add(map.get(ids_inv[internalexec[i]]));
		*j = i > 0 && !g.is_depending(internalexec[i], internalexec[i-1]);
	}
	for (ids_inv_t::iterator i = ids_inv.begin(); i != ids_inv.end(); ++i) {
		pipe_segment * representative = map.get(ids_inv[phases.find_set(i->first)]);
		pipe_segment * current = map.get(i->second);
		if (current == representative) continue;
		for (size_t i = 0; i < m_phases.size(); ++i) {
			if (m_phases[i].count(representative)) {
				m_phases[i].add(current);
				break;
			}
		}
	}
}

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
