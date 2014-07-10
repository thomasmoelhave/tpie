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
#include <tpie/pipelining/node_name.h>
#include <tpie/pipelining/node_traits.h>

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

struct node_parameters {
	node_parameters()
		: minimumMemory(0)
		, maximumMemory(std::numeric_limits<memory_size_type>::max())
		, memoryFraction(0.0)
		, name()
		, namePriority(PRIORITY_NO_NAME)
		, stepsTotal(0)
	{
	}

	memory_size_type minimumMemory;
	memory_size_type maximumMemory;
	double memoryFraction;

	std::string name;
	priority_type namePriority;

	stream_size_type stepsTotal;
};

///////////////////////////////////////////////////////////////////////////////
/// Base class of all nodes. A node should inherit from the node class,
/// have a single template parameter dest_t if it is not a terminus node,
/// and implement methods begin(), push() and end(), if it is not a source
/// node.
///////////////////////////////////////////////////////////////////////////////
class node {
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Options for how to plot this node
	//////////////////////////////////////////////////////////////////////////
	enum PLOT {
		PLOT_SIMPLIFIED_HIDE=1,
		PLOT_BUFFERED=2
	};

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Used internally to check order of method calls.
	///////////////////////////////////////////////////////////////////////////
	enum STATE {
		STATE_FRESH,
		STATE_IN_PREPARE,
		STATE_AFTER_PREPARE,
		STATE_IN_PROPAGATE,
		STATE_AFTER_PROPAGATE,
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
		return m_parameters.minimumMemory;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the maximum amount of memory declared by this node.
	/// Defaults to maxint when no maximum has been set.
	///////////////////////////////////////////////////////////////////////////
	inline memory_size_type get_maximum_memory() const {
		return m_parameters.maximumMemory;
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
		m_parameters.memoryFraction = f;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the memory priority of this node.
	///////////////////////////////////////////////////////////////////////////
	inline double get_memory_fraction() const {
		return m_parameters.memoryFraction;
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
	/// \brief  Propagate stream metadata.
	///
	/// The implementation may fetch() and forward() metadata such as number of
	/// items or the size of a single item.
	///
	/// The pipelining framework calls propagate() on the nodes in the
	/// item flow graph in a topological order.
	///
	/// The default implementation does nothing.
	///////////////////////////////////////////////////////////////////////////
	virtual void propagate() {
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Begin pipeline processing phase.
	///
	/// The implementation may pull() from a pull destination in begin(),
	/// and it may push() to a push destination.
	///
	/// The pipelining framework calls begin() on the nodes in the
	/// actor graph in a reverse topological order. The framework calls
	/// node::begin() on a node after calling begin() on its pull and push
	/// destinations.
	///
	/// The default implementation does nothing.
	///////////////////////////////////////////////////////////////////////////
	virtual void begin() {
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief For initiator nodes, execute this phase by pushing all items
	/// to be pushed. For non-initiator nodes, the default implementation
	/// throws a not_initiator_node exception.
	///////////////////////////////////////////////////////////////////////////
	virtual void go() {
		log_warning() << "node subclass " << typeid(*this).name()
			<< " is not an initiator node" << std::endl;
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
		return m_parameters.namePriority;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get this node's name. For purposes of pipeline debugging and
	/// phase naming for progress indicator breadcrumbs.
	///////////////////////////////////////////////////////////////////////////
	inline const std::string & get_name() {
		if (m_parameters.name.empty()) {
			m_parameters.name = bits::extract_pipe_name(typeid(*this).name());
		}
		return m_parameters.name;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Set this node's name. For purposes of pipeline debugging and
	/// phase naming for progress indicator breadcrumbs.
	///////////////////////////////////////////////////////////////////////////
	inline void set_name(const std::string & name, priority_type priority = PRIORITY_USER) {
		m_parameters.name = name;
		m_parameters.namePriority = priority;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Used internally when a pair_factory has a name set.
	///////////////////////////////////////////////////////////////////////////
	inline void set_breadcrumb(const std::string & breadcrumb) {
		m_parameters.name = m_parameters.name.empty() ? breadcrumb : (breadcrumb + " | " + m_parameters.name);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Used internally for progress indication. Get the number of times
	/// the node expects to call step() at most.
	///////////////////////////////////////////////////////////////////////////
	inline stream_size_type get_steps() {
		return m_parameters.stepsTotal;
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

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Used internally to check order of method calls.
	///////////////////////////////////////////////////////////////////////////
	STATE get_state() const {
		return m_state;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Used internally to check order of method calls.
	///////////////////////////////////////////////////////////////////////////
	void set_state(STATE s) {
		m_state = s;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Get options specified for plot(), as a combination of
	/// \c node::PLOT values.
	///////////////////////////////////////////////////////////////////////////
	int get_plot_options() const {
		return m_plotOptions;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Set options specified for plot(), as a combination of
	/// \c node::PLOT values.
	///////////////////////////////////////////////////////////////////////////
	void set_plot_options(int options) {
		m_plotOptions = options;
	}
protected:
#ifdef _WIN32
	// Disable warning C4355: 'this' : used in base member initializer list
	// node_token does not access members of the `node *`,
	// it merely uses it as a value in the node map.
	// Only after this node object is completely constructed are node members accessed.
#pragma warning( push )
#pragma warning( disable : 4355 )
#endif // _WIN32
	///////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor, using a new node_token.
	///////////////////////////////////////////////////////////////////////////
	inline node()
		: token(this)
		, m_availableMemory(0)
		, m_stepsLeft(0)
		, m_pi(0)
		, m_state(STATE_FRESH)
		, m_plotOptions(0)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Copy constructor. We need to define this explicitly since the
	/// node_token needs to know its new owner.
	///////////////////////////////////////////////////////////////////////////
	inline node(const node & other)
		: token(other.token, this)
		, m_parameters(other.m_parameters)
		, m_availableMemory(other.m_availableMemory)
		, m_stepsLeft(other.m_stepsLeft)
		, m_pi(other.m_pi)
		, m_state(other.m_state)
		, m_plotOptions(other.m_plotOptions)
	{
		if (m_state != STATE_FRESH) 
			throw call_order_exception(
				"Tried to copy pipeline node after prepare had been called");
	}

#ifdef TPIE_CPP_RVALUE_REFERENCE
	///////////////////////////////////////////////////////////////////////////
	/// \brief Move constructor. We need to define this explicitly since the
	/// node_token needs to know its new owner.
	///////////////////////////////////////////////////////////////////////////
	node(node && other)
		: token(std::move(other.token), this)
		, m_parameters(std::move(other.m_parameters))
		, m_availableMemory(std::move(other.m_availableMemory))
		, m_stepsLeft(std::move(other.m_stepsLeft))
		, m_pi(std::move(other.m_pi))
		, m_state(std::move(other.m_state))
		, m_plotOptions(std::move(other.m_plotOptions))
	{
		if (m_state != STATE_FRESH)
			throw call_order_exception(
				"Tried to move pipeline node after prepare had been called");
	}
#endif // TPIE_CPP_RVALUE_REFERENCE

	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor using a given fresh node_token.
	///////////////////////////////////////////////////////////////////////////
	inline node(const node_token & token)
		: token(token, this, true)
		, m_parameters()
		, m_availableMemory(0)
		, m_stepsLeft(0)
		, m_pi(0)
		, m_state(STATE_FRESH)
		, m_plotOptions(0)
	{
	}
#ifdef _WIN32
#pragma warning( pop )
#endif // _WIN32

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
	/// \brief Called by implementers to declare a pull source.
	///////////////////////////////////////////////////////////////////////////
	void add_pull_source(const node_token & dest) {
		if (get_state() != STATE_FRESH) {
			throw call_order_exception("add_pull_source called too late");
		}
		bits::node_map::ptr m = token.map_union(dest);
		m->add_relation(token.id(), dest.id(), bits::pulls);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare a pull source.
	///////////////////////////////////////////////////////////////////////////
	void add_pull_source(const node & dest) {
		add_pull_source(dest.token);
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
		m_parameters.minimumMemory = minimumMemory;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to declare maximum memory requirements.
	///
	/// To signal that you don't want any memory, set minimum memory and the
	/// memory fraction to zero.
	///////////////////////////////////////////////////////////////////////////
	inline void set_maximum_memory(memory_size_type maximumMemory) {
		if (get_state() != STATE_FRESH && get_state() != STATE_IN_PREPARE) {
			throw call_order_exception("set_maximum_memory");
		}
		m_parameters.maximumMemory = maximumMemory;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by the memory manager to set the amount of memory
	/// assigned to this node.
	///////////////////////////////////////////////////////////////////////////
	virtual void set_available_memory(memory_size_type availableMemory) {
		m_availableMemory = availableMemory;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers to forward auxiliary data to successors.
	/// If explicitForward is false, the data will not override data forwarded
	/// with explicitForward == true.
	///////////////////////////////////////////////////////////////////////////
	// Implementation note: If the type of the `value` parameter is changed
	// from `T` to `const T &`, this will yield linker errors if an application
	// attempts to pass a const reference to a static data member inside a
	// templated class.
	// See http://stackoverflow.com/a/5392050
	///////////////////////////////////////////////////////////////////////////
	template <typename T>
	void forward(std::string key, T value) {
		forward_any(key, boost::any(value));
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief See \ref node::forward.
	///////////////////////////////////////////////////////////////////////////
	void forward_any(std::string key, boost::any value) {
		switch (get_state()) {
			case STATE_FRESH:
			case STATE_IN_PREPARE:
			case STATE_AFTER_PREPARE:
				// Allowed since forward() is allowed in prepare()
				break;
			case STATE_IN_PROPAGATE:
			case STATE_AFTER_PROPAGATE:
				// Allowed since forward() is allowed in propagate()
				break;
			case STATE_IN_BEGIN:
				throw call_order_exception("forward");
			case STATE_AFTER_BEGIN:
			case STATE_IN_END:
			case STATE_AFTER_END:
				// Allowed since forward() is allowed in end()
				break;
			default:
				log_debug() << "forward in unknown state " << get_state() << std::endl;
				break;
		}

		add_forwarded_data(key, value, true);

		bits::node_map::ptr nodeMap = get_node_map()->find_authority();

		typedef node_token::id_t id_t;
		std::vector<id_t> successors;
		nodeMap->get_successors(get_id(), successors);
		for (size_t i = 0; i < successors.size(); ++i) {
			nodeMap->get(successors[i])->add_forwarded_data(key, value, false);
		}
	}

private:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by forward_any to add forwarded data.
	//
	/// If explicitForward is false, the data will not override data forwarded
	/// with explicitForward == true.
	///////////////////////////////////////////////////////////////////////////
	void add_forwarded_data(std::string key, boost::any value, bool explicitForward) {
		if (m_values.count(key) &&
			!explicitForward && m_values[key].second) return;
		m_values[key].first = value;
		m_values[key].second = explicitForward;
	}

public:
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
	const node_token & get_token() const {
		return token;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Called by implementers that intend to call step().
	/// \param steps  The number of times step() is called at most.
	///////////////////////////////////////////////////////////////////////////
	void set_steps(stream_size_type steps) {
		switch (get_state()) {
			case STATE_FRESH:
			case STATE_IN_PREPARE:
			case STATE_IN_PROPAGATE:
				break;
			case STATE_IN_BEGIN:
				log_error() << "set_steps in begin(); use set_steps in propagate() instead." << std::endl;
				throw call_order_exception("set_steps");
			default:
				log_error() << "set_steps in unknown state " << get_state() << std::endl;
				throw call_order_exception("set_steps");
		}
		m_parameters.stepsTotal = m_stepsLeft = steps;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Step the progress indicator.
	/// \param steps  How many steps to step.
	///////////////////////////////////////////////////////////////////////////
	void step(stream_size_type steps = 1) {
		assert(get_state() == STATE_IN_END || get_state() == STATE_AFTER_BEGIN || get_state() == STATE_IN_END);
		if (m_stepsLeft < steps) {
			if (m_parameters.stepsTotal != std::numeric_limits<stream_size_type>::max()) {
				log_warning() << typeid(*this).name() << " ==== Too many steps " << m_parameters.stepsTotal << std::endl;
				m_stepsLeft = 0;
				m_parameters.stepsTotal = std::numeric_limits<stream_size_type>::max();
			}
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

	friend class bits::memory_runtime;

	friend class factory_base;

	friend class bits::pipeline_base;

private:
	node_token token;

	node_parameters m_parameters;
	memory_size_type m_availableMemory;

	typedef std::map<std::string, std::pair<boost::any, bool> > valuemap;
	valuemap m_values;

	stream_size_type m_stepsLeft;
	progress_indicator_base * m_pi;
	STATE m_state;
	std::auto_ptr<progress_indicator_base> m_piProxy;
	int m_plotOptions;

	friend class bits::proxy_progress_indicator;
};

namespace bits {

void proxy_progress_indicator::refresh() {
	double proxyMax = static_cast<double>(get_range());
	double proxyCur = static_cast<double>(get_current());
	double parentMax = static_cast<double>(m_node.m_parameters.stepsTotal);
	double parentCur = static_cast<double>(m_node.m_parameters.stepsTotal-m_node.m_stepsLeft);
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
