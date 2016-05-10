// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
//
// Copyright 2011, 2014, The TPIE development team
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

#include "memory.h"
#include <iostream>
#include <sstream>
#include "tpie_log.h"
#include <cstring>
#include <cstdlib>
#include "pretty_print.h"

namespace tpie {

resource_manager::resource_manager()
	: m_used(0), m_limit(0), m_maxExceeded(0), m_nextWarning(0), m_enforce(ENFORCE_WARN) {}

inline void segfault() {
	std::abort();
}

size_t resource_manager::used() const throw() {
	return m_used.load();
}

size_t resource_manager::available() const throw() {
	size_t used = m_used.load();
	size_t limit = m_limit;
	if (used < limit) return limit-used;
	return 0;
}

} // namespace tpie

void tpie_print_resource_complaint(std::ostream & os, const std::string & name, size_t amount, size_t usage, size_t limit) {
	os << "Resource " << name << " limit exceeded by " << tpie::bits::pretty_print::size_type(usage - limit)
	   << " (" << (usage-limit) * 100 / limit << "%), while trying to increase usage by " << tpie::bits::pretty_print::size_type(amount) << "."
	   << " Limit is " << tpie::bits::pretty_print::size_type(limit) << ", but " << tpie::bits::pretty_print::size_type(usage) << " would be used.";
}

namespace tpie {

void resource_manager::register_increased_usage(size_t amount) {
	switch(m_enforce) {
	case ENFORCE_IGNORE:
		m_used.fetch_add(amount);
		break;
	case ENFORCE_THROW: {
		size_t usage = m_used.fetch_add(amount);
		if (usage > m_limit && m_limit > 0) {
			std::stringstream ss;
			tpie_print_resource_complaint(ss, name, amount, usage, m_limit);
			throw out_of_resource_error(ss.str().c_str());
		}
		break; }
	case ENFORCE_DEBUG:
	case ENFORCE_WARN: {
		size_t usage = m_used.fetch_add(amount);
		if (usage > m_limit && usage - m_limit > m_maxExceeded && m_limit > 0) {
			m_maxExceeded = usage - m_limit;
			if (m_maxExceeded >= m_nextWarning) {
				m_nextWarning = m_maxExceeded + m_maxExceeded/8;
				std::ostream & os = (m_enforce == ENFORCE_DEBUG) ? log_debug() : log_warning();
				tpie_print_resource_complaint(os, name, amount, usage, m_limit);
				os << std::endl;
			}
		}
		break; }
	};
}

void resource_manager::register_decreased_usage(size_t amount) {
#ifndef TPIE_NDEBUG
	size_t usage = m_used.fetch_sub(amount);
	if (amount > usage) {
		log_error() << "Error in deallocation, trying to deallocate " << amount << " bytes, while only " <<
			usage << " were allocated" << std::endl;
		segfault();
	}
#else
	m_used.fetch_sub(amount);
#endif
}

void resource_manager::set_limit(size_t new_limit) {
	m_limit = new_limit;
}

void resource_manager::set_enforcement(enforce_t e) {
	m_enforce = e;
}

void resource_manager::set_name(const std::string & name) {
	this->name = name;
}

} //namespace tpie
