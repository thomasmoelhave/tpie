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

#include <tpie/pipelining/sort_parameters.h>
#include <tpie/pipelining/merger.h>
#include <tpie/pipelining/exception.h>
#include <tpie/dummy_progress.h>

namespace tpie {

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
template <typename T, bool UseProgress, typename pred_t = std::less<T> >
class merge_sorter {
public:
	typedef boost::shared_ptr<merge_sorter> ptr;
	typedef progress_types<UseProgress> Progress;

	inline merge_sorter(pred_t pred = pred_t())
		: m_state(stParameters)
		, p()
		, m_parametersSet(false)
		, m_merger(pred)
		, pred(pred)
		, m_evacuated(false)
		, m_finalMergeInitialized(false)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Enable setting run length and fanout manually (for testing
	/// purposes).
	///////////////////////////////////////////////////////////////////////////
	inline void set_parameters(stream_size_type runLength, memory_size_type fanout) {
		tp_assert(m_state == stParameters, "Merge sorting already begun");
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
	/// \param m1 Memory available for phase 1
	/// \param m2 Memory available for phase 2
	/// \param m3 Memory available for phase 3
	///////////////////////////////////////////////////////////////////////////
	inline void set_available_memory(memory_size_type m1, memory_size_type m2, memory_size_type m3) {
		calculate_parameters(m1, m2, m3);
	}

private:
	// set_phase_?_memory helper
	inline void maybe_calculate_parameters() {
		if (p.memoryPhase1 > 0 &&
			p.memoryPhase2 > 0 &&
			p.memoryPhase3 > 0)
			calculate_parameters(p.memoryPhase1,
								 p.memoryPhase2,
								 p.memoryPhase3);
	}

public:
	inline void set_phase_1_memory(memory_size_type m1) {
		p.memoryPhase1 = m1;
		maybe_calculate_parameters();
	}

	inline void set_phase_2_memory(memory_size_type m2) {
		p.memoryPhase2 = m2;
		maybe_calculate_parameters();
	}

	inline void set_phase_3_memory(memory_size_type m3) {
		p.memoryPhase3 = m3;
		maybe_calculate_parameters();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Initiate phase 1: Formation of input runs.
	///////////////////////////////////////////////////////////////////////////
	inline void begin() {
		tp_assert(m_state == stParameters, "Merge sorting already begun");
		if (!m_parametersSet) throw merge_sort_not_ready();
		log_debug() << "Start forming input runs" << std::endl;
		m_currentRunItems.resize(p.runLength);
		m_runFiles.resize(p.fanout*2);
		m_currentRunItemCount = 0;
		m_finishedRuns = 0;
		m_state = stRunFormation;
		m_itemCount = 0;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Push item to merge sorter during phase 1.
	///////////////////////////////////////////////////////////////////////////
	inline void push(const T & item) {
		tp_assert(m_state == stRunFormation, "Wrong phase");
		if (m_currentRunItemCount >= p.runLength) {
			sort_current_run();
			empty_current_run();
		}
		m_currentRunItems[m_currentRunItemCount] = item;
		++m_currentRunItemCount;
		++m_itemCount;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief End phase 1.
	///////////////////////////////////////////////////////////////////////////
	inline void end() {
		tp_assert(m_state == stRunFormation, "Wrong phase");
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
		m_state = stMerge;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Perform phase 2: Performing all merges in the merge tree except
	/// the last one.
	///////////////////////////////////////////////////////////////////////////
	inline void calc(typename Progress::base & pi) {
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

	inline void evacuate() {
		tp_assert(m_state == stMerge || m_state == stReport, "Wrong phase");
		if (m_reportInternal) {
			log_debug() << "Evacuate merge_sorter (" << this << ") in internal reporting mode" << std::endl;
			m_reportInternal = false;
			memory_size_type runCount = (m_currentRunItemCount > 0) ? 1 : 0;
			empty_current_run();
			m_currentRunItems.resize(0);
			initialize_final_merger(0, runCount);
		} else if (m_state == stMerge) {
			log_debug() << "Evacuate merge_sorter (" << this << ") before merge in external reporting mode (noop)" << std::endl;
			return;
		}
		log_debug() << "Evacuate merge_sorter (" << this << ") before reporting in external reporting mode" << std::endl;
		m_merger.reset();
		m_evacuated = true;
	}

	inline void evacuate_before_merging() {
		if (m_state == stMerge) evacuate();
	}

	inline void evacuate_before_reporting() {
		if (m_state == stReport && (!m_reportInternal || m_itemsPulled == 0)) evacuate();
	}

private:
	///////////////////////////////////////////////////////////////////////////
	// Phase 1 helpers.
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
		for (memory_size_type i = 0; i < m_currentRunItemCount; ++i) {
			fs.write(m_currentRunItems[i]);
		}
		m_currentRunItemCount = 0;
		++m_finishedRuns;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Prepare m_merger for merging the runNumber'th to the
	/// (runNumber+runCount)'th run in mergeLevel.
	///////////////////////////////////////////////////////////////////////////
	inline void initialize_merger(memory_size_type mergeLevel, memory_size_type runNumber, memory_size_type runCount) {
		// runCount is a memory_size_type since we must be able to have that
		// many file_streams open at the same time.

		// Open files and seek to the first item in the run.
		array<file_stream<T> > in(runCount);
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
	inline void initialize_final_merger(memory_size_type finalMergeLevel, memory_size_type runCount) {
		if (m_finalMergeInitialized) {
			reinitialize_final_merger();
			return;
		}

		m_finalMergeInitialized = true;
		m_finalMergeLevel = finalMergeLevel;
		m_finalRunCount = runCount;
		if (runCount > p.finalFanout) {
			log_debug() << "Run count in final level (" << runCount << ") is greater than the final fanout (" << p.finalFanout << ")\n";

			memory_size_type i = p.finalFanout-1;
			memory_size_type n = runCount-i;
			log_debug() << "Merge " << n << " runs starting from #" << i << std::endl;
			dummy_progress_indicator pi;
			m_finalMergeSpecialRunNumber = merge_runs(finalMergeLevel, i, n, pi);
		} else {
			log_debug() << "Run count in final level (" << runCount << ") is less or equal to the final fanout (" << p.finalFanout << ")" << std::endl;
			m_finalMergeSpecialRunNumber = std::numeric_limits<memory_size_type>::max();
		}
		reinitialize_final_merger();
	}

public:
	inline void reinitialize_final_merger() {
		tp_assert(m_finalMergeInitialized, "reinitialize_final_merger while !m_finalMergeInitialized");
		if (m_finalMergeSpecialRunNumber != std::numeric_limits<memory_size_type>::max()) {
			array<file_stream<T> > in(p.finalFanout);
			for (memory_size_type i = 0; i < p.finalFanout-1; ++i) {
				open_run_file_read(in[i], m_finalMergeLevel, i);
				log_debug() << "Run " << i << " is at offset " << in[i].offset() << " and has size " << in[i].size() << std::endl;
			}
			open_run_file_read(in[p.finalFanout-1], m_finalMergeLevel+1, m_finalMergeSpecialRunNumber);
			log_debug() << "Special large run is at offset " << in[p.finalFanout-1].offset() << " and has size " << in[p.finalFanout-1].size() << std::endl;
			stream_size_type runLength = calculate_run_length(p.runLength, p.fanout, m_finalMergeLevel+1);
			log_debug() << "Run length " << runLength << std::endl;
			m_merger.reset(in, runLength);
		} else {
			initialize_merger(m_finalMergeLevel, 0, m_finalRunCount);
		}
		m_evacuated = false;
	}

private:
	///////////////////////////////////////////////////////////////////////////
	/// initialize_merger helper.
	///////////////////////////////////////////////////////////////////////////
	static inline stream_size_type calculate_run_length(stream_size_type initialRunLength, memory_size_type fanout, memory_size_type mergeLevel) {
		stream_size_type runLength = initialRunLength;
		for (memory_size_type i = 0; i < mergeLevel; ++i) {
			runLength *= fanout;
		}
		return runLength;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Merge the runNumber'th to the (runNumber+runCount)'th in mergeLevel
	/// into mergeLevel+1.
	/// \returns The run number in mergeLevel+1 that was written to.
	///////////////////////////////////////////////////////////////////////////
	template <typename ProgressIndicator>
	inline memory_size_type merge_runs(memory_size_type mergeLevel, memory_size_type runNumber, memory_size_type runCount, ProgressIndicator & pi) {
		initialize_merger(mergeLevel, runNumber, runCount);
		file_stream<T> out;
		memory_size_type nextRunNumber = runNumber/p.fanout;
		open_run_file_write(out, mergeLevel+1, nextRunNumber);
		while (m_merger.can_pull()) {
			pi.step();
			out.write(m_merger.pull());
		}
		return nextRunNumber;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Phase 2: Merge all runs and initialize merger for public pulling.
	///////////////////////////////////////////////////////////////////////////
	inline void prepare_pull(typename Progress::base & pi) {
		// Compute merge depth (number of passes over data).
		int treeHeight= static_cast<int>(ceil(log(static_cast<float>(m_finishedRuns)) /
											  log(static_cast<float>(p.fanout))));
		pi.init(item_count()*treeHeight);

		memory_size_type mergeLevel = 0;
		memory_size_type runCount = m_finishedRuns;
		while (runCount > p.fanout) {
			log_debug() << "Merge " << runCount << " runs in merge level " << mergeLevel << '\n';
			memory_size_type newRunCount = 0;
			for (memory_size_type i = 0; i < runCount; i += p.fanout) {
				memory_size_type n = std::min(runCount-i, p.fanout);

				if (newRunCount < 10)
					log_debug() << "Merge " << n << " runs starting from #" << i << std::endl;
				else if (newRunCount == 10)
					log_debug() << "..." << std::endl;

				merge_runs(mergeLevel, i, n, pi);
				++newRunCount;
			}
			++mergeLevel;
			runCount = newRunCount;
		}
		log_debug() << "Final merge level " << mergeLevel << " has " << runCount << " runs" << std::endl;
		initialize_final_merger(mergeLevel, runCount);

		m_state = stReport;
		pi.done();
	}

public:
	///////////////////////////////////////////////////////////////////////////
	/// In phase 3, return true if there are more items in the final merge
	/// phase.
	///////////////////////////////////////////////////////////////////////////
	inline bool can_pull() {
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
	inline T pull() {
		tp_assert(m_state == stReport, "Wrong phase");
		if (m_reportInternal && m_itemsPulled < m_currentRunItemCount) {
			T el = m_currentRunItems[m_itemsPulled++];
			if (!can_pull()) m_currentRunItems.resize(0);
			return el;
		} else {
			if (m_evacuated) reinitialize_final_merger();
			return m_merger.pull();
		}
	}

	inline stream_size_type item_count() {
		return m_itemCount;
	}

	static memory_size_type memory_usage_phase_1(const sort_parameters & params) {
		return params.runLength * sizeof(T)
			+ file_stream<T>::memory_usage()
			+ 2*params.fanout*sizeof(temp_file);
	}

	static memory_size_type minimum_memory_phase_1() {
		// Our *absolute minimum* memory requirements are a single item and
		// twice as many temp_files as the fanout.
		// However, our fanout calculation does not take the memory available
		// in this phase (run formation) into account.
		// Thus, we assume the largest fanout, meaning we might overshoot.
		// If we do overshoot, we will just spend the extra bytes on a run length
		// longer than 1, which is probably what the user wants anyway.
		sort_parameters p((sort_parameters()));
		p.runLength = 1;
		p.fanout = calculate_fanout(std::numeric_limits<memory_size_type>::max());
		return memory_usage_phase_1(p);
	}

	static memory_size_type memory_usage_phase_2(const sort_parameters & params) {
		return fanout_memory_usage(params.fanout);
	}

	static memory_size_type minimum_memory_phase_2() {
		return fanout_memory_usage(calculate_fanout(0));
	}

	static memory_size_type memory_usage_phase_3(const sort_parameters & params) {
		return fanout_memory_usage(params.finalFanout);
	}

	static memory_size_type minimum_memory_phase_3() {
		return fanout_memory_usage(calculate_fanout(0));
	}

	inline memory_size_type evacuated_memory_usage() const {
		return 2*p.fanout*sizeof(temp_file);
	}

private:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate parameters from given memory amount.
	/// \param m1 Memory available for phase 1
	/// \param m2 Memory available for phase 2
	/// \param m3 Memory available for phase 3
	///////////////////////////////////////////////////////////////////////////
	inline void calculate_parameters(const memory_size_type m1, const memory_size_type m2, const memory_size_type m3) {
		tp_assert(m_state == stParameters, "Merge sorting already begun");

		p.memoryPhase1 = m1;
		p.memoryPhase2 = m2;
		p.memoryPhase3 = m3;

		// We must set aside memory for temp_files in m_runFiles.
		// m_runFiles contains fanout*2 temp_files, so calculate fanout before run length.

		// Phase 2 (merge):
		// Run length: unbounded
		// Fanout: determined by the size of our merge heap and the stream memory usage.
		log_debug() << "Phase 2: " << p.memoryPhase2 << " b available memory\n";
		p.fanout = calculate_fanout(p.memoryPhase2);
		if (fanout_memory_usage(p.fanout) > p.memoryPhase2) {
			log_debug() << "Not enough memory for fanout " << p.fanout << "! (" << p.memoryPhase2 << " < " << fanout_memory_usage(p.fanout) << ")\n";
			p.memoryPhase2 = fanout_memory_usage(p.fanout);
		}

		// Phase 3 (final merge & report):
		// Run length: unbounded
		// Fanout: determined by the stream memory usage.
		log_debug() << "Phase 3: " << p.memoryPhase3 << " b available memory\n";
		p.finalFanout = calculate_fanout(p.memoryPhase3);

		if (p.finalFanout > p.fanout)
			p.finalFanout = p.fanout;

		if (fanout_memory_usage(p.finalFanout) > p.memoryPhase3) {
			log_debug() << "Not enough memory for fanout " << p.finalFanout << "! (" << p.memoryPhase3 << " < " << fanout_memory_usage(p.finalFanout) << ")\n";
			p.memoryPhase3 = fanout_memory_usage(p.finalFanout);
		}

		// Phase 1 (run formation):
		// Run length: determined by the number of items we can hold in memory.
		// Fanout: unbounded

		memory_size_type streamMemory = file_stream<T>::memory_usage();
		memory_size_type tempFileMemory = 2*p.fanout*sizeof(temp_file);

		log_debug() << "Phase 1: " << p.memoryPhase1 << " b available memory; " << streamMemory << " b for a single stream; " << tempFileMemory << " b for temp_files\n";
		memory_size_type min_m1 = sizeof(T) + streamMemory + tempFileMemory;
		if (p.memoryPhase1 < min_m1) {
			log_warning() << "Not enough phase 1 memory for an item and an open stream! (" << p.memoryPhase1 << " < " << min_m1 << ")\n";
			p.memoryPhase1 = min_m1;
		}
		p.runLength = (p.memoryPhase1 - streamMemory - tempFileMemory)/sizeof(T);

		p.internalReportThreshold = (std::min(p.memoryPhase1,
											  std::min(p.memoryPhase2,
													   p.memoryPhase3))
									 - tempFileMemory)/sizeof(T);
		if (p.internalReportThreshold > p.runLength)
			p.internalReportThreshold = p.runLength;

		m_parametersSet = true;

		log_debug() << "Calculated merge sort parameters\n";
		p.dump(log_debug());
		log_debug() << std::endl;

		log_debug() << "Merge sort phase 1: "
			<< m1 << " b available, " << memory_usage_phase_1(p) << " b expected" << std::endl;
		if (memory_usage_phase_1(p) > m1) {
			log_warning() << "Merge sort phase 1 exceeds the alloted memory usage: "
				<< m1 << " b available, but " << memory_usage_phase_1(p) << " b expected" << std::endl;
		}
		log_debug() << "Merge sort phase 2: "
			<< m2 << " b available, " << memory_usage_phase_2(p) << " b expected" << std::endl;
		if (memory_usage_phase_2(p) > m2) {
			log_warning() << "Merge sort phase 2 exceeds the alloted memory usage: "
				<< m2 << " b available, but " << memory_usage_phase_2(p) << " b expected" << std::endl;
		}
		log_debug() << "Merge sort phase 3: "
			<< m3 << " b available, " << memory_usage_phase_3(p) << " b expected" << std::endl;
		if (memory_usage_phase_3(p) > m3) {
			log_warning() << "Merge sort phase 3 exceeds the alloted memory usage: "
				<< m3 << " b available, but " << memory_usage_phase_3(p) << " b expected" << std::endl;
		}
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
	static inline memory_size_type fanout_memory_usage(memory_size_type fanout) {
		return merger<T, pred_t>::memory_usage(fanout) // accounts for the `fanout' open streams
			+ file_stream<T>::memory_usage() // output stream
			+ 2*sizeof(temp_file); // merge_sorter::m_runFiles
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Figure out the index in m_runFiles of the given run.
	/// \param mergeLevel  Distance from leaves of merge tree.
	/// \param runNumber  Index in current merge level.
	///////////////////////////////////////////////////////////////////////////
	inline memory_size_type run_file_index(memory_size_type mergeLevel, memory_size_type runNumber) {
		// runNumber is a memory_size_type since it is used as an index into
		// m_runFiles.

		return (mergeLevel % 2)*p.fanout + (runNumber % p.fanout);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Open a new run file and seek to the end.
	///////////////////////////////////////////////////////////////////////////
	inline void open_run_file_write(file_stream<T> & fs, memory_size_type mergeLevel, memory_size_type runNumber) {
		// see run_file_index comment about runNumber

		memory_size_type idx = run_file_index(mergeLevel, runNumber);
		if (runNumber < p.fanout) m_runFiles[idx].free();
		fs.open(m_runFiles[idx], access_read_write);
		fs.seek(0, file_stream<T>::end);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Open an existing run file and seek to the correct offset.
	///////////////////////////////////////////////////////////////////////////
	inline void open_run_file_read(file_stream<T> & fs, memory_size_type mergeLevel, memory_size_type runNumber) {
		// see run_file_index comment about runNumber

		memory_size_type idx = run_file_index(mergeLevel, runNumber);
		fs.open(m_runFiles[idx], access_read);
		fs.seek(calculate_run_length(p.runLength, p.fanout, mergeLevel) * (runNumber / p.fanout), file_stream<T>::beginning);
	}

	enum state_type {
		stParameters,
		stRunFormation,
		stMerge,
		stReport
	};

	state_type m_state;

	sort_parameters p;
	bool m_parametersSet;

	merger<T, pred_t> m_merger;

	array<temp_file> m_runFiles;

	// number of runs already written to disk.
	stream_size_type m_finishedRuns;

	// current run buffer. size 0 before begin(), size runLength after begin().
	array<T> m_currentRunItems;

	// Number of items in current run buffer.
	// Used to index into m_currentRunItems, so memory_size_type.
	memory_size_type m_currentRunItemCount;

	bool m_reportInternal;

	// When doing internal reporting: the number of items already reported
	// Used in comparison with m_currentRunItemCount
	memory_size_type m_itemsPulled;

	stream_size_type m_itemCount;

	pred_t pred;

	bool m_evacuated;
	bool m_finalMergeInitialized;
	memory_size_type m_finalMergeLevel;
	memory_size_type m_finalRunCount;
	memory_size_type m_finalMergeSpecialRunNumber;
};

} // namespace tpie

#endif // __TPIE_PIPELINING_MERGE_SORTER_H__
