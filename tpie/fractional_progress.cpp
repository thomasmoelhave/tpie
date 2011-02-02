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
#include <tpie/backtrace.h>

namespace tpie {


fractional_subindicator::fractional_subindicator(
	fractional_progress & fp,
	const char * id,
	double fraction,
	TPIE_OS_OFFSET n,
	const char * crumb,
	bool display_subcrumbs,
	bool enabled):
	progress_indicator_subindicator(fp.m_pi, 42, crumb, display_subcrumbs),
#ifndef NDEBUG
	m_init_called(false), m_done_called(false), 
#endif
	m_fraction(enabled?fraction:0.0), m_estimate(-1), m_n(enabled?n:0), m_fp(fp), m_predict(fp.m_id() + ";" + id)
#ifdef TPIE_FRACTION_STATS
	,m_id(id)
#endif
{
	if (enabled)
		m_estimate = m_predict.estimate_execution_time(n, m_confidence);
	else {
		m_estimate = 0;
		m_confidence = 1;
	}
	fp.add_sub_indicator(*this);
};

void fractional_subindicator::init(TPIE_OS_OFFSET range, TPIE_OS_OFFSET step) {
	softassert(m_n != 0);
	softassert(m_fp.m_init_called);
	m_predict.start_execution(m_n);
	if (m_parent) {
		double f = m_fp.get_fraction(*this);
		double t = m_parent->get_max_range() - m_parent->get_min_range();
		m_outerRange = t * f;
	}
#ifndef NDEBUG
	m_init_called=true;
#endif

	progress_indicator_subindicator::init(range, step);	
}

void fractional_subindicator::done() {
	m_predict.end_execution();
	progress_indicator_subindicator::done();
}

fractional_subindicator::~fractional_subindicator() {
#ifndef NDEBUG
	if (!m_init_called && m_fraction > 0.00001) {
		std::cerr << "A fractional_subindicator was assigned a non-zero fraction but never initialized" 
#ifdef TPIE_FRACTION_STATS
				  << " id: \"" << m_id << "\""
#endif
				  << std::endl;
		tpie::backtrace(std::cerr, 5);
	}
#endif
}

fractional_progress::fractional_progress(progress_indicator_base * pi):
#ifndef NDEBUG
	m_init_called(false),
#endif
	m_pi(pi), m_add_state(true), 
#ifndef NDEBUG
	m_done_called(false),
#endif
	m_confidence(1.0), m_total_sum(0), m_time_sum(0), m_timed_sum(0) {}
	
void fractional_progress::init() {
#ifndef NDEBUG
	if (m_init_called) {
		std::cerr << "Init was called on a fractional_progress where init had already been called" << std::endl;
		tpie::backtrace(std::cerr, 5);
	}
	m_init_called=true;
#endif
	if (m_pi) m_pi->init(23000);
}

void fractional_progress::done() {
#ifndef NDEBUG
	if (m_done_called || !m_init_called) {
		std::cerr << "Done was called on a fractional_progress where done had allready been called" << std::endl;
		tpie::backtrace(std::cerr, 5);
	}
	m_done_called=true;
#endif
	if (m_pi) m_pi->done();
}

fractional_progress::~fractional_progress() {
#ifndef NDEBUG
	if (m_init_called && !m_done_called && !std::uncaught_exception()) {
		std::cerr << "A fractional_progress was destructed without done being called" << std::endl;
		tpie::backtrace(std::cerr, 5);
	}
#endif
}

unique_id_type & fractional_progress::id() {return m_id;}

void fractional_progress::add_sub_indicator(fractional_subindicator & sub) {
	softassert(m_add_state==true);
	if (sub.m_fraction < 0.000000001 && sub.m_confidence > 0.5) return;
	m_total_sum += sub.m_fraction;
	m_confidence = std::min(sub.m_confidence, m_confidence);
	m_time_sum += sub.m_estimate;
}

double fractional_progress::get_fraction(fractional_subindicator & sub) {
	m_add_state=false;

	if (sub.m_fraction < 0.000000001 && sub.m_confidence > 0.5) return 0.0;
	
	double f1 = (m_total_sum > 0.00001)?sub.m_fraction / m_total_sum: 0.0;
	double f2 = (m_time_sum > 0.00001)?((double)sub.m_estimate / (double)m_time_sum):0.0;
	
	double f = f1 * (1.0 - m_confidence) + f2*m_confidence;
	
#ifdef TPIE_FRACTION_STATS
	std::cout << "Fraction: name: " << m_id() << ";" << sub.m_id << "; "
			  << "Confidence: " << m_confidence << "; "
			  << "Supplied: " << f1 << "; "
			  << "Estimated: " << f2 << "; "
			  << "Chosen: " << f << "; " << std::endl;
#endif
	return f;
}
} //namespace tpie
