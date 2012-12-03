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

#ifndef __TPIE_PIPELINING_PARALLEL_H__
#define __TPIE_PIPELINING_PARALLEL_H__

#include <tpie/pipelining/pipe_segment.h>
#include <tpie/pipelining/factory_base.h>
#include <tpie/array_view.h>
#include <tpie/job.h>
#include <boost/shared_ptr.hpp>

///////////////////////////////////////////////////////////////////////////////
/// \file parallel.h  Parallel execution of pipe segments.
///
/// Given a sequential computation as a partial pipeline, this parallel
/// framework naively parallelizes it by having multiple thread handle some
/// items each.
///
/// Throughout the code, the input type is named T1, and the output type is
/// named T2.
///
/// Each worker has a pipeline instance of a parallel_before pushing items to
/// the user-supplied pipeline which pushes to an instance of parallel_after.
///
/// The parallel_producer sits in the main thread and distributes item buffers
/// to parallel_befores running in different threads, and the parallel_consumer
/// receives the items pushed to each parallel_after instance.
///
/// All pipe_segments have access to a single parallel_state instance which has
/// the mutex and the necessary condition variables.
///    It also has pointers to the parallel_before and parallel_after instances
/// and it holds an array of worker states (of enum type
/// parallel_worker_state).
///    It also has a parallel_options struct which contains the user-supplied
/// parameters to the framework (size of item buffer and number of concurrent
/// workers).
///
/// TODO at some future point: Optimize code for the case where the buffer size
/// is one.
///////////////////////////////////////////////////////////////////////////////

namespace tpie {

namespace pipelining {

namespace bits {

// predeclare
template <typename T>
class parallel_before;
template <typename dest_t>
class parallel_before_impl;
template <typename T>
class parallel_after;
template <typename T1, typename T2>
class parallel_state;

///////////////////////////////////////////////////////////////////////////////
/// \brief  User-supplied options to the parallelism framework.
///////////////////////////////////////////////////////////////////////////////
struct parallel_options {
	bool maintainOrder;
	size_t numJobs;
	size_t bufSize;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  States of the parallel worker state machine.
///////////////////////////////////////////////////////////////////////////////
enum parallel_worker_state {
	/** The input is being written by the producer. */
	INITIALIZING,

	/** The input is being written by the producer. */
	IDLE,

	/** The worker is writing output. */
	PROCESSING,

	/** The output is being read by the consumer. */
	OUTPUTTING
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Class containing an array of pipe_segment instances. We cannot use
/// tpie::array or similar, since we need to construct the elements in a
/// special way. This class is non-copyable since it resides in the refcounted
/// parallel_state class.
/// \tparam fact_t  Type of factory constructing the worker
/// \tparam Output  Type of output items
///////////////////////////////////////////////////////////////////////////////
template <typename Input, typename Output>
class parallel_pipes {
	typedef parallel_before<Input> before_t;

protected:
	std::vector<before_t *> m_dests;

public:
	before_t & operator[](size_t idx) {
		return *m_dests[idx];
	}

	virtual ~parallel_pipes() {}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Subclass of parallel_pipes instantiating and managing the pipelines.
///////////////////////////////////////////////////////////////////////////////
template <typename Input, typename Output, typename fact_t>
class parallel_pipes_impl : public parallel_pipes<Input, Output> {
private:
	typedef parallel_after<Output> after_t;
	typedef typename fact_t::template generated<after_t>::type worker_t;
	typedef typename worker_t::item_type T1;
	typedef Output T2;
	typedef parallel_before_impl<worker_t> before_t;

	/** Size of the m_dests array. */
	size_t numJobs;

	/** Allocated array buffer. Size sizeof(gen_t)*numJobs/sizeof(uint8_t). */
	uint8_t * m_data;

	/** Reinterpreted array - points to m_data. */
	before_t * m_destImpl;
public:
	parallel_pipes_impl(fact_t fact,
						parallel_state<T1, T2> & st)
		: numJobs(st.opts.numJobs)
	{
		// uninitialized allocation
		m_data = new uint8_t[sizeof(before_t)*numJobs];
		m_destImpl = reinterpret_cast<before_t *>(m_data);
		this->m_dests.resize(numJobs);

		// construct elements manually
		for (size_t i = 0; i < numJobs; ++i) {
			this->m_dests[i] =
				new(&m_destImpl[i])
				before_t(st, i, fact.construct(after_t(st, i)));
		}
	}

	virtual ~parallel_pipes_impl() {
		for (size_t i = 0; i < numJobs; ++i) {
			m_destImpl[i].~before_t();
		}
		delete[] m_data;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  Non-templated virtual base class of parallel_after.
///////////////////////////////////////////////////////////////////////////////
class parallel_after_base : public pipe_segment {
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Called by parallel_before::worker to initialize buffers.
	///////////////////////////////////////////////////////////////////////////
	virtual void worker_initialize() = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Called by parallel_before::worker after a batch of items has
	/// been pushed.
	///////////////////////////////////////////////////////////////////////////
	virtual void flush_buffer() = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  Common state in parallel pipelining library.
/// This class is instantiated once and kept in a boost::shared_ptr, and it is
/// not copy constructible.
///
/// Unless noted otherwise, a thread must own the state mutex to access other
/// parts of this instance.
///////////////////////////////////////////////////////////////////////////////
class parallel_state_base {
public:
	typedef boost::mutex mutex_t;
	typedef boost::condition_variable cond_t;
	typedef boost::unique_lock<boost::mutex> lock_t;

	const parallel_options opts;

	/** Single mutex. */
	mutex_t mutex;

	/** Condition variable.
	 *
	 * Who waits: The producer, with the single mutex (waits until at least one
	 * worker has state = IDLE or state = OUTPUTTING).
	 *
	 * Who signals: The par_after, when a worker is OUTPUTTING. */
	cond_t producerCond;

	/** Condition variable, one per worker.
	 *
	 * Who waits: The worker's par_before when waiting for input (wait for
	 * state = PROCESSING), the worker's par_after when waiting for output to
	 * be read (wait for state = IDLE). Waits with the single mutex.
	 *
	 * Who signals: par_producer, when input has been written (sets state to PROCESSING).
	 * par_consumer, when output has been read (sets state to IDLE).
	 */
	cond_t * workerCond;

	/** Are we done? Shared state, must have mutex to write. */
	bool done;

	/** Shared state, must have mutex to write. */
	size_t runningWorkers;

	/// Must not be used concurrently.
	void set_input_ptr(size_t idx, pipe_segment * v) {
		m_inputs[idx] = v;
	}

	/// Must not be used concurrently.
	void set_output_ptr(size_t idx, parallel_after_base * v) {
		m_outputs[idx] = v;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Get the specified parallel_before instance.
	///
	/// Enables easy construction of the pipeline graph at runtime.
	///
	/// Shared state, must have mutex to use.
	///////////////////////////////////////////////////////////////////////////
	pipe_segment & input(size_t idx) { return *m_inputs[idx]; }

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Get the specified parallel_after instance.
	///
	/// Serves two purposes:
	/// First, it enables easy construction of the pipeline graph at runtime.
	/// Second, it is used by parallel_before to send batch signals to
	/// parallel_after.
	///
	/// Shared state, must have mutex to use.
	///////////////////////////////////////////////////////////////////////////
	parallel_after_base & output(size_t idx) { return *m_outputs[idx]; }

	/// Shared state, must have mutex to use.
	parallel_worker_state get_state(size_t idx) {
		return m_states[idx];
	}

	/// Shared state, must have mutex to use.
	void set_state(size_t idx, parallel_worker_state st) {
		m_states[idx] = st;
	}

protected:
	std::vector<pipe_segment *> m_inputs;
	std::vector<parallel_after_base *> m_outputs;
	std::vector<parallel_worker_state> m_states;

	parallel_state_base(const parallel_options opts)
		: opts(opts)
		, done(false)
		, runningWorkers(0)
		, m_inputs(opts.numJobs, 0)
		, m_outputs(opts.numJobs, 0)
		, m_states(opts.numJobs, INITIALIZING)
	{
		workerCond = new cond_t[opts.numJobs];
	}

	virtual ~parallel_state_base() {
		delete[] workerCond;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Instantiated in each thread.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class parallel_input_buffer {
	memory_size_type m_inputSize;
	array<T> m_inputBuffer;

public:
	array_view<T> get_input() {
		return array_view<T>(&m_inputBuffer[0], m_inputSize);
	}

	void set_input(array_view<T> input) {
		if (input.size() > m_inputBuffer.size())
			throw tpie::exception(m_inputBuffer.size() ? "Input too large" : "Input buffer not initialized");

		memory_size_type items =
			std::copy(input.begin(), input.end(), m_inputBuffer.begin())
			-m_inputBuffer.begin();

		m_inputSize = items;
	}

	parallel_input_buffer(const parallel_options & opts)
		: m_inputSize(0)
		, m_inputBuffer(opts.bufSize)
	{
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Instantiated in each thread.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class parallel_output_buffer {
	memory_size_type m_outputSize;
	array<T> m_outputBuffer;
	friend class parallel_after<T>;

public:
	array_view<T> get_output() {
		return array_view<T>(&m_outputBuffer[0], m_outputSize);
	}

	parallel_output_buffer(const parallel_options & opts)
		: m_outputSize(0)
		, m_outputBuffer(opts.bufSize)
	{
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief State subclass containing the item type specific state, i.e. the
/// input/output buffers and the concrete pipes.
///////////////////////////////////////////////////////////////////////////////
template <typename T1, typename T2>
class parallel_state : public parallel_state_base {
public:
	typedef boost::shared_ptr<parallel_state> ptr;
	typedef parallel_state_base::mutex_t mutex_t;
	typedef parallel_state_base::cond_t cond_t;
	typedef parallel_state_base::lock_t lock_t;

	array<parallel_input_buffer<T1> *> m_inputBuffers;
	array<parallel_output_buffer<T2> *> m_outputBuffers;

	std::auto_ptr<parallel_pipes<T1, T2> > pipes;

	template <typename fact_t>
	parallel_state(const parallel_options opts, const fact_t & fact)
		: parallel_state_base(opts)
		, m_inputBuffers(opts.numJobs)
		, m_outputBuffers(opts.numJobs)
	{
		typedef parallel_pipes_impl<T1, T2, fact_t> pipes_impl_t;
		pipes.reset(new pipes_impl_t(fact, *this));
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Accepts output items and sends them to the main thread.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class parallel_after : public parallel_after_base {
protected:
	parallel_state_base & st;
	size_t parId;
	std::auto_ptr<parallel_output_buffer<T> > m_buffer;
	array<parallel_output_buffer<T> *> & m_outputBuffers;
	typedef parallel_state_base::lock_t lock_t;

public:
	typedef T item_type;

	template <typename Input>
	parallel_after(parallel_state<Input, T> & state,
				   size_t parId)
		: st(state)
		, parId(parId)
		, m_outputBuffers(state.m_outputBuffers)
	{
		state.set_output_ptr(parId, this);
		set_name("Parallel after", PRIORITY_INSIGNIFICANT);
	}

	parallel_after(const parallel_after & other)
		: parallel_after_base(other)
		, st(other.st)
		, parId(other.parId)
		, m_outputBuffers(other.m_outputBuffers)
	{
		st.set_output_ptr(parId, this);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Push to thread-local buffer; flush it when full.
	///////////////////////////////////////////////////////////////////////////
	void push(const T & item) {
		if (m_buffer->m_outputSize >= m_buffer->m_outputBuffer.size())
			throw std::runtime_error("Buffer overrun in parallel_after");

		m_buffer->m_outputBuffer[m_buffer->m_outputSize++] = item;

		if (m_buffer->m_outputSize >= m_buffer->m_outputBuffer.size())
			flush_buffer_impl();
	}

	virtual void worker_initialize() {
		m_buffer.reset(new parallel_output_buffer<T>(st.opts));
		m_outputBuffers[parId] = m_buffer.get();
	}

	virtual void flush_buffer() {
		flush_buffer_impl();
	}

private:
	bool is_done() const {
		switch (st.get_state(parId)) {
			case INITIALIZING:
				throw tpie::exception("INITIALIZING not expected in parallel_after::is_done");
			case IDLE:
				return true;
			case PROCESSING:
				// This case is reached if our state changes from Outputting to
				// Idle to Processing and we miss a state change
				return true;
			case OUTPUTTING:
				return false;
		}
		throw std::runtime_error("Unknown state");
	}

	void flush_buffer_impl() {
		if (m_buffer->m_outputSize == 0)
			return;
		lock_t lock(st.mutex);
		st.set_state(parId, OUTPUTTING);
		// notify producer that output is ready
		st.producerCond.notify_one();
		while (!is_done()) {
			if (st.done) return;
			st.workerCond[parId].wait(lock);
		}
		m_buffer->m_outputSize = 0;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Accepts input items from the main thread and sends them down the
/// pipeline. This class contains the bulk of the code that is run in each
/// worker thread.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class parallel_before : public pipe_segment {
	class worker_job : public tpie::job {
	public:
		parallel_before<T> * self;

		virtual void operator()() /*override*/ {
			self->worker();
		}
	};

	friend class worker_job;

protected:
	parallel_state_base & st;
	size_t parId;
	std::auto_ptr<parallel_input_buffer<T> > m_buffer;
	array<parallel_input_buffer<T> *> & m_inputBuffers;
	worker_job job;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Overridden in subclass to push a buffer of items.
	///////////////////////////////////////////////////////////////////////////
	virtual void push_all(array_view<T> items) = 0;

	template <typename Output>
	parallel_before(parallel_state<T, Output> & st, size_t parId)
		: st(st)
		, parId(parId)
		, m_inputBuffers(st.m_inputBuffers)
	{
		job.self = this;
		set_name("Parallel before", PRIORITY_INSIGNIFICANT);
	}
	// virtual dtor in pipe_segment

	parallel_before(const parallel_before & other)
		: st(other.st)
		, parId(other.parId)
		, m_inputBuffers(other.m_inputBuffers)
	{
		job.self = this;
	}

public:
	typedef T item_type;

	virtual void begin() /*override*/ {
		pipe_segment::begin();
		job.enqueue();
	}

private:
	bool ready() {
		switch (st.get_state(parId)) {
			case INITIALIZING:
				throw tpie::exception("INITIALIZING not expected in parallel_before::ready");
			case IDLE:
				return false;
			case PROCESSING:
				return true;
			case OUTPUTTING:
				throw std::runtime_error("State 'outputting' was not expected in parallel_before::ready");
		}
		throw std::runtime_error("Unknown state");
	}

	class running_signal {
		typedef parallel_state_base::cond_t cond_t;
		memory_size_type & sig;
		cond_t & producerCond;
	public:
		running_signal(memory_size_type & sig, cond_t & producerCond)
			: sig(sig)
			, producerCond(producerCond)
		{
			++sig;
			producerCond.notify_one();
		}

		~running_signal() {
			--sig;
			producerCond.notify_one();
		}
	};

	void worker() {
		parallel_state_base::lock_t lock(st.mutex);

		m_buffer.reset(new parallel_input_buffer<T>(st.opts));
		m_inputBuffers[parId] = m_buffer.get();

		// virtual invocation
		st.output(parId).worker_initialize();

		st.set_state(parId, IDLE);
		running_signal _(st.runningWorkers, st.producerCond);
		while (true) {
			// wait for state = processing
			while (!ready()) {
				if (st.done) {
					return;
				}
				st.workerCond[parId].wait(lock);
			}
			lock.unlock();

			// virtual invocation
			push_all(m_buffer->get_input());

			lock.lock();
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Concrete parallel_before class.
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
class parallel_before_impl : public parallel_before<typename dest_t::item_type> {
	typedef typename dest_t::item_type item_type;

	dest_t dest;

public:
	template <typename Output>
	parallel_before_impl(parallel_state<item_type, Output> & st,
						 size_t parId,
						 dest_t dest)
		: parallel_before<item_type>(st, parId)
		, dest(dest)
	{
		this->add_push_destination(dest);
		st.set_input_ptr(parId, this);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Push all items from buffer and flush output buffer afterwards.
	///
	/// If pipeline is one-to-one, that is, one item output for each item
	/// input, then the flush at the end is not needed.
	///////////////////////////////////////////////////////////////////////////
	virtual void push_all(array_view<item_type> items) {
		for (size_t i = 0; i < items.size(); ++i) {
			dest.push(items[i]);
		}

		// virtual invocation
		this->st.output(this->parId).flush_buffer();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  Pipe segment running in main thread, accepting an output buffer
/// from the managing producer and forwards them down the pipe. The overhead
/// concerned with switching threads dominates the overhead of a virtual method
/// call, so this class only depends on the output type and leaves the pushing
/// of items to a virtual subclass.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class parallel_consumer : public pipe_segment {
public:
	typedef T item_type;

	virtual void consume(array_view<T>) = 0;
	// pipe_segment has virtual dtor
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Concrete consumer implementation.
///////////////////////////////////////////////////////////////////////////////
template <typename Input, typename Output, typename dest_t>
class parallel_consumer_impl : public parallel_consumer<typename dest_t::item_type> {
	typedef parallel_state<Input, Output> state_t;
	typedef typename state_t::ptr stateptr;
	dest_t dest;
	stateptr st;
public:
	typedef typename dest_t::item_type item_type;

	parallel_consumer_impl(const dest_t & dest, stateptr st)
		: dest(dest)
		, st(st)
	{
		this->add_push_destination(dest);
		this->set_name("Parallel output", PRIORITY_INSIGNIFICANT);
		for (size_t i = 0; i < st->opts.numJobs; ++i) {
			this->add_pull_destination(st->output(i));
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Push all items from output buffer to the rest of the pipeline.
	///////////////////////////////////////////////////////////////////////////
	virtual void consume(array_view<item_type> a) /*override*/ {
		for (size_t i = 0; i < a.size(); ++i) {
			dest.push(a[i]);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Producer, running in main thread, managing the parallel execution.
///
/// This class contains the bulk of the code that is run in the main thread.
///////////////////////////////////////////////////////////////////////////////
template <typename T1, typename T2>
class parallel_producer : public pipe_segment {
public:
	typedef T1 item_type;

private:
	typedef parallel_state<T1, T2> state_t;
	typedef typename state_t::ptr stateptr;
	stateptr st;
	array<T1> inputBuffer;
	size_t written;
	size_t readyIdx;
	boost::shared_ptr<parallel_consumer<T2> > cons;
	internal_queue<memory_size_type> m_outputOrder;

	bool has_ready_pipe() {
		for (size_t i = 0; i < st->opts.numJobs; ++i) {
			switch (st->get_state(i)) {
				case INITIALIZING:
				case PROCESSING:
					break;
				case OUTPUTTING:
					if (st->opts.maintainOrder && m_outputOrder.front() != i)
						break;
					// fallthrough
				case IDLE:
					readyIdx = i;
					return true;
			}
		}
		return false;
	}

	bool has_outputting_pipe() {
		for (size_t i = 0; i < st->opts.numJobs; ++i) {
			switch (st->get_state(i)) {
				case INITIALIZING:
				case IDLE:
				case PROCESSING:
					break;
				case OUTPUTTING:
					if (st->opts.maintainOrder && m_outputOrder.front() != i)
						break;
					readyIdx = i;
					return true;
			}
		}
		return false;
	}

	bool has_processing_pipe() {
		for (size_t i = 0; i < st->opts.numJobs; ++i) {
			switch (st->get_state(i)) {
				case INITIALIZING:
				case IDLE:
				case OUTPUTTING:
					break;
				case PROCESSING:
					return true;
			}
		}
		return false;
	}

public:
	template <typename consumer_t>
	parallel_producer(stateptr st, const consumer_t & cons)
		: st(st)
		, written(0)
		, cons(new consumer_t(cons))
	{
		for (size_t i = 0; i < st->opts.numJobs; ++i) {
			this->add_push_destination(st->input(i));
		}
		this->set_name("Parallel input", PRIORITY_INSIGNIFICANT);
		memory_size_type usage =
			st->opts.numJobs * st->opts.bufSize * (sizeof(T1) + sizeof(T2)) // workers
			+ st->opts.bufSize * sizeof(item_type) // our buffer
			;
		this->set_minimum_memory(usage);
		this->add_push_destination(cons);

		if (st->opts.maintainOrder) {
			m_outputOrder.resize(st->opts.numJobs);
		}
	}

	virtual void begin() /*override*/ {
		pipe_segment::begin();
		inputBuffer.resize(st->opts.bufSize);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Accumulate input buffer and send off to workers.
	///
	/// Since the parallel producer and parallel consumer run single-threaded
	/// in the main thread, producer::push is our only opportunity to have the
	/// consumer call push on its destination. Thus, when we accumulate an
	/// input buffer, before sending it off to a worker, we might want to have
	/// the consumer consume an output buffer to free up a parallel worker.
	///////////////////////////////////////////////////////////////////////////
	void push(item_type item) {
		inputBuffer[written++] = item;
		if (written < st->opts.bufSize) {
			// Wait for more items before doing anything expensive such as
			// locking.
			return;
		}
		parallel_state_base::lock_t lock(st->mutex);
		empty_input_buffer(lock);
	}

private:
	void empty_input_buffer(parallel_state_base::lock_t & lock) {
		while (written > 0) {
			while (!has_ready_pipe()) {
				st->producerCond.wait(lock);
			}
			switch (st->get_state(readyIdx)) {
				case INITIALIZING:
					throw tpie::exception("State 'INITIALIZING' not expected at this point");
				case IDLE:
				{
					// Send buffer to ready worker
					item_type * first = &inputBuffer[0];
					item_type * last = first + written;
					parallel_input_buffer<T1> & dest = *st->m_inputBuffers[readyIdx];
					dest.set_input(array_view<T1>(first, last));
					st->set_state(readyIdx, PROCESSING);
					st->workerCond[readyIdx].notify_one();
					written = 0;
					if (st->opts.maintainOrder)
						m_outputOrder.push(readyIdx);
					break;
				}
				case PROCESSING:
					throw std::runtime_error("State 'processing' not expected at this point");
				case OUTPUTTING:
					// Receive buffer (virtual invocation)
					cons->consume(st->m_outputBuffers[readyIdx]->get_output());

					st->set_state(readyIdx, IDLE);
					st->workerCond[readyIdx].notify_one();
					if (st->opts.maintainOrder) {
						if (m_outputOrder.front() != readyIdx) {
							log_error() << "Producer: Expected " << readyIdx << " in front; got "
								<< m_outputOrder.front() << std::endl;
							throw tpie::exception("Producer got wrong entry from has_ready_pipe");
						}
						m_outputOrder.pop();
					}
					break;
			}
		}
	}

public:
	virtual void end() /*override*/ {
		parallel_state_base::lock_t lock(st->mutex);
		empty_input_buffer(lock);

		bool done = false;
		while (!done) {
			while (!has_outputting_pipe()) {
				if (!has_processing_pipe()) {
					done = true;
					break;
				}
				// All items pushed; wait for processors to complete
				st->producerCond.wait(lock);
			}
			if (done) break;

			// virtual invocation
			cons->consume(st->m_outputBuffers[readyIdx]->get_output());

			st->set_state(readyIdx, IDLE);
			if (st->opts.maintainOrder) {
				if (m_outputOrder.front() != readyIdx) {
					log_error() << "Producer: Expected " << readyIdx << " in front; got "
						<< m_outputOrder.front() << std::endl;
					throw tpie::exception("Producer got wrong entry from has_ready_pipe");
				}
				m_outputOrder.pop();
			}
		}
		// Notify all workers that all processing is done
		st->done = true;
		for (size_t i = 0; i < st->opts.numJobs; ++i) {
			st->workerCond[i].notify_one();
		}
		while (st->runningWorkers > 0) {
			st->producerCond.wait(lock);
		}
		// All workers terminated

		inputBuffer.resize(0);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Factory instantiating a parallel multithreaded pipeline.
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
class parallel_factory : public factory_base {
	fact_t fact;
	const parallel_options opts;
public:
	template <typename dest_t>
	struct generated {
		typedef typename dest_t::item_type T2;

		// We need to know the type that our processor wants as input,
		// but we don't yet know the type of its destination (par_after<...>).
		// The following dummy destination type is hopefully an adequate substitute.
		struct dummy_dest : public pipe_segment { typedef T2 item_type; void push(T2); };
		typedef typename fact_t::template generated<dummy_dest>::type::item_type T1;

		typedef parallel_after<T2> after_t;
		typedef typename fact_t::template generated<after_t>::type processor_t;

		// Check that our processor still wants the input we expect it to.
		// This will yield a compile-time error if it now wants another type.
		// (For instance, it could have a template specialization on par_after
		// - but we do not allow that.)
		template <typename U1, typename U2> struct is_same;
		template <typename U> struct is_same<U, U> { typedef void type; };
		typedef typename is_same<T1, typename processor_t::item_type>::type dummy;

		typedef parallel_producer<T1, T2> type;
	};

	parallel_factory(const fact_t & fact, const parallel_options opts)
		: fact(fact)
		, opts(opts)
	{
	}

	template <typename dest_t>
	typename generated<dest_t>::type
	construct(const dest_t & dest) const {
		typedef generated<dest_t> gen_t;

		typedef typename gen_t::T1 input_type;
		typedef typename gen_t::T2 output_type;
		typedef parallel_state<input_type, output_type> state_t;

		typedef parallel_consumer_impl<input_type, output_type, dest_t> consumer_t;

		typedef typename gen_t::type producer_t;

		typename state_t::ptr st(new state_t(opts, fact));

		consumer_t consumer(dest, st);
		this->init_segment(consumer);
		producer_t producer(st, consumer);
		this->init_segment(producer);
		return producer;
	}
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief  Runs a pipeline in multiple threads.
/// \param numJobs  The number of threads (TPIE jobs) to utilize for parallel
/// execution.
/// \param bufSize  The number of items to store in the buffer sent between
/// threads.
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
inline pipe_middle<bits::parallel_factory<fact_t> >
parallel(const pipe_middle<fact_t> & fact, bool maintainOrder = false, size_t numJobs = 4, size_t bufSize = 1024) {
	bits::parallel_options opts;
	opts.maintainOrder = maintainOrder;
	opts.numJobs = numJobs;
	opts.bufSize = bufSize;
	return pipe_middle<bits::parallel_factory<fact_t> >
		(bits::parallel_factory<fact_t>
		 (fact.factory, opts));
}

} // namespace pipelining

} // namespace tpie

#endif
