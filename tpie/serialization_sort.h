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

#ifndef TPIE_SERIALIZATION_SORT_H
#define TPIE_SERIALIZATION_SORT_H

#include <queue>
#include <boost/filesystem.hpp>

#include <tpie/array.h>
#include <tpie/tempname.h>
#include <tpie/tpie_log.h>
#include <tpie/stats.h>
#include <tpie/parallel_sort.h>

#include <tpie/serialization2.h>
#include <tpie/serialization_stream.h>

namespace tpie {

template <typename T, typename pred_t>
class serialization_internal_sort {
	array<T> m_buffer;
	memory_size_type m_items;
	memory_size_type m_serializedSize;
	memory_size_type m_memAvail;

	memory_size_type m_itemsRead;
	memory_size_type m_largestItem;

	pred_t m_pred;

	bool m_full;

public:
	serialization_internal_sort(pred_t pred = pred_t())
		: m_items(0)
		, m_serializedSize(0)
		, m_itemsRead(0)
		, m_largestItem(sizeof(T))
		, m_pred(pred)
		, m_full(false)
	{
	}

	void begin(memory_size_type memAvail) {
		m_buffer.resize(memAvail / sizeof(T) / 2);
		m_items = m_serializedSize = m_itemsRead = 0;
		m_largestItem = sizeof(T);
		m_full = false;
		m_memAvail = memAvail;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief True if all items up to and including this one fits in buffer.
	///
	/// Once push() returns false, it will keep returning false until
	/// the sequence is sorted, read out, and the buffer has been cleared.
	///////////////////////////////////////////////////////////////////////////
	bool push(const T & item) {
		if (m_full) return false;

		if (m_items == m_buffer.size()) {
			m_full = true;
			return false;
		}

		memory_size_type serSize = serialized_size(item);

		if (serSize > sizeof(T)) {
			// amount of memory this item needs for its extra stuff (stuff not in the buffer).
			memory_size_type serializedExtra = serSize - sizeof(T);

			// amount of memory not used for the buffer and not used for extra stuff already.
			memory_size_type memRemainingExtra = m_memAvail - (m_buffer.size() * sizeof(T) + (m_serializedSize - m_items * sizeof(T)));

			if (serializedExtra > memRemainingExtra) {
				m_full = true;
				return false;
			}

			if (serSize > m_largestItem)
				m_largestItem = serSize;
		}

		m_buffer[m_items++] = item;

		return true;
	}

	memory_size_type get_largest_item_size() {
		return m_largestItem;
	}

	void sort() {
		parallel_sort(m_buffer.get(), m_buffer.get() + m_items, m_pred);
		m_itemsRead = 0;
	}

	void pull(T & item) {
		item = m_buffer[m_itemsRead++];
	}

	bool can_read() {
		return m_itemsRead < m_items;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Deallocate buffer and call reset().
	///////////////////////////////////////////////////////////////////////////
	void end() {
		m_buffer.resize(0);
		reset();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Reset sorter, but keep the remembered largest item size and
	/// buffer size.
	///////////////////////////////////////////////////////////////////////////
	void reset() {
		m_items = m_serializedSize = m_itemsRead = 0;
		m_full = false;
	}
};

template <typename T, typename pred_t>
class serialization_sort {
	serialization_internal_sort<T, pred_t> m_sorter;
	memory_size_type m_sortedRunsCount;
	memory_size_type m_sortedRunsOffset;
	memory_size_type m_memAvail;
	memory_size_type m_runSize;
	memory_size_type m_minimumItemSize;
	serialization_reader m_reader;
	bool m_open;
	pred_t m_pred;

	memory_size_type m_firstFileUsed;
	memory_size_type m_lastFileUsed;

	std::string m_tempDir;

public:
	serialization_sort(memory_size_type memAvail, memory_size_type minimumItemSize = sizeof(T), pred_t pred = pred_t())
		: m_sorter(pred)
		, m_sortedRunsCount(0)
		, m_sortedRunsOffset(0)
		, m_memAvail(memAvail)
		, m_minimumItemSize(minimumItemSize)
		, m_open(false)
		, m_pred(pred)
		, m_firstFileUsed(0)
		, m_lastFileUsed(0)
	{
	}

	~serialization_sort() {
		m_sortedRunsOffset = 0;
		log_info() << "Remove " << m_firstFileUsed << " through " << m_lastFileUsed << std::endl;
		for (memory_size_type i = m_firstFileUsed; i <= m_lastFileUsed; ++i) {
			std::string path = sorted_run_path(i);
			if (!boost::filesystem::exists(path)) continue;

			serialization_reader rd;
			rd.open(path);
			log_info() << "- " << i << ' ' << rd.file_size() << std::endl;
			increment_temp_file_usage(-static_cast<stream_offset_type>(rd.file_size()));
			rd.close();
			boost::filesystem::remove(path);
		}
	}

	void calc_sorter_params(memory_size_type memAvail) {
		if (memAvail <= serialization_writer::memory_usage()) {
			log_error() << "Not enough memory for run formation; have " << memAvail
				<< " bytes but " << serialization_writer::memory_usage()
				<< " is required for writing a run." << std::endl;
			throw exception("Not enough memory for run formation");
		}
		memAvail -= serialization_writer::memory_usage();

		m_sorter.begin(memAvail);
	}

	void begin() {
		log_info() << "Before begin; mem usage = "
			<< get_memory_manager().used() << std::endl;
		calc_sorter_params(m_memAvail);
		log_info() << "After internal sorter begin; mem usage = "
			<< get_memory_manager().used() << std::endl;
		m_tempDir = tempname::tpie_dir_name();
		boost::filesystem::create_directory(m_tempDir);
	}

	void push(const T & item) {
		if (m_sorter.push(item)) return;
		end_run();
		if (!m_sorter.push(item)) {
			throw exception("Couldn't fit a single item in buffer");
		}
	}

	void end() {
		end_run();
		m_sorter.end();
		log_info() << "After internal sorter end; mem usage = "
			<< get_memory_manager().used() << std::endl;
		memory_size_type largestItem = m_sorter.get_largest_item_size();
		if (largestItem == 0) {
			log_warning() << "Largest item is 0 bytes; doing nothing." << std::endl;
			return;
		}
		if (m_memAvail <= serialization_writer::memory_usage()) {
			log_error() << "Not enough memory for merging. "
				<< "mem avail = " << m_memAvail
				<< ", writer usage = " << serialization_writer::memory_usage()
				<< std::endl;
			throw exception("Not enough memory for merging.");
		}
		memory_size_type perFanout = largestItem + serialization_reader::memory_usage();
		memory_size_type fanoutMemory = m_memAvail - serialization_writer::memory_usage();
		memory_size_type fanout = fanoutMemory / perFanout;
		if (fanout < 2) {
			log_error() << "Not enough memory for merging. "
				<< "mem avail = " << m_memAvail
				<< ", fanout memory = " << fanoutMemory
				<< ", per fanout = " << perFanout
				<< std::endl;
			throw exception("Not enough memory for merging.");
		}
		while (m_sortedRunsCount > 1) {
			memory_size_type newCount = 0;
			for (size_t i = 0; i < m_sortedRunsCount; i += fanout, ++newCount) {
				size_t till = std::min(m_sortedRunsCount, i + fanout);
				merge_runs(i, till, m_sortedRunsCount + newCount);
			}
			log_info() << "Advance offset by " << m_sortedRunsCount << std::endl;
			m_sortedRunsOffset += m_sortedRunsCount;
			m_sortedRunsCount = newCount;
		}
	}

private:
	std::string sorted_run_path(memory_size_type idx) {
		std::stringstream ss;
		ss << m_tempDir << '/' << (m_sortedRunsOffset + idx) << ".tpie";
		return ss.str();
	}

	void end_run() {
		m_sorter.sort();
		if (!m_sorter.can_read()) return;
		log_info() << "Write run " << m_sortedRunsCount << std::endl;
		serialization_writer ser;
		m_lastFileUsed = std::max(m_lastFileUsed, m_sortedRunsCount + m_sortedRunsOffset);
		ser.open(sorted_run_path(m_sortedRunsCount++));
		T item;
		while (m_sorter.can_read()) {
			m_sorter.pull(item);
			ser.serialize(item);
		}
		ser.close();
		log_info() << "+ " << (m_sortedRunsCount-1 + m_sortedRunsOffset) << ' ' << ser.file_size() << std::endl;
		increment_temp_file_usage(static_cast<stream_offset_type>(ser.file_size()));
		m_sorter.reset();
	}

	class mergepred {
		pred_t m_pred;

	public:
		typedef std::pair<T, size_t> item_type;

		mergepred(const pred_t & pred) : m_pred(pred) {}

		// Used with std::priority_queue, so invert the original relation.
		bool operator()(const item_type & a, const item_type & b) const {
			return m_pred(b.first, a.first);
		}
	};

	void merge_runs(memory_size_type a, memory_size_type b, memory_size_type dst) {
		log_info() << "Merge runs [" << a << ", " << b << ") into " << dst << std::endl;
		serialization_writer wr;
		wr.open(sorted_run_path(dst));
		m_lastFileUsed = std::max(m_lastFileUsed, m_sortedRunsOffset + dst);
		std::vector<serialization_reader> rd(b-a);
		mergepred p(m_pred);
		std::priority_queue<typename mergepred::item_type, std::vector<typename mergepred::item_type>, mergepred> pq(p);
		size_t i = 0;
		for (memory_size_type p = a; p != b; ++p, ++i) {
			rd[i].open(sorted_run_path(p));
			if (rd[i].can_read()) {
				T item;
				rd[i].unserialize(item);
				pq.push(std::make_pair(item, i));
			}
		}
		while (!pq.empty()) {
			wr.serialize(pq.top().first);
			size_t i = pq.top().second;
			pq.pop();
			if (rd[i].can_read()) {
				T item;
				rd[i].unserialize(item);
				pq.push(std::make_pair(item, i));
			}
		}
		i = 0;
		for (memory_size_type p = a; p != b; ++p, ++i) {
			increment_temp_file_usage(-static_cast<stream_offset_type>(rd[i].file_size()));
			log_info() << "- " << (p+m_sortedRunsOffset) << ' ' << rd[i].file_size() << std::endl;
			rd[i].close();
			boost::filesystem::remove(sorted_run_path(p));
		}
		if (m_firstFileUsed == m_sortedRunsOffset + a) m_firstFileUsed = m_sortedRunsOffset + b;
		wr.close();
		log_info() << "+ " << (dst+m_sortedRunsOffset) << ' ' << wr.file_size() << std::endl;
		increment_temp_file_usage(static_cast<stream_offset_type>(wr.file_size()));
	}

public:
	T pull() {
		T item;
		if (!m_open) {
			m_reader.open(sorted_run_path(0));
			m_open = true;
		}
		m_reader.unserialize(item);
		return item;
	}

	bool can_pull() {
		if (!m_open) return true;
		return m_reader.can_read();
	}
};

}

#endif // TPIE_SERIALIZATION_SORT_H
