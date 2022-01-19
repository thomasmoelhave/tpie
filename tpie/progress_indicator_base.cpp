// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014, The TPIE development team
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

#include "progress_indicator_base.h"
#include <tpie/tpie_assert.h>

namespace tpie {

struct progress_indicator_base::refresh_impl {
	/**  The approximate frequency of calls to refresh in hz */
	static const unsigned int FREQUENCY = 5;
	std::chrono::time_point<std::chrono::steady_clock> m_firstSample;
};

progress_indicator_base::progress_indicator_base(stream_size_type range)
	: m_range(range)
	, m_current(0)
	, m_predictor(nullptr)
{
	m_refreshImpl = new progress_indicator_base::refresh_impl;
}

progress_indicator_base::progress_indicator_base(progress_indicator_base&& o)
	: m_range(o.m_range)
	, m_current(o.m_current)
	, m_predictor(o.m_predictor)
	, m_refreshImpl(o.m_refreshImpl) {
	o.m_predictor = nullptr;
	o.m_refreshImpl = nullptr;
}

progress_indicator_base & progress_indicator_base::operator=(progress_indicator_base&& o) {
	std::swap(m_range, o.m_range);
	std::swap(m_current, o.m_current);
	std::swap(m_predictor, o.m_predictor);
	std::swap(m_refreshImpl, o.m_refreshImpl);
	return *this;
}


/*virtual*/ progress_indicator_base::~progress_indicator_base() {
	delete m_refreshImpl;
	m_refreshImpl = nullptr;
};

void progress_indicator_base::call_refresh() {
	refresh_impl * const impl = this->m_refreshImpl;
	if (m_current == 0) {
		impl->m_firstSample = std::chrono::steady_clock::now();
		this->m_remainingSteps = 1;
		this->refresh();
		return;
	}

	// Time since beginning
	const double t = std::max(0.000001, std::chrono::duration<double>(std::chrono::steady_clock::now() -impl->m_firstSample).count());

	// From t0 (target time between calls to refresh)
	// and f (measured step frequency),
	// compute k' (estimated steps until next refresh) as such:
	//     k := m_current (steps since beginning)
	//    t0 := 1 / FREQUENCY  (seconds),
	//     f := k / t  (steps per second),
	//    k' := t0 * f
	//        = k / (t * m_frequency).
	// However, we limit k' to be at most 2*k.
	const stream_size_type k_new = static_cast<stream_size_type>(
		m_current / (t * refresh_impl::FREQUENCY));
	const stream_size_type a = 1;
	const stream_size_type b = 2 * m_current;
	this->m_remainingSteps = std::max(a, std::min(k_new, b));

	this->refresh();
}

} // namespace tpie
