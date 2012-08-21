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

#ifndef __TPIE_PIPELINING_SORT_H__
#define __TPIE_PIPELINING_SORT_H__

#include <tpie/pipelining/core.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/merge_sorter.h>
#include <tpie/sort.h>
#include <tpie/parallel_sort.h>
#include <tpie/file_stream.h>
#include <tpie/tempname.h>
#include <tpie/memory.h>
#include <queue>
#include <boost/shared_ptr.hpp>

namespace tpie {

namespace pipelining {

template <typename T, typename pred_t>
struct sort_input_t : public pipe_segment {
	typedef T item_type;
	typedef merge_sorter<item_type, true, pred_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;

	inline sort_input_t(sorterptr sorter, const segment_token & token)
		: pipe_segment(token)
		, sorter(sorter)
	{
		set_minimum_memory(sorter_t::minimum_memory_phase_1());
		set_name("Form input runs", PRIORITY_SIGNIFICANT);
	}

	inline void begin() {
		sorter->begin();
	}

	inline void push(const T & item) {
		sorter->push(item);
	}

	inline void end() {
		sorter->end();
	}

	bool can_evacuate() {
		return true;
	}

	void evacuate() {
		sorter->evacuate_before_merging();
	}

protected:
	void set_available_memory(memory_size_type availableMemory) {
		pipe_segment::set_available_memory(availableMemory);
		sorter->set_phase_1_memory(availableMemory);
	}

private:
	sorterptr sorter;
};

template <typename T, typename pred_t>
struct sort_calc_t : public pipe_segment {
	typedef T item_type;
	typedef merge_sorter<item_type, true, pred_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;

	inline sort_calc_t(const segment_token & input, const sorterptr & sorter)
		: sorter(sorter)
	{
		add_dependency(input);
		set_minimum_memory(sorter_t::minimum_memory_phase_2());
		set_name("Perform merge heap", PRIORITY_SIGNIFICANT);
	}

	inline sort_calc_t(const pipe_segment & input, const sorterptr & sorter)
		: sorter(sorter)
	{
		add_dependency(input);
		set_minimum_memory(sorter_t::minimum_memory_phase_2());
		set_name("Perform merge heap", PRIORITY_SIGNIFICANT);
	}

	inline void go(progress_indicator_base & pi) {
		sorter->calc(pi);
	}

	bool can_evacuate() {
		return true;
	}

	void evacuate() {
		sorter->evacuate_before_reporting();
	}

protected:
	void set_available_memory(memory_size_type availableMemory) {
		pipe_segment::set_available_memory(availableMemory);
		sorter->set_phase_2_memory(availableMemory);
	}

private:
	sorterptr sorter;
};

template <typename pred_t>
struct sort_output_t {
	template <typename dest_t>
	struct type : public pipe_segment {
		typedef typename dest_t::item_type item_type;
		typedef merge_sorter<item_type, true, pred_t> sorter_t;
		typedef typename sorter_t::ptr sorterptr;

		inline type(const dest_t & dest, const pipe_segment & calc, const sorterptr & sorter)
			: dest(dest)
			, sorter(sorter)
			{
				add_dependency(calc);
				add_push_destination(dest);
				set_minimum_memory(sorter_t::minimum_memory_phase_3());
				set_name("Write sorted output", PRIORITY_INSIGNIFICANT);
			}
		
		void go(progress_indicator_base & pi) {
			pi.init(sorter->item_count());
			dest.begin();
			while (sorter->can_pull()) {
				dest.push(sorter->pull());
				pi.step();
			}
			dest.end();
			pi.done();
		}
		
	protected:
		void set_available_memory(memory_size_type availableMemory) {
			pipe_segment::set_available_memory(availableMemory);
			sorter->set_phase_3_memory(availableMemory);
		}
		
	private:
		dest_t dest;
		sorterptr sorter;
	};
};

template <typename T, typename pred_t>
struct sort_pull_output_t : public pipe_segment {
	typedef T item_type;
	typedef merge_sorter<item_type, true, pred_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;

	inline sort_pull_output_t(const pipe_segment & calc, const sorterptr & sorter)
		: sorter(sorter)
	{
		add_dependency(calc);
		set_minimum_memory(sorter_t::minimum_memory_phase_3());
		set_name("Write sorted output", PRIORITY_INSIGNIFICANT);
	}

	inline void begin() {
	}

	inline bool can_pull() const {
		return sorter->can_pull();
	}

	inline T pull() {
		return sorter->pull();
	}

	inline void end() {
	}

protected:
	void set_available_memory(memory_size_type availableMemory) {
		pipe_segment::set_available_memory(availableMemory);
		sorter->set_phase_3_memory(availableMemory);
	}

private:
	sorterptr sorter;
};

template <typename pred_t>
struct sort_t {
	template <typename dest_t>
	struct type : public pipe_segment {
		
		typedef typename dest_t::item_type item_type;
		typedef merge_sorter<item_type, true, pred_t> sorter_t;
		typedef typename sorter_t::ptr sorterptr;
		typedef sort_calc_t<item_type, pred_t> calc_t;
		typedef typename sort_output_t<pred_t>::template type<dest_t> output_t;
		
		inline type(const type<dest_t> & other)
			: pipe_segment(other)
			, sorter(other.sorter)
			, calc(other.calc)
			, output(other.output)
			{
				set_minimum_memory(sorter_t::minimum_memory_phase_1());
				set_name("Form input runs", PRIORITY_SIGNIFICANT);
			}
		
		inline type(const dest_t & dest, const pred_t & pred=pred_t())
			: sorter(new sorter_t(pred))
			, calc(*this, sorter)
			, output(dest, calc, sorter)
			{
				set_minimum_memory(sorter_t::minimum_memory_phase_1());
			}
		
		inline void begin() {
			sorter->begin();
		}
		
		inline void push(const item_type & item) {
			sorter->push(item);
		}
		
		inline void end() {
			sorter->end();
		}
		
	protected:
		void set_available_memory(memory_size_type availableMemory) {
			pipe_segment::set_available_memory(availableMemory);
			sorter->set_phase_1_memory(availableMemory);
		}
	private:
		sorterptr sorter;
		calc_t calc;
		output_t output;
	};
};

template <typename dest_t>
struct default_pred_sort_t: public sort_t<typename std::less<typename dest_t::item_type> >::template type<dest_t > {
	default_pred_sort_t(const dest_t & d): sort_t<typename std::less<typename dest_t::item_type> >::template type<dest_t >(d) {}
};

inline pipe_middle<factory_0<default_pred_sort_t> >
pipesort() {
	return factory_0<default_pred_sort_t>();
}


template <typename pred_t>
inline pipe_middle<factory_1<sort_t<pred_t>::template type, const pred_t &> >
pipesort(const pred_t & p) {
	return factory_1<sort_t<pred_t>::template type, const pred_t &>(p);
}

template <typename T, typename pred_t>
struct passive_sorter {
	typedef T item_type;
	typedef merge_sorter<item_type, true, pred_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;
	typedef sort_input_t<item_type, pred_t> input_t;
	typedef sort_calc_t<item_type, pred_t> calc_t;
	typedef sort_pull_output_t<item_type, pred_t> output_t;

	inline passive_sorter()
		: sorter(new sorter_t())
		, calc(input_token, sorter)
		, m_output(calc, sorter)
	{
	}

	inline pipe_end<termfactory_2<input_t, sorterptr, const segment_token &> > input() {
		return termfactory_2<input_t, sorterptr, const segment_token &>(sorter, input_token);
	}

	inline output_t output() {
		return m_output;
	}

private:
	segment_token input_token;
	pred_t pred;
	sorterptr sorter;
	calc_t calc;
	output_t m_output;
	passive_sorter(const passive_sorter &);
	passive_sorter & operator=(const passive_sorter &);
};

}

}

#endif
