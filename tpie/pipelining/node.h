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

#ifndef __TPIE_PIPELINING_NODE_H__
#define __TPIE_PIPELINING_NODE_H__

#include <tpie/pipelining/exception.h>
#include <tpie/pipelining/tokens.h>
#include <tpie/progress_indicator_base.h>
#include <tpie/progress_indicator_null.h>
#include <boost/any.hpp>
#include <tpie/pipelining/priority_type.h>
#include <tpie/pipelining/predeclare.h>

namespace tpie {

namespace pipelining {

namespace bits {

class proxy_progress_indicator : public tpie::progress_indicator_base {
	node & m_node;

public:
	proxy_progress_indicator(node & s)
		: progress_indicator_base(1)
		, m_node(s)
	{
	}

	inline void refresh();
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// Base class of all nodes. A node should inherit from the node class,
/// have a single template parameter dest_t if it is not a terminus node,
/// and implement methods begin(), push() and end(), if it is not a source
/// node.
///////////////////////////////////////////////////////////////////////////////
class node {
public:
	enum STATE {
		STATE_FRESH,
		STATE_IN_PREPARE,
		STATE_AFTER_PREPARE,
		STATE_IN_BEGIN,
		STATE_AFTER_BEGIN,
		STATE_IN_END,
		STATE_AFTER_END
	};

	///////////////////////////////////////////////////////////////////////////
	/// \brief Virtual dtor.
	///////////////////////////////////////////////////////////////////////////
	virtual ~node() {}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the minimum amount of memory declared by this node.
	/// Defaults to zero when no minimum has been set.
	///////////////////////////////////////////////////////////////////////////
	inline memory_size_type get_minimum_memory() const {
		return m_minimumMemory;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the amount of memory assigned to this node.
	///////////////////////////////////////////////////////////////////////////
	inline memory_size_type get_available_memory() const {
		return m_availableMemory;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Set the memory priority of this node. Memory is distributed
	/// proportionally to the priorities of the nodes in the given phase.
	///////////////////////////////////////////////////////////////////////////
	inline void set_memory_fraction(double f) {
		m_memoryFraction = f;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the memory priority of this node.
	///////////////////////////////////////////////////////////////////////////
	inline double get_memory_fraction() const {
		return m_memoryFraction;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the local node map, mapping node IDs to node
	/// pointers for all the nodes reachable from this one.
	///////////////////////////////////////////////////////////////////////////
	inline bits::node_map::ptr get_node_map() const {
		return token.get_map();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the internal node ID of this node (mainly
	/// for debugging purposes).
	///////////////////////////////////////////////////////////////////////////
	inline node_token::id_t get_id() const {
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
	/// The pipelining framework calls begin() on the nodes in the
	/// pipeline graph in a topological order. The framework calls
	/// node::begin() on a node after its pull destinations and
	/// before its push destination.
	///
	/// The default implementation does nothing.
	///////////////////////////////////////////////////////////////////////////
	virtual void begin() {
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief For initiator nodes, execute this phase by pushing all items
	/// to be pushed. For non-initiator nodes, the default implementation
	/// throws a not_initiator_segment exception.
	///////////////////////////////////////////////////////////////////////////
	virtual void go() {
		progress_indicator_null pi;
		go(pi);
		// if go didn't throw, it was overridden - but it shouldn't be
		log_warning() << "node subclass " << typeid(*this).name() << " uses old go() interface" << std::endl;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Deprecated go()-implementation signature. The progress indicator
	/// argument does nothing. Instead, use step() and set_steps().
	///////////////////////////////////////////////////////////////////////////
	virtual void go(progress_indicator_base &) {
		log_warning() << "node subclass " << typeid(*this).name() << " is not an initiator node" << std::endl;
		throw not_initiator_node();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief End pipeline processing phase.
	///
	/// The implementation may pull() from a pull destination in end(),
	/// and it may push() to a push destination.
	///
	/// The pipelining framework calls end() on the nodes in the
	/// pipeline graph in a topological order. The framework calls
	/// node::end() on a node before its pull and push
	/// destinations.
	///
	/// The default implementation does nothing, so it does not matter if the
	/// implementation calls the parent end().
	///////////////////////////////////////////////////////////////////////////
	virtual void end() {
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Overridden by nodes that have data to evacuate.
	///////////////////////////////////////////////////////////////////////////
	virtual bool can_evacuate() {
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Overridden by nodes that have data to evacuate.
	///////////////////////////////////////////////////////////////////////////
	virtual void evacuate() {
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the priority of this node's name. For purposes of
	/// pipeline debugging and phase naming for progress indicator breadcrumbs.
	///////////////////////////////////////////////////////////////////////////
	inline priority_type get_name_priority() {
		return m_namePriority;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get this node's name. For purposes of pipeline debugging and
	/// phase naming for progress indicator breadcrumbs.
	///////////////////////////////////////////////////////////////////////////
	inline const std::string & get_name() {
		return m_name;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Set this node's name. For purposes of pipeline debugging and
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
	/// node_map::send_successors.
	///////////////////////////////////////////////////////////////////////////
	inline void add_successor(node * succ) {
		m_successors.push_back(succ);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Used internally for progress indication. Get the number of times
	/// the node expects to call step() at most.
	///////////////////////////////////////////////////////////////////////////
	inline stream_size_type get_steps() {
		return m_stepsTotal;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Used internally. Set the progress indicator to use.
	///////////////////////////////////////////////////////////////////////////
	inline void set_progress_indicator(progress_indicator_base * pi) {
		m_pi = pi;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Used internally. Get the progress indicator used.
	///////////////////////////////////////////////////////////////////////////
	progress_indicator_base * get_progress_indicator() {
		return m_pi;
	}

	STATE get_state() const {
		return m_state;
	}

	void set_state(STATE s) {
		m_state = s;
	}

protected:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor, using a new node_token.
	///////////////////////////////////////////////////////////////////////////
	inline node()
		: token(this)
		, m_minimumMemory(0)
		, m_availableMemory(0)
		, m_memoryFraction(0.0)
		, m_namePriority(PRIORITY_NO_NAME)
		, m_stepsTotal(0)
		, m_stepsLeft(0)
		, m_pi(0)
		, m_state(STATE_FRESH)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Copy constructor. We need to define this explicitly since the
	/// node_token needs to know its new owner.
	///////////////////////////////////////////////////////////////////////////
	inline node(const node & other)
		: token(other.token, this)
		, m_minimumMemory(other.m_minimumMemory)
		, m_availableMemory(other.m_availableMemory)
		, m_memoryFraction(other.m_memoryFraction)
		, m_name(other.m_name)
		, m_namePriority(other.m_namePriority)
		, m_stepsTotal(other.m_stepsTotal)
		, m_stepsLeft(other.m_stepsLeft)
		, m_pi(other.m_pi)
		, m_state(other.m_state)
	{
		if (m_state != STATE_FRESH) 
			throw call_order_exception(
				"Tried to copy pipeline node after prepare had been called");
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor using a given fresh node_token.
	///////////////////////////////////////////////////////////////////////////
	inline node(const node_token & token)
		: token(token, this, true)
		, m_minimumMemory(0)
		, m_availableMemory(0)
		, m_memoryFraction(0.0)
		, m_namePriority(PRIORITY_NO_NAME)
		, m_stepsTotal(0)
		, m_stepsLeft(0)
		, m_pi(0)
		, m_state(STATE_FRESH)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a push destination.
	///////////////////////////////////////////////////////////////////////////
	inline void add_push_destination(const node_token & dest) {
		bits::node_map::ptr m = token.map_union(dest);
		m->add_relation(token.id(), dest.id(), bits::pushes);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a push destination.
	///////////////////////////////////////////////////////////////////////////
	inline void add_push_destination(const node & dest) {
		if (get_state() != STATE_FRESH) {
			throw call_order_exception("add_push_destination called too late");
		}
		add_push_destination(dest.token);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a pull destination.
	///////////////////////////////////////////////////////////////////////////
	inline void add_pull_destination(const node_token & dest) {
		if (get_state() != STATE_FRESH) {
			throw call_order_exception("add_pull_destination called too late");
		}
		bits::node_map::ptr m = token.map_union(dest);
		m->add_relation(token.id(), dest.id(), bits::pulls);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a pull destination.
	///////////////////////////////////////////////////////////////////////////
	inline void add_pull_destination(const node & dest) {
		add_pull_destination(dest.token);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a node dependency, that is,
	/// a requirement that another node has end() called before the begin()
	/// of this node.
	///////////////////////////////////////////////////////////////////////////
	inline void add_dependency(const node_token & dest) {
		bits::node_map::ptr m = token.map_union(dest);
		m->add_relation(token.id(), dest.id(), bits::depends);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a node dependency, that is,
	/// a requirement that another node has end() called before the begin()
	/// of this node.
	///////////////////////////////////////////////////////////////////////////
	inline void add_dependency(const node & dest) {
		add_dependency(dest.token);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare minimum memory requirements.
	///////////////////////////////////////////////////////////////////////////
	inline void set_minimum_memory(memory_size_type minimumMemory) {
		if (get_state() != STATE_FRESH && get_state() != STATE_IN_PREPARE) {
			throw call_order_exception("set_minimum_memory");
		}
		m_minimumMemory = minimumMemory;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by the memory manager to set the amount of memory
	/// assigned to this node.
	///////////////////////////////////////////////////////////////////////////
	virtual void set_available_memory(memory_size_type availableMemory) {
		m_availableMemory = availableMemory;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to forward auxiliary data to successors.
	/// If explicitForward is false, the data will not override data forwarded
	/// with explicitForward == true.
	///////////////////////////////////////////////////////////////////////////
	template <typename T>
	inline void forward(std::string key, T value, bool explicitForward = true) {
		if (get_state() == STATE_AFTER_END) {
			throw call_order_exception("forward");
		}

		for (size_t i = 0; i < m_successors.size(); ++i) {
			if (m_successors[i]->m_values.count(key) &&
				!explicitForward && m_successors[i]->m_values[key].second) return;
			m_successors[i]->m_values[key].first = value;
			m_successors[i]->m_values[key].second = explicitForward;
			m_successors[i]->forward(key, value, false);
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
		if (m_values.count(key) != 0) {
			return m_values[key].first;
		} else {
			std::stringstream ss;
			ss << "Tried to fetch nonexistent key '" << key << '\'';
			throw invalid_argument_exception(ss.str());
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Fetch piece of auxiliary data, expecting a given value type.
	///////////////////////////////////////////////////////////////////////////
	template <typename T>
	inline T fetch(std::string key) {
		if (m_values.count(key) == 0) {
			std::stringstream ss;
			ss << "Tried to fetch nonexistent key '" << key
			   << "' of type " << typeid(T).name();
			throw invalid_argument_exception(ss.str());
		}
		try {
			return boost::any_cast<T>(m_values[key].first);
		} catch (boost::bad_any_cast m) {
			std::stringstream ss;
			ss << "Trying to fetch key '" << key << "' of type "
			   << typeid(T).name() << " but forwarded data was of type "
			   << m_values[key].first.type().name() << ". Message was: " << m.what();
			throw invalid_argument_exception(ss.str());
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the node_token that maps this node's ID to a pointer
	/// to this.
	///////////////////////////////////////////////////////////////////////////
	const node_token & get_token() {
		return token;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers that intend to call step().
	/// \param steps  The number of times step() is called at most.
	///////////////////////////////////////////////////////////////////////////
	void set_steps(stream_size_type steps) {
		if (get_state() != STATE_IN_PREPARE &&
			get_state() != STATE_IN_BEGIN &&
			get_state() != STATE_FRESH) {
			throw call_order_exception("set_steps");
		}
		m_stepsTotal = m_stepsLeft = steps;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Step the progress indicator.
	/// \param steps  How many steps to step.
	///////////////////////////////////////////////////////////////////////////
	void step(stream_size_type steps = 1) {
		assert(get_state() == STATE_IN_END || get_state() == STATE_AFTER_BEGIN);
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
	/// indicator, step() is called on the node according to the number
	/// of steps declared in progress_indicator_base::init() and in
	/// node::set_steps().
	///////////////////////////////////////////////////////////////////////////
	progress_indicator_base * proxy_progress_indicator() {
		if (m_piProxy.get() != 0) return m_piProxy.get();
		progress_indicator_base * pi = new bits::proxy_progress_indicator(*this);
		m_piProxy.reset(pi);
		return pi;
	}

#ifdef DOXYGEN
	///////////////////////////////////////////////////////////////////////////
	/// \brief For pull nodes, return true if there are more items to be
	/// pulled.
	///////////////////////////////////////////////////////////////////////////
	inline bool can_pull() const;

	///////////////////////////////////////////////////////////////////////////
	/// \brief For pull nodes, pull the next item from this node.
	///////////////////////////////////////////////////////////////////////////
	inline item_type pull();

	///////////////////////////////////////////////////////////////////////////
	/// \brief For push nodes, push the next item to this node.
	///////////////////////////////////////////////////////////////////////////
	inline void push(const item_type & item);
#endif

	friend class bits::phase;

private:
	node_token token;

	memory_size_type m_minimumMemory;
	memory_size_type m_availableMemory;
	double m_memoryFraction;

	std::string m_name;
	priority_type m_namePriority;

	std::vector<node *> m_successors;
	typedef std::map<std::string, std::pair<boost::any, bool> > valuemap;
	valuemap m_values;

	stream_size_type m_stepsTotal;
	stream_size_type m_stepsLeft;
	progress_indicator_base * m_pi;
	STATE m_state;
	std::auto_ptr<progress_indicator_base> m_piProxy;

	friend class bits::proxy_progress_indicator;
};

namespace bits {

void proxy_progress_indicator::refresh() {
	double proxyMax = static_cast<double>(get_range());
	double proxyCur = static_cast<double>(get_current());
	double parentMax = static_cast<double>(m_node.m_stepsTotal);
	double parentCur = static_cast<double>(m_node.m_stepsTotal-m_node.m_stepsLeft);
	double missing = parentMax*proxyCur/proxyMax - parentCur;
	if (missing < 1.0) return;
	stream_size_type times = static_cast<stream_size_type>(1.0+missing);
	times = std::min(m_node.m_stepsLeft, times);
	m_node.step(times);
}

} // namespace bits

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_NODE_H__
