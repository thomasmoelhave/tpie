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

template <typename T>
struct merger {
	inline merger() {
	}

	inline merger(const merger & other) {
		tp_assert(!other.in.size(), "Cannot copy ongoing merger");
		unused(other);
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
		return el;
	}

	inline void reset(array<file_stream<T> > & inputs, size_t runLength) {
		//TP_LOG_DEBUG_ID("Run length is " << runLength);
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

private:
	std::priority_queue<std::pair<T, size_t>, std::vector<std::pair<T, size_t> >, std::greater<std::pair<T, size_t> > > pq;
	array<file_stream<T> > in;
	std::vector<size_t> itemsRead;
	size_t runLength;
	size_t n;
};

template <typename T>
struct merge_sorter {
	inline merge_sorter()
		: m_runFiles(new array<temp_file>())
		, pull_prepared(false)
	{
		calculate_parameters();
	}

	inline void set_parameters(size_t runLength, size_t fanout) {
		p.runLength = runLength;
		p.fanout = fanout;
	}

	inline void begin() {
		m_currentRunItems.resize(p.runLength);
		m_runFiles->resize(p.fanout*2);
		m_currentRunItemCount = 0;
		m_finishedRuns = 0;
	}

	inline void push(const T & item) {
		if (m_currentRunItemCount >= p.runLength) {
			sort_current_run();
			empty_current_run();
		}
		m_currentRunItems[m_currentRunItemCount] = item;
		++m_currentRunItemCount;
	}

	inline void end() {
		sort_current_run();
		if (m_finishedRuns == 0) {
			m_reportInternal = true;
			m_itemsPulled = 0;
		} else {
			m_reportInternal = false;
			empty_current_run();
		}
	}

	inline void calc() {
		if (!m_reportInternal) {
			std::cout << "Preparing external calc!" << std::endl;
			prepare_pull();
		} else {
			std::cout << "Preparing internal calc!" << std::endl;
			pull_prepared = true;
		}
	}

	inline void sort_current_run() {
		parallel_sort(m_currentRunItems.begin(), m_currentRunItems.begin()+m_currentRunItemCount, std::less<T>());
	}

	// postcondition: m_currentRunItemCount = 0
	inline void empty_current_run() {
		file_stream<T> fs;
		//TP_LOG_DEBUG_ID("Empty run no. " << m_finishedRuns);
		open_run_file(fs, 0, m_finishedRuns, true);
		for (size_t i = 0; i < m_currentRunItemCount; ++i) {
			fs.write(m_currentRunItems[i]);
		}
		m_currentRunItemCount = 0;
		++m_finishedRuns;
	}

	// merge the runNumber'th to the (runNumber+runCount)'th run in mergeLevel
	inline void initialize_merger(size_t mergeLevel, size_t runNumber, size_t runCount) {
		//TP_LOG_DEBUG_ID("Initialize merger at level " << mergeLevel << " from run " << runNumber << " with " << runCount << " runs");
		array<file_stream<T> > in(runCount);
		for (size_t i = 0; i < runCount; ++i) {
			//TP_LOG_DEBUG_ID(runNumber+i);
			open_run_file(in[i], mergeLevel, runNumber+i, false);
		}
		size_t runLength = p.runLength;
		for (size_t i = 0; i < mergeLevel; ++i) {
			runLength *= p.fanout;
		}
		m_merger.reset(in, runLength);
	}

	inline void merge_runs(size_t mergeLevel, size_t runNumber, size_t runCount) {
		initialize_merger(mergeLevel, runNumber, runCount);
		file_stream<T> out;
		open_run_file(out, mergeLevel+1, runNumber/p.fanout, true);
		while (m_merger.can_pull()) {
			out.write(m_merger.pull());
		}
	}

	// merge all runs and initialize merger for public pulling
	inline void prepare_pull() {
		size_t mergeLevel = 0;
		size_t runCount = m_finishedRuns;
		while (runCount > p.fanout) {
			//TP_LOG_DEBUG_ID("Level " << mergeLevel << " has " << runCount << " runs");
			size_t newRunCount = 0;
			for (size_t i = 0; i < runCount; i += p.fanout) {
				merge_runs(mergeLevel, i, std::min(runCount-i, p.fanout));
				++newRunCount;
			}
			++mergeLevel;
			runCount = newRunCount;
		}
		//TP_LOG_DEBUG_ID("Final level " << mergeLevel << " has " << runCount << " runs");
		initialize_merger(mergeLevel, 0, runCount);

		pull_prepared = true;
	}

	inline bool can_pull() {
		tp_assert(pull_prepared, "Pull not prepared");
		if (m_reportInternal) return m_itemsPulled < m_currentRunItemCount;
		else return m_merger.can_pull();
	}

	inline T pull() {
		tp_assert(pull_prepared, "Pull not prepared");
		if (m_reportInternal && m_itemsPulled < m_currentRunItemCount) return m_currentRunItems[m_itemsPulled++];
		else return m_merger.pull();
	}

private:
	inline void calculate_parameters() {
		p.runLength = 64;
		p.fanout = 4;
	}

	// forWriting = false: open an existing run and seek to correct offset
	// forWriting = true: open run file and seek to end
	inline void open_run_file(file_stream<T> & fs, size_t mergeLevel, size_t runNumber, bool forWriting) {
		size_t idx = (mergeLevel % 2)*p.fanout + (runNumber % p.fanout);
		//TP_LOG_DEBUG_ID("mrglvl " << mergeLevel << " run no. " << runNumber << " has index " << idx);
		if (forWriting) {
			if (runNumber < p.fanout) m_runFiles->at(idx).free();
			fs.open(m_runFiles->at(idx), file_base::read_write);
			fs.seek(0, file_base::end);
		} else {
			fs.open(m_runFiles->at(idx), file_base::read);
			//TP_LOG_DEBUG_ID("seek to " << p.runLength * (runNumber / p.fanout) << " stream size " << fs.size());
			fs.seek(p.runLength * (runNumber / p.fanout), file_base::beginning);
		}
	}

	sort_parameters p;

	merger<T> m_merger;

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
};

template <typename dest_t>
struct sort_t : public pipe_segment {

	typedef typename dest_t::item_type item_type;
	typedef merge_sorter<item_type> sorter_t;
	typedef boost::shared_ptr<sorter_t> sorterptr;

	inline sort_t(const sort_t<dest_t> & other)
		: pipe_segment(other)
		, sorter(other.sorter)
		, calc(other.calc)
		, output(other.output)
	{
	}

	struct calc_t : public pipe_segment {
		inline calc_t(const calc_t & other)
			: pipe_segment(other)
			, sorter(other.sorter)
		{
		}

		inline calc_t(const pipe_segment & input, const sorterptr & sorter)
			: sorter(sorter)
		{
			add_dependency(input);
		}

		inline void go() {
			std::cout << "Gonna sort the sort sort!" << std::endl;
			sorter->calc();
		}
	private:
		sorterptr sorter;
	};

	struct output_t : public pipe_segment {
		inline output_t(const dest_t & dest, const pipe_segment & calc, const sorterptr & sorter)
			: dest(dest)
			, sorter(sorter)
		{
			add_dependency(calc);
			add_push_destination(dest);
		}

		void go() {
			std::cout << "Gonna push the sorted numbers!" << std::endl;
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

	inline sort_t(const dest_t & dest)
		: sorter(new sorter_t())
		, calc(*this, sorter)
		, output(dest, calc, sorter)
	{
	}

	inline void begin() {
		std::cout << "Gonna accept some sort input!" << std::endl;
		sorter->begin();
	}

	inline void push(const item_type & item) {
		sorter->push(item);
	}

	inline void end() {
		sorter->end();
		std::cout << "Accepted the sort input!" << std::endl;
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
	inline passive_sorter()
	{
	}

	struct output_t;

	struct input_t : public pipe_segment {
		typedef T item_type;

		inline input_t(temp_file * file, const segment_token & token)
			: pipe_segment(token)
			, file(file)
		{
		}

		inline void begin() {
			pbuffer = tpie_new<file_stream<T> >();
			pbuffer->open(*file);
		}

		inline void push(const T & item) {
			pbuffer->write(item);
		}

		inline void end() {
			pbuffer->seek(0);
			pred_t pred;
			progress_indicator_null pi;
			sort(*pbuffer, *pbuffer, pred, pi);
			pbuffer->close();

			tpie_delete(pbuffer);
		}

	private:
		temp_file * file;
		file_stream<T> * pbuffer;

		input_t();
		input_t & operator=(const input_t &);
	};

	struct output_t : public pipe_segment {
		typedef T item_type;

		inline output_t(temp_file * file, const segment_token & input)
			: file(file)
		{
			add_dependency(input);
		}

		inline void begin() {
			buffer = tpie_new<file_stream<T> >();
			buffer->open(*file);
		}

		inline bool can_pull() {
			return buffer->can_read();
		}

		inline T pull() {
			return buffer->read();
		}

		inline void end() {
			buffer->close();
		}

	private:
		temp_file * file;
		file_stream<T> * buffer;

		output_t();
		output_t & operator=(const output_t &);
	};

	inline pipe_end<termfactory_2<input_t, temp_file *, const segment_token &> > input() {
		std::cout << "Construct input factory " << typeid(pred_t).name() << " with " << &file << std::endl;
		return termfactory_2<input_t, temp_file *, const segment_token &>(&file, input_token);
	}

	inline output_t output() {
		return output_t(&file, input_token);
	}

private:
	pred_t pred;
	temp_file file;
	segment_token input_token;
	passive_sorter(const passive_sorter &);
	passive_sorter & operator=(const passive_sorter &);
};

}

}

#endif
