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
#include <boost/shared_array.hpp>

///////////////////////////////////////////////////////////////////////////////
/// \file parallel.h  Parallel execution of pipe segments
///////////////////////////////////////////////////////////////////////////////

namespace tpie {

namespace pipelining {

namespace bits {

struct parallel_options {
	size_t numJobs;
	size_t bufSize;
};

enum parallel_worker_state {
	/** The input is being written by the producer. */
	IDLE,

	/** The worker is writing output. */
	PROCESSING,

	/** The output is being read by the consumer. */
	OUTPUTTING
};

template <typename T1>
class parallel_input_buffer {
protected:
	array<T1> inputBuffer;
	array<memory_size_type> items;

	void resize_input(memory_size_type n) {
		inputBuffer.resize(n);
		items.resize(n);
	}

public:
	array_view<T1> get_input_buffer(const parallel_options & opts, size_t job) {
		return array_view<T1>(&inputBuffer[opts.bufSize * job], opts.bufSize);
	}
	memory_size_type get_input_size(size_t job) {
		return items[job];
	}
	void set_input_size(size_t job, memory_size_type n) {
		items[job] = n;
	}
};

template <typename T2>
class parallel_output_buffer {
protected:
	array<T2> outputBuffer;
	array<memory_size_type> items;

	void resize_output(memory_size_type n) {
		outputBuffer.resize(n);
		items.resize(n);
	}

public:
	array_view<T2> get_output_buffer(const parallel_options & opts, size_t job) {
		return array_view<T2>(&outputBuffer[opts.bufSize * job], opts.bufSize);
	}
	memory_size_type get_output_size(size_t job) {
		return items[job];
	}
	void set_output_size(size_t job, memory_size_type n) {
		items[job] = n;
	}
};

template <typename T1, typename T2>
class parallel_buffer : public parallel_input_buffer<T1>, public parallel_output_buffer<T2> {
private:
	size_t bufferSize;

public:
	parallel_buffer(const parallel_options opts)
		: bufferSize(opts.numJobs * opts.bufSize)
	{
	}

	void alloc() {
		this->resize_input(bufferSize);
		this->resize_output(bufferSize);
	}

	void dealloc() {
		this->resize_input(0);
		this->resize_output(0);
	}

	memory_size_type memory_usage() {
		return bufferSize*(sizeof(T1)+sizeof(T2));
	}
};

template <typename T>
class parallel_before;
template <typename dest_t>
class parallel_before_impl;
template <typename T>
class parallel_after;

///////////////////////////////////////////////////////////////////////////////
/// \brief Class containing an array of pipe_segment instances. We cannot use
/// tpie::array or similar, since we need to construct the elements in a
/// special way.
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

template <typename T1, typename T2>
class parallel_state;

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
/// \brief  Common state in parallel pipelining library.
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
	void set_output_ptr(size_t idx, pipe_segment * v) {
		m_outputs[idx] = v;
	}

	/// Must not be used concurrently.
	pipe_segment & input(size_t idx) { return *m_inputs[idx]; }

	/// Must not be used concurrently.
	pipe_segment & output(size_t idx) { return *m_outputs[idx]; }

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
	std::vector<pipe_segment *> m_outputs;
	std::vector<parallel_worker_state> m_states;

	parallel_state_base(const parallel_options opts)
		: opts(opts)
		, done(false)
		, runningWorkers(0)
		, m_inputs(opts.numJobs, 0)
		, m_outputs(opts.numJobs, 0)
		, m_states(opts.numJobs, IDLE)
	{
		workerCond = new cond_t[opts.numJobs];
	}

	~parallel_state_base() {
		delete[] workerCond;
	}
};

template <typename T1, typename T2>
class parallel_state : public parallel_state_base {
public:
	typedef boost::shared_ptr<parallel_state> ptr;
	typedef parallel_state_base::mutex_t mutex_t;
	typedef parallel_state_base::cond_t cond_t;
	typedef parallel_state_base::lock_t lock_t;

	parallel_buffer<T1, T2> buffer;

	std::auto_ptr<parallel_pipes<T1, T2> > pipes;

	template <typename fact_t>
	parallel_state(const parallel_options opts, const fact_t & fact)
		: parallel_state_base(opts)
		, buffer(opts)
	{
		typedef parallel_pipes_impl<T1, T2, fact_t> pipes_impl_t;
		pipes.reset(new pipes_impl_t(fact, *this));
	}
};

template <typename T>
class parallel_after : public pipe_segment {
protected:
	parallel_state_base & st;
	size_t parId;
	size_t m_bufferPosition;
	parallel_output_buffer<T> & m_bufferProvider;
	array_view<T> m_buffer;
	typedef parallel_state_base::lock_t lock_t;

public:
	typedef T item_type;

	template <typename Input>
	parallel_after(parallel_state<Input, T> & state,
				   size_t parId)
		: st(state)
		, parId(parId)
		, m_bufferPosition(0)
		, m_bufferProvider(state.buffer)
		, m_buffer(0, (size_t) 0)
	{
		state.set_output_ptr(parId, this);
		set_name("Parallel after", PRIORITY_INSIGNIFICANT);
	}

	parallel_after(const parallel_after & other)
		: pipe_segment(other)
		, st(other.st)
		, parId(other.parId)
		, m_bufferPosition(other.m_bufferPosition)
		, m_bufferProvider(other.m_bufferProvider)
		, m_buffer(other.m_buffer)
	{
		st.set_output_ptr(parId, this);
	}

	virtual void begin() /*override*/ {
		m_buffer = m_bufferProvider.get_output_buffer(st.opts, parId);
	}

	void push(const T & item) {
		if (m_bufferPosition >= m_buffer.size())
			throw std::runtime_error("Buffer overrun in parallel_after");

		m_buffer[m_bufferPosition++] = item;

		if (m_bufferPosition >= m_buffer.size()) flush_buffer();
	}

private:
	bool is_done() const {
		switch (st.get_state(parId)) {
			case IDLE:
				log_debug() << parId << " is now idle" << std::endl;
				return true;
			case PROCESSING:
				log_debug() << parId << " is now directly to processing" << std::endl;
				// This case is reached if our state changes from Outputting to
				// Idle to Processing and we miss a state change
				return true;
			case OUTPUTTING:
				log_debug() << parId << " is still outputting" << std::endl;
				return false;
		}
	}

	void flush_buffer() {
		lock_t lock(st.mutex);
		st.set_state(parId, OUTPUTTING);
		log_debug() << parId << " parallel_after notifying producer that output is ready" << std::endl;
		log_debug() << parId << " parallel_after: wait for state != OUTPUTTING" << std::endl;
		st.producerCond.notify_one();
		while (!is_done()) {
			if (st.done) return;
			st.workerCond[parId].wait(lock);
		}
		m_bufferPosition = 0;
	}
};

template <typename T>
class parallel_before : public pipe_segment {
	class worker_job : public tpie::job {
	public:
		parallel_before<T> * self;

		virtual void operator()() /*override*/ {
			log_debug() << "Job starting" << std::endl;
			self->worker();
		}
	};

	friend class worker_job;

protected:
	parallel_state_base & st;
	size_t parId;
	parallel_input_buffer<T> & m_bufferProvider;
	array_view<T> buffer;
	worker_job job;

	virtual void push_all(array_view<T> items, memory_size_type n) = 0;

	template <typename Output>
	parallel_before(parallel_state<T, Output> & st, size_t parId)
		: st(st)
		, parId(parId)
		, m_bufferProvider(st.buffer)
		, buffer(0, (size_t) 0)
	{
		job.self = this;
		set_name("Parallel before", PRIORITY_INSIGNIFICANT);
	}
	// virtual dtor in pipe_segment

	parallel_before(const parallel_before & other)
		: st(other.st)
		, parId(other.parId)
		, m_bufferProvider(other.m_bufferProvider)
		, buffer(other.buffer)
	{
		job.self = this;
	}

public:
	typedef T item_type;

	virtual void begin() /*override*/ {
		buffer = m_bufferProvider.get_input_buffer(st.opts, parId);
		log_debug() << "Enqueue job" << std::endl;
		job.enqueue();
	}

private:
	bool ready() {
		switch (st.get_state(parId)) {
			case IDLE:
				log_debug() << parId << " is idle" << std::endl;
				return false;
			case PROCESSING:
				log_debug() << parId << " is now processing" << std::endl;
				return true;
			case OUTPUTTING:
				throw std::runtime_error("State 'outputting' was not expected at this point");
		}
	}

	void worker() {
		parallel_state_base::lock_t lock(st.mutex);
		st.runningWorkers++;
		while (true) {
			log_debug() << parId << ": wait for state = processing" << std::endl;
			while (!ready()) {
				if (st.done) {
					log_debug() << parId << " done signal received; return" << std::endl;
					st.runningWorkers--;
					st.producerCond.notify_one();
					return;
				}
				st.workerCond[parId].wait(lock);
			}
			lock.unlock();
			push_all(buffer, m_bufferProvider.get_input_size(parId));
			lock.lock();
		}
	}
};

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

	virtual void push_all(array_view<item_type> items, memory_size_type n) {
		for (size_t i = 0; i < n; ++i) {
			dest.push(items[i]);
		}
	}
};

template <typename T>
class parallel_consumer : public pipe_segment {
public:
	typedef T item_type;

	virtual void consume(array_view<T>) = 0;
	// pipe_segment has virtual dtor
};

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

	virtual void end() /*override*/ {
		st->buffer.dealloc();
	}

	virtual void consume(array_view<item_type> a) /*override*/ {
		for (size_t i = 0; i < a.size(); ++i) {
			dest.push(a[i]);
		}
	}
};

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
	stream_size_type m_remainingItems;

	bool has_ready_pipe() {
		for (size_t i = 0; i < st->opts.numJobs; ++i) {
			switch (st->get_state(i)) {
				case PROCESSING:
					break;
				case IDLE:
				case OUTPUTTING:
					readyIdx = i;
					log_debug() << "Producer: Ready pipe is " << readyIdx << std::endl;
					return true;
			}
		}
		log_debug() << "Producer: No ready pipe" << std::endl;
		return false;
	}

	bool has_outputting_pipe() {
		for (size_t i = 0; i < st->opts.numJobs; ++i) {
			switch (st->get_state(i)) {
				case IDLE:
				case PROCESSING:
					break;
				case OUTPUTTING:
					readyIdx = i;
					log_debug() << "Producer: Outputting pipe is " << readyIdx << std::endl;
					return true;
			}
		}
		log_debug() << "Producer: No outputting pipe" << std::endl;
		return false;
	}

	bool has_processing_pipe() {
		for (size_t i = 0; i < st->opts.numJobs; ++i) {
			switch (st->get_state(i)) {
				case IDLE:
				case OUTPUTTING:
					break;
				case PROCESSING:
					log_debug() << "Producer: Processing pipe is " << i << std::endl;
					return true;
			}
		}
		log_debug() << "Producer: No processing pipe" << std::endl;
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
		this->set_minimum_memory(st->buffer.memory_usage() + st->opts.bufSize * sizeof(item_type));
		this->add_push_destination(cons);
	}

	virtual void begin() /*override*/ {
		st->buffer.alloc();
		inputBuffer.resize(st->opts.bufSize);
		if (!can_fetch("items"))
			throw std::runtime_error("Parallel processing requires 'items' to be known");
		m_remainingItems = fetch<stream_size_type>("items");
	}

	void push(item_type item) {
		if (m_remainingItems == 0)
			throw std::runtime_error("Got more items than expected");

		inputBuffer[written++] = item;
		--m_remainingItems;
		if (written < st->opts.bufSize && m_remainingItems > 0)
			return;

		parallel_state_base::lock_t lock(st->mutex);
		while (written > 0) {
			while (!has_ready_pipe()) {
				log_debug() << "Producer: Has no ready pipe; producerCond.wait" << std::endl;
				st->producerCond.wait(lock);
			}
			switch (st->get_state(readyIdx)) {
				case IDLE:
					std::copy(&inputBuffer[0],
							&inputBuffer[written],
							&st->buffer.get_input_buffer(st->opts, readyIdx)[0]);
					st->buffer.set_input_size(readyIdx, written);
					log_debug() << "Producer: Send buffer to readyIdx " << readyIdx << std::endl;
					st->set_state(readyIdx, PROCESSING);
					st->workerCond[readyIdx].notify_one();
					written = 0;
					break;
				case PROCESSING:
					throw std::runtime_error("State 'processing' not expected at this point");
				case OUTPUTTING:
					log_debug() << "Producer: Receive buffer from readyIdx " << readyIdx << std::endl;
					cons->consume(st->buffer.get_output_buffer(st->opts, readyIdx));
					st->set_state(readyIdx, IDLE);
					st->workerCond[readyIdx].notify_one();
			}
		}

		if (m_remainingItems > 0) return;
		bool done = false;
		while (!done) {
			while (!has_outputting_pipe()) {
				if (!has_processing_pipe()) {
					done = true;
					break;
				}
				log_debug() << "Producer: All items pushed; waiting for processors to complete; producerCond.wait" << std::endl;
				st->producerCond.wait(lock);
			}
			if (done) break;
			cons->consume(st->buffer.get_output_buffer(st->opts, readyIdx));
			st->set_state(readyIdx, IDLE);
		}
		log_debug() << "Producer: Set done = true and notify all workers" << std::endl;
		st->done = true;
		for (size_t i = 0; i < st->opts.numJobs; ++i) {
			st->workerCond[i].notify_one();
		}
		while (st->runningWorkers > 0) {
			log_debug() << "Producer: " << st->runningWorkers << " running workers" << std::endl;
			st->producerCond.wait(lock);
		}
		log_debug() << "Producer: All workers terminated" << std::endl;
	}

	virtual void end() /*override*/ {
		inputBuffer.resize(0);
	}
};

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

template <typename fact_t>
inline pipe_middle<bits::parallel_factory<fact_t> >
parallel(const pipe_middle<fact_t> & fact, size_t numJobs = 2, size_t bufSize = 1) {
	bits::parallel_options opts;
	opts.numJobs = numJobs;
	opts.bufSize = bufSize;
	return pipe_middle<bits::parallel_factory<fact_t> >
		(bits::parallel_factory<fact_t>
		 (fact.factory, opts));
}

} // namespace pipelining

} // namespace tpie

#endif
