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

#include <tpie/pipelining.h>

namespace tpie {

namespace pipelining {

segment_map::id_t segment_map::nextId = 0;

// Called by graph_traits
void segment_map::send_successors() const {
	for (relmapit i = m_relations.begin(); i != m_relations.end(); ++i) {
		switch (i->second.second) {
			case pushes:
				m_tokens.find(i->first)->second->add_successor(m_tokens.find(i->second.first)->second);
				break;
			case pulls:
			case depends:
				m_tokens.find(i->second.first)->second->add_successor(m_tokens.find(i->first)->second);
				break;
		}
	}
}

} // namespace pipelining

} // namespace tpie
