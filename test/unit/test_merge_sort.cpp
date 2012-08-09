// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
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

#include "common.h"
#include <tpie/pipelining/merge_sorter.h>
#include <tpie/parallel_sort.h>
#include <tpie/sysinfo.h>
#include <boost/random.hpp>

using namespace tpie;
using namespace tpie::pipelining;

typedef uint64_t test_t;

struct relative_memory_usage {
	inline relative_memory_usage()
		: m_startMemory(actual_used())
	{
	}

	inline memory_size_type used() {
		return actual_used() - m_startMemory;
	}

	inline void set_threshold(memory_size_type threshold) {
		m_threshold = threshold;
		get_memory_manager().set_limit(m_startMemory + m_threshold);
	}

	inline bool below() {
		if (used() > m_threshold) {
			report_overusage();
			return false;
		}
		return true;
	}

	void report_overusage() {
		log_error() << "Memory usage " << used() << " exceeds threshold " << m_threshold << std::endl;
	}

private:
	inline static memory_size_type actual_used() {
		return get_memory_manager().used();
	}

	memory_size_type m_startMemory;
	memory_size_type m_threshold;
};

bool sort_test(memory_size_type m2,
			   memory_size_type m3,
			   memory_size_type m4,
			   double mb_data)
{
	m2 *= 1024*1024;
	m3 *= 1024*1024;
	m4 *= 1024*1024;
	stream_size_type items = static_cast<stream_size_type>(mb_data*1024/sizeof(test_t)*1024);
	log_debug() << "sort_test with " << items << " items\n";
	boost::rand48 rng;
	relative_memory_usage m;
	merge_sorter<test_t> s;
	s.set_available_memory(m2, m3, m4);

	log_debug() << "Begin phase 2" << std::endl;
	m.set_threshold(m2);
	s.begin();
	if (!m.below()) return false;
	for (stream_size_type i = 0; i < items; ++i) {
		s.push(rng());
		if (!m.below()) return false;
	}
	s.end();
	if (!m.below()) return false;

	log_debug() << "Begin phase 3" << std::endl;
	m.set_threshold(m3);
	if (!m.below()) return false;
	s.calc();
	if (!m.below()) return false;

	log_debug() << "Begin phase 4" << std::endl;
	m.set_threshold(m4);
	if (!m.below()) return false;
	test_t prev = std::numeric_limits<test_t>::min();
	while (s.can_pull()) {
		if (!m.below()) return false;
		test_t read = s.pull();
		if (!m.below()) return false;
		if (read < prev) {
			log_error() << "Out of order" << std::endl;
			return false;
		}
		prev = read;
	}

	return true;
}

bool internal_report_test() {
	return sort_test(100,100,100,50);
}

bool one_run_external_report_test() {
	return sort_test(100,7,7,7);
}

bool external_report_test() {
	return sort_test(20,20,20,50);
}

bool small_final_fanout_test(double mb) {
	return sort_test(3,10,7,mb);
}

int main(int argc, char ** argv) {
	return tests(argc, argv)
		.test(internal_report_test, "internal_report")
		.test(one_run_external_report_test, "one_run_external_report")
		.test(external_report_test, "external_report")
		.test(small_final_fanout_test, "small_final_fanout", "mb", 8.99707)
		;
}
