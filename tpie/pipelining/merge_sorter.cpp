// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2013, The TPIE development team
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

#include <tpie/pipelining/merge_sorter.h>

namespace tpie {

namespace bits {

run_positions::run_positions()
	: m_open(false)
{
}

run_positions::~run_positions() {
	close();
}

/*static*/ memory_size_type run_positions::memory_usage() noexcept {
	return sizeof(run_positions)
		+ 2 * file_stream<stream_position>::memory_usage();
}

void run_positions::open() {
	m_positions[0].open(m_positionsFile[0]);
	m_positions[1].open(m_positionsFile[1]);
	m_runs[0] = m_runs[1] = 0;
	m_levels = 1;
	m_open = true;
	m_final = m_evacuated = false;
	m_finalExtraSet = false;
	m_finalExtra = stream_position();
	m_finalPositions.resize(0);
}

void run_positions::close() {
	if (m_open) {
		m_positions[0].close();
		m_positions[1].close();
		m_open = m_final = m_evacuated = false;
		m_finalExtraSet = false;
		m_finalExtra = stream_position();
		m_finalPositions.resize(0);
	}
}

void run_positions::evacuate() {
	if (m_evacuated) return;
	m_evacuated = true;
	if (m_final) {
		log_pipe_debug() << "run_positions::evacuate while final" << std::endl;
		m_positionsFile[0].free();
		m_positions[0].open(m_positionsFile[0]);
		m_positions[0].write(m_finalPositions.begin(), m_finalPositions.end());
		m_finalPositions.resize(0);
		m_positions[0].close();
	} else {
		log_pipe_debug() << "run_positions::evacuate while not final" << std::endl;
		m_positionsPosition[0] = m_positions[0].get_position();
		m_positionsPosition[1] = m_positions[1].get_position();
		m_positions[0].close();
		m_positions[1].close();
	}
}

void run_positions::unevacuate() {
	if (!m_evacuated) return;
	m_evacuated = false;
	if (m_final) {
		log_pipe_debug() << "run_positions::unevacuate while final" << std::endl;
		m_positions[0].open(m_positionsFile[0]);
		m_finalPositions.resize(static_cast<memory_size_type>(m_positions[0].size()));
		m_positions[0].read(m_finalPositions.begin(), m_finalPositions.end());
		m_positions[0].close();
	} else {
		log_pipe_debug() << "run_positions::unevacuate while not final" << std::endl;
		m_positions[0].open(m_positionsFile[0]);
		m_positions[1].open(m_positionsFile[1]);
		m_positions[0].set_position(m_positionsPosition[0]);
		m_positions[1].set_position(m_positionsPosition[1]);
	}
}

void run_positions::next_level() {
	if (m_final)
		throw exception("next_level: m_final == true");
	if (m_evacuated)
		throw exception("next_level: m_evacuated == true");
	if (!m_open)
		throw exception("next_level: m_open == false");
	m_positions[m_levels % 2].truncate(0);
	++m_levels;
	m_positions[m_levels % 2].seek(0);

	m_runs[0] = m_runs[1] = 0;
}

void run_positions::final_level(memory_size_type fanout) {
	if (m_final)
		throw exception("final_level: m_final == true");
	if (m_evacuated)
		throw exception("final_level: m_evacuated == true");
	if (!m_open)
		throw exception("final_level: m_open == false");

	m_final = true;
	file_stream<stream_position> & s = m_positions[m_levels % 2];
	if (fanout > s.size() - s.offset()) {
		log_pipe_debug() << "Decrease final level fanout from " << fanout << " to ";
		fanout = static_cast<memory_size_type>(s.size() - s.offset());
		log_pipe_debug() << fanout << std::endl;
	}
	m_finalPositions.resize(fanout);
	s.read(m_finalPositions.begin(), m_finalPositions.end());
	m_positions[0].close();
	m_positions[1].close();
}

void run_positions::set_position(memory_size_type mergeLevel, memory_size_type runNumber, stream_position pos) {
	if (!m_open) open();

	if (mergeLevel+1 != m_levels) {
		throw exception("set_position: incorrect mergeLevel");
	}
	if (m_final) {
		log_pipe_debug() << "run_positions set_position setting m_finalExtra" << std::endl;
		m_finalExtra = pos;
		m_finalExtraSet = true;
		return;
	}
	file_stream<stream_position> & s = m_positions[mergeLevel % 2];
	memory_size_type & expectedRunNumber = m_runs[mergeLevel % 2];
	if (runNumber != expectedRunNumber) {
		throw exception("set_position: Wrong run number");
	}
	++expectedRunNumber;
	s.write(pos);
}

stream_position run_positions::get_position(memory_size_type mergeLevel, memory_size_type runNumber) {
	if (!m_open) throw exception("get_position: !open");

	if (m_final && mergeLevel+1 == m_levels) {
		log_pipe_debug() << "run_positions get_position returning m_finalExtra" << std::endl;
		if (!m_finalExtraSet)
			throw exception("get_position: m_finalExtra uninitialized");
		return m_finalExtra;
	}

	if (mergeLevel+2 != m_levels) {
		throw exception("get_position: incorrect mergeLevel");
	}
	if (m_final) {
		return m_finalPositions[runNumber];
	}
	file_stream<stream_position> & s = m_positions[mergeLevel % 2];
	memory_size_type & expectedRunNumber = m_runs[mergeLevel % 2];
	if (runNumber != expectedRunNumber) {
		throw exception("get_position: Wrong run number");
	}
	++expectedRunNumber;
	if (!s.can_read())
		throw exception("get_position: !can_read");
	return s.read();
}

} // namespace bits

void merge_sorter_base::set_parameters(memory_size_type runLength, memory_size_type fanout) {
	tp_assert(m_state == stNotStarted, "Merge sorting already begun");
	p.runLength = p.internalReportThreshold = runLength;
	p.fanout = p.finalFanout = fanout;
	m_parametersSet = true;
	log_pipe_debug() << "Manually set merge sort run length and fanout\n";
	log_pipe_debug() << "Run length =       " << p.runLength << " (uses memory " << (p.runLength*m_item_size + m_element_file_stream_memory_usage) << ")\n";
	log_pipe_debug() << "Fanout =           " << p.fanout << " (uses memory " << m_fanout_memory_usage(p.fanout) << ")" << std::endl;
}


merge_sorter_base::merge_sorter_base(
	linear_memory_usage fanout_memory_usage,
	memory_size_type item_size,
	memory_size_type element_file_stream_memory_usage)
	: m_fanout_memory_usage(fanout_memory_usage)
	, m_item_size(item_size)
	, m_element_file_stream_memory_usage(element_file_stream_memory_usage)
	, m_bucketPtr(new memory_bucket())
	, m_bucket(memory_bucket_ref(m_bucketPtr.get()))
	, m_state(stNotStarted)
	, p()
	, m_parametersSet(false)
	, m_maxItems(std::numeric_limits<stream_size_type>::max())
	, m_evacuated(false)
	, m_finalMergeInitialized(false)
	, m_owning_node(nullptr)
{}


void merge_sorter_base::set_items(stream_size_type n) {
	if (m_state != stNotStarted)
		throw exception("Wrong state in set_items: state is not stNotStarted");
	
	m_maxItems = n;
	
	if (!m_parametersSet) {
		// We will handle this later in calculate_parameters
		return;
	}
	
	// If the item upper bound is less than a run,
	// then it might pay off to decrease the length of a run
	// so that we can avoid I/O altogether.
	if (m_maxItems < p.runLength) {
		memory_size_type newRunLength =
			std::max(memory_size_type(m_maxItems), p.internalReportThreshold);
		log_pipe_debug() << "Decreasing run length from " << p.runLength
						 << " to " << newRunLength
						 << " since at most " << m_maxItems << " items will be pushed,"
						 << " and the internal report threshold is "
						 << p.internalReportThreshold
						 << ". New merge sort parameters:\n";
		// In principle, we could decrease runLength to m_maxItems,
		// but setting runLength below internalReportThreshold does not
		// give additional benefits.
		// Furthermore, buggy code could call set_items with a very low
		// upper bound, leading to unacceptable performance in practice;
		// thus, internalReportThreshold is used as a stopgap/failsafe.
		p.runLength = newRunLength;
		p.dump(log_pipe_debug());
		log_pipe_debug() << std::endl;
	}
}


void merge_sorter_base::set_owner(tpie::pipelining::node * n) {
	if (m_owning_node != nullptr)
		m_bucketPtr = std::move(m_owning_node->bucket(0));
	
	if (n != nullptr)
		n->bucket(0) = std::move(m_bucketPtr);
	
	m_owning_node = n;
}

	
static memory_size_type clamp(memory_size_type lo, memory_size_type val, memory_size_type hi) {
	return std::max(lo, std::min(val, hi));
}

void merge_sorter_base::calculate_parameters() {
	tp_assert(m_state == stNotStarted, "Merge sorting already begun");
	
	if(!p.filesPhase1)
		p.filesPhase1 = clamp(minimumFilesPhase1, defaultFiles, maximumFilesPhase1);
	if(!p.filesPhase2)
		p.filesPhase2 = clamp(minimumFilesPhase2, defaultFiles, maximumFilesPhase2);
	if(!p.filesPhase3)
		p.filesPhase3 = clamp(minimumFilesPhase3, defaultFiles, maximumFilesPhase3);
	
	if(p.filesPhase1 < minimumFilesPhase1)
		throw tpie::exception("file limit for phase 1 too small (" + std::to_string(p.filesPhase1) + " < " + std::to_string(minimumFilesPhase1) + ")");
	if(p.filesPhase2 < minimumFilesPhase2)
		throw tpie::exception("file limit for phase 2 too small (" + std::to_string(p.filesPhase2) + " < " + std::to_string(minimumFilesPhase2) + ")");
	if(p.filesPhase3 < minimumFilesPhase3)
		throw tpie::exception("file limit for phase 3 too small (" + std::to_string(p.filesPhase3) + " < " + std::to_string(minimumFilesPhase3) + ")");
	
	if (!p.filesPhase1)
		throw tpie::exception("memory limit for phase 1 not set");
	if (!p.filesPhase2)
		throw tpie::exception("memory limit for phase 2 not set");
	if (!p.filesPhase3)
		throw tpie::exception("memory limit for phase 3 not set");
	
	// We must set aside memory for temp_files in m_runFiles.
	// m_runFiles contains fanout*2 temp_files, so calculate fanout before run length.
	
	// Phase 2 (merge):
	// Run length: unbounded
	// Fanout: determined by the size of our merge heap and the stream memory usage.
	log_pipe_debug() << "Phase 2: " << p.memoryPhase2 << " b available memory\n";
	p.fanout = calculate_fanout(p.memoryPhase2, p.filesPhase2);
	if (m_fanout_memory_usage(p.fanout) > p.memoryPhase2) {
		log_pipe_debug() << "Not enough memory for fanout " << p.fanout << "! (" << p.memoryPhase2 << " < " << m_fanout_memory_usage(p.fanout) << ")\n";
		p.memoryPhase2 = m_fanout_memory_usage(p.fanout);
	}
	
	// Phase 3 (final merge & report):
	// Run length: unbounded
	// Fanout: determined by the stream memory usage.
	log_pipe_debug() << "Phase 3: " << p.memoryPhase3 << " b available memory\n";
	p.finalFanout = calculate_fanout(p.memoryPhase3, p.filesPhase3);
	
	if (p.finalFanout > p.fanout)
		p.finalFanout = p.fanout;
	
	if (m_fanout_memory_usage(p.finalFanout) > p.memoryPhase3) {
		log_pipe_debug() << "Not enough memory for fanout " << p.finalFanout << "! (" << p.memoryPhase3 << " < " << m_fanout_memory_usage(p.finalFanout) << ")\n";
		p.memoryPhase3 = m_fanout_memory_usage(p.finalFanout);
	}
	
	// Phase 1 (run formation):
	// Run length: determined by the number of items we can hold in memory.
	// Fanout: unbounded
	
	memory_size_type streamMemory = m_element_file_stream_memory_usage;
	memory_size_type tempFileMemory = 2*p.fanout*sizeof(temp_file);
	
	log_pipe_debug() << "Phase 1: " << p.memoryPhase1 << " b available memory; " << streamMemory << " b for a single stream; " << tempFileMemory << " b for temp_files\n";
	memory_size_type min_m1 = std::max<memory_size_type>(128*1024UL, 16*m_item_size) + bits::run_positions::memory_usage() + streamMemory + tempFileMemory;
	if (p.memoryPhase1 < min_m1) {
		log_warning() << "Not enough phase 1 memory for 128 KB items and an open stream! (" << p.memoryPhase1 << " < " << min_m1 << ")\n";
		p.memoryPhase1 = min_m1;
	}
	p.runLength = (p.memoryPhase1 - bits::run_positions::memory_usage() - streamMemory - tempFileMemory)/m_item_size;
	
	p.internalReportThreshold = (std::min(p.memoryPhase1,
										  std::min(p.memoryPhase2,
												   p.memoryPhase3))
								 - tempFileMemory)/m_item_size;
	if (p.internalReportThreshold > p.runLength)
		p.internalReportThreshold = p.runLength;
	
	m_parametersSet = true;
	
	set_items(m_maxItems);
	
	log_pipe_debug() << "Calculated merge sort parameters\n";
	p.dump(log_pipe_debug());
	log_pipe_debug() << std::endl;
	
	auto phase_1_mem = phase_1_memory(p);
	log_pipe_debug() << "Merge sort phase 1: "
					 << p.memoryPhase1 << " b available, "
					 <<  phase_1_mem << " b expected" << std::endl;
	if (phase_1_mem > p.memoryPhase1) {
		log_warning() << "Merge sort phase 1 exceeds the alloted memory usage: "
					  << p.memoryPhase1 << " b available, but "
					  << phase_1_mem << " b expected" << std::endl;
	}
	log_pipe_debug() << "Merge sort phase 2: "
					 << p.memoryPhase2 << " b available, " << phase_2_memory(p) << " b expected" << std::endl;
	if (phase_2_memory(p) > p.memoryPhase2) {
		log_warning() << "Merge sort phase 2 exceeds the alloted memory usage: "
					  << p.memoryPhase2 << " b available, but " << phase_2_memory(p) << " b expected" << std::endl;
	}
	log_pipe_debug() << "Merge sort phase 3: "
					 << p.memoryPhase3 << " b available, " << phase_3_memory(p) << " b expected" << std::endl;
	if (phase_3_memory(p) > p.memoryPhase3) {
		log_warning() << "Merge sort phase 3 exceeds the alloted memory usage: "
					  << p.memoryPhase3 << " b available, but " << phase_3_memory(p) << " b expected" << std::endl;
	}
}

memory_size_type merge_sorter_base::calculate_fanout(
	memory_size_type availableMemory, memory_size_type availableFiles) noexcept {
	memory_size_type fanout_lo = 2;
	memory_size_type fanout_hi = availableFiles - 2;
	// binary search
	while (fanout_lo < fanout_hi - 1) {
		memory_size_type mid = fanout_lo + (fanout_hi-fanout_lo)/2;
		if (m_fanout_memory_usage(mid) <= availableMemory) {
			fanout_lo = mid;
		} else {
			fanout_hi = mid;
			}
	}
	return fanout_lo;
}	

} // namespace tpie
