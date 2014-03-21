// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
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

#ifndef TPIE_SERIALIZATION_SORTER_H
#define TPIE_SERIALIZATION_SORTER_H

#include <queue>
#include <boost/filesystem.hpp>

#include <tpie/array.h>
#include <tpie/array_view.h>
#include <tpie/tempname.h>
#include <tpie/tpie_log.h>
#include <tpie/stats.h>
#include <tpie/parallel_sort.h>

#include <tpie/serialization2.h>
#include <tpie/serialization_stream.h>

namespace tpie {

namespace serialization_bits {

struct sort_parameters {
	/** Memory available while forming sorted runs. */
	memory_size_type memoryPhase1;
	/** Memory available while merging runs. */
	memory_size_type memoryPhase2;
	/** Memory available during output phase. */
	memory_size_type memoryPhase3;
	/** Directory in which temporary files are stored. */
	std::string tempDir;

	void dump(std::ostream & out) const {
		out << "Serialization merge sort parameters\n"
			<< "Phase 1 memory:              " << memoryPhase1 << '\n'
			<< "Phase 2 memory:              " << memoryPhase2 << '\n'
			<< "Phase 3 memory:              " << memoryPhase3 << '\n'
			<< "Temporary directory:         " << tempDir << '\n';
	}
};

class memcpy_reader {
public:
	memcpy_reader(const char * src)
		: m_src(src)
	{
	}

	void read(void * dst, size_t n) {
		memcpy(dst, m_src, n);
		m_src += n;
	}

private:
	const char * m_src;
};

class memcpy_writer {
public:
	memcpy_writer(char * dst, char * end)
		: m_dst(dst)
		, m_end(end)
	{
	}

	void write(const void * src, size_t n) {
		if (m_dst + n <= m_end)
			memcpy(m_dst, src, n);
		m_dst += n;
	}

	char * dst() {
		return m_dst;
	}

	bool overflow() {
		return m_dst > m_end;
	}

private:
	char * m_dst;
	char * m_end;
};

} // namespace serialization_bits

template <typename pred_t>
class serialized_compare {
public:
	typedef pred_t inner_pred_type;

	typedef bool result_type;
	typedef const void * first_argument_type;
	typedef const void * second_argument_type;

	typedef typename pred_t::first_argument_type first_item_type;
	typedef typename pred_t::second_argument_type second_item_type;

	serialized_compare(pred_t pred=pred_t())
		: m_pred(pred)
	{
	}

	bool operator()(const char * lhs, const char * rhs) const {
		first_item_type lhsItem;
		second_item_type rhsItem;

		using tpie::unserialize;
		serialization_bits::memcpy_reader lhsReader(lhs);
		serialization_bits::memcpy_reader rhsReader(rhs);
		unserialize(lhsReader, lhsItem);
		unserialize(rhsReader, rhsItem);

		return m_pred(lhsItem, rhsItem);
	}

private:
	pred_t m_pred;
};

namespace serialization_bits {

template <typename pred_t>
class base_offset_compare {
public:
	base_offset_compare(const char * base, size_t limit, pred_t pred)
		: m_base(base)
		, m_limit(limit)
		, m_pred(pred)
	{
	}

	bool operator()(const size_t & lhs, const size_t & rhs) const {
		if (lhs > m_limit || rhs > m_limit)
			throw tpie::exception("Out of bounds!");
		return m_pred(m_base + lhs, m_base + rhs);
	}

private:
	const char * m_base;
	size_t m_limit;
	pred_t m_pred;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Internal serialization sorter.
///
/// \tparam pred_t  less than-predicate accepting pointers to serialized items.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t>
class internal_sort {
	array<size_t> m_buffer;
	size_t * m_pointers;
	size_t * m_pointersEnd;
	char * m_nextItem;
	memory_size_type m_memAvail;
	memory_size_type m_largestItem;

	pred_t m_pred;

	bool m_full;

public:
	internal_sort(pred_t pred)
		: m_largestItem(0)
		, m_pred(pred)
		, m_full(false)
	{
	}

	void begin(memory_size_type memAvail) {
		m_memAvail = memAvail;

		m_buffer.resize(memAvail / sizeof(size_t));
		size_t n = m_buffer.size();
		m_pointers = m_pointersEnd = static_cast<size_t *>(m_buffer.get()) + n;
		m_nextItem = reinterpret_cast<char *>(m_buffer.get());

		m_full = false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief True if all items up to and including this one fits in buffer.
	///
	/// Once push() returns false, it will keep returning false until
	/// the sequence is sorted, read out, and the buffer has been cleared.
	///////////////////////////////////////////////////////////////////////////
	bool push(const T & item) {
		if (m_full) return false;

		char * firstItem = reinterpret_cast<char *>(m_buffer.get());
		*--m_pointers = m_nextItem - firstItem;
		serialization_bits::memcpy_writer wr(m_nextItem,
				reinterpret_cast<char *>(m_pointers));
		using tpie::serialize;
		serialize(wr, item);
		if (wr.overflow()) {
			m_full = true;
			++m_pointers;
			return false;
		}

		memory_size_type serializedSize = wr.dst() - m_nextItem;
		m_largestItem = std::max(serializedSize, m_largestItem);

		m_nextItem = wr.dst();

		return true;
	}

	memory_size_type get_largest_item_size() {
		return m_largestItem;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Get the serialized size of the items written.
	///
	/// This is exactly the size the current run will use when serialized to
	/// disk.
	///////////////////////////////////////////////////////////////////////////
	/*
	memory_size_type current_serialized_size() {
		return m_serializedSize;
	}
	*/

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compute current memory usage.
	///
	/// This includes the item buffer array as well as the extra serialized
	/// size of the items already written to the buffer.
	/// This assumes that items use as much primary memory as their serialized
	/// size. If this assumption does not hold, the memory usage reported may
	/// be useless. Nevertheless, this is the memory usage we use in our
	/// calculations.
	///////////////////////////////////////////////////////////////////////////
	memory_size_type memory_usage() {
		return m_buffer.size() * sizeof(size_t);
	}

	memory_size_type current_buffer_usage() {
		const char * firstItem = reinterpret_cast<char *>(m_buffer.get());
		return (m_nextItem - firstItem) * sizeof(char)
			+ (m_pointersEnd - m_pointers) * sizeof(size_t);
	}

	bool can_shrink_buffer() {
		return current_buffer_usage() <= get_memory_manager().available();
	}

	void shrink_buffer() {
		const char * firstItem = reinterpret_cast<char *>(m_buffer.get());
		size_t itemBytes = m_nextItem - firstItem;
		size_t itemElts = (itemBytes + sizeof(size_t) - 1) / sizeof(size_t);
		size_t pointers = m_pointersEnd - m_pointers;
		array<size_t> newBuffer(itemElts + pointers);
		std::copy(m_buffer.get(), m_buffer.get() + itemElts, newBuffer.get());
		std::copy(m_pointers, m_pointersEnd, newBuffer.get() + itemElts);

		m_pointersEnd = newBuffer.get() + newBuffer.size();
		m_pointers = m_pointersEnd - pointers;
		char * newFirstItem = reinterpret_cast<char *>(newBuffer.get());
		m_nextItem = newFirstItem + itemBytes;

		m_buffer.swap(newBuffer);
	}

	void sort() {
		const char * firstItem = reinterpret_cast<char *>(m_buffer.get());
		size_t limit = m_nextItem - firstItem;
		for (size_t * x = m_pointers; x != m_pointersEnd; ++x) {
			if (*x > limit) throw tpie::exception("Out of bounds!");
		}
		base_offset_compare<pred_t> pred(firstItem, m_nextItem - firstItem, m_pred);
		// TODO predicate might be very expensive (2*unserialize per compare),
		// so we should use merge sort instead of parallel_sort
		// since it does minimal no. of comparisons even in the worst case
		parallel_sort(m_pointers, m_pointersEnd, pred);
	}

	/*
	const T * begin() const {
		return m_buffer.get();
	}

	const T * end() const {
		return m_buffer.get() + m_items;
	}
	*/

	T pull() {
		const char * firstItem = reinterpret_cast<char *>(m_buffer.get());
		T item;
		memcpy_reader src(firstItem + *m_pointers++);
		using tpie::unserialize;
		unserialize(src, item);
		return item;
	}

	bool can_pull() const {
		return m_pointers != m_pointersEnd;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Deallocate buffer and call reset().
	///////////////////////////////////////////////////////////////////////////
	void free() {
		m_buffer.resize(0);
		reset();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Reset sorter, but keep the remembered largest item size and
	/// buffer size.
	///////////////////////////////////////////////////////////////////////////
	void reset() {
		if (m_buffer.size() == 0) {
			m_pointers = m_pointersEnd = NULL;
			m_nextItem = NULL;
		} else {
			size_t n = m_buffer.size();
			m_pointers = m_pointersEnd = static_cast<size_t *>(m_buffer.get()) + n;
			m_nextItem = reinterpret_cast<char *>(m_buffer.get());
		}
		m_full = false;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  File handling for merge sort.
///
/// This class abstracts away the details of numbering run files; tracking the
/// number of runs in each merge level; informing the TPIE stats framework of
/// the temporary size; deleting run files after use.
///
/// The important part of the state is the tuple consisting of
/// (a, b, c) := (fileOffset, nextLevelFileOffset, nextFileOffset).
/// `a` is the first file in the level currently being merged;
/// `b` is the first file in the level being merged into;
/// `c` is the next file to write output to.
///
/// ## Transition system
///
/// We let remainingRuns := b - a, and nextLevelRuns := c - b.
///
/// The tuple (remainingRuns, nextLevelRuns) has the following transitions:
/// On open_new_writer(): (x, y) -> (x, 1+y),
/// On open_readers(fanout): (fanout+x, y) -> (fanout+x, y),
/// On open_readers(fanout): (0, fanout+y) -> (fanout+y, 0),
/// On close_readers_and_delete(): (fanout+x, y) -> (x, y).
///
/// ## Merge sorter usage
///
/// During run formation (the first phase of merge sort), we repeatedly call
/// open_new_writer() and close_writer() to write out runs to the disk.
///
/// After run formation, we call open_readers(fanout) to advance into the first
/// level of the merge heap (so one can think of run formation as a "zeroth
/// level" in the merge heap).
///
/// As a slight optimization, when remaining_runs() == 1, one may call
/// move_last_reader_to_next_level() to move the remaining run into the next
/// merge level without scanning through and copying the single remaining run.
///
/// See serialization_sorter::merge_runs() for the logic involving
/// next_level_runs() and remaining_runs() in a loop.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class file_handler {
	// Physical index of the run file with logical index 0.
	size_t m_fileOffset;
	// Physical index of the run file that begins the next run.
	size_t m_nextLevelFileOffset;
	// Physical index of the next run file to write
	size_t m_nextFileOffset;

	bool m_writerOpen;
	size_t m_readersOpen;

	serialization_writer m_writer;
	stream_size_type m_currentWriterByteSize;

	array<serialization_reader> m_readers;

	std::string m_tempDir;

	std::string run_file(size_t physicalIndex) {
		if (m_tempDir.size() == 0) throw exception("run_file: temp dir is the empty string");
		std::stringstream ss;
		ss << m_tempDir << '/' << physicalIndex << ".tpie";
		return ss.str();
	}

public:
	file_handler()
		: m_fileOffset(0)
		, m_nextLevelFileOffset(0)
		, m_nextFileOffset(0)

		, m_writerOpen(false)
		, m_readersOpen(0)

		, m_writer()
		, m_currentWriterByteSize(0)
	{
	}

	~file_handler() {
		reset();
	}

	void set_temp_dir(const std::string & tempDir) {
		if (m_nextFileOffset != 0)
			throw exception("set_temp_dir: trying to change path after files already open");
		m_tempDir = tempDir;
	}

	void open_new_writer() {
		if (m_writerOpen) throw exception("open_new_writer: Writer already open");
		m_writer.open(run_file(m_nextFileOffset++));
		m_currentWriterByteSize = m_writer.file_size();
		m_writerOpen = true;
	}

	void write(const T & v) {
		if (!m_writerOpen) throw exception("write: No writer open");
		m_writer.serialize(v);
	}

	void close_writer() {
		if (!m_writerOpen) throw exception("close_writer: No writer open");
		m_writer.close();
		stream_size_type sz = m_writer.file_size();
		increase_usage(m_nextFileOffset-1, static_cast<stream_offset_type>(sz));
		m_writerOpen = false;
	}

	size_t remaining_runs() {
		return m_nextLevelFileOffset - m_fileOffset;
	}

	size_t next_level_runs() {
		return m_nextFileOffset - m_nextLevelFileOffset;
	}

	bool readers_open() {
		return m_readersOpen > 0;
	}

	void open_readers(size_t fanout) {
		if (m_readersOpen != 0) throw exception("open_readers: readers already open");
		if (fanout == 0) throw exception("open_readers: fanout == 0");
		if (remaining_runs() == 0) {
			if (m_writerOpen) throw exception("Writer open while moving to next merge level");
			m_nextLevelFileOffset = m_nextFileOffset;
		}
		if (fanout > remaining_runs()) throw exception("open_readers: fanout out of bounds");

		if (m_readers.size() < fanout) m_readers.resize(fanout);
		for (size_t i = 0; i < fanout; ++i) {
			m_readers[i].open(run_file(m_fileOffset + i));
		}
		m_readersOpen = fanout;
	}

	bool can_read(size_t idx) {
		if (m_readersOpen == 0) throw exception("can_read: no readers open");
		if (m_readersOpen < idx) throw exception("can_read: index out of bounds");
		return m_readers[idx].can_read();
	}

	T read(size_t idx) {
		if (m_readersOpen == 0) throw exception("read: no readers open");
		if (m_readersOpen < idx) throw exception("read: index out of bounds");
		T res;
		m_readers[idx].unserialize(res);
		return res;
	}

	void close_readers_and_delete() {
		if (m_readersOpen == 0) throw exception("close_readers_and_delete: no readers open");

		for (size_t i = 0; i < m_readersOpen; ++i) {
			decrease_usage(m_fileOffset + i, m_readers[i].file_size());
			m_readers[i].close();
			boost::filesystem::remove(run_file(m_fileOffset + i));
		}
		m_fileOffset += m_readersOpen;
		m_readersOpen = 0;
	}

	void move_last_reader_to_next_level() {
		if (remaining_runs() != 1)
			throw exception("move_last_reader_to_next_level: remaining_runs != 1");
		m_nextLevelFileOffset = m_fileOffset;
	}

	void reset() {
		if (m_readersOpen > 0) {
			log_debug() << "reset: Close readers" << std::endl;
			close_readers_and_delete();
		}
		m_readers.resize(0);
		if (m_writerOpen) {
			log_debug() << "reset: Close writer" << std::endl;
			close_writer();
		}
		log_debug() << "Remove " << m_fileOffset << " through " << m_nextFileOffset << std::endl;
		for (size_t i = m_fileOffset; i < m_nextFileOffset; ++i) {
			std::string runFile = run_file(i);
			serialization_reader rd;
			rd.open(runFile);
			decrease_usage(i, rd.file_size());
			rd.close();
			boost::filesystem::remove(runFile);
		}
		m_fileOffset = m_nextLevelFileOffset = m_nextFileOffset = 0;
	}

private:
	void increase_usage(size_t idx, stream_size_type sz) {
		log_debug() << "+ " << idx << ' ' << sz << std::endl;
		increment_temp_file_usage(static_cast<stream_offset_type>(sz));
	}

	void decrease_usage(size_t idx, stream_size_type sz) {
		log_debug() << "- " << idx << ' ' << sz << std::endl;
		increment_temp_file_usage(-static_cast<stream_offset_type>(sz));
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Serialization merger.
///
/// \tparam pred_t  less than-predicate accepting unserialized items.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename pred_t>
class merger {
	class mergepred_t {
		pred_t m_pred;

	public:
		typedef std::pair<T, size_t> item_type;

		mergepred_t(const pred_t & pred) : m_pred(pred) {}

		// Used with std::priority_queue, so invert the original relation.
		bool operator()(const item_type & a, const item_type & b) const {
			return m_pred(b.first, a.first);
		}
	};

	typedef typename mergepred_t::item_type item_type;

	file_handler<T> & files;
	pred_t pred;
	std::vector<serialization_reader> rd;
	typedef std::priority_queue<item_type, std::vector<item_type>, mergepred_t> priority_queue_type;
	priority_queue_type pq;

public:
	merger(file_handler<T> & files, const pred_t & pred)
		: files(files)
		, pred(pred)
		, pq(mergepred_t(pred))
	{
	}

	// Assume files.open_readers(fanout) has just been called
	void init(size_t fanout) {
		rd.resize(fanout);
		for (size_t i = 0; i < fanout; ++i)
			push_from(i);
	}

	bool empty() const {
		return pq.empty();
	}

	const T & top() const {
		return pq.top().first;
	}

	void pop() {
		size_t idx = pq.top().second;
		pq.pop();
		push_from(idx);
	}

	// files.close_readers_and_delete() should be called after this
	void free() {
		{
			priority_queue_type tmp(pred);
			std::swap(pq, tmp);
		}
		rd.resize(0);
	}

private:
	void push_from(size_t idx) {
		if (files.can_read(idx)) {
			pq.push(std::make_pair(files.read(idx), idx));
		}
	}
};

} // namespace serialization_bits

template <typename T, typename internal_pred_t, typename serialized_pred_t>
class serialization_sorter {
public:
	typedef boost::shared_ptr<serialization_sorter> ptr;

private:
	enum sorter_state { state_initial, state_1, state_2, state_3 };

	sorter_state m_state;
	serialization_bits::internal_sort<T, serialized_pred_t> m_sorter;
	serialization_bits::sort_parameters m_params;
	bool m_parametersSet;
	serialization_bits::file_handler<T> m_files;
	serialization_bits::merger<T, internal_pred_t> m_merger;

	stream_size_type m_items;
	bool m_reportInternal;

public:
	serialization_sorter(internal_pred_t p1=internal_pred_t(),
			serialized_pred_t p2=serialized_pred_t())
		: m_state(state_initial)
		, m_sorter(p2)
		, m_parametersSet(false)
		, m_files()
		, m_merger(m_files, p1)
		, m_items(0)
		, m_reportInternal(false)
	{
		m_params.memoryPhase1 = 0;
		m_params.memoryPhase2 = 0;
		m_params.memoryPhase3 = 0;
	}

private:
	// set_phase_?_memory helper
	inline void maybe_calculate_parameters() {
		if (m_state != state_initial)
			throw tpie::exception("Bad state in maybe_calculate_parameters");
		if (m_params.memoryPhase1 > 0 &&
			m_params.memoryPhase2 > 0 &&
			m_params.memoryPhase3 > 0)

			calculate_parameters();
	}

public:
	void set_phase_1_memory(memory_size_type m1) {
		m_params.memoryPhase1 = m1;
		maybe_calculate_parameters();
	}

	void set_phase_2_memory(memory_size_type m2) {
		m_params.memoryPhase2 = m2;
		maybe_calculate_parameters();
	}

	void set_phase_3_memory(memory_size_type m3) {
		m_params.memoryPhase3 = m3;
		maybe_calculate_parameters();
	}

	void set_available_memory(memory_size_type m) {
		set_phase_1_memory(m);
		set_phase_2_memory(m);
		set_phase_3_memory(m);
	}

	void set_available_memory(memory_size_type m1, memory_size_type m2, memory_size_type m3) {
		set_phase_1_memory(m1);
		set_phase_2_memory(m2);
		set_phase_3_memory(m3);
	}

	static memory_size_type minimum_memory_phase_1() {
		return serialization_writer::memory_usage()*2;
	}

	static memory_size_type minimum_memory_phase_2() {
		return serialization_writer::memory_usage()
			+ 2*serialization_reader::memory_usage();
	}

	static memory_size_type minimum_memory_phase_3() {
		return 2*serialization_reader::memory_usage();
	}

private:
	void calculate_parameters() {
		if (m_state != state_initial)
			throw tpie::exception("Bad state in calculate_parameters");

		memory_size_type memAvail1 = m_params.memoryPhase1;
		if (memAvail1 <= serialization_writer::memory_usage()) {
			log_error() << "Not enough memory for run formation; have " << memAvail1
				<< " bytes but " << serialization_writer::memory_usage()
				<< " is required for writing a run." << std::endl;
			throw exception("Not enough memory for run formation");
		}

		memory_size_type memAvail2 = m_params.memoryPhase2;

		// We have to keep a writer open no matter what.
		if (memAvail2 <= serialization_writer::memory_usage()) {
			log_error() << "Not enough memory for merging. "
				<< "mem avail = " << memAvail2
				<< ", writer usage = " << serialization_writer::memory_usage()
				<< std::endl;
			throw exception("Not enough memory for merging.");
		}

		memory_size_type memAvail3 = m_params.memoryPhase3;

		// We have to keep a writer open no matter what.
		if (memAvail2 <= serialization_writer::memory_usage()) {
			log_error() << "Not enough memory for outputting. "
				<< "mem avail = " << memAvail3
				<< ", writer usage = " << serialization_writer::memory_usage()
				<< std::endl;
			throw exception("Not enough memory for outputting.");
		}

		memory_size_type memForMerge = std::min(memAvail2, memAvail3);

		// We do not yet know the serialized size of the largest item,
		// so this calculation has to be redone.
		// Instead, we assume that all items have minimum size.

		// We have to keep a writer open no matter what.
		memory_size_type fanoutMemory = memForMerge - serialization_writer::memory_usage();

		// This is a lower bound on the memory used per fanout.
		memory_size_type perFanout = sizeof(T) + serialization_reader::memory_usage();

		// Floored division to compute the largest possible fanout.
		memory_size_type fanout = fanoutMemory / perFanout;
		if (fanout < 2) {
			log_error() << "Not enough memory for merging, even when minimum item size is assumed. "
				<< "mem avail = " << memForMerge
				<< ", fanout memory = " << fanoutMemory
				<< ", per fanout >= " << perFanout
				<< std::endl;
			throw exception("Not enough memory for merging.");
		}

		m_params.tempDir = tempname::tpie_dir_name();
		m_files.set_temp_dir(m_params.tempDir);

		log_info() << "Calculated serialization_sorter parameters.\n";
		m_params.dump(log_info());
		log_info() << std::flush;

		m_parametersSet = true;
	}

public:
	void begin() {
		if (!m_parametersSet)
			throw tpie::exception("Parameters not set in serialization_sorter");
		if (m_state != state_initial)
			throw tpie::exception("Bad state in begin");
		m_state = state_1;

		log_info() << "Before begin; mem usage = "
			<< get_memory_manager().used() << std::endl;
		m_sorter.begin(m_params.memoryPhase1 - serialization_writer::memory_usage());
		log_info() << "After internal sorter begin; mem usage = "
			<< get_memory_manager().used() << std::endl;
		boost::filesystem::create_directory(m_params.tempDir);
	}

	void push(const T & item) {
		if (m_state != state_1)
			throw tpie::exception("Bad state in push");

		++m_items;

		if (m_sorter.push(item)) return;
		end_run();
		if (!m_sorter.push(item)) {
			throw exception("Couldn't fit a single item in buffer");
		}
	}

	void end() {
		if (m_state != state_1)
			throw tpie::exception("Bad state in end");

		memory_size_type internalThreshold =
			std::min(m_params.memoryPhase2, m_params.memoryPhase3);

		log_debug() << "m_sorter.memory_usage == " << m_sorter.memory_usage() << '\n'
			<< "internalThreshold == " << internalThreshold << std::endl;

		if (m_items == 0) {
			m_reportInternal = true;
			m_sorter.free();
			log_debug() << "Got no items. Internal reporting mode." << std::endl;
		} else if (m_files.next_level_runs() == 0
			&& m_sorter.memory_usage()
			   <= internalThreshold) {

			m_sorter.sort();
			m_reportInternal = true;
			log_debug() << "Got " << m_sorter.current_buffer_usage()
				<< " bytes of items. Internal reporting mode." << std::endl;
		} else if (m_files.next_level_runs() == 0
				   && m_sorter.current_buffer_usage() <= internalThreshold
				   && m_sorter.can_shrink_buffer()) {

			m_sorter.sort();
			m_sorter.shrink_buffer();
			m_reportInternal = true;
			log_debug() << "Got " << m_sorter.current_buffer_usage()
				<< " bytes of items. Internal reporting mode after shrinking buffer." << std::endl;

		} else {

			end_run();
			log_debug() << "Got " << m_files.next_level_runs() << " runs. "
				<< "External reporting mode." << std::endl;
			m_sorter.free();
			m_reportInternal = false;
		}

		log_info() << "After internal sorter end; mem usage = "
			<< get_memory_manager().used() << std::endl;

		m_state = state_2;
	}

	stream_size_type item_count() {
		return m_items;
	}

	void evacuate() {
		switch (m_state) {
			case state_initial:
				throw tpie::exception("Cannot evacuate in state initial");
			case state_1:
				throw tpie::exception("Cannot evacuate in state 1");
			case state_2:
			case state_3:
				if (m_reportInternal) {
					end_run();
					m_sorter.free();
					m_reportInternal = false;
					log_debug() << "Evacuate out of internal reporting mode." << std::endl;
				} else {
					log_debug() << "Evacuate in external reporting mode - noop." << std::endl;
				}
				break;
		}
	}

	memory_size_type evacuated_memory_usage() const {
		return 0;
	}

	void merge_runs() {
		if (m_state != state_2)
			throw tpie::exception("Bad state in end");

		if (m_reportInternal) {
			log_debug() << "merge_runs: internal reporting; doing nothing." << std::endl;
			m_state = state_3;
			return;
		}

		memory_size_type largestItem = m_sorter.get_largest_item_size();
		if (largestItem == 0) {
			log_warning() << "Largest item is 0 bytes; doing nothing." << std::endl;
			m_state = state_3;
			return;
		}

		if (m_params.memoryPhase2 <= serialization_writer::memory_usage())
			throw exception("Not enough memory for merging.");

		// Perform almost the same computation as in calculate_parameters.
		// Only change the item size to largestItem rather than sizeof(T).
		memory_size_type fanoutMemory = m_params.memoryPhase2 - serialization_writer::memory_usage();
		memory_size_type perFanout = largestItem + serialization_reader::memory_usage();
		memory_size_type fanout = fanoutMemory / perFanout;

		if (fanout < 2) {
			log_error() << "Not enough memory for merging. "
				<< "mem avail = " << m_params.memoryPhase2
				<< ", fanout memory = " << fanoutMemory
				<< ", per fanout = " << perFanout
				<< std::endl;
			throw exception("Not enough memory for merging.");
		}

		memory_size_type finalFanoutMemory = m_params.memoryPhase3;
		memory_size_type finalFanout =
			std::min(fanout,
					 finalFanoutMemory / perFanout);

		if (finalFanout < 2) {
			log_error() << "Not enough memory for merging (final fanout < 2). "
				<< "mem avail = " << m_params.memoryPhase3
				<< ", final fanout memory = " << finalFanoutMemory
				<< ", per fanout = " << perFanout
				<< std::endl;
			throw exception("Not enough memory for merging.");
		}

		log_debug() << "Calculated merge phase parameters for serialization sort.\n"
			<< "Fanout:       " << fanout << '\n'
			<< "Final fanout: " << finalFanout << '\n'
			;

		while (m_files.next_level_runs() > finalFanout) {
			if (m_files.remaining_runs() != 0)
				throw exception("m_files.remaining_runs() != 0");
			log_debug() << "Runs in current level: " << m_files.next_level_runs() << '\n';
			for (size_t remainingRuns = m_files.next_level_runs(); remainingRuns > 0;) {
				size_t f = std::min(fanout, remainingRuns);
				merge_runs(f);
				remainingRuns -= f;
				if (remainingRuns != m_files.remaining_runs())
					throw exception("remainingRuns != m_files.remaining_runs()");
			}
		}

		m_state = state_3;
	}

private:
	void end_run() {
		m_sorter.sort();
		if (!m_sorter.can_pull()) return;
		m_files.open_new_writer();

		// TODO Maybe we could get rid of this unserialize+serialize.
		// Either just keep the serialized size of each item somehow,
		// or compute serialized size by unserializing into a counter and
		// memcpying the serialized item.
		// On the other hand, we just did log(n) expensive comparisons
		// with each item in sort(), so what the heck...
		while (m_sorter.can_pull())
			m_files.write(m_sorter.pull());
		m_files.close_writer();
		m_sorter.reset();
	}

	void initialize_merger(size_t fanout) {
		if (fanout == 0) throw exception("initialize_merger: fanout == 0");
		m_files.open_readers(fanout);
		m_merger.init(fanout);
	}

	void free_merger_and_files() {
		m_merger.free();
		m_files.close_readers_and_delete();
	}

	void merge_runs(size_t fanout) {
		if (fanout == 0) throw exception("merge_runs: fanout == 0");

		if (fanout == 1 && m_files.remaining_runs() == 1) {
			m_files.move_last_reader_to_next_level();
			return;
		}

		initialize_merger(fanout);
		m_files.open_new_writer();
		while (!m_merger.empty()) {
			m_files.write(m_merger.top());
			m_merger.pop();
		}
		free_merger_and_files();
		m_files.close_writer();
	}

public:
	T pull() {
		if (!can_pull())
			throw exception("pull: !can_pull");

		if (m_reportInternal) {
			T item = m_sorter.pull();
			if (!m_sorter.can_pull()) m_sorter.free();
			return item;
		}

		if (!m_files.readers_open()) {
			if (m_files.next_level_runs() == 0)
				throw exception("pull: next_level_runs == 0");
			initialize_merger(m_files.next_level_runs());
		}

		T item = m_merger.top();
		m_merger.pop();

		if (m_merger.empty()) {
			free_merger_and_files();
			m_files.reset();
		}

		return item;
	}

	bool can_pull() {
		if (m_reportInternal) return m_sorter.can_pull();
		if (!m_files.readers_open()) return m_files.next_level_runs() > 0;
		return !m_merger.empty();
	}
};

}

#endif // TPIE_SERIALIZATION_SORTER_H
