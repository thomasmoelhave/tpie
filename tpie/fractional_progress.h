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
#ifndef __TPIE_FRACTIONAL_PROGRESS__
#define __TPIE_FRACTIONAL_PROGRESS__

#include <tpie/portability.h>
#include <tpie/util.h>
#include <tpie/progress_indicator_subindicator.h>
namespace tpie {

class fractional_progress;

class fractional_subindicator: public progress_indicator_subindicator {
public:
	void set_title(const std::string&) {}
	virtual void init(const std::string& text = std::string());
	virtual void done(const std::string& text = std::string());
	fractional_subindicator(fractional_progress & fp, 
							const char * id, 
							double fraction, 
							TPIE_OS_OFFSET n,
							TPIE_OS_OFFSET minRange=0,
							TPIE_OS_OFFSET maxRange=10000,
							TPIE_OS_OFFSET stepValue=1);
private:
	double m_fraction;
	TPIE_OS_OFFSET m_estimate;
	TPIE_OS_OFFSET m_n;
	fractional_progress & m_fp;
	execution_time_predictor m_predict;
	friend class fractional_progress;
};

class fractional_progress {
public:
	fractional_progress(progress_indicator_base * pi, const std::string & description);
	~fractional_progress();
	void done();
	unique_id_type & id();
private:
	double get_fraction(fractional_subindicator & sub);
	void add_sub_indicator(fractional_subindicator & sub);
	progress_indicator_base * m_pi;
	bool m_add_state;
	bool m_done_called;
	unique_id_type m_id;
	double m_total_sum;
	TPIE_OS_OFFSET m_time_sum;
	double m_timed_sum;
	friend class fractional_subindicator;
};

fractional_subindicator::fractional_subindicator(
	fractional_progress & fp, const char * id, double fraction, TPIE_OS_OFFSET n,
	TPIE_OS_OFFSET minRange, TPIE_OS_OFFSET maxRange, TPIE_OS_OFFSET stepValue):
	progress_indicator_subindicator(fp.m_pi, 42, minRange, maxRange, stepValue),
	m_fraction(fraction), m_estimate(-1), m_n(n), m_fp(fp), m_predict(fp.m_id() + ";" + id) {
	m_estimate = m_predict.estimate_execution_time(n);
	fp.add_sub_indicator(*this);
}


void fractional_subindicator::init(const std::string& text) {
	if (m_parent) {
		double f = m_fp.get_fraction(*this);
		double t = m_parent->get_max_range() - m_parent->get_min_range();
		m_range = t * f;
	}
	m_predict.start_execution(m_n);
	progress_indicator_subindicator::init(text);
}

void fractional_subindicator::done(const std::string&) {
	m_predict.end_execution();
	progress_indicator_subindicator::done();
}

fractional_progress::fractional_progress(progress_indicator_base * pi, const std::string & description):
	m_pi(pi), m_add_state(true), m_done_called(false),
	m_total_sum(0), m_time_sum(0), m_timed_sum(0) {	
	if (m_pi) {
		m_pi->set_range(0, 23000, 1);
		m_pi->init(description);
	}
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

};

#endif //__TPIE_FRACTIONAL_PROGRESS__

