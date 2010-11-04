// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
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
#include "fractional_progress.h"

namespace tpie {


fractional_subindicator::fractional_subindicator(
	fractional_progress & fp,
	const char * id,
	double fraction,
	TPIE_OS_OFFSET n,
	const char * crumb,
	bool display_subcrumbs):
	progress_indicator_subindicator(fp.m_pi, 42, crumb, display_subcrumbs),
	m_fraction(fraction), m_estimate(-1), m_n(n), m_fp(fp), m_predict(fp.m_id() + ";" + id) {
	m_estimate = m_predict.estimate_execution_time(n);
	fp.add_sub_indicator(*this);
};

void fractional_subindicator::init(TPIE_OS_OFFSET range, TPIE_OS_OFFSET step) {
	m_predict.start_execution(m_n);
	if (m_parent) {
		double f = m_fp.get_fraction(*this);
		double t = m_parent->get_max_range() - m_parent->get_min_range();
		m_outerRange = t * f;
	}
	progress_indicator_subindicator::init(range, step);	
}

void fractional_subindicator::done() {
	m_predict.end_execution();
	progress_indicator_subindicator::done();
}

fractional_progress::fractional_progress(progress_indicator_base * pi):
	m_pi(pi), m_add_state(true), m_done_called(false),
	m_total_sum(0), m_time_sum(0), m_timed_sum(0) {	
}
	
void fractional_progress::init() {
	if (m_pi) m_pi->init(23000);
}

void fractional_progress::done() {
	if (!m_done_called && m_pi) m_pi->done();
	m_done_called=true;
}

fractional_progress::~fractional_progress() {done();}

unique_id_type & fractional_progress::id() {return m_id;}

void fractional_progress::add_sub_indicator(fractional_subindicator & sub) {
	assert(m_add_state==true);
	m_total_sum += sub.m_fraction;
	if (sub.m_estimate != -1) {
		m_timed_sum += sub.m_fraction;
		m_time_sum += sub.m_estimate;
	}
}

double fractional_progress::get_fraction(fractional_subindicator & sub) {
	m_add_state=false;
	if (sub.m_estimate == -1) return sub.m_fraction / m_total_sum;
	else return (double)sub.m_estimate / (double)m_time_sum * m_timed_sum / m_total_sum;
}

} //namespace tpie
