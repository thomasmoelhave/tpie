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
#include <tpie/prime.h>
#include <map>
#include <fstream>
#include <sstream>
#include <locale>
namespace tpie {

class fraction_db {
public:
	std::map<uint32_t, float> db;
#ifdef TPIE_FRACTION_STATS
	std::map<uint32_t, TPIE_OS_OFFSET> n;
	bool dirty;
#endif //TPIE_FRACTION_STATS
	fraction_db() {
#ifdef TPIE_FRACTIONDB_DIR_INL

#ifdef TPIE_FRACTION_STATS
		std::locale::global(std::locale::classic());
		dirty=false;
		std::fstream f;
#ifndef NDEBUG
		f.open(TPIE_FRACTIONDB_DIR_INL "/tpie_fraction_db_debug.inl", std::fstream::in);
#else
		f.open(TPIE_FRACTIONDB_DIR_INL "/tpie_fraction_db.inl", std::fstream::in);
#endif
		if (f.is_open()) {
			std::string skip;
			uint32_t hash;
			float frac;
			TPIE_OS_OFFSET n_;
			while (f >> skip >> hash >> skip >> frac >> skip >> n_) {
				db[hash] = frac;
				n[hash] = n_;
			}
			return;
		}
#endif
#ifdef NDEBUG
#include <tpie_fraction_db_debug.inl>
#else
#include <tpie_fraction_db.inl>
#endif
#endif //TPIE_FRACTIONDB_DIR_INL
	}

#ifdef TPIE_FRACTION_STATS
#ifdef TPIE_FRACTIONDB_DIR_INL
	~fraction_db() {
		if (!dirty) return;
		std::locale::global(std::locale::classic());
		std::fstream f;
#ifndef NDEBUG
		f.open(TPIE_FRACTIONDB_DIR_INL "/tpie_fraction_db_debug.inl", std::fstream::out | std::fstream::trunc);
#else
		f.open(TPIE_FRACTIONDB_DIR_INL "/tpie_fraction_db.inl", std::fstream::out | std::fstream::trunc);
#endif
		if (!f.is_open()) return;

		for (std::map<uint32_t, float>::iterator i=db.begin(); i != db.end(); ++i)
			f << "db[ " << i->first << " ]= " << i->second << " ;// " << n[i->first] << '\n';
	}
#endif //TPIE_FRACTIONDB_DIR_INL
#endif //TPIE_FRACTION_STATS
};

static fraction_db * fdb = 0;

void init_fraction_db() {
	if (fdb) return;
	fdb = new fraction_db();
}

void finish_fraction_db() {
	delete fdb;
	fdb=NULL;
}



inline std::string fname(const char * file, const char * function, const char * name) {
	std::string x;
	x += file;
	x += ":";
	x += function;
	x += ":";
	x += name;
	return x;
}

inline uint32_t fhash(const std::string & name) {return is_prime.prime_hash(name);}

inline double getFraction(const std::string & name) {
	std::map<uint32_t, float>::iterator i=fdb->db.find(fhash(name));
	if (i == fdb->db.end()) {
		log_info() <<
			"A fraction was missing in the fraction database\n"
			<< "    " << name << "\n"
			<< "    To fix this run this command on a large dataset with fraction statics enabled."<< std::endl;
		return 1.0;
	}
	return i->second;
}

fractional_subindicator::fractional_subindicator(
	fractional_progress & fp,
	const char * id,
	const char * file,
	const char * function,
	TPIE_OS_OFFSET n,
	const char * crumb,
	bool display_subcrumbs,
	bool enabled):
	progress_indicator_subindicator(fp.m_pi, 42, crumb, display_subcrumbs),
#ifndef NDEBUG
	m_init_called(false), m_done_called(false), 
#endif
	m_fraction(enabled?getFraction(fname(file, function, id)):0.0), m_estimate(-1), m_n(enabled?n:0), m_fp(fp), m_predict(fp.m_id() + ";" + id)
#ifdef TPIE_FRACTION_STATS
	,m_stat_hash(fhash(fname(file, function, id)))
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
#ifdef TPIE_FRACTION_STATS
	TPIE_OS_OFFSET r = m_predict.end_execution();
	if(m_n > 0) 
		m_fp.stat(m_stat_hash, r, m_n);
#else
	m_predict.end_execution();
#endif
	progress_indicator_subindicator::done();
}

fractional_subindicator::~fractional_subindicator() {
#ifndef NDEBUG
	if (!m_init_called && m_fraction > 0.00001) {
		std::stringstream s;
		s << "A fractional_subindicator was assigned a non-zero fraction but never initialized" << std::endl;
		tpie::backtrace(s, 5);
		TP_LOG_FATAL(s.str());
		TP_LOG_FLUSH_LOG;
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
		std::stringstream s;
		s << "Init was called on a fractional_progress where init had already been called" << std::endl;
		tpie::backtrace(s, 5);
		TP_LOG_FATAL(s.str());
		TP_LOG_FLUSH_LOG;
	}
	m_init_called=true;
#endif
	if (m_pi) m_pi->init(23000);
}

void fractional_progress::done() {
#ifndef NDEBUG
	if (m_done_called || !m_init_called) {
		std::stringstream s;
		s << "Done was called on a fractional_progress where done had allready been called" << std::endl;
		tpie::backtrace(s, 5);
		TP_LOG_FATAL(s.str());
		TP_LOG_FLUSH_LOG;
	}
	m_done_called=true;
#endif
	if (m_pi) m_pi->done();
}

fractional_progress::~fractional_progress() {
#ifndef NDEBUG
	if (m_init_called && !m_done_called && !std::uncaught_exception()) {
		std::stringstream s;
		s << "A fractional_progress was destructed without done being called" << std::endl;
		tpie::backtrace(s, 5);
		TP_LOG_FATAL(s.str());
		TP_LOG_FLUSH_LOG;
	}
#endif
#ifdef TPIE_FRACTION_STATS
	TPIE_OS_OFFSET time_sum=0;
	for (size_t i=0; i < m_stat.size(); ++i)
		time_sum += m_stat[i].second.first;

	if (time_sum > 0) {
		for (size_t i=0; i < m_stat.size(); ++i) {
			const std::pair<uint32_t, std::pair<TPIE_OS_OFFSET, TPIE_OS_OFFSET> > & x=m_stat[i];
			float f= (float)x.second.first / (float)time_sum;
			if (fdb->n.count(x.first) && fdb->n[x.first] > x.second.second) continue;
			fdb->n[x.first] = x.second.second;
			fdb->db[x.first] = f;
			fdb->dirty = true;
		}
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
	return f;
}

#ifdef TPIE_FRACTION_STATS
void fractional_progress::stat(uint32_t hash, TPIE_OS_OFFSET time, TPIE_OS_OFFSET n) {
	if (time < 0 || n <= 0) return;
	m_stat.push_back(std::make_pair(hash, std::make_pair(time, n)));
}
#endif

} //namespace tpie
