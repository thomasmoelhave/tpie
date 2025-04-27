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

#ifndef __TPIE_PIPELINING_MERGE_SORTER_H__
#define __TPIE_PIPELINING_MERGE_SORTER_H__

#include <tpie/tpie_export.h>
#include <tpie/compressed/stream.h>
#include <tpie/pipelining/sort_parameters.h>
#include <tpie/pipelining/merger.h>
#include <tpie/pipelining/node.h>
#include <tpie/pipelining/exception.h>
#include <tpie/dummy_progress.h>
#include <tpie/array_view.h>
#include <tpie/parallel_sort.h>

namespace tpie {

namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \brief  Class to maintain the positions where sorted runs start.
///
/// The run_positions object has the following states:
/// * closed
/// * open
/// * open, evacuated
/// * open, final
/// * open, evacuated, final
///
/// When open and not evacuated, the memory usage is two stream blocks
/// (as reported by memory_usage()).
/// When evacuated, the memory usage is nothing.
///
/// The object remembers the merge tree depth `d`.
/// Initially, d = 1, and only set_position may be called with mergeLevel = 0
/// and runNumbers always increasing by one.
/// When next_level is called, d increases to 2, and now set_position may be
/// called with mergeLevel = 1, and get_position may be called with
/// mergeLevel = 0, runNumber ranging from 0 and upwards in each case.
/// When final_level is called, the restriction on the runNumber order is
/// lifted, but set_position may only be called with mergeLevel = d-1 and
/// runNumber = 0. get_position may be called with mergeLevel = d-2 and any
/// runNumber in any order.
///////////////////////////////////////////////////////////////////////////////
class TPIE_EXPORT run_positions {
public:
	run_positions();
	~run_positions();

	///////////////////////////////////////////////////////////////////////////
	/// Memory usage when open and not evacuated.
	///////////////////////////////////////////////////////////////////////////
	static memory_size_type memory_usage() noexcept;

	///////////////////////////////////////////////////////////////////////////
	/// Switch from `closed` to `open` state.
	///////////////////////////////////////////////////////////////////////////
	void open();

	///////////////////////////////////////////////////////////////////////////
	/// Switch from any state to `closed` state.
	///////////////////////////////////////////////////////////////////////////
	void close();

	///////////////////////////////////////////////////////////////////////////
	/// Switch from any state to the corresponding evacuated state.
	///////////////////////////////////////////////////////////////////////////
	void evacuate();

	///////////////////////////////////////////////////////////////////////////
	/// Switch from any state to the corresponding non-evacuated state.
	///////////////////////////////////////////////////////////////////////////
	void unevacuate();

	///////////////////////////////////////////////////////////////////////////
	/// Go to next level in the merge heap - see class docstring.
	///////////////////////////////////////////////////////////////////////////
	void next_level();

	///////////////////////////////////////////////////////////////////////////
	/// Set this to be the final level in the merge heap - see class docstring.
	///////////////////////////////////////////////////////////////////////////
	void final_level(memory_size_type fanout);

	///////////////////////////////////////////////////////////////////////////
	/// Store a stream position - see class docstring.
	///////////////////////////////////////////////////////////////////////////
	void set_position(memory_size_type mergeLevel, memory_size_type runNumber, stream_position pos);

	///////////////////////////////////////////////////////////////////////////
	/// Fetch a stream position - see class docstring.
	///////////////////////////////////////////////////////////////////////////
	stream_position get_position(memory_size_type mergeLevel, memory_size_type runNumber);

private:
	/** Object state: Whether we are open. */
	bool m_open;
	/** Object state: Whether we are evacuated. */
	bool m_evacuated;
	/** Object state: Whether we are in the final merge level. */
	bool m_final;

	/** The merge tree depth, denoted `d`. */
	memory_size_type m_levels;

	memory_size_type m_runs[2];
	temp_file m_positionsFile[2];
	stream_position m_positionsPosition[2];
	file_stream<stream_position> m_positions[2];

	/** If final: the stream positions in mergeLevel = d-2. */
	array<stream_position> m_finalPositions;
	/** If final: Whether the (d-1, 0)-position is stored. */
	bool m_finalExtraSet;
	/** If finalExtraSet: The (d-1, 0)-position. */
	stream_position m_finalExtra;
};

} // namespace bits

class TPIE_EXPORT merge_sorter_base {
public:
	merge_sorter_base(
		linear_memory_usage fanout_memory_usage,
		memory_size_type item_size,
		memory_size_type element_file_stream_memory_usage);

	static const memory_size_type defaultFiles = 253; // Default number of files available, when not using set_available_files
	static const memory_size_type minimumFilesPhase1 = 1;
	static const memory_size_type maximumFilesPhase1 = 1;
	static const memory_size_type minimumFilesPhase2 = 5;
	static const memory_size_type maximumFilesPhase2 = std::numeric_limits<memory_size_type>::max();
	static const memory_size_type minimumFilesPhase3 = 5;
	static const memory_size_type maximumFilesPhase3 = std::numeric_limits<memory_size_type>::max();
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Enable setting run length and fanout manually (for testing
	/// purposes).
	///////////////////////////////////////////////////////////////////////////
	void set_parameters(memory_size_type runLength, memory_size_type fanout);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate parameters from given amount of files.
	/// \param f Files available for phase 1, 2 and 3
	///////////////////////////////////////////////////////////////////////////
	void set_available_files(memory_size_type f) {
		p.filesPhase1 = p.filesPhase2 = p.filesPhase3 = f;
		check_not_started();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate parameters from given amount of files.
	/// \param f1 Files available for phase 1
	/// \param f2 Files available for phase 2
	/// \param f3 Files available for phase 3
	///////////////////////////////////////////////////////////////////////////
	void set_available_files(memory_size_type f1, memory_size_type f2, memory_size_type f3) {
		p.filesPhase1 = f1;
		p.filesPhase2 = f2;
		p.filesPhase3 = f3;
		check_not_started();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate parameters from given memory amount.
	/// \param m Memory available for phase 1, 2 and 3
	///////////////////////////////////////////////////////////////////////////
	void set_available_memory(memory_size_type m) {
		p.memoryPhase1 = p.memoryPhase2 = p.memoryPhase3 = m;
		check_not_started();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate parameters from given memory amount.
	/// \param m1 Memory available for phase 1
	/// \param m2 Memory available for phase 2
	/// \param m3 Memory available for phase 3
	///////////////////////////////////////////////////////////////////////////
	void set_available_memory(memory_size_type m1, memory_size_type m2, memory_size_type m3) {
		p.memoryPhase1 = m1;
		p.memoryPhase2 = m2;
		p.memoryPhase3 = m3;
		check_not_started();
	}

	stream_size_type item_count() {
		return m_itemCount;
	}


	memory_size_type evacuated_memory_usage() const {
		return 2*p.fanout*sizeof(temp_file);
	}


	///////////////////////////////////////////////////////////////////////////
	/// \brief Set upper bound on number of items pushed.
	///
	/// If the number of items to push is less than the size of a single run,
	/// this method will decrease the run size to that.
	/// This may make it easier for the sorter to go into internal reporting
	/// mode.
	///////////////////////////////////////////////////////////////////////////
	void set_items(stream_size_type n);

	void set_owner(tpie::pipelining::node * n);

	void set_phase_1_files(memory_size_type f1) {
		p.filesPhase1 = f1;
		check_not_started();
	}

	void set_phase_2_files(memory_size_type f2) {
		p.filesPhase2 = f2;
		check_not_started();
	}

	void set_phase_3_files(memory_size_type f3) {
		p.filesPhase3 = f3;
		check_not_started();
	}

	void set_phase_1_memory(memory_size_type m1) {
		p.memoryPhase1 = m1;
		check_not_started();
	}

	void set_phase_2_memory(memory_size_type m2) {
		p.memoryPhase2 = m2;
		check_not_started();
	}

	void set_phase_3_memory(memory_size_type m3) {
		p.memoryPhase3 = m3;
		check_not_started();
	}

	bool is_calc_free() const {
		tp_assert(m_state == stMerge, "Wrong phase");
		return m_reportInternal || m_finishedRuns <= p.fanout;
	}

	memory_size_type minimum_memory_phase_1() noexcept {
		// Our fanout calculation cannot take the memory available in this phase
		// (run formation) into account. Thus, we assume the largest fanout, meaning
		// we might overshoot. If we do overshoot, we will just spend the extra
		// bytes on a run length longer than 1, which is probably what the user
		// wants anyway.
		sort_parameters tmp_p((sort_parameters()));
		tmp_p.runLength = std::max<memory_size_type>(128*1024 / m_item_size + 1, 16);
		tmp_p.fanout = calculate_fanout(std::numeric_limits<memory_size_type>::max(), 0);
		return phase_1_memory(tmp_p);
	}

	memory_size_type minimum_memory_phase_2() noexcept {
		return m_fanout_memory_usage(calculate_fanout(0, 0));
	}

	memory_size_type minimum_memory_phase_3() noexcept {
		return m_fanout_memory_usage(calculate_fanout(0, 0));
	}

	memory_size_type maximum_memory_phase_3() noexcept {
		return std::numeric_limits<memory_size_type>::max();
	}

	memory_size_type phase_1_memory(const sort_parameters & params) noexcept {
		return params.runLength * m_item_size
			+ bits::run_positions::memory_usage()
			+ m_element_file_stream_memory_usage
			+ 2*params.fanout*sizeof(temp_file);
	}

	memory_size_type phase_2_memory(const sort_parameters & params) noexcept {
		return m_fanout_memory_usage(params.fanout);
	}

	memory_size_type phase_3_memory(const sort_parameters & params) noexcept {
		return m_fanout_memory_usage(params.finalFanout);
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// calculate_parameters helper
	///////////////////////////////////////////////////////////////////////////
	memory_size_type calculate_fanout(memory_size_type availableMemory, memory_size_type availableFiles) noexcept;
	
protected:

	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate parameters from given memory amount.
	///////////////////////////////////////////////////////////////////////////
	void calculate_parameters();
	
	// Checks if we should still be able to change parameters
	void check_not_started() {
		if (m_state != stNotStarted) {
			throw tpie::exception("Can't change parameters after merge sorting has started");
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// initialize_merger helper.
	///////////////////////////////////////////////////////////////////////////
	static stream_size_type calculate_run_length(stream_size_type initialRunLength, memory_size_type fanout, memory_size_type mergeLevel) {
		stream_size_type runLength = initialRunLength;
		for (memory_size_type i = 0; i < mergeLevel; ++i) {
			runLength *= fanout;
		}
		return runLength;
	}

	enum state_type {
		stNotStarted,
		stRunFormation,
		stMerge,
		stReport
	};

	const linear_memory_usage m_fanout_memory_usage;
    const memory_size_type m_item_size, m_element_file_stream_memory_usage;
	
	std::unique_ptr<memory_bucket> m_bucketPtr;
	memory_bucket_ref m_bucket;

	array<temp_file> m_runFiles;

	state_type m_state;

	sort_parameters p;
	bool m_parametersSet;

	bits::run_positions m_runPositions;

	// Number of runs already written to disk.
	// On 32-bit systems, we could in principle support more than 2^32 finished runs,
	// but keeping this as a memory_size_type is nicer when doing the actual merges.
	stream_size_type m_finishedRuns;

	// Number of items in current run buffer.
	// Used to index into m_currentRunItems, so memory_size_type.
	memory_size_type m_currentRunItemCount;

	bool m_reportInternal;

	// When doing internal reporting: the number of items already reported
	// Used in comparison with m_currentRunItemCount
	memory_size_type m_itemsPulled;

	stream_size_type m_itemCount;

	stream_size_type m_maxItems;
	
	bool m_evacuated;
	bool m_finalMergeInitialized;
	memory_size_type m_finalMergeLevel;
	memory_size_type m_finalRunCount;
	memory_size_type m_finalMergeSpecialRunNumber;

	tpie::pipelining::node * m_owning_node;
};


///////////////////////////////////////////////////////////////////////////////
/// Merge sorting consists of three phases.
///
/// 1. Sorting and forming runs
/// 2. Merging runs
/// 3. Final merge and report
///
/// If the number of elements received during phase 1 is less than the length
/// of a single run, we are in "report internal" mode, meaning we do not write
/// anything to disk. This causes phase 2 to be a no-op and phase 3 to be a
/// simple array traversal.
///////////////////////////////////////////////////////////////////////////////
template <typename T, bool UseProgress, typename pred_t = std::less<T>, typename store_t=default_store>
class merge_sorter: public merge_sorter_base {
private:
	typedef typename store_t::template element_type<T>::type TT;
	typedef typename store_t::template specific<TT> specific_store_t;
	typedef typename specific_store_t::outer_type outer_type;	//Should be the same as T
	typedef typename specific_store_t::store_type store_type;
	typedef typename specific_store_t::element_type element_type;	//Should be the same as TT
	typedef outer_type item_type;
	static const size_t item_size = specific_store_t::item_size;
public:

	typedef std::shared_ptr<merge_sorter> ptr;
	typedef progress_types<UseProgress> Progress;
	
	merge_sorter(pred_t pred = pred_t(), store_t store = store_t())
		: merge_sorter_base(fanout_memory_usage(), specific_store_t::item_size, file_stream<element_type>::memory_usage())
		, m_store(store.template get_specific<element_type>())
		, m_merger(pred, m_store, m_bucket)
		, m_currentRunItems(m_bucket)
		, pred(pred)
		{}
	

public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Initiate phase 1: Formation of input runs.
	///////////////////////////////////////////////////////////////////////////
	void begin() {
		tp_assert(m_state == stNotStarted, "Merge sorting already begun");
		if (!m_parametersSet) calculate_parameters();
		log_pipe_debug() << "Start forming input runs" << std::endl;
		m_currentRunItems = array<store_type>(0, allocator<store_type>(m_bucket));
		m_currentRunItems.resize((size_t)p.runLength);
		m_runFiles.resize(p.fanout*2);
		m_currentRunItemCount = 0;
		m_finishedRuns = 0;
		m_state = stRunFormation;
		m_itemCount = 0;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Push item to merge sorter during phase 1.
	///////////////////////////////////////////////////////////////////////////
	void push(item_type && item) {
		tp_assert(m_state == stRunFormation, "Wrong phase");
		if (m_currentRunItemCount >= p.runLength) {
			sort_current_run();
			empty_current_run();
		}
		m_currentRunItems[m_currentRunItemCount] = m_store.outer_to_store(std::move(item));
		++m_currentRunItemCount;
		++m_itemCount;
	}
	
	void push(const item_type & item) {
		tp_assert(m_state == stRunFormation, "Wrong phase");
		if (m_currentRunItemCount >= p.runLength) {
			sort_current_run();
			empty_current_run();
		}
		m_currentRunItems[m_currentRunItemCount] = m_store.outer_to_store(item);
		++m_currentRunItemCount;
		++m_itemCount;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief End phase 1.
	///////////////////////////////////////////////////////////////////////////
	void end() {
		tp_assert(m_state == stRunFormation, "Wrong phase");
		sort_current_run();

		if (m_itemCount == 0) {
			tp_assert(m_currentRunItemCount == 0, "m_itemCount == 0, but m_currentRunItemCount != 0");
			m_reportInternal = true;
			m_itemsPulled = 0;
			m_currentRunItems.resize(0);
			log_debug() << "Got no items. Internal reporting mode." << std::endl;
		} else if (m_finishedRuns == 0 && m_currentRunItems.size() <= p.internalReportThreshold) {
			// Our current buffer fits within the memory requirements of phase 2.
			m_reportInternal = true;
			m_itemsPulled = 0;
			log_debug() << "Got " << m_currentRunItemCount << " items. Internal reporting mode." << std::endl;

		} else if (m_finishedRuns == 0
				   && m_currentRunItemCount <= p.internalReportThreshold
				   && array<store_type>::memory_usage(m_currentRunItemCount) <= get_memory_manager().available()) {
			// Our current buffer does not fit within the memory requirements
			// of phase 2, but we have enough temporary memory to copy and
			// resize the buffer.

			array<store_type> currentRun(m_currentRunItemCount);
			for (size_t i=0; i < m_currentRunItemCount; ++i)
				currentRun[i] = std::move(m_currentRunItems[i]);
			m_currentRunItems.swap(currentRun);

			m_reportInternal = true;
			m_itemsPulled = 0;
			log_debug() << "Got " << m_currentRunItemCount << " items. Internal reporting mode "
				<< "after resizing item buffer." << std::endl;

		} else {
			m_reportInternal = false;
			empty_current_run();
			m_currentRunItems.resize(0);
			log_debug() << "Got " << m_finishedRuns << " runs. External reporting mode." << std::endl;
		}
		m_state = stMerge;
	}

	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Perform phase 2: Performing all merges in the merge tree except
	/// the last one.
	///////////////////////////////////////////////////////////////////////////
	void calc(typename Progress::base & pi) {
		tp_assert(m_state == stMerge, "Wrong phase");
		if (!m_reportInternal) {
			prepare_pull(pi);
		} else {
			pi.init(1);
			pi.step();
			pi.done();
		}
		m_state = stReport;
	}

	void evacuate() {
		tp_assert(m_state == stMerge || m_state == stReport, "Wrong phase");
		if (m_reportInternal) {
			log_pipe_debug() << "Evacuate merge_sorter (" << this << ") in internal reporting mode" << std::endl;
			m_reportInternal = false;
			memory_size_type runCount = (m_currentRunItemCount > 0) ? 1 : 0;
			empty_current_run();
			m_currentRunItems.resize(0);
			initialize_final_merger(0, runCount);
		} else if (m_state == stMerge) {
			log_pipe_debug() << "Evacuate merge_sorter (" << this << ") before merge in external reporting mode (noop)" << std::endl;
			m_runPositions.evacuate();
			return;
		}
		log_pipe_debug() << "Evacuate merge_sorter (" << this << ") before reporting in external reporting mode" << std::endl;
		m_merger.reset();
		m_evacuated = true;
		m_runPositions.evacuate();
	}

	void evacuate_before_merging() {
		if (m_state == stMerge) evacuate();
	}

	void evacuate_before_reporting() {
		if (m_state == stReport && (!m_reportInternal || m_itemsPulled == 0)) evacuate();
	}

private:
	///////////////////////////////////////////////////////////////////////////
	// Phase 1 helpers.
	///////////////////////////////////////////////////////////////////////////

	void sort_current_run() {
		parallel_sort(m_currentRunItems.begin(), m_currentRunItems.begin()+m_currentRunItemCount, 
					  bits::store_pred<pred_t, specific_store_t>(pred));
	}

	// postcondition: m_currentRunItemCount = 0
	void empty_current_run() {
		if (m_finishedRuns < 10)
			log_pipe_debug() << "Write " << m_currentRunItemCount << " items to run file " << m_finishedRuns << std::endl;
		else if (m_finishedRuns == 10)
			log_pipe_debug() << "..." << std::endl;
		file_stream<element_type> fs;
		open_run_file_write(fs, 0, m_finishedRuns);
		for (memory_size_type i = 0; i < m_currentRunItemCount; ++i)
			fs.write(m_store.store_to_element(std::move(m_currentRunItems[i])));
		m_currentRunItemCount = 0;
		++m_finishedRuns;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Prepare m_merger for merging the runNumber'th to the
	/// (runNumber+runCount)'th run in mergeLevel.
	///////////////////////////////////////////////////////////////////////////
	void initialize_merger(memory_size_type mergeLevel, memory_size_type runNumber, memory_size_type runCount) {
		// runCount is a memory_size_type since we must be able to have that
		// many file_streams open at the same time.

		// Open files and seek to the first item in the run.
		array<file_stream<element_type> > in(runCount);
		for (memory_size_type i = 0; i < runCount; ++i) {
			open_run_file_read(in[i], mergeLevel, runNumber+i);
		}
		stream_size_type runLength = calculate_run_length(p.runLength, p.fanout, mergeLevel);
		// Pass file streams with correct stream offsets to the merger
		m_merger.reset(in, runLength);
	}

	///////////////////////////////////////////////////////////////////////////
	/// Prepare m_merger for merging the runCount runs in finalMergeLevel.
	///////////////////////////////////////////////////////////////////////////
	void initialize_final_merger(memory_size_type finalMergeLevel, memory_size_type runCount) {
		if (m_finalMergeInitialized) {
			reinitialize_final_merger();
			return;
		}

		m_finalMergeInitialized = true;
		m_finalMergeLevel = finalMergeLevel;
		m_finalRunCount = runCount;
		m_runPositions.next_level();
		m_runPositions.final_level(p.fanout);
		if (runCount > p.finalFanout) {
			log_pipe_debug() << "Run count in final level (" << runCount << ") is greater than the final fanout (" << p.finalFanout << ")\n";

			memory_size_type i = p.finalFanout-1;
			memory_size_type n = runCount-i;
			log_pipe_debug() << "Merge " << n << " runs starting from #" << i << std::endl;
			dummy_progress_indicator pi;
			m_finalMergeSpecialRunNumber = merge_runs(finalMergeLevel, i, n, pi);
		} else {
			log_pipe_debug() << "Run count in final level (" << runCount << ") is less or equal to the final fanout (" << p.finalFanout << ")" << std::endl;
			m_finalMergeSpecialRunNumber = std::numeric_limits<memory_size_type>::max();
		}
		reinitialize_final_merger();
	}

public:
	void reinitialize_final_merger() {
		tp_assert(m_finalMergeInitialized, "reinitialize_final_merger while !m_finalMergeInitialized");
		m_runPositions.unevacuate();
		if (m_finalMergeSpecialRunNumber != std::numeric_limits<memory_size_type>::max()) {
			array<file_stream<element_type> > in(p.finalFanout);
			for (memory_size_type i = 0; i < p.finalFanout-1; ++i) {
				open_run_file_read(in[i], m_finalMergeLevel, i);
				log_pipe_debug() << "Run " << i << " is at offset " << in[i].offset() << " and has size " << in[i].size() << std::endl;
			}
			open_run_file_read(in[p.finalFanout-1], m_finalMergeLevel+1, m_finalMergeSpecialRunNumber);
			log_debug() << "Special large run is at offset " << in[p.finalFanout-1].offset() << " and has size " << in[p.finalFanout-1].size() << std::endl;
			stream_size_type runLength = calculate_run_length(p.runLength, p.fanout, m_finalMergeLevel+1);
			log_pipe_debug() << "Run length " << runLength << std::endl;
			m_merger.reset(in, runLength);
		} else {
			initialize_merger(m_finalMergeLevel, 0, m_finalRunCount);
		}
		m_evacuated = false;
	}

private:
	///////////////////////////////////////////////////////////////////////////
	/// Merge the runNumber'th to the (runNumber+runCount)'th in mergeLevel
	/// into mergeLevel+1.
	/// \returns The run number in mergeLevel+1 that was written to.
	///////////////////////////////////////////////////////////////////////////
	template <typename ProgressIndicator>
	memory_size_type merge_runs(memory_size_type mergeLevel, memory_size_type runNumber, memory_size_type runCount, ProgressIndicator & pi) {
		initialize_merger(mergeLevel, runNumber, runCount);
		file_stream<element_type> out;
		memory_size_type nextRunNumber = runNumber/p.fanout;
		open_run_file_write(out, mergeLevel+1, nextRunNumber);
		while (m_merger.can_pull()) {
			pi.step();
			out.write(m_store.store_to_element(m_merger.pull()));
		}
		return nextRunNumber;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Phase 2: Merge all runs and initialize merger for public pulling.
	///////////////////////////////////////////////////////////////////////////
	void prepare_pull(typename Progress::base & pi) {
		m_runPositions.unevacuate();

		// Compute merge depth (number of passes over data).
		int treeHeight= static_cast<int>(ceil(log(static_cast<float>(m_finishedRuns)) /
											  log(static_cast<float>(p.fanout))));
		pi.init(item_count()*treeHeight);

		memory_size_type mergeLevel = 0;
		memory_size_type runCount = m_finishedRuns;
		while (runCount > p.fanout) {
			log_pipe_debug() << "Merge " << runCount << " runs in merge level " << mergeLevel << '\n';
			m_runPositions.next_level();
			memory_size_type newRunCount = 0;
			for (memory_size_type i = 0; i < runCount; i += p.fanout) {
				memory_size_type n = std::min(runCount-i, p.fanout);

				if (newRunCount < 10)
					log_pipe_debug() << "Merge " << n << " runs starting from #" << i << std::endl;
				else if (newRunCount == 10)
					log_pipe_debug() << "..." << std::endl;

				merge_runs(mergeLevel, i, n, pi);
				++newRunCount;
			}
			++mergeLevel;
			runCount = newRunCount;
		}
		log_pipe_debug() << "Final merge level " << mergeLevel << " has " << runCount << " runs" << std::endl;
		initialize_final_merger(mergeLevel, runCount);

		m_state = stReport;
		pi.done();
	}

public:
	///////////////////////////////////////////////////////////////////////////
	/// In phase 3, return true if there are more items in the final merge
	/// phase.
	///////////////////////////////////////////////////////////////////////////
	bool can_pull() {
		tp_assert(m_state == stReport, "Wrong phase");
		if (m_reportInternal) return m_itemsPulled < m_currentRunItemCount;
		else {
			if (m_evacuated) reinitialize_final_merger();
			return m_merger.can_pull();
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// In phase 3, fetch next item in the final merge phase.
	///////////////////////////////////////////////////////////////////////////
	item_type pull() {
		tp_assert(m_state == stReport, "Wrong phase");
		if (m_reportInternal && m_itemsPulled < m_currentRunItemCount) {
			store_type el = std::move(m_currentRunItems[m_itemsPulled++]);
			if (!can_pull()) m_currentRunItems.resize(0);
			return m_store.store_to_outer(std::move(el));
		} else {
			if (m_evacuated) reinitialize_final_merger();
			m_runPositions.close();
			return m_store.store_to_outer(m_merger.pull());
		}
	}

	
	memory_size_type actual_memory_phase_3() {
		tp_assert(m_state == stReport, "Wrong phase");
		if (m_reportInternal)
			return m_runFiles.memory_usage(m_runFiles.size())
				+ m_currentRunItems.memory_usage(m_currentRunItems.size());
		else
			return fanout_memory_usage(m_finalRunCount);
	}
	
private:

	///////////////////////////////////////////////////////////////////////////
	/// calculate_parameters helper
	///////////////////////////////////////////////////////////////////////////
	static constexpr linear_memory_usage fanout_memory_usage() noexcept {
		return merger<specific_store_t, pred_t>::memory_usage()
			+ bits::run_positions::memory_usage()
			+ file_stream<element_type>::memory_usage() // output stream
			+ 2*sizeof(temp_file); // merge_sorter::m_runFiles
	}
	
	static constexpr memory_size_type fanout_memory_usage(memory_size_type fanout) noexcept {
		return fanout_memory_usage()(fanout);
	}

 private:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Figure out the index in m_runFiles of the given run.
	/// \param mergeLevel  Distance from leaves of merge tree.
	/// \param runNumber  Index in current merge level.
	///////////////////////////////////////////////////////////////////////////
	memory_size_type run_file_index(memory_size_type mergeLevel, memory_size_type runNumber) {
		// runNumber is a memory_size_type since it is used as an index into
		// m_runFiles.

		return (mergeLevel % 2)*p.fanout + (runNumber % p.fanout);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Open a new run file and seek to the end.
	///////////////////////////////////////////////////////////////////////////
	void open_run_file_write(file_stream<element_type> & fs, memory_size_type mergeLevel, memory_size_type runNumber) {
		// see run_file_index comment about runNumber

		memory_size_type idx = run_file_index(mergeLevel, runNumber);
		if (runNumber < p.fanout) m_runFiles[idx].free();
		fs.open(m_runFiles[idx], access_read_write, 0, access_sequential, compression_normal);
		fs.seek(0, file_stream_base::end);
		m_runPositions.set_position(mergeLevel, runNumber, fs.get_position());
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Open an existing run file and seek to the correct offset.
	///////////////////////////////////////////////////////////////////////////
	void open_run_file_read(file_stream<element_type> & fs, memory_size_type mergeLevel, memory_size_type runNumber) {
		// see run_file_index comment about runNumber

		memory_size_type idx = run_file_index(mergeLevel, runNumber);
		fs.open(m_runFiles[idx], access_read, 0, access_sequential, compression_normal);
		fs.set_position(m_runPositions.get_position(mergeLevel, runNumber));
	}

	specific_store_t m_store;
	merger<specific_store_t, pred_t> m_merger;

	// current run buffer. size 0 before begin(), size runLength after begin().
	array<store_type> m_currentRunItems;

	pred_t pred;
};

} // namespace tpie

#endif // __TPIE_PIPELINING_MERGE_SORTER_H__
