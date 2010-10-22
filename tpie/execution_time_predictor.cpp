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
#include <map>
#include <algorithm>
#include <tpie/prime.h>
#include <iostream>
#include <iomanip>
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
	static const int max_points=10;
	size_t count;
	p_t points[max_points];

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
	time_estimator_database() {
		ifstream f;
		f.open("/var/tmp/tpie_time_estimation_database");
		if (f.is_open()) {
			size_t id;
			size_t cnt;
			while (f >> std::hex >> id >> std::dec >> cnt) {
				entry & e=db[id];
				for (size_t i=0; i < cnt; ++i) {
					TPIE_OS_OFFSET n, time;
					f >> n >> time;
					e.add_point(p_t(n, time));
				}
			}
		}
	}

	~time_estimator_database() {
		ofstream f;
		f.open("/var/tmp/tpie_time_estimation_database");
		if (f.is_open()) {
			for(db_type::iterator i=db.begin(); i != db.end(); ++i) {
				f << std::hex << i->first << " " << std::dec << i->second.count << std::endl;
				for (p_t * j=i->second.begin(); j != i->second.end(); ++j)
					f << "  " << j->first << " " << j->second << std::endl;
			}
			f.close();
		} else {
			std::cerr << "Failed to store DB" << std::endl;
		}
	}
	
	TPIE_OS_OFFSET estimate(size_t & id, TPIE_OS_OFFSET n) {
		db_type::iterator i=db.find(id);
		if (i == db.end()) return -1;
		entry & e=i->second;
		p_t * l = std::lower_bound(e.begin(), e.end(), p_t(n, 0), cmp_t());
		if (l == e.end()) --l;
		if (l->first == 0) return -1; 
		return l->second * n / l->first;
	}

};
  
static time_estimator_database d;

namespace tpie {

execution_time_predictor::execution_time_predictor(const std::string & id): m_id(is_prime.prime_hash(id)), m_start_time(boost::posix_time::not_a_date_time) {}

execution_time_predictor::~execution_time_predictor() {
	assert(m_start_time.is_not_a_date_time());
}

TPIE_OS_OFFSET execution_time_predictor::estimate_execution_time(TPIE_OS_OFFSET n) {
	TPIE_OS_OFFSET tmp = d.estimate(m_id, n);
	//std::cout << std::endl <<  "Estimate " << m_id << " " << tmp << std::endl;
	return tmp;
}

void execution_time_predictor::start_execution(TPIE_OS_OFFSET n) {
    m_n = n;
    m_estimate = estimate_execution_time(n);
    m_start_time = boost::posix_time::microsec_clock::local_time();
}

	
void execution_time_predictor::end_execution() {
	entry & e = d.db[m_id];
	e.add_point( p_t(m_n, (boost::posix_time::microsec_clock::local_time() - m_start_time).total_milliseconds()) );
	m_start_time = boost::posix_time::not_a_date_time;
}

std::string execution_time_predictor::estimate_remaining_time(double progress) {
    double time = (boost::posix_time::microsec_clock::local_time()-m_start_time).total_milliseconds();
    if (time < 10 && m_estimate == -1) return "estimating remaining time";
    double estimate = time / progress;
	//std::cout << m_estimate << " " << estimate << std::endl;
	if (m_estimate != -1)
		estimate = m_estimate * (1.0-progress) + estimate * progress;
    double remaining = estimate * (1.0-progress);

    stringstream s;
	remaining /= 1000;
    if (remaining < 60*10) {
		s << (int)remaining << "sec";
		return s.str();
    }
    remaining /= 60;
    if (remaining < 60*10) {
		s << (int)remaining << "min";
		return s.str();
    }
    remaining /= 60;
    if (remaining < 24*10) {
		s << (int)remaining << "hrs";
		return s.str();
    }
    remaining /= 24;
    s << (int)remaining << "days";
    return s.str();
}

};


