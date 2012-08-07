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
	/** Memory available while forming sorted runs. */
	memory_size_type memoryPhase2;
	/** Memory available while merging runs. */
	memory_size_type memoryPhase3;
	/** Memory available during output phase. */
	memory_size_type memoryPhase4;
	/** Run length, subject to memory restrictions during phase 2. */
	memory_size_type runLength;
	/** Maximum item count for internal reporting, subject to memory
	 * restrictions in all phases. Less or equal to runLength. */
	memory_size_type internalReportThreshold;
	/** Fanout of merge tree during phase 3. */
	memory_size_type fanout;
	/** Fanout of merge tree during phase 4. Less or equal to fanout. */
	memory_size_type finalFanout;

	void dump(std::ostream & out) const {
		out << "Merge sort parameters\n"
			<< "Phase 2 memory:              " << memoryPhase2 << '\n'
			<< "Run length:                  " << runLength << '\n'
			<< "Phase 3 memory:              " << memoryPhase3 << '\n'
			<< "Fanout:                      " << fanout << '\n'
			<< "Phase 4 memory:              " << memoryPhase4 << '\n'
			<< "Final merge level fanout:    " << finalFanout << '\n'
			<< "Internal report threshold:   " << internalReportThreshold << '\n';
	}
};

template <typename T, typename pred_t>
struct merger {
	inline merger(pred_t pred)
		: pq(0, predwrap(pred))
	{
	}

	inline bool can_pull() {
		return !pq.empty();
	}

	inline T pull() {
		tp_assert(can_pull(), "pull() while !can_pull()");
		T el = pq.top().first;
		size_t i = pq.top().second;
		if (in[i].can_read() && itemsRead[i] < runLength) {
			pq.pop_and_push(std::make_pair(in[i].read(), i));
			++itemsRead[i];
		} else {
			pq.pop();
		}
		if (!can_pull()) {
			reset();
		}
		return el;
	}

	inline void reset() {
		in.resize(0);
		pq.resize(0);
	}

	// Initialize merger with given sorted input runs. Each file stream is
	// assumed to have a stream offset pointing to the first item in the run,
	// and runLength items are read from each stream (unless end of stream
	// occurs earlier).
	// Precondition: !can_pull()
	inline void reset(array<file_stream<T> > & inputs, size_t runLength) {
		this->runLength = runLength;
		tp_assert(pq.empty(), "Reset before we are done");
		n = inputs.size();
		in.swap(inputs);
		pq.resize(n);
		for (size_t i = 0; i < n; ++i) {
			pq.unsafe_push(std::make_pair(in[i].read(), i));
		}
		pq.make_safe();
		itemsRead.resize(n, 1);
	}

	inline static stream_size_type memory_usage(stream_size_type fanout) {
		return sizeof(merger)
			- sizeof(internal_priority_queue<std::pair<T, size_t>, predwrap>) // pq
			+ internal_priority_queue<std::pair<T, size_t>, predwrap>::memory_usage(fanout) // pq
			- sizeof(array<file_stream<T> >) // in
			+ array<file_stream<T> >::memory_usage(fanout) // in
			- fanout*sizeof(file_stream<T>) // in file_streams
			+ fanout*file_stream<T>::memory_usage() // in file_streams
			- sizeof(array<size_t>) // itemsRead
			+ array<size_t>::memory_usage(fanout) // itemsRead
			;
	}

	struct predwrap {
		typedef std::pair<T, size_t> item_type;
		typedef item_type first_argument_type;
		typedef item_type second_argument_type;
		typedef bool result_type;

		predwrap(pred_t pred)
			: pred(pred)
		{
		}

		inline bool operator()(const item_type & lhs, const item_type & rhs) {
			return pred(lhs.first, rhs.first);
		}

	private:
		pred_t pred;
	};

private:
	internal_priority_queue<std::pair<T, size_t>, predwrap> pq;
	array<file_stream<T> > in;
	array<size_t> itemsRead;
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

	inline merge_sorter(pred_t pred = pred_t())
		: m_parametersSet(false)
		, m_merger(pred)
		, m_runFiles(new array<temp_file>())
		, pull_prepared(false)
		, pred(pred)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Enable setting run length and fanout manually (for testing
	/// purposes).
	///////////////////////////////////////////////////////////////////////////
	inline void set_parameters(size_t runLength, size_t fanout) {
		p.runLength = p.internalReportThreshold = runLength;
		p.fanout = p.finalFanout = fanout;
		m_parametersSet = true;
		log_debug() << "Manually set merge sort run length and fanout\n";
		log_debug() << "Run length =       " << p.runLength << " (uses memory " << (p.runLength*sizeof(T) + file_stream<T>::memory_usage()) << ")\n";
		log_debug() << "Fanout =           " << p.fanout << " (uses memory " << fanout_memory_usage(p.fanout) << ")" << std::endl;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate parameters from given memory amount.
	/// \param m Memory available for phase 2, 3 and 4
	///////////////////////////////////////////////////////////////////////////
	inline void set_available_memory(memory_size_type m) {
		calculate_parameters(m, m, m);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate parameters from given memory amount.
	/// \param m2 Memory available for phase 2
	/// \param m3 Memory available for phase 3
	/// \param m4 Memory available for phase 4
	///////////////////////////////////////////////////////////////////////////
	inline void set_available_memory(memory_size_type m2, memory_size_type m3, memory_size_type m4) {
		calculate_parameters(m2, m3, m4);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Initiate phase 2: Formation of input runs.
	///////////////////////////////////////////////////////////////////////////
	inline void begin() {
		tp_assert(m_parametersSet, "Parameters not set");
		log_debug() << "Start forming input runs" << std::endl;
		m_currentRunItems.resize(p.runLength);
		m_runFiles->resize(p.fanout*2);
		m_currentRunItemCount = 0;
		m_finishedRuns = 0;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Push item to merge sorter during phase 2.
	///////////////////////////////////////////////////////////////////////////
	inline void push(const T & item) {
		tp_assert(m_parametersSet, "Parameters not set");
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
		tp_assert(m_parametersSet, "Parameters not set");
		sort_current_run();
		if (m_finishedRuns == 0 && m_currentRunItemCount <= p.internalReportThreshold) {
			m_reportInternal = true;
			m_itemsPulled = 0;
			log_debug() << "Got " << m_currentRunItemCount << " items. Internal reporting mode." << std::endl;
		} else {
			m_reportInternal = false;
			empty_current_run();
			m_currentRunItems.resize(0);
			log_debug() << "Got " << m_finishedRuns << " runs. External reporting mode." << std::endl;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Perform phase 3: Performing all merges in the merge tree except
	/// the last one.
	///////////////////////////////////////////////////////////////////////////
	inline void calc() {
		tp_assert(m_parametersSet, "Parameters not set");
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
			log_debug() << "Write " << m_currentRunItemCount << " items to run file " << m_finishedRuns << std::endl;
		else if (m_finishedRuns == 10)
			log_debug() << "..." << std::endl;
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
		// Open files and seek to the first item in the run.
		array<file_stream<T> > in(runCount);
		for (size_t i = 0; i < runCount; ++i) {
			open_run_file_read(in[i], mergeLevel, runNumber+i);
		}
		size_t runLength = calculate_run_length(p.runLength, p.fanout, mergeLevel);
		// Pass file streams with correct stream offsets to the merger
		m_merger.reset(in, runLength);
	}

	///////////////////////////////////////////////////////////////////////////
	/// Prepare m_merger for merging the runCount runs in finalMergeLevel.
	///////////////////////////////////////////////////////////////////////////
	inline void initialize_final_merger(size_t finalMergeLevel, size_t runCount) {
		if (runCount > p.finalFanout) {
			log_debug() << "Run count in final level (" << runCount << ") is greater than the final fanout (" << p.finalFanout << ")\n";
			size_t runNumber;
			{
				size_t i = p.finalFanout-1;
				size_t n = runCount-(p.finalFanout-1);
				log_debug() << "Merge " << n << " runs starting from #" << i << std::endl;
				runNumber = merge_runs(finalMergeLevel, i, n);
			}
			array<file_stream<T> > in(p.finalFanout);
			for (size_t i = 0; i < p.finalFanout-1; ++i) {
				open_run_file_read(in[i], finalMergeLevel, i);
				log_debug() << "Run " << i << " is at offset " << in[i].offset() << " and has size " << in[i].size() << std::endl;
			}
			open_run_file_read(in[p.finalFanout-1], finalMergeLevel+1, runNumber);
			log_debug() << "Special large run is at offset " << in[p.finalFanout-1].offset() << " and has size " << in[p.finalFanout-1].size() << std::endl;
			size_t runLength = calculate_run_length(p.runLength, p.fanout, finalMergeLevel+1);
			log_debug() << "Run length " << runLength << std::endl;
			m_merger.reset(in, runLength);
		} else {
			log_debug() << "Run count in final level (" << runCount << ") is less or equal to the final fanout (" << p.finalFanout << ")" << std::endl;
			initialize_merger(finalMergeLevel, 0, runCount);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// initialize_merger helper.
	///////////////////////////////////////////////////////////////////////////
	static inline size_t calculate_run_length(size_t initialRunLength, size_t fanout, size_t mergeLevel) {
		size_t runLength = initialRunLength;
		for (size_t i = 0; i < mergeLevel; ++i) {
			runLength *= fanout;
		}
		return runLength;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Merge the runNumber'th to the (runNumber+runCount)'th in mergeLevel
	/// into mergeLevel+1.
	/// \returns The run number in mergeLevel+1 that was written to.
	///////////////////////////////////////////////////////////////////////////
	inline size_t merge_runs(size_t mergeLevel, size_t runNumber, size_t runCount) {
		initialize_merger(mergeLevel, runNumber, runCount);
		file_stream<T> out;
		size_t nextRunNumber = runNumber/p.fanout;
		open_run_file_write(out, mergeLevel+1, nextRunNumber);
		while (m_merger.can_pull()) {
			out.write(m_merger.pull());
		}
		return nextRunNumber;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Phase 3: Merge all runs and initialize merger for public pulling.
	///////////////////////////////////////////////////////////////////////////
	inline void prepare_pull() {
		size_t mergeLevel = 0;
		size_t runCount = m_finishedRuns;
		while (runCount > p.fanout) {
			log_debug() << "Merge " << runCount << " runs in merge level " << mergeLevel << '\n';
			size_t newRunCount = 0;
			for (size_t i = 0; i < runCount; i += p.fanout) {
				size_t n = std::min(runCount-i, p.fanout);

				if (newRunCount < 10)
					log_debug() << "Merge " << n << " runs starting from #" << i << std::endl;
				else if (newRunCount == 10)
					log_debug() << "..." << std::endl;

				merge_runs(mergeLevel, i, n);
				++newRunCount;
			}
			++mergeLevel;
			runCount = newRunCount;
		}
		log_debug() << "Final merge level " << mergeLevel << " has " << runCount << " runs" << std::endl;
		initialize_final_merger(mergeLevel, runCount);

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
	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate parameters from given memory amount.
	/// \param m2 Memory available for phase 2
	/// \param m3 Memory available for phase 3
	/// \param m4 Memory available for phase 4
	///////////////////////////////////////////////////////////////////////////
	inline void calculate_parameters(memory_size_type m2, memory_size_type m3, memory_size_type m4) {
		// We must set aside memory for temp_files in m_runFiles.
		// m_runFiles contains fanout*2 temp_files, so calculate fanout before run length.

		// Phase 3 (merge):
		// Run length: unbounded
		// Fanout: determined by the size of our merge heap and the stream memory usage.
		log_debug() << "Phase 3: " << m3 << " b available memory\n";
		p.fanout = calculate_fanout(m3);
		if (fanout_memory_usage(p.fanout) > m3) {
			log_debug() << "Not enough memory for fanout " << p.fanout << "! (" << m3 << " < " << fanout_memory_usage(p.fanout) << ")\n";
			m3 = fanout_memory_usage(p.fanout);
		}

		// Phase 4 (final merge & report):
		// Run length: unbounded
		// Fanout: determined by the stream memory usage.
		log_debug() << "Phase 4: " << m4 << " b available memory\n";
		p.finalFanout = calculate_fanout(m4);

		if (p.finalFanout > p.fanout)
			p.finalFanout = p.fanout;

		if (fanout_memory_usage(p.finalFanout) > m4) {
			log_debug() << "Not enough memory for fanout " << p.finalFanout << "! (" << m4 << " < " << fanout_memory_usage(p.finalFanout) << ")\n";
			m4 = fanout_memory_usage(p.finalFanout);
		}

		// Phase 2 (run formation):
		// Run length: determined by the number of items we can hold in memory.
		// Fanout: unbounded

		memory_size_type streamMemory = file_stream<T>::memory_usage();
		memory_size_type tempFileMemory = 2*p.fanout*sizeof(temp_file);

		log_debug() << "Phase 2: " << m2 << " b available memory; " << streamMemory << " b for a single stream; " << tempFileMemory << " b for temp_files\n";
		memory_size_type min_m2 = sizeof(T) + streamMemory + tempFileMemory;
		if (m2 < min_m2) {
			log_warning() << "Not enough phase 2 memory for an item and an open stream! (" << m2 << " < " << min_m2 << ")\n";
			m2 = min_m2;
		}
		p.runLength = (m2 - streamMemory - tempFileMemory)/sizeof(T);

		p.internalReportThreshold = (std::min(m2, std::min(m3, m4)) - tempFileMemory)/sizeof(T);
		if (p.internalReportThreshold > p.runLength)
			p.internalReportThreshold = p.runLength;

		p.memoryPhase2 = m2;
		p.memoryPhase3 = m3;
		p.memoryPhase4 = m4;

		m_parametersSet = true;

		log_debug() << "Calculated merge sort parameters\n";
		p.dump(log_debug());
		log_debug() << std::endl;
	}

	///////////////////////////////////////////////////////////////////////////
	/// calculate_parameters helper
	///////////////////////////////////////////////////////////////////////////
	static inline memory_size_type calculate_fanout(memory_size_type availableMemory) {
		memory_size_type fanout_lo = 2;
		memory_size_type fanout_hi = 251; // arbitrary. TODO: run experiments to find threshold
		// binary search
		while (fanout_lo < fanout_hi - 1) {
			memory_size_type mid = fanout_lo + (fanout_hi-fanout_lo)/2;
			if (fanout_memory_usage(mid) < availableMemory) {
				fanout_lo = mid;
			} else {
				fanout_hi = mid;
			}
		}
		return fanout_lo;
	}

	///////////////////////////////////////////////////////////////////////////
	/// calculate_parameters helper
	///////////////////////////////////////////////////////////////////////////
	static inline stream_size_type fanout_memory_usage(memory_size_type fanout) {
		return merger<T, pred_t>::memory_usage(fanout) // accounts for the `fanout' open streams
			+ file_stream<T>::memory_usage() // output stream
			+ 2*sizeof(temp_file); // merge_sorter::m_runFiles
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
	bool m_parametersSet;

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
		: sorter(new sorter_t())
		, calc(*this, sorter)
		, output(dest, calc, sorter)
	{
	}

	inline void begin() {
		sorter->set_available_memory(this->get_available_memory(), calc.get_available_memory(), output.get_available_memory());
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
		: sorter(new sorter_t())
		, calc(input_token, sorter)
		, m_output(calc, sorter)
	{
		// XXX: we need to call sorter->set_available_memory at some point
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
