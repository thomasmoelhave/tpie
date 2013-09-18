// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2012, 2013, The TPIE development team
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

#include "common.h"
#include <tpie/pipelining/merge_sorter.h>
#include <tpie/parallel_sort.h>
#include <tpie/sysinfo.h>
#include <boost/random.hpp>

using namespace tpie;
using namespace tpie::pipelining;

typedef uint64_t test_t;

struct relative_memory_usage {
	inline relative_memory_usage(memory_size_type extraMemory)
		: m_startMemory(actual_used())
		, m_threshold(0)
		, m_extraMemory(extraMemory)
	{
	}

	inline memory_size_type used() {
		return actual_used() - m_startMemory;
	}

	inline void set_threshold(memory_size_type threshold) {
		m_threshold = threshold;
		get_memory_manager().set_limit(m_startMemory + m_threshold + m_extraMemory);
	}

	inline bool below(memory_size_type threshold) {
		if (used() > threshold) {
			report_overusage(threshold);
			return false;
		}
		return true;
	}

	inline bool below() {
		return below(m_threshold);
	}

	void report_overusage(memory_size_type threshold) {
		log_error() << "Memory usage " << used() << " exceeds threshold " << threshold << std::endl;
	}

private:
	inline static memory_size_type actual_used() {
		return get_memory_manager().used();
	}

	memory_size_type m_startMemory;
	memory_size_type m_threshold;
	memory_size_type m_extraMemory;
};

bool sort_upper_bound_test() {
	memory_size_type m1 = 100 *1024*1024;
	memory_size_type m2 = 20  *1024*1024;
	memory_size_type m3 = 20  *1024*1024;
	memory_size_type dataSize = 15*1024*1024;
	memory_size_type dataUpperBound = 80*1024*1024;

	memory_size_type items = dataSize / sizeof(test_t);

	stream_size_type io = get_bytes_written();

	relative_memory_usage m(0);
	merge_sorter<test_t, false> s;
	s.set_available_memory(m1, m2, m3);
	s.set_items(dataUpperBound / sizeof(test_t));

	m.set_threshold(m1);
	s.begin();
	for (stream_size_type i = 0; i < items; ++i) {
		s.push(i);
	}
	s.end();
	dummy_progress_indicator pi;
	s.calc(pi);
	while (s.can_pull()) s.pull();

	return io == get_bytes_written();
}

bool sort_test(memory_size_type m1,
			   memory_size_type m2,
			   memory_size_type m3,
			   double mb_data,
			   memory_size_type extraMemory = 0,
			   bool evacuateBeforeMerge = false,
			   bool evacuateBeforeReport = false)
{
	m1 *= 1024*1024;
	m2 *= 1024*1024;
	m3 *= 1024*1024;
	extraMemory *= 1024*1024;
	stream_size_type items = static_cast<stream_size_type>(mb_data*1024/sizeof(test_t)*1024);
	log_debug() << "sort_test with " << items << " items\n";
	boost::rand48 rng;
	relative_memory_usage m(extraMemory);
	merge_sorter<test_t, false> s;
	s.set_available_memory(m1, m2, m3);

	log_debug() << "Begin phase 1" << std::endl;
	m.set_threshold(m1);
	s.begin();
	if (!m.below()) return false;
	for (stream_size_type i = 0; i < items; ++i) {
		s.push(rng());
		if (!m.below()) return false;
	}
	s.end();
	if (!m.below()) return false;

	if (evacuateBeforeMerge) {
		s.evacuate();
		log_debug() << "MEMORY USED: " << m.used() << '/' << s.evacuated_memory_usage() << std::endl;
		if (!m.below(s.evacuated_memory_usage())) return false;
	}

	log_debug() << "Begin phase 2" << std::endl;
	m.set_threshold(m2);
	if (!m.below()) return false;
	dummy_progress_indicator pi;
	s.calc(pi);
	if (!m.below()) return false;

	if (evacuateBeforeReport) {
		s.evacuate();
		log_debug() << "MEMORY USED: " << m.used() << '/' << s.evacuated_memory_usage() << std::endl;
		if (!m.below(s.evacuated_memory_usage())) return false;
	}

	log_debug() << "Begin phase 3" << std::endl;
	m.set_threshold(m3);
	if (!m.below()) return false;
	test_t prev = std::numeric_limits<test_t>::min();
	memory_size_type itemsRead = 0;
	while (s.can_pull()) {
		if (!m.below()) return false;
		test_t read = s.pull();
		if (!m.below()) return false;
		if (read < prev) {
			log_error() << "Out of order" << std::endl;
			return false;
		}
		prev = read;
		++itemsRead;
	}
	if (itemsRead != items) {
		log_error() << "Read the wrong number of items. Got " << itemsRead << ", expected " << items << std::endl;
		return false;
	}

	log_debug() << "MEMORY USED: " << m.used() << '/' << s.evacuated_memory_usage() << std::endl;
	if (!m.below(s.evacuated_memory_usage())) return false;

	return true;
}

bool internal_report_test() {
	return sort_test(100,100,100,50);
}

bool internal_report_after_resize_test() {
	return sort_test(100,80,80,50, 100);
}

bool one_run_external_report_test() {
	return sort_test(100,11,11,7);
}

bool external_report_test() {
	return sort_test(20,20,20,50);
}

bool small_final_fanout_test(double mb) {
	return sort_test(7,20,11,mb);
}

bool evacuate_before_merge_test() {
	return sort_test(20,20,20,10, 0, true, false);
}

bool evacuate_before_report_test() {
	return sort_test(20,20,20,50, 0, false, true);
}

bool temp_file_usage_test() {
	const stream_size_type initialUsage = get_temp_file_usage();
	log_debug() << "Offsetting get_temp_file_usage by " << initialUsage
				<< std::endl;
	const stream_size_type runLength = get_block_size() / sizeof(test_t);
	const memory_size_type runs = 16;
	const memory_size_type fanout = runs;
	{
		merge_sorter<test_t, false> s;
		s.set_parameters(runLength, fanout);
		s.begin();
		for (test_t i = 0; i < runs * runLength; ++i) {
			s.push(i);
		}
		s.end();
		log_debug() << "After phase 1: " << (get_temp_file_usage()
											 - initialUsage) << std::endl;
		dummy_progress_indicator pi;
		s.calc(pi);
		log_debug() << "After phase 2: " << (get_temp_file_usage()
											 - initialUsage) << std::endl;
		while (s.can_pull()) s.pull();
		log_debug() << "After phase 3: " << (get_temp_file_usage()
											 - initialUsage) << std::endl;
	}
	log_debug() << "After destroying merge_sorter: "
				<< (get_temp_file_usage() - initialUsage) << std::endl;
	return true;
}

int main(int argc, char ** argv) {
	return tests(argc, argv)
		.test(internal_report_test, "internal_report")
		.test(internal_report_after_resize_test, "internal_report_after_resize")
		.test(one_run_external_report_test, "one_run_external_report")
		.test(external_report_test, "external_report")
		.test(small_final_fanout_test, "small_final_fanout", "mb", 4.5)
		.test(evacuate_before_merge_test, "evacuate_before_merge")
		.test(evacuate_before_report_test, "evacuate_before_report")
		.test(sort_upper_bound_test, "sort_upper_bound")
		.test(temp_file_usage_test, "temp_file_usage")
		;
}
