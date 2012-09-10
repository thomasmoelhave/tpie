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

template <typename pred_t, typename Output>
struct sort_calc_t;

template <typename pred_t, typename Output>
struct sort_input_t;

template <typename T, typename pred_t>
struct sort_pull_output_t : public pipe_segment {
	typedef T item_type;
	typedef merge_sorter<item_type, true, pred_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;

	inline sort_pull_output_t() {
		set_minimum_memory(sorter_t::minimum_memory_phase_3());
		set_name("Write sorted output", PRIORITY_INSIGNIFICANT);
	}

	inline bool can_pull() const {
		return m_sorter->can_pull();
	}

	inline item_type pull() {
		return m_sorter->pull();
	}

protected:
	virtual void set_available_memory(memory_size_type availableMemory) /*override*/ {
		pipe_segment::set_available_memory(availableMemory);
		m_sorter->set_phase_3_memory(availableMemory);
	}

private:
	sorterptr m_sorter;

	void set_calc_segment(pipe_segment & calc, sorterptr & sorter) {
		add_dependency(calc);
		m_sorter = sorter;
	}

	friend struct sort_calc_t<pred_t, sort_pull_output_t>;
};

template <typename pred_t, typename dest_t>
struct sort_output_t : public pipe_segment {
	typedef typename dest_t::item_type item_type;
	typedef merge_sorter<item_type, true, pred_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;

	inline sort_output_t(const dest_t & dest)
		: dest(dest)
	{
		add_push_destination(dest);
		set_minimum_memory(sorter_t::minimum_memory_phase_3());
		set_name("Write sorted output", PRIORITY_INSIGNIFICANT);
	}

	virtual void go(progress_indicator_base & pi) /*override*/ {
		pi.init(m_sorter->item_count());
		while (m_sorter->can_pull()) {
			dest.push(m_sorter->pull());
			pi.step();
		}
		pi.done();
	}

protected:
	virtual void set_available_memory(memory_size_type availableMemory) /*override*/ {
		pipe_segment::set_available_memory(availableMemory);
		m_sorter->set_phase_3_memory(availableMemory);
	}

private:
	dest_t dest;
	sorterptr m_sorter;

	void set_calc_segment(pipe_segment & calc, sorterptr & sorter) {
		add_dependency(calc);
		m_sorter = sorter;
	}

	friend struct sort_calc_t<pred_t, sort_output_t>;
};

template <typename pred_t, typename Output>
struct sort_calc_t : public pipe_segment {
	typedef typename Output::item_type item_type;
	typedef merge_sorter<item_type, true, pred_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;

	inline sort_calc_t(Output dest)
		: dest(dest)
	{
		set_minimum_memory(sorter_t::minimum_memory_phase_2());
		set_name("Perform merge heap", PRIORITY_SIGNIFICANT);
	}

	virtual void go(progress_indicator_base & pi) /*override*/ {
		m_sorter->calc(pi);
	}

	virtual bool can_evacuate() /*override*/ {
		return true;
	}

	virtual void evacuate() /*override*/ {
		m_sorter->evacuate_before_reporting();
	}

protected:
	virtual void set_available_memory(memory_size_type availableMemory) /*override*/ {
		pipe_segment::set_available_memory(availableMemory);
		m_sorter->set_phase_2_memory(availableMemory);
	}

private:
	sorterptr m_sorter;
	Output dest;

	void set_input_segment(pipe_segment & input, sorterptr & sorter) {
		add_dependency(input);
		m_sorter = sorter;
		dest.set_calc_segment(*this, sorter);
	}

	friend struct sort_input_t<pred_t, Output>;
};

template <typename pred_t, typename Output>
struct sort_input_t : public pipe_segment {
	typedef typename Output::item_type item_type;
	typedef merge_sorter<item_type, true, pred_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;

	inline sort_input_t(sort_calc_t<pred_t, Output> dest, const pred_t & pred)
		: dest(dest)
		, m_sorter(new sorter_t(pred))
	{
		this->dest.set_input_segment(*this, m_sorter);
		set_minimum_memory(sorter_t::minimum_memory_phase_1());
		set_name("Form input runs", PRIORITY_SIGNIFICANT);
	}

	virtual void begin() /*override*/ {
		pipe_segment::begin();
		m_sorter->begin();
	}

	inline void push(const item_type & item) {
		m_sorter->push(item);
	}

	virtual void end() /*override*/ {
		pipe_segment::end();
		m_sorter->end();
	}

	virtual bool can_evacuate() /*override*/ {
		return true;
	}

	virtual void evacuate() /*override*/ {
		m_sorter->evacuate_before_merging();
	}

protected:
	virtual void set_available_memory(memory_size_type availableMemory) /*override*/ {
		pipe_segment::set_available_memory(availableMemory);
		m_sorter->set_phase_1_memory(availableMemory);
	}

private:
	sorterptr m_sorter;
	sort_calc_t<pred_t, Output> dest;
};

struct default_pred_sort_factory : public factory_base {
	template <typename dest_t>
	struct generated {
	private:
		typedef typename dest_t::item_type item_type;
		typedef std::less<item_type> pred_type;
		typedef sort_output_t<pred_type, dest_t> Output;
	public:
		typedef sort_input_t<std::less<item_type>, Output> type;
	};

	template <typename dest_t>
	inline typename generated<dest_t>::type construct(const dest_t & dest) const {
		typedef typename dest_t::item_type item_type;
		typedef std::less<item_type> pred_type;
		typedef sort_output_t<pred_type, dest_t> Output;
		Output output(dest);
		this->init_segment(output);
		sort_calc_t<pred_type, Output> calc(output);
		this->init_segment(calc);
		sort_input_t<pred_type, Output> input(calc, pred_type());
		this->init_segment(input);
		return input;
	}
};

template <typename pred_t>
struct sort_factory : public factory_base {
	template <typename dest_t>
	struct generated {
	private:
		typedef typename dest_t::item_type item_type;
		typedef sort_output_t<pred_t, dest_t> Output;
	public:
		typedef sort_input_t<std::less<item_type>, Output> type;
	};

	template <typename dest_t>
	inline typename generated<dest_t>::type construct(const dest_t & dest) const {
		typedef typename dest_t::item_type item_type;
		typedef sort_output_t<pred_t, dest_t> Output;
		Output output(dest);
		this->init_segment(output);
		sort_calc_t<pred_t, Output> calc(output);
		this->init_segment(calc);
		sort_input_t<pred_t, Output> input(calc, pred);
		this->init_segment(input);
		return input;
	}

	sort_factory(const pred_t & p)
		: pred(p)
	{
	}

private:
	pred_t pred;
};

inline pipe_middle<default_pred_sort_factory>
pipesort() {
	typedef default_pred_sort_factory fact;
	return pipe_middle<fact>(fact()).breadcrumb("Sort");
}

template <typename pred_t>
inline pipe_middle<sort_factory<pred_t> >
pipesort(const pred_t & p) {
	typedef sort_factory<pred_t> fact;
	return pipe_middle<fact>(fact(p)).breadcrumb("Sort");
}

template <typename T, typename pred_t>
struct passive_sorter_factory : public factory_base {
	typedef sort_pull_output_t<T, pred_t> output_t;
	typedef sort_calc_t<pred_t, output_t> calc_t;
	typedef sort_input_t<pred_t, output_t> input_t;
	typedef input_t generated_type;

	passive_sorter_factory(const output_t & output)
		: m_output(output)
	{
	}

	inline generated_type construct() const {
		calc_t calc(m_output);
		this->init_segment(calc);
		input_t input(calc, pred_t());
		this->init_segment(input);
		return input;
	}

private:
	output_t m_output;
};

template <typename T, typename pred_t>
struct passive_sorter {
	typedef T item_type;
	typedef merge_sorter<item_type, true, pred_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;
	typedef sort_pull_output_t<item_type, pred_t> output_t;
	typedef sort_calc_t<pred_t, output_t> calc_t;
	typedef sort_input_t<pred_t, output_t> input_t;

	inline passive_sorter()
		: pred()
		, m_output()
	{
	}

	inline pipe_end<passive_sorter_factory<item_type, pred_t> > input() {
		return passive_sorter_factory<item_type, pred_t>(m_output);
	}

	inline output_t output() {
		return m_output;
	}

private:
	pred_t pred;
	output_t m_output;
	passive_sorter(const passive_sorter &);
	passive_sorter & operator=(const passive_sorter &);
};

}

}

#endif
