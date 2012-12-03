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

#ifndef __TPIE_PIPELINING_PIPE_SEGMENT_H__
#define __TPIE_PIPELINING_PIPE_SEGMENT_H__

#include <tpie/pipelining/exception.h>
#include <tpie/pipelining/tokens.h>
#include <tpie/progress_indicator_base.h>
#include <tpie/progress_indicator_null.h>
#include <boost/any.hpp>
#include <tpie/pipelining/priority_type.h>

namespace tpie {

namespace pipelining {

struct pipe_segment;

namespace bits {

class proxy_progress_indicator : public tpie::progress_indicator_base {
	pipe_segment & m_segment;

public:
	proxy_progress_indicator(pipe_segment & s)
		: progress_indicator_base(1)
		, m_segment(s)
	{
	}

	inline void refresh();
};

class phase;

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// Base class of all segments. A segment should inherit from pipe_segment,
/// have a single template parameter dest_t if it is not a terminus segment,
/// and implement methods begin(), push() and end(), if it is not a source
/// segment.
///////////////////////////////////////////////////////////////////////////////
struct pipe_segment {
	///////////////////////////////////////////////////////////////////////////
	/// \brief Virtual dtor.
	///////////////////////////////////////////////////////////////////////////
	virtual ~pipe_segment() {}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the minimum amount of memory declared by this pipe_segment.
	/// Defaults to zero when no minimum has been set.
	///////////////////////////////////////////////////////////////////////////
	inline memory_size_type get_minimum_memory() const {
		return m_minimumMemory;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the amount of memory assigned to this pipe_segment.
	///////////////////////////////////////////////////////////////////////////
	inline memory_size_type get_available_memory() const {
		return m_availableMemory;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Set the memory priority of this segment. Memory is distributed
	/// proportionally to the priorities of the segments in the given phase.
	///////////////////////////////////////////////////////////////////////////
	inline void set_memory_fraction(double f) {
		m_memoryFraction = f;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the memory priority of this segment.
	///////////////////////////////////////////////////////////////////////////
	inline double get_memory_fraction() const {
		return m_memoryFraction;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the local segment map, mapping segment IDs to pipe_segment
	/// pointers for all the pipe_segments reachable from this one.
	///////////////////////////////////////////////////////////////////////////
	inline bits::segment_map::ptr get_segment_map() const {
		return token.get_map();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the internal pipe_segment ID of this pipe_segment (mainly
	/// for debugging purposes).
	///////////////////////////////////////////////////////////////////////////
	inline segment_token::id_t get_id() const {
		return token.id();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called before memory assignment but after depending phases have
	/// executed and ended. The implementer may use fetch and forward in this
	/// phase. The implementer does not have to call the super prepare-method;
	/// its default implementation is empty.
	///////////////////////////////////////////////////////////////////////////
	virtual void prepare() {
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Begin pipeline processing phase.
	///
	/// The implementation may pull() from a pull destination in begin(),
	/// but it is not allowed to push() to a push destination.
	///
	/// The pipelining framework calls begin() on the pipe_segments in the
	/// pipeline graph in a topological order. The framework calls
	/// pipe_segment::begin() on a pipe_segment after its pull destinations and
	/// before its push destination.
	///
	/// The default implementation just calls forward_all(), and an
	/// implementation is not required to call the parent begin() if this is
	/// not the wanted behavior.
	///////////////////////////////////////////////////////////////////////////
	virtual void begin() {
		forward_all();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief For initiator segments, execute this phase by pushing all items
	/// to be pushed. For non-initiator segments, the default implementation
	/// throws a not_initiator_segment exception.
	///////////////////////////////////////////////////////////////////////////
	virtual void go() {
		progress_indicator_null pi;
		go(pi);
		// if go didn't throw, it was overridden - but it shouldn't be
		log_warning() << "pipe_segment subclass " << typeid(*this).name() << " uses old go() interface" << std::endl;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Deprecated go()-implementation signature. The progress indicator
	/// argument does nothing. Instead, use step() and set_steps().
	///////////////////////////////////////////////////////////////////////////
	virtual void go(progress_indicator_base &) {
		log_warning() << "pipe_segment subclass " << typeid(*this).name() << " is not an initiator segment" << std::endl;
		throw not_initiator_segment();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief End pipeline processing phase.
	///
	/// The implementation may pull() from a pull destination in end(),
	/// and it may push() to a push destination.
	///
	/// The pipelining framework calls end() on the pipe_segments in the
	/// pipeline graph in a topological order. The framework calls
	/// pipe_segment::end() on a pipe_segment before its pull and push
	/// destinations.
	///
	/// The default implementation does nothing, so it does not matter if the
	/// implementation calls the parent end().
	///////////////////////////////////////////////////////////////////////////
	virtual void end() {
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Overridden by pipe segments that have data to evacuate.
	///////////////////////////////////////////////////////////////////////////
	virtual bool can_evacuate() {
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Overridden by pipe segments that have data to evacuate.
	///////////////////////////////////////////////////////////////////////////
	virtual void evacuate() {
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the priority of this segment's name. For purposes of
	/// pipeline debugging and phase naming for progress indicator breadcrumbs.
	///////////////////////////////////////////////////////////////////////////
	inline priority_type get_name_priority() {
		return m_namePriority;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get this segment's name. For purposes of pipeline debugging and
	/// phase naming for progress indicator breadcrumbs.
	///////////////////////////////////////////////////////////////////////////
	inline const std::string & get_name() {
		return m_name;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Set this segment's name. For purposes of pipeline debugging and
	/// phase naming for progress indicator breadcrumbs.
	///////////////////////////////////////////////////////////////////////////
	inline void set_name(const std::string & name, priority_type priority = PRIORITY_USER) {
		m_name = name;
		m_namePriority = priority;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Used internally when a pair_factory has a name set.
	///////////////////////////////////////////////////////////////////////////
	inline void set_breadcrumb(const std::string & breadcrumb) {
		m_name = m_name.empty() ? breadcrumb : (breadcrumb + " | " + m_name);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Used internally to facilitate forwarding parameters to
	/// successors in the item flow graph. Called by
	/// segment_map::send_successors.
	///////////////////////////////////////////////////////////////////////////
	inline void add_successor(pipe_segment * succ) {
		m_successors.push_back(succ);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Used internally for progress indication. Get the number of times
	/// the pipe_segment expects to call step() at most.
	///////////////////////////////////////////////////////////////////////////
	inline stream_size_type get_steps() {
		return m_stepsTotal;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Used internally for progress indication. Set the progress
	/// indicator to use.
	///////////////////////////////////////////////////////////////////////////
	inline void set_progress_indicator(progress_indicator_base * pi) {
		m_pi = pi;
	}

protected:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor, using a new segment_token.
	///////////////////////////////////////////////////////////////////////////
	inline pipe_segment()
		: token(this)
		, m_minimumMemory(0)
		, m_availableMemory(0)
		, m_memoryFraction(0.0)
		, m_namePriority(PRIORITY_NO_NAME)
		, m_stepsTotal(0)
		, m_stepsLeft(0)
		, m_pi(0)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Copy constructor. We need to define this explicitly since the
	/// segment_token needs to know its new owner.
	///////////////////////////////////////////////////////////////////////////
	inline pipe_segment(const pipe_segment & other)
		: token(other.token, this)
		, m_minimumMemory(other.m_minimumMemory)
		, m_availableMemory(other.m_availableMemory)
		, m_memoryFraction(other.m_memoryFraction)
		, m_name(other.m_name)
		, m_namePriority(other.m_namePriority)
		, m_stepsTotal(other.m_stepsTotal)
		, m_stepsLeft(other.m_stepsLeft)
		, m_pi(other.m_pi)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor using a given fresh segment_token.
	///////////////////////////////////////////////////////////////////////////
	inline pipe_segment(const segment_token & token)
		: token(token, this, true)
		, m_minimumMemory(0)
		, m_availableMemory(0)
		, m_memoryFraction(0.0)
		, m_namePriority(PRIORITY_NO_NAME)
		, m_stepsTotal(0)
		, m_stepsLeft(0)
		, m_pi(0)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a push destination.
	///////////////////////////////////////////////////////////////////////////
	inline void add_push_destination(const segment_token & dest) {
		bits::segment_map::ptr m = token.map_union(dest);
		m->add_relation(token.id(), dest.id(), bits::pushes);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a push destination.
	///////////////////////////////////////////////////////////////////////////
	inline void add_push_destination(const pipe_segment & dest) {
		add_push_destination(dest.token);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a pull destination.
	///////////////////////////////////////////////////////////////////////////
	inline void add_pull_destination(const segment_token & dest) {
		bits::segment_map::ptr m = token.map_union(dest);
		m->add_relation(token.id(), dest.id(), bits::pulls);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a pull destination.
	///////////////////////////////////////////////////////////////////////////
	inline void add_pull_destination(const pipe_segment & dest) {
		add_pull_destination(dest.token);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a segment dependency, that is,
	/// a requirement that another segment has end() called before the begin()
	/// of this segment.
	///////////////////////////////////////////////////////////////////////////
	inline void add_dependency(const segment_token & dest) {
		bits::segment_map::ptr m = token.map_union(dest);
		m->add_relation(token.id(), dest.id(), bits::depends);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a segment dependency, that is,
	/// a requirement that another segment has end() called before the begin()
	/// of this segment.
	///////////////////////////////////////////////////////////////////////////
	inline void add_dependency(const pipe_segment & dest) {
		add_dependency(dest.token);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare minimum memory requirements.
	///////////////////////////////////////////////////////////////////////////
	inline void set_minimum_memory(memory_size_type minimumMemory) {
		m_minimumMemory = minimumMemory;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by the memory manager to set the amount of memory
	/// assigned to this pipe_segment.
	///////////////////////////////////////////////////////////////////////////
	virtual void set_available_memory(memory_size_type availableMemory) {
		m_availableMemory = availableMemory;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to forward auxiliary data to successors.
	///////////////////////////////////////////////////////////////////////////
	template <typename T>
	inline void forward(std::string key, T value) {
		for (size_t i = 0; i < m_successors.size(); ++i) {
			m_successors[i]->m_values[key] = value;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to forward all auxiliary data that was
	/// forwarded to this segment. Called in the default implementation of
	/// begin().
	///////////////////////////////////////////////////////////////////////////
	inline void forward_all() {
		for (valuemap::iterator i = m_values.begin(); i != m_values.end(); ++i) {
			forward(i->first, i->second);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Find out if there is a piece of auxiliary data forwarded with a
	/// given name.
	///////////////////////////////////////////////////////////////////////////
	inline bool can_fetch(std::string key) {
		return m_values.count(key) != 0;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Fetch piece of auxiliary data as boost::any (the internal
	/// representation).
	///////////////////////////////////////////////////////////////////////////
	inline boost::any fetch_any(std::string key) {
		return m_values[key];
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Fetch piece of auxiliary data, expecting a given value type.
	///////////////////////////////////////////////////////////////////////////
	template <typename T>
	inline T fetch(std::string key) {
		return boost::any_cast<T>(m_values[key]);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the segment_token that maps this segment's ID to a pointer
	/// to this.
	///////////////////////////////////////////////////////////////////////////
	const segment_token & get_token() {
		return token;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers that intend to call step().
	/// \param steps  The number of times step() is called at most.
	///////////////////////////////////////////////////////////////////////////
	void set_steps(stream_size_type steps) {
		m_stepsTotal = m_stepsLeft = steps;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Step the progress indicator.
	/// \param steps  How many steps to step.
	///////////////////////////////////////////////////////////////////////////
	void step(stream_size_type steps = 1) {
		if (m_stepsLeft < steps) {
			log_warning() << typeid(*this).name() << " ==== Too many steps!" << std::endl;
			m_stepsLeft = 0;
		} else {
			m_stepsLeft -= steps;
		}
		m_pi->step(steps);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get a non-initialized progress indicator for use with external
	/// implementations. When step() is called on a proxying progress
	/// indicator, step() is called on the pipe_segment according to the number
	/// of steps declared in progress_indicator_base::init() and in
	/// pipe_segment::set_steps().
	///////////////////////////////////////////////////////////////////////////
	progress_indicator_base * proxy_progress_indicator() {
		if (m_piProxy.get() != 0) return m_piProxy.get();
		progress_indicator_base * pi = new bits::proxy_progress_indicator(*this);
		m_piProxy.reset(pi);
		return pi;
	}

#ifdef DOXYGEN
	///////////////////////////////////////////////////////////////////////////
	/// \brief For pull segments, return true if there are more items to be
	/// pulled.
	///////////////////////////////////////////////////////////////////////////
	inline bool can_pull() const;

	///////////////////////////////////////////////////////////////////////////
	/// \brief For pull segments, pull the next item from this segment.
	///////////////////////////////////////////////////////////////////////////
	inline item_type pull();

	///////////////////////////////////////////////////////////////////////////
	/// \brief For push segments, push the next item to this segment.
	///////////////////////////////////////////////////////////////////////////
	inline void push(const item_type & item);
#endif

	friend class bits::phase;

private:
	segment_token token;

	memory_size_type m_minimumMemory;
	memory_size_type m_availableMemory;
	double m_memoryFraction;

	std::string m_name;
	priority_type m_namePriority;

	std::vector<pipe_segment *> m_successors;
	typedef std::map<std::string, boost::any> valuemap;
	valuemap m_values;

	stream_size_type m_stepsTotal;
	stream_size_type m_stepsLeft;
	progress_indicator_base * m_pi;
	std::auto_ptr<progress_indicator_base> m_piProxy;

	friend class bits::proxy_progress_indicator;
};

namespace bits {

void proxy_progress_indicator::refresh() {
	double proxyMax = static_cast<double>(get_range());
	double proxyCur = static_cast<double>(get_current());
	double parentMax = static_cast<double>(m_segment.m_stepsTotal);
	double parentCur = static_cast<double>(m_segment.m_stepsTotal-m_segment.m_stepsLeft);
	double missing = parentMax*proxyCur/proxyMax - parentCur;
	if (missing < 1.0) return;
	stream_size_type times = static_cast<stream_size_type>(1.0+missing);
	times = std::min(m_segment.m_stepsLeft, times);
	m_segment.step(times);
}

} // namespace bits

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_PIPE_SEGMENT_H__
