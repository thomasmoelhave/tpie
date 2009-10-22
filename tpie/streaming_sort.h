// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, The TPIE development team
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

#ifndef _TPIE_STREAMING_SORT_H
#define _TPIE_STREAMING_SORT_H

#include <vector>
#include <algorithm>

namespace tpie {

	template <class T>
	struct key_identity {
		inline const T & operator()(const T & item) const {
			return item;
		}
	};

	template <class dest_t,
			  class comp_t=std::less<typename dest_t::item_type>,
			  class key_t=key_identity<typename dest_t::item_type> >
	class streaming_sort {
	public:
		typedef typename dest_t::item_type item_type;
	private:
		struct icomp_t {
			comp_t comp;
			key_t key;
			icomp_t(comp_t & c, key_t & k): comp(c), key(k) {};
			bool operator()(const item_type & a, const item_type & b) const {
				return comp(key(a), key(b));
			}
		};
		
		icomp_t comp;
		dest_t & dest;
		std::vector<item_type> mem;
	public:
		streaming_sort(dest_t & d, comp_t c=comp_t(), key_t k=key_t()):
			comp(c,k), dest(d) {};

		void begin(TPIE_OS_OFFSET size=0) {}
		void end() {
			std::sort(mem.begin(), mem.end(), comp);
			dest.begin(mem.size());
			for(size_t i=0; i < mem.size(); ++i)
				dest.push(mem[i]);
			dest.end();
		}
		void push(const item_type & item) {
			mem.push_back(item);
		}
	};

}



#endif //_TPIE_STREAMING_SORT_H
