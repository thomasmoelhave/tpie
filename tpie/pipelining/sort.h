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

#include <tpie/pipelining/pipe_segment.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_base.h>
#include <tpie/pipelining/merge_sorter.h>
#include <tpie/parallel_sort.h>
#include <tpie/file_stream.h>
#include <tpie/tempname.h>
#include <tpie/memory.h>
#include <queue>
#include <boost/shared_ptr.hpp>

namespace tpie {

namespace pipelining {

namespace bits {

template <typename T, typename pred_t>
struct sort_calc_t;

template <typename T, typename pred_t>
struct sort_input_t;

template <typename T, typename pred_t>
class sort_output_base : public pipe_segment {
	// pipe_segment has virtual dtor
public:
	/** Type of items sorted. */
	typedef T item_type;
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<item_type, true, pred_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;

	sorterptr get_sorter() const {
		return m_sorter;
	}

	void set_calc_segment(pipe_segment & calc) {
		add_dependency(calc);
	}

protected:
	sort_output_base(pred_t pred)
		: m_sorter(new sorter_t(pred))
	{
	}

	sorterptr m_sorter;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipe sorter pull output segment.
/// \tparam pred_t   The less-than predicate.
/// \tparam dest_t   Destination segment type.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t>
struct sort_pull_output_t : public sort_output_base<T, pred_t> {
	/** Type of items sorted. */
	typedef T item_type;
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<item_type, true, pred_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;

	inline sort_pull_output_t(pred_t pred)
		: sort_output_base<T, pred_t>(pred)
	{
		this->set_minimum_memory(sorter_t::minimum_memory_phase_3());
		this->set_name("Write sorted output", PRIORITY_INSIGNIFICANT);
		this->set_memory_fraction(1.0);
	}

	virtual void begin() /*override*/ {
		pipe_segment::begin();
		this->set_steps(this->m_sorter->item_count());
		this->forward("items", static_cast<stream_size_type>(this->m_sorter->item_count()));
	}

	inline bool can_pull() const {
		return this->m_sorter->can_pull();
	}

	inline item_type pull() {
		this->step();
		return this->m_sorter->pull();
	}

protected:
	virtual void set_available_memory(memory_size_type availableMemory) /*override*/ {
		pipe_segment::set_available_memory(availableMemory);
		this->m_sorter->set_phase_3_memory(availableMemory);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipe sorter push output segment.
/// \tparam pred_t   The less-than predicate.
/// \tparam dest_t   Destination segment type.
///////////////////////////////////////////////////////////////////////////////
template <typename pred_t, typename dest_t>
struct sort_output_t : public sort_output_base<typename dest_t::item_type, pred_t> {
	/** Type of items sorted. */
	typedef typename dest_t::item_type item_type;
	/** Base class */
	typedef sort_output_base<item_type, pred_t> p_t;
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<item_type, true, pred_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;

	inline sort_output_t(const dest_t & dest, pred_t pred)
		: p_t(pred)
		, dest(dest)
	{
		this->add_push_destination(dest);
		this->set_minimum_memory(sorter_t::minimum_memory_phase_3());
		this->set_name("Write sorted output", PRIORITY_INSIGNIFICANT);
		this->set_memory_fraction(1.0);
	}

	virtual void begin() /*override*/ {
		pipe_segment::begin();
		this->set_steps(this->m_sorter->item_count());
		this->forward("items", static_cast<stream_size_type>(this->m_sorter->item_count()));
	}

	virtual void go() /*override*/ {
		while (this->m_sorter->can_pull()) {
			dest.push(this->m_sorter->pull());
			this->step();
		}
	}

protected:
	virtual void set_available_memory(memory_size_type availableMemory) /*override*/ {
		pipe_segment::set_available_memory(availableMemory);
		this->m_sorter->set_phase_3_memory(availableMemory);
	}

private:
	dest_t dest;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipe sorter middle segment.
/// \tparam T        The type of items sorted
/// \tparam pred_t   The less-than predicate
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t>
struct sort_calc_t : public pipe_segment {
	/** Type of items sorted. */
	typedef T item_type;
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<item_type, true, pred_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;

	typedef sort_output_base<T, pred_t> Output;

	inline sort_calc_t(const sort_calc_t & other)
		: pipe_segment(other)
		, m_sorter(other.m_sorter)
		, dest(other.dest)
	{
	}

	template <typename dest_t>
	inline sort_calc_t(dest_t dest)
		: dest(new dest_t(dest))
	{
		m_sorter = this->dest->get_sorter();
		this->dest->set_calc_segment(*this);
		init();
	}

	inline sort_calc_t(sorterptr sorter)
		: m_sorter(sorter)
	{
		init();
	}

	void init() {
		set_minimum_memory(sorter_t::minimum_memory_phase_2());
		set_name("Perform merge heap", PRIORITY_SIGNIFICANT);
		set_memory_fraction(1.0);
	}

	virtual void begin() /*override*/ {
		pipe_segment::begin();
		set_steps(1000);
	}

	virtual void go() /*override*/ {
		progress_indicator_base * pi = proxy_progress_indicator();
		m_sorter->calc(*pi);
	}

	virtual bool can_evacuate() /*override*/ {
		return true;
	}

	virtual void evacuate() /*override*/ {
		m_sorter->evacuate_before_reporting();
	}

	sorterptr get_sorter() const {
		return m_sorter;
	}

	void set_input_segment(pipe_segment & input) {
		add_dependency(input);
	}

protected:
	virtual void set_available_memory(memory_size_type availableMemory) /*override*/ {
		pipe_segment::set_available_memory(availableMemory);
		m_sorter->set_phase_2_memory(availableMemory);
	}

private:
	sorterptr m_sorter;
	boost::shared_ptr<Output> dest;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipe sorter input segment.
/// \tparam T        The type of items sorted
/// \tparam pred_t   The less-than predicate
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t>
struct sort_input_t : public pipe_segment {
	/** Type of items sorted. */
	typedef T item_type;
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<item_type, true, pred_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;

	inline sort_input_t(sort_calc_t<T, pred_t> dest)
		: m_sorter(dest.get_sorter())
		, dest(dest)
	{
		this->dest.set_input_segment(*this);
		set_minimum_memory(sorter_t::minimum_memory_phase_1());
		set_name("Form input runs", PRIORITY_SIGNIFICANT);
		set_memory_fraction(1.0);
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
	sort_calc_t<T, pred_t> dest;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort factory using std::less<T> as comparator.
///////////////////////////////////////////////////////////////////////////////
struct default_pred_sort_factory : public factory_base {

	///////////////////////////////////////////////////////////////////////////
	/// \brief Declare generated type based on destination type.
	///////////////////////////////////////////////////////////////////////////
	template <typename dest_t>
	struct generated {
	private:
		/** Type of items sorted. */
		typedef typename dest_t::item_type item_type;
	public:
		typedef sort_input_t<item_type, std::less<item_type> > type;
	};

	template <typename dest_t>
	inline typename generated<dest_t>::type construct(const dest_t & dest) const {
		typedef typename dest_t::item_type item_type;
		typedef std::less<item_type> pred_type;
		sort_output_t<pred_type, dest_t> output(dest, pred_type());
		this->init_segment(output);
		sort_calc_t<item_type, pred_type> calc(output);
		this->init_segment(calc);
		sort_input_t<item_type, pred_type> input(calc);
		this->init_segment(input);
		return input;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort factory using the given predicate as comparator.
///////////////////////////////////////////////////////////////////////////////
template <typename pred_t>
struct sort_factory : public factory_base {

	///////////////////////////////////////////////////////////////////////////
	/// \brief Declare generated type based on destination type.
	///////////////////////////////////////////////////////////////////////////
	template <typename dest_t>
	struct generated {
		typedef sort_input_t<typename dest_t::item_type, pred_t> type;
	};

	template <typename dest_t>
	inline typename generated<dest_t>::type construct(const dest_t & dest) const {
		typedef typename dest_t::item_type item_type;
		sort_output_t<pred_t, dest_t> output(dest);
		this->init_segment(output);
		sort_calc_t<item_type, pred_t> calc(output);
		this->init_segment(calc);
		sort_input_t<item_type, pred_t> input(calc, pred);
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

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining sorter using std::less.
///////////////////////////////////////////////////////////////////////////////
inline pipe_middle<bits::default_pred_sort_factory>
pipesort() {
	typedef bits::default_pred_sort_factory fact;
	return pipe_middle<fact>(fact()).breadcrumb("Sort");
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining sorter using the given predicate.
///////////////////////////////////////////////////////////////////////////////
template <typename pred_t>
inline pipe_middle<bits::sort_factory<pred_t> >
pipesort(const pred_t & p) {
	typedef bits::sort_factory<pred_t> fact;
	return pipe_middle<fact>(fact(p)).breadcrumb("Sort");
}

template <typename T, typename pred_t>
class passive_sorter;

namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \brief Factory for the passive sorter input segment.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t>
struct passive_sorter_factory : public factory_base {
	typedef sort_pull_output_t<T, pred_t> output_t;
	typedef sort_calc_t<T, pred_t> calc_t;
	typedef sort_input_t<T, pred_t> input_t;
	typedef input_t generated_type;
	typedef merge_sorter<T, true, pred_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;

	passive_sorter_factory(output_t & output)
		: output(&output)
	{
	}

	inline generated_type construct() const {
		calc_t calc(output->get_sorter());
		output->set_calc_segment(calc);
		this->init_segment(calc);
		input_t input(calc);
		this->init_segment(input);
		return input;
	}

private:
	output_t * output;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Factory for the passive sorter output segment.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t>
struct passive_sorter_factory_2 : public factory_base {
	typedef sort_pull_output_t<T, pred_t> output_t;
	typedef output_t generated_type;

	passive_sorter_factory_2(const passive_sorter<T, pred_t> & sorter)
		: m_sorter(sorter)
	{
	}

	inline generated_type construct() const;

private:
	const passive_sorter<T, pred_t> & m_sorter;
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelined sorter with push input and pull output.
/// Get the input pipe with \c input() and the output pullpipe with \c output().
/// input() must not be called after output().
/// \tparam T The type of item to sort
/// \tparam pred_t The predicate (e.g. std::less<T>) indicating the predicate
/// on which to order an item before another.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t>
class passive_sorter {
public:
	/** Type of items sorted. */
	typedef T item_type;
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<item_type, true, pred_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;
	/** Type of pipe sorter output. */
	typedef bits::sort_pull_output_t<item_type, pred_t> output_t;

	inline passive_sorter(pred_t pred = pred_t())
		: m_sorter(new sorter_t())
		, pred(pred)
		, m_output(pred)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the input push pipe segment.
	///////////////////////////////////////////////////////////////////////////
	inline pipe_end<bits::passive_sorter_factory<item_type, pred_t> > input() {
		return bits::passive_sorter_factory<item_type, pred_t>(m_output);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the output pull pipe segment.
	///////////////////////////////////////////////////////////////////////////
	inline pullpipe_begin<bits::passive_sorter_factory_2<item_type, pred_t> > output() {
		return bits::passive_sorter_factory_2<item_type, pred_t>(*this);
	}

private:
	sorterptr m_sorter;
	pred_t pred;
	output_t m_output;
	passive_sorter(const passive_sorter &);
	passive_sorter & operator=(const passive_sorter &);

	friend struct bits::passive_sorter_factory_2<T, pred_t>;
};

namespace bits {

template <typename T, typename pred_t>
typename passive_sorter_factory_2<T, pred_t>::generated_type
passive_sorter_factory_2<T, pred_t>::construct() const {
	generated_type res = m_sorter.m_output;
	init_segment(res);
	return res;
}

} // namespace bits

} // namespace pipelining

} // namespace tpie

#endif
