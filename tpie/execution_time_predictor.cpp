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
#include "execution_time_predictor.h"
// #include <boost/numeric/ublas/matrix.hpp>
// #include <boost/numeric/ublas/lu.hpp>
// #include <boost/numeric/ublas/io.hpp>
#include "serialization.h"
#include <map>
#include <algorithm>
#include <tpie/prime.h>
#include <iostream>
#include <iomanip>

#ifdef WIN32
#include <windows.h>
#include <Shlobj.h>
#else
#include <sys/types.h>
#include <pwd.h>
#endif

//using namespace boost::numeric::ublas;
using namespace std;

// template <typename T>
// bool invert_matrix(const matrix<T> & input, matrix<T> & output) {
//   matrix<T> A(input);
//   permutation_matrix<size_t> p(A.size1());
//   if (lu_factorize(A, p) != 0) return false;
//   output.assign(identity_matrix<T>(A.size1()));
//   lu_substitute(A, p, output);
//   return true;
// }

typedef std::pair<TPIE_OS_OFFSET, TPIE_OS_OFFSET> p_t;

struct cmp_t {
	bool operator()(const p_t & a, const p_t & b) const {return a.first < b.first;}
};

struct entry {
	static const size_t max_points=10;
	size_t count;
	p_t points[max_points];

	entry() : count(0) {}

	inline p_t * begin() {return points;}
	inline p_t * end() {return points+count;}
	
	void add_point(p_t p) {
		p_t * l = std::lower_bound(begin(), end(), p, cmp_t());
		if (l != end() && l->first == p.first) {
			l->second = (l->second + p.second) / 2;
			return;
		}
		p_t * replace=end();
		if (count == max_points) {
			TPIE_OS_OFFSET best_dist=points[2].first - points[0].first;
			replace=begin()+1;
			for(p_t * i=begin()+1; i < end()-1; ++i) {
				TPIE_OS_OFFSET dist=(i+1)->first - (i-1)->first;
				if (dist < best_dist) {replace=i; best_dist=dist;}
			}
		} else 
			++count;
		if (l <= replace) {
			for (p_t * i=replace-1; i >= l; --i) *(i+1) = *i;
		} else {
			--l;
			for (p_t * i=replace; i < l; ++i) *i=*(i+1);
		}
		*l = p;
	}
};


class time_estimator_database {
public:
	typedef std::map<size_t, entry> db_type;
	db_type db;
	std::string path;
	
	time_estimator_database() {
#ifdef WIN32
		//path 
		TCHAR p[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, p))) {
			path=p;
			path += "\\";
		}
#else
		const char * p = getenv("HOME");
		if (p != 0) path=p;
		if (path == "") path = getpwuid(getuid())->pw_dir;
		path += "/.";
#endif	

		path += "tpie_time_estimation_db";
#ifndef NDEBUG
		path += "_debug";
#endif
		ifstream f;
		f.open(path.c_str(), ifstream::binary | ifstream::in);
		if (f.is_open()) {
			try {
				tpie::unserializer u(f);
				u << "TPIE time execution database";
				size_t c;
				u >> c;
				for(size_t i=0; i < c; ++i) {
					size_t id;
					size_t cnt;
					u >> id >> cnt;
					entry & e=db[id];
					for (size_t j=0; j < cnt; ++j) {
						TPIE_OS_OFFSET n, time;
						u >> n >> time;
						e.add_point(p_t(n, time));
					}
				}
			} catch(tpie::serialization_error &) {
				
			}
		}
	}

	~time_estimator_database() {
		ofstream f;
		f.open(path.c_str(), ifstream::binary | ifstream::out);
		if (f.is_open()) {
			tpie::serializer s(f);
			s << "TPIE time execution database";
			s << (size_t)db.size();
			for(db_type::iterator i=db.begin(); i != db.end(); ++i) {
				s << (size_t)i->first << (size_t)i->second.count;
				for (p_t * j=i->second.begin(); j != i->second.end(); ++j)
					s << (TPIE_OS_OFFSET)j->first << (TPIE_OS_OFFSET)j->second;
			}
		} else {
			std::cerr << "Failed to store DB" << std::endl;
		}
	}
	
	TPIE_OS_OFFSET estimate(size_t & id, TPIE_OS_OFFSET n, double & confidence) {
		db_type::iterator i=db.find(id);
		if (i == db.end()) {
			confidence=0.0;
			return -1;
		}
		entry & e=i->second;
		
		p_t * l = std::lower_bound(e.begin(), e.end(), p_t(n, 0), cmp_t());


		if (l != e.end() && l->first == n) {
			confidence=1.0;
			return l->second;
		}

		if (l == e.end()) {
			--l;
			if (l->first == 0) {
#ifdef NDEBUG
				std::cerr << "First is 0 ";
#endif
				confidence=0.0;
				return -1; 
			}
			confidence = std::min(1.3 / (1.0 + std::log(double(n / l->first))/std::log(2.0)), 1.0);
			return (l->second*n)/l->first;
		}
		
		p_t p0(0,0);
		if (l != e.begin()) p0 = *(l-1);
		TPIE_OS_OFFSET w=(l->first-p0.first);
		TPIE_OS_OFFSET x=(n - p0.first);
		confidence=1.0;
		return p0.second * (w - x) / w + l->second * (x/w);
	}
};
  
static time_estimator_database d;

namespace tpie {

execution_time_predictor::execution_time_predictor(const std::string & id): 
	m_id(is_prime.prime_hash(id)), m_start_time(boost::posix_time::not_a_date_time), 
	m_estimate(-1), m_pause_time_at_start(0)
#ifndef NDEBUG
	,m_name(id)
#endif //NDEBUG
 {}

execution_time_predictor::~execution_time_predictor() {
}

TPIE_OS_OFFSET execution_time_predictor::estimate_execution_time(TPIE_OS_OFFSET n, double & confidence) {
	if (m_id == is_prime.prime_hash(std::string())) {
		confidence=0.0;
		return -1;
	}
	if (d.estimate(m_id, n, confidence) == -1)
#ifndef NDEBUG
		std::cerr << "Do database entry for " << m_name << " (" << m_id << ")" << std::endl;
#else
	;
#endif
	return d.estimate(m_id, n, confidence);
}

void execution_time_predictor::start_execution(TPIE_OS_OFFSET n) {
    m_n = n;
    m_estimate = estimate_execution_time(n, m_confidence);
    m_start_time = boost::posix_time::microsec_clock::local_time();
	m_pause_time_at_start = s_pause_time;
}

TPIE_OS_OFFSET execution_time_predictor::end_execution() {
	if (m_id == is_prime.prime_hash(std::string()) || !s_store_times) return 0;
	TPIE_OS_OFFSET t = (boost::posix_time::microsec_clock::local_time() - m_start_time).total_milliseconds();
	t -= (s_pause_time - m_pause_time_at_start);
	entry & e = d.db[m_id];
	e.add_point( p_t(m_n, t) );
	m_start_time = boost::posix_time::not_a_date_time;
	return t;
}

std::string execution_time_predictor::estimate_remaining_time(double progress) {
    double time = static_cast<double>((boost::posix_time::microsec_clock::local_time()-m_start_time).total_milliseconds());
	time -= (s_pause_time - m_pause_time_at_start);

	double a = m_confidence * (1.0 - progress);
	double b = (1.0-m_confidence) * (1.0 - progress) + progress;

	double t2 = (progress < 0.00001)?0:time/progress;
	if (m_confidence * a + progress * b < 0.2) return "Estimating";
	double estimate = m_estimate * a  + t2 * b;
	
	double remaining = estimate * (1.0-progress);

    stringstream s;
	remaining /= 1000;
    if (remaining < 60*10) {
		s << (int)remaining << " sec";
		return s.str();
    }
    remaining /= 60;
    if (remaining < 60*10) {
		s << (int)remaining << " min";
		return s.str();
    }
    remaining /= 60;
    if (remaining < 24*10) {
		s << (int)remaining << " hrs";
		return s.str();
    }
    remaining /= 24;
    s << (int)remaining << " days";
    return s.str();
}

void execution_time_predictor::start_pause() {
	s_start_pause_time = boost::posix_time::microsec_clock::local_time();
}

void execution_time_predictor::end_pause() {
	s_pause_time += (boost::posix_time::microsec_clock::local_time() - s_start_pause_time).total_milliseconds();
}

void execution_time_predictor::disable_time_storing() {
	s_store_times = false;
}

TPIE_OS_OFFSET execution_time_predictor::s_pause_time = 0;
boost::posix_time::ptime execution_time_predictor::s_start_pause_time;
bool execution_time_predictor::s_store_times = true;

};


