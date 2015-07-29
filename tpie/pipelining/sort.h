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

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_base.h>
#include <tpie/pipelining/merge_sorter.h>
#include <tpie/parallel_sort.h>
#include <tpie/file_stream.h>
#include <tpie/tempname.h>
#include <tpie/memory.h>
#include <queue>
#include <memory>

namespace tpie {

namespace pipelining {

namespace bits {

template <typename T, typename pred_t, typename store_t>
class sort_calc_t;

template <typename T, typename pred_t, typename store_t>
class sort_input_t;

template <typename T, typename pred_t, typename store_t>
class sort_output_base : public node {
	// node has virtual dtor
public:
	/** Type of items sorted. */
	typedef T item_type;
	
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<T, true, pred_t, store_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;

	sorterptr get_sorter() const {
		return m_sorter;
	}

	void set_calc_node(node & calc) {
		add_dependency(calc);
	}

	virtual void propagate() override {
		set_steps(m_sorter->item_count());
		forward("items", static_cast<stream_size_type>(m_sorter->item_count()));
		memory_size_type memory_usage = m_sorter->actual_memory_phase_3();
		set_minimum_memory(memory_usage);
		set_maximum_memory(memory_usage);
		set_memory_fraction(0);
		m_propagate_called = true;
	}

protected:
	virtual void set_available_memory(memory_size_type availableMemory) override {
		node::set_available_memory(availableMemory);
		if (!m_propagate_called)
			m_sorter->set_phase_3_memory(availableMemory);
	}

	sort_output_base(pred_t pred, store_t store)
		: m_sorter(new sorter_t(pred, store))
		, m_propagate_called(false)
	{
	}

	sorterptr m_sorter;
	bool m_propagate_called;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipe sorter pull output node.
/// \tparam pred_t   The less-than predicate.
/// \tparam dest_t   Destination node type.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t, typename store_t>
class sort_pull_output_t : public sort_output_base<T, pred_t, store_t> {
public:
	/** Type of items sorted. */
	typedef T item_type;
	
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<item_type, true, pred_t, store_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;

	sort_pull_output_t(pred_t pred, store_t store)
		: sort_output_base<T, pred_t, store_t>(pred, store)
	{
		this->set_minimum_memory(sorter_t::minimum_memory_phase_3());
		this->set_maximum_memory(sorter_t::maximum_memory_phase_3());
		this->set_name("Write sorted output", PRIORITY_INSIGNIFICANT);
		this->set_memory_fraction(1.0);
		this->set_plot_options(node::PLOT_BUFFERED);
	}

	bool can_pull() const {
		return this->m_sorter->can_pull();
	}

	item_type pull() {
		this->step();
		return this->m_sorter->pull();
	}

	// Despite this go() implementation, a sort_pull_output_t CANNOT be used as
	// an initiator node. Normally, it is a type error to have a phase without
	// an initiator, but with a passive_sorter you can circumvent this
	// mechanism. Thus we customize the error message printed (but throw the
	// same type of exception.)
	virtual void go() override {
		log_warning() << "Passive sorter used without an initiator in the final merge and output phase.\n"
			<< "Define an initiator and pair it up with the pipe from passive_sorter::output()." << std::endl;
		throw not_initiator_node();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipe sorter push output node.
/// \tparam pred_t   The less-than predicate.
/// \tparam dest_t   Destination node type.
///////////////////////////////////////////////////////////////////////////////
template <typename pred_t, typename dest_t, typename store_t>
class sort_output_t : public sort_output_base<typename push_type<dest_t>::type, pred_t, store_t> {
public:
	/** Type of items sorted. */
	typedef typename push_type<dest_t>::type item_type;
	
	/** Base class */
	typedef sort_output_base<item_type, pred_t, store_t> p_t;
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<item_type, true, pred_t, store_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;

	inline sort_output_t(const dest_t & dest, pred_t pred, store_t store)
		: p_t(pred, store)
		, dest(dest)
	{
		this->add_push_destination(dest);
		this->set_minimum_memory(sorter_t::minimum_memory_phase_3());
		this->set_maximum_memory(sorter_t::maximum_memory_phase_3());
		this->set_name("Write sorted output", PRIORITY_INSIGNIFICANT);
		this->set_memory_fraction(1.0);
		this->set_plot_options(node::PLOT_BUFFERED);
	}

	virtual void go() override {
		while (this->m_sorter->can_pull()) {
			item_type && y=this->m_sorter->pull();
			dest.push(std::move(y));
			this->step();
		}
	}
private:
	dest_t dest;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipe sorter middle node.
/// \tparam T        The type of items sorted
/// \tparam pred_t   The less-than predicate
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t, typename store_t>
class sort_calc_t : public node {
public:
	/** Type of items sorted. */
	typedef T item_type;
	
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<item_type, true, pred_t, store_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;

	typedef sort_output_base<T, pred_t, store_t> Output;

	inline sort_calc_t(const sort_calc_t & other)
		: node(other)
		, m_sorter(other.m_sorter)
		, m_propagate_called(other.m_propagate_called)
		, dest(other.dest)
	{
	}

	template <typename dest_t>
	inline sort_calc_t(dest_t dest)
		: dest(new dest_t(dest))
	{
		m_sorter = this->dest->get_sorter();
		this->dest->set_calc_node(*this);
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
		set_plot_options(PLOT_BUFFERED | PLOT_SIMPLIFIED_HIDE);
		m_propagate_called = false;
	}

	virtual void propagate() override {
		set_steps(1000);
		m_propagate_called = true;
	}

	virtual void go() override {
		progress_indicator_base * pi = proxy_progress_indicator();
		m_sorter->calc(*pi);
	}

	virtual bool can_evacuate() override {
		return true;
	}

	virtual void evacuate() override {
		m_sorter->evacuate_before_reporting();
	}

	sorterptr get_sorter() const {
		return m_sorter;
	}

	void set_input_node(node & input) {
		add_dependency(input);
	}

protected:
	virtual void set_available_memory(memory_size_type availableMemory) override {
		node::set_available_memory(availableMemory);
		if (!m_propagate_called)
			m_sorter->set_phase_2_memory(availableMemory);
	}

private:
	sorterptr m_sorter;
	bool m_propagate_called;
	std::shared_ptr<Output> dest;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipe sorter input node.
/// \tparam T        The type of items sorted
/// \tparam pred_t   The less-than predicate
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t, typename store_t>
class sort_input_t : public node {
public:
	/** Type of items sorted. */
	typedef T item_type;
	
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<item_type, true, pred_t, store_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;

	inline sort_input_t(sort_calc_t<T, pred_t, store_t> dest)
		: m_sorter(dest.get_sorter())
		, m_propagate_called(false)
		, dest(dest)
	{
		this->dest.set_input_node(*this);
		set_minimum_memory(sorter_t::minimum_memory_phase_1());
		set_name("Form input runs", PRIORITY_SIGNIFICANT);
		set_memory_fraction(1.0);
		set_plot_options(PLOT_BUFFERED | PLOT_SIMPLIFIED_HIDE);
	}

	virtual void propagate() override {
		if (this->can_fetch("items"))
			m_sorter->set_items(this->fetch<stream_size_type>("items"));
		m_sorter->begin();
		m_propagate_called = true;
	}

	void push(item_type && item) {
		m_sorter->push(std::move(item));
	}

	void push(const item_type & item) {
		m_sorter->push(item);
	}
	
	virtual void end() override {
		node::end();
		m_sorter->end();
	}

	virtual bool can_evacuate() override {
		return true;
	}

	virtual void evacuate() override {
		m_sorter->evacuate_before_merging();
	}

protected:
	virtual void set_available_memory(memory_size_type availableMemory) override {
		node::set_available_memory(availableMemory);
		if (!m_propagate_called)
			m_sorter->set_phase_1_memory(availableMemory);
	}
private:
	sorterptr m_sorter;
	bool m_propagate_called;
	sort_calc_t<T, pred_t, store_t> dest;
};

template <typename child_t, typename store_t>
class sort_factory_base : public factory_base {
	const child_t & self() const { return *static_cast<const child_t *>(this); }
public:
	template <typename dest_t>
	struct constructed {
	private:
		/** Type of items sorted. */
		typedef typename push_type<dest_t>::type item_type;
		typedef typename store_t::template element_type<item_type>::type element_type;
	public:
		typedef typename child_t::template predicate<element_type>::type pred_type;
		typedef sort_input_t<item_type, pred_type, store_t> type;
	};
	
	template <typename dest_t>
	typename constructed<dest_t>::type construct(const dest_t & dest) const {
		typedef typename push_type<dest_t>::type item_type;
		typedef typename store_t::template element_type<item_type>::type element_type;
		typedef typename constructed<dest_t>::pred_type pred_type;

		sort_output_t<pred_type, dest_t, store_t> output(
			dest, 
			self().template get_pred<element_type>(), 
			m_store);
		this->init_sub_node(output);
		sort_calc_t<item_type, pred_type, store_t> calc(output);
		this->init_sub_node(calc);
		sort_input_t<item_type, pred_type, store_t> input(calc);
		this->init_sub_node(input);

		return input;
	}

	sort_factory_base(store_t store): m_store(store) {}
private:
	store_t m_store;

};

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort factory using std::less<T> as comparator.
///////////////////////////////////////////////////////////////////////////////
template <typename store_t>
class default_pred_sort_factory : public sort_factory_base<default_pred_sort_factory<store_t>, store_t> {
public:
	template <typename item_type>
	class predicate {
	public:
		typedef std::less<item_type> type;
	};
	
	template <typename T>
	std::less<T> get_pred() const {
		return std::less<T>();
	}

	default_pred_sort_factory(const store_t & store)
		: sort_factory_base<default_pred_sort_factory<store_t>, store_t>(store) 
	{
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Sort factory using the given predicate as comparator.
///////////////////////////////////////////////////////////////////////////////
template <typename pred_t, typename store_t>
class sort_factory : public sort_factory_base<sort_factory<pred_t, store_t>, store_t> {
public:
	template <typename Dummy>
	class predicate {
	public:
		typedef pred_t type;
	};

	sort_factory(const pred_t & p, const store_t & store)
		: sort_factory_base<sort_factory<pred_t, store_t>, store_t>(store)
		, pred(p)
	{
	}

	template <typename T>
	pred_t get_pred() const {
		return pred;
	}
private:
	pred_t pred;
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining sorter using std::less.
///////////////////////////////////////////////////////////////////////////////
inline pipe_middle<bits::default_pred_sort_factory<default_store> >
sort() {
	typedef bits::default_pred_sort_factory<default_store> fact;
	return pipe_middle<fact>(fact(default_store())).name("Sort");
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that sorts large elements indirectly by using 
/// a store and std::less.
///////////////////////////////////////////////////////////////////////////////
template <typename store_t>
inline pipe_middle<bits::default_pred_sort_factory<store_t> >
store_sort(store_t store=store_t()) {
	typedef bits::default_pred_sort_factory<store_t> fact;
	return pipe_middle<fact>(fact(store)).name("Sort");
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining sorter using the given predicate.
///////////////////////////////////////////////////////////////////////////////
template <typename pred_t>
inline pipe_middle<bits::sort_factory<pred_t, default_store> >
sort(const pred_t & p) {
	typedef bits::sort_factory<pred_t, default_store> fact;
	return pipe_middle<fact>(fact(p, default_store())).name("Sort");
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that sorts large elements indirectly by using 
/// a storeand a given predicate.
///////////////////////////////////////////////////////////////////////////////
template <typename pred_t, typename store_t>
inline pipe_middle<bits::sort_factory<pred_t, store_t> >
sort(const pred_t & p, store_t store) {
	typedef bits::sort_factory<pred_t, store_t> fact;
	return pipe_middle<fact>(fact(p, store)).name("Sort");
}

template <typename T, typename pred_t=std::less<T>, typename store_t=default_store>
class passive_sorter;

namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \brief Factory for the passive sorter input node.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t, typename store_t>
class passive_sorter_factory : public factory_base {
public:
	typedef sort_pull_output_t<T, pred_t, store_t> output_t;
	typedef sort_calc_t<T, pred_t, store_t> calc_t;
	typedef sort_input_t<T, pred_t, store_t> input_t;
	typedef input_t constructed_type;
	typedef merge_sorter<T, true, pred_t, store_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;

	passive_sorter_factory(output_t & output)
		: output(&output)
	{
	}

	constructed_type construct() const {
		calc_t calc(output->get_sorter());
		output->set_calc_node(calc);
		this->init_node(calc);
		input_t input(calc);
		this->init_node(input);
		return input;
	}

private:
	output_t * output;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Factory for the passive sorter output node.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t, typename store_t>
class passive_sorter_factory_2 : public factory_base {
public:
	typedef sort_pull_output_t<T, pred_t, store_t> output_t;
	typedef output_t constructed_type;

	passive_sorter_factory_2(const passive_sorter<T, pred_t, store_t> & sorter)
		: m_sorter(sorter)
	{
	}

	constructed_type construct() const;

private:
	const passive_sorter<T, pred_t, store_t> & m_sorter;
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
template <typename T, typename pred_t, typename store_t>
class passive_sorter {
public:
	/** Type of items sorted. */
	typedef T item_type;
	/** Type of the merge sort implementation used. */
	typedef merge_sorter<item_type, true, pred_t, store_t> sorter_t;
	/** Smart pointer to sorter_t. */
	typedef typename sorter_t::ptr sorterptr;
	/** Type of pipe sorter output. */
	typedef bits::sort_pull_output_t<item_type, pred_t, store_t> output_t;

	inline passive_sorter(pred_t pred = pred_t(),
						  store_t store = store_t())
		: m_sorter(new sorter_t())
		, m_output(pred, store)
	{
	}

	typedef pipe_end<bits::passive_sorter_factory<item_type, pred_t, store_t> > input_pipe_t;
	typedef pullpipe_begin<bits::passive_sorter_factory_2<item_type, pred_t, store_t> > output_pipe_t;
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the input push node.
	///////////////////////////////////////////////////////////////////////////
	inline input_pipe_t input() {
		return bits::passive_sorter_factory<item_type, pred_t, store_t>(m_output);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the output pull node.
	///////////////////////////////////////////////////////////////////////////
	inline output_pipe_t output() {
		return bits::passive_sorter_factory_2<item_type, pred_t, store_t>(*this);
	}
	
private:
	sorterptr m_sorter;
	output_t m_output;
	passive_sorter(const passive_sorter &);
	passive_sorter & operator=(const passive_sorter &);

	friend class bits::passive_sorter_factory_2<T, pred_t, store_t>;
};


namespace bits {

template <typename T, typename pred_t, typename store_t>
typename passive_sorter_factory_2<T, pred_t, store_t>::constructed_type
passive_sorter_factory_2<T, pred_t, store_t>::construct() const {
	constructed_type res = m_sorter.m_output;
	init_node(res);
	return res;
}

} // namespace bits

} // namespace pipelining

} // namespace tpie

#endif
