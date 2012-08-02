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
#include <tpie/sort.h>
#include <tpie/parallel_sort.h>
#include <tpie/file_stream.h>
#include <tpie/tempname.h>
#include <tpie/memory.h>
#include <queue>
#include <boost/shared_ptr.hpp>

namespace tpie {

namespace pipelining {

struct sort_parameters {
	memory_size_type runLength;
	memory_size_type fanout;
};

template <typename T, typename pred_t>
struct merger {
	inline merger(pred_t pred)
		: pq(predwrap(pred))
	{
	}

	inline bool can_pull() {
		return !pq.empty();
	}

	inline T pull() {
		T el = pq.top().first;
		size_t i = pq.top().second;
		pq.pop();
		if (in[i].can_read() && itemsRead[i] < runLength) {
			pq.push(std::make_pair(in[i].read(), i));
			++itemsRead[i];
		}
		if (!can_pull()) {
			reset();
		}
		return el;
	}

	inline void reset() {
		in.resize(0);
	}

	inline void reset(array<file_stream<T> > & inputs, size_t runLength) {
		this->runLength = runLength;
		tp_assert(pq.empty(), "Reset before we are done");
		n = inputs.size();
		in.swap(inputs);
		for (size_t i = 0; i < n; ++i) {
			pq.push(std::make_pair(in[i].read(), i));
		}
		itemsRead.resize(0);
		itemsRead.resize(n, 1);
	}

	inline static stream_size_type memory_usage(stream_size_type fanout) {
		return array<file_stream<T> >::memory_usage(0)
			+ fanout*file_stream<T>::memory_usage(); // in
	}

	struct predwrap {
		typedef std::pair<T, size_t> item_type;

		predwrap(pred_t pred)
			: pred(pred)
		{
		}

		inline bool operator()(const item_type & lhs, const item_type & rhs) {
			return pred(rhs.first, lhs.first);
		}

	private:
		pred_t pred;
	};

private:
	std::priority_queue<std::pair<T, size_t>, std::vector<std::pair<T, size_t> >, predwrap> pq;
	array<file_stream<T> > in;
	std::vector<size_t> itemsRead;
	size_t runLength;
	size_t n;
};

///////////////////////////////////////////////////////////////////////////////
/// Merge sorting consists of four phases.
///
/// 1. Calculating parameters
/// 2. Sorting and forming runs
/// 3. Merging runs
/// 4. Final merge and report
///
/// If the number of elements received during phase 2 is less than the length
/// of a single run, we are in "report internal" mode, meaning we do not write
/// anything to disk. This causes phase 3 to be a no-op and phase 4 to be a
/// simple array traversal.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t = std::less<T> >
struct merge_sorter {
	typedef boost::shared_ptr<merge_sorter> ptr;

	inline merge_sorter(memory_size_type mem, pred_t pred = pred_t())
		: m_merger(pred)
		, m_runFiles(new array<temp_file>())
		, pull_prepared(false)
		, pred(pred)
	{
		if (mem == 0)
			availableMemory = tpie::get_memory_manager().available();
		else
			availableMemory = mem;

		calculate_parameters();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Enable setting run length and fanout manually (for testing
	/// purposes).
	///////////////////////////////////////////////////////////////////////////
	inline void set_parameters(size_t runLength, size_t fanout) {
		p.runLength = runLength;
		p.fanout = fanout;
		TP_LOG_DEBUG_ID("Set merge sort parameters");
		TP_LOG_DEBUG("Memory available = " << availableMemory << '\n');
		TP_LOG_DEBUG("Run length =       " << p.runLength << " (uses memory " << (p.runLength*sizeof(T) + file_stream<T>::memory_usage()) << ")\n");
		TP_LOG_DEBUG("Fanout =           " << p.fanout << " (uses memory " << fanout_memory_usage(p.fanout) << ")" << std::endl);
	}

	inline void set_available_memory(memory_size_type m) {
		availableMemory = m;
		calculate_parameters();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Initiate phase 2: Formation of input runs.
	///////////////////////////////////////////////////////////////////////////
	inline void begin() {
		TP_LOG_DEBUG_ID("Start forming input runs");
		m_currentRunItems.resize(p.runLength);
		m_runFiles->resize(p.fanout*2);
		m_currentRunItemCount = 0;
		m_finishedRuns = 0;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Push item to merge sorter during phase 2.
	///////////////////////////////////////////////////////////////////////////
	inline void push(const T & item) {
		if (m_currentRunItemCount >= p.runLength) {
			sort_current_run();
			empty_current_run();
		}
		m_currentRunItems[m_currentRunItemCount] = item;
		++m_currentRunItemCount;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief End phase 2.
	///////////////////////////////////////////////////////////////////////////
	inline void end() {
		sort_current_run();
		if (m_finishedRuns == 0) {
			m_reportInternal = true;
			m_itemsPulled = 0;
			TP_LOG_DEBUG_ID("Got " << m_currentRunItemCount << " items. Internal reporting mode.");
		} else {
			m_reportInternal = false;
			empty_current_run();
			m_currentRunItems.resize(0);
			TP_LOG_DEBUG_ID("Got " << m_finishedRuns << " runs. External reporting mode.");
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Perform phase 3: Performing all merges in the merge tree except
	/// the last one.
	///////////////////////////////////////////////////////////////////////////
	inline void calc() {
		if (!m_reportInternal) {
			prepare_pull();
		} else {
			pull_prepared = true;
		}
	}

private:
	///////////////////////////////////////////////////////////////////////////
	// Phase 2 helpers.
	///////////////////////////////////////////////////////////////////////////

	inline void sort_current_run() {
		parallel_sort(m_currentRunItems.begin(), m_currentRunItems.begin()+m_currentRunItemCount, pred);
	}

	// postcondition: m_currentRunItemCount = 0
	inline void empty_current_run() {
		if (m_finishedRuns < 10)
			TP_LOG_DEBUG("Write " << m_currentRunItemCount << " items to run file " << m_finishedRuns << std::endl);
		else if (m_finishedRuns == 10)
			TP_LOG_DEBUG("..." << std::endl);
		file_stream<T> fs;
		open_run_file_write(fs, 0, m_finishedRuns);
		for (size_t i = 0; i < m_currentRunItemCount; ++i) {
			fs.write(m_currentRunItems[i]);
		}
		m_currentRunItemCount = 0;
		++m_finishedRuns;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Prepare m_merger for merging the runNumber'th to the
	/// (runNumber+runCount)'th run in mergeLevel.
	///////////////////////////////////////////////////////////////////////////
	inline void initialize_merger(size_t mergeLevel, size_t runNumber, size_t runCount) {
		array<file_stream<T> > in(runCount);
		for (size_t i = 0; i < runCount; ++i) {
			open_run_file_read(in[i], mergeLevel, runNumber+i);
		}
		size_t runLength = p.runLength;
		for (size_t i = 0; i < mergeLevel; ++i) {
			runLength *= p.fanout;
		}
		m_merger.reset(in, runLength);
	}

	///////////////////////////////////////////////////////////////////////////
	/// Merge the runNumber'th to the (runNumber+runCount)'th in mergeLevel
	/// into mergeLevel+1.
	///////////////////////////////////////////////////////////////////////////
	inline void merge_runs(size_t mergeLevel, size_t runNumber, size_t runCount) {
		initialize_merger(mergeLevel, runNumber, runCount);
		file_stream<T> out;
		open_run_file_write(out, mergeLevel+1, runNumber/p.fanout);
		while (m_merger.can_pull()) {
			out.write(m_merger.pull());
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// Merge all runs and initialize merger for public pulling.
	///////////////////////////////////////////////////////////////////////////
	inline void prepare_pull() {
		size_t mergeLevel = 0;
		size_t runCount = m_finishedRuns;
		while (runCount > p.fanout) {
			TP_LOG_DEBUG_ID("Merge " << runCount << " runs in merge level " << mergeLevel);
			size_t newRunCount = 0;
			for (size_t i = 0; i < runCount; i += p.fanout) {
				size_t n = std::min(runCount-i, p.fanout);

				if (newRunCount < 10)
					TP_LOG_DEBUG("Merge " << n << " runs starting from #" << i << std::endl);
				else if (newRunCount == 10)
					TP_LOG_DEBUG("..." << std::endl);

				merge_runs(mergeLevel, i, std::min(runCount-i, p.fanout));
				++newRunCount;
			}
			++mergeLevel;
			runCount = newRunCount;
		}
		TP_LOG_DEBUG_ID("Final merge level " << mergeLevel << " has " << runCount << " runs");
		initialize_merger(mergeLevel, 0, runCount);

		pull_prepared = true;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	/// In phase 4, return true if there are more items in the final merge
	/// phase.
	///////////////////////////////////////////////////////////////////////////
	inline bool can_pull() {
		tp_assert(pull_prepared, "Pull not prepared");
		if (m_reportInternal) return m_itemsPulled < m_currentRunItemCount;
		else return m_merger.can_pull();
	}

	///////////////////////////////////////////////////////////////////////////
	/// In phase 4, fetch next item in the final merge phase.
	///////////////////////////////////////////////////////////////////////////
	inline T pull() {
		tp_assert(pull_prepared, "Pull not prepared");
		if (m_reportInternal && m_itemsPulled < m_currentRunItemCount) {
			T el = m_currentRunItems[m_itemsPulled++];
			if (!can_pull()) m_currentRunItems.resize(0);
			return el;
		} else {
			return m_merger.pull();
		}
	}

private:
	inline void calculate_parameters() {
		// In the run formation phase, our run length is determined by the number of items we can hold in memory.
		// In the merge phase, our fanout is determined by the size of our merge heap and the stream memory usage.
		// In the final merge, our fanout is determined by the stream memory usage.
		memory_size_type streamMemory = file_stream<T>::memory_usage();
		if (availableMemory < 3*streamMemory) {
			TP_LOG_WARNING_ID("Not enough memory for three open streams! (" << availableMemory << " < " << 3*streamMemory << ")");
			availableMemory = 3*streamMemory;
		}
		const size_t runLength = (availableMemory - streamMemory)/sizeof(T);

		memory_size_type fanout_lo = 2;
		memory_size_type fanout_hi = 4;
		// exponential search
		while (fanout_memory_usage(fanout_hi) < availableMemory) {
			fanout_lo = fanout_hi;
			fanout_hi = fanout_hi*2;
		}
		// binary search
		while (fanout_lo < fanout_hi - 1) {
			memory_size_type mid = fanout_lo + (fanout_hi-fanout_lo)/2;
			if (fanout_memory_usage(mid) < availableMemory) {
				fanout_lo = mid;
			} else {
				fanout_hi = mid;
			}
		}
		set_parameters(runLength, fanout_lo);
	}

	///////////////////////////////////////////////////////////////////////////
	/// calculate_parameters helper
	///////////////////////////////////////////////////////////////////////////
	inline stream_size_type fanout_memory_usage(memory_size_type fanout) {
		return merger<T, pred_t>::memory_usage(fanout) + file_stream<T>::memory_usage();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Figure out the index in m_runFiles of the given run.
	///////////////////////////////////////////////////////////////////////////
	inline size_t run_file_index(size_t mergeLevel, size_t runNumber) {
		return (mergeLevel % 2)*p.fanout + (runNumber % p.fanout);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Open a new run file and seek to the end.
	///////////////////////////////////////////////////////////////////////////
	inline void open_run_file_write(file_stream<T> & fs, size_t mergeLevel, size_t runNumber) {
		size_t idx = run_file_index(mergeLevel, runNumber);
		if (runNumber < p.fanout) m_runFiles->at(idx).free();
		fs.open(m_runFiles->at(idx), file_stream_base::read_write);
		fs.seek(0, file_stream<T>::end);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Open an existing run file and seek to the correct offset.
	///////////////////////////////////////////////////////////////////////////
	inline void open_run_file_read(file_stream<T> & fs, size_t mergeLevel, size_t runNumber) {
		size_t idx = run_file_index(mergeLevel, runNumber);
		fs.open(m_runFiles->at(idx), file_stream_base::read);
		fs.seek(p.runLength * (runNumber / p.fanout), file_stream<T>::beginning);
	}

	sort_parameters p;

	merger<T, pred_t> m_merger;

	boost::shared_ptr<array<temp_file> > m_runFiles;

	// number of runs already written to disk.
	size_t m_finishedRuns;

	// current run buffer. size 0 before begin(), size runLength after begin().
	array<T> m_currentRunItems;

	// number of items in current run buffer.
	size_t m_currentRunItemCount;

	bool m_reportInternal;
	
	// when doing internal reporting: the number of items already reported
	size_t m_itemsPulled;

	bool pull_prepared;

	memory_size_type availableMemory;

	pred_t pred;
};

template <typename T, typename pred_t>
struct sort_input_t : public pipe_segment {
	typedef T item_type;
	typedef merge_sorter<item_type, pred_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;

	inline sort_input_t(sorterptr sorter, const segment_token & token)
		: pipe_segment(token)
		, sorter(sorter)
	{
	}

	inline void begin() {
		sorter->set_available_memory(this->get_available_memory());
		sorter->begin();
	}

	inline void push(const T & item) {
		sorter->push(item);
	}

	inline void end() {
		sorter->end();
	}

private:
	sorterptr sorter;
};

template <typename T, typename pred_t>
struct sort_calc_t : public pipe_segment {
	typedef T item_type;
	typedef merge_sorter<item_type, pred_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;

	inline sort_calc_t(const segment_token & input, const sorterptr & sorter)
		: sorter(sorter)
	{
		add_dependency(input);
	}

	inline sort_calc_t(const pipe_segment & input, const sorterptr & sorter)
		: sorter(sorter)
	{
		add_dependency(input);
	}

	inline void go() {
		sorter->calc();
	}
private:
	sorterptr sorter;
};

template <typename dest_t>
struct sort_output_t : public pipe_segment {
	typedef typename dest_t::item_type item_type;
	typedef merge_sorter<item_type> sorter_t;
	typedef typename sorter_t::ptr sorterptr;

	inline sort_output_t(const dest_t & dest, const pipe_segment & calc, const sorterptr & sorter)
		: dest(dest)
		, sorter(sorter)
	{
		add_dependency(calc);
		add_push_destination(dest);
	}

	void go() {
		dest.begin();
		while (sorter->can_pull()) {
			dest.push(sorter->pull());
		}
		dest.end();
	}
private:
	dest_t dest;
	sorterptr sorter;
};

template <typename T, typename pred_t>
struct sort_pull_output_t : public pipe_segment {
	typedef T item_type;
	typedef merge_sorter<item_type, pred_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;

	inline sort_pull_output_t(const pipe_segment & calc, const sorterptr & sorter)
		: sorter(sorter)
	{
		add_dependency(calc);
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

private:
	sorterptr sorter;
};

template <typename dest_t>
struct sort_t : public pipe_segment {

	typedef typename dest_t::item_type item_type;
	typedef merge_sorter<item_type> sorter_t;
	typedef typename sorter_t::ptr sorterptr;
	typedef sort_calc_t<item_type, std::less<item_type> > calc_t;
	typedef sort_output_t<dest_t> output_t;

	inline sort_t(const sort_t<dest_t> & other)
		: pipe_segment(other)
		, sorter(other.sorter)
		, calc(other.calc)
		, output(other.output)
	{
	}

	inline sort_t(const dest_t & dest)
		: sorter(new sorter_t(0))
		, calc(*this, sorter)
		, output(dest, calc, sorter)
	{
	}

	inline void begin() {
		sorter->set_available_memory(this->get_available_memory());
		sorter->begin();
	}

	inline void push(const item_type & item) {
		sorter->push(item);
	}

	inline void end() {
		sorter->end();
	}

private:
	sorterptr sorter;
	calc_t calc;
	output_t output;
};

inline pipe_middle<factory_0<sort_t> >
pipesort() {
	return factory_0<sort_t>();
}

template <typename T, typename pred_t>
struct passive_sorter {
	typedef T item_type;
	typedef merge_sorter<item_type, pred_t> sorter_t;
	typedef typename sorter_t::ptr sorterptr;
	typedef sort_input_t<item_type, pred_t> input_t;
	typedef sort_calc_t<item_type, pred_t> calc_t;
	typedef sort_pull_output_t<item_type, pred_t> output_t;

	inline passive_sorter()
		: sorter(new sorter_t(0))
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
