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
#include <filesystem>
#include <tpie/serialization2.h>
#include <functional>
#include <map>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fcntl.h>
#include <tpie/tpie_log.h>
#include <tpie/tempname.h>
#include <tpie/util.h>
#ifdef WIN32
#include <windows.h>
#undef NO_ERROR
#include <Shlobj.h>
#else
#include <sys/types.h>
#include <pwd.h>
#endif

using namespace std;
using namespace tpie;

#ifdef TPIE_EXECUTION_TIME_PREDICTOR
namespace {

typedef std::pair<stream_size_type, time_type> p_t;

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
			stream_size_type best_dist=points[2].first - points[0].first;
			replace=begin()+1;
			for(p_t * i=begin()+1; i < end()-1; ++i) {
				stream_size_type dist=(i+1)->first - (i-1)->first;
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
	std::string dir_name;
	std::string file_name;
	
	time_estimator_database() {
#ifdef WIN32
		//dir_name 
		TCHAR p[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, p))) {
			dir_name=p;
			file_name = "\\"; //path separator
		}
#else
		const char * p = getenv("HOME");
		if (p != 0) dir_name=p;
		if (dir_name == "") dir_name = getpwuid(getuid())->pw_dir;
		file_name = "/."; //make hidden, include path separator
#endif	

		file_name += "tpie_time_estimation_db";
#ifndef TPIE_NDEBUG
		file_name += "_debug";
#endif
	}
	
	void load() {
		ifstream f;
		std::string full_name = dir_name+file_name;
		f.open(full_name.c_str(), ifstream::binary | ifstream::in);
		if (f.is_open()) {
			using tpie::unserialize;
			uint64_t magic;
			unserialize(f, magic);
			if (magic != 0x2767e5399eada7d4ull) return;
			size_t c;
			unserialize(f, c);
			for(size_t i=0; i < c; ++i) {
				size_t id;
				size_t cnt;
				unserialize(f, id);
				unserialize(f, cnt);
				entry & e=db[id];
				for (size_t j=0; j < cnt; ++j) {
					stream_size_type n;
					time_type time;
					unserialize(f, n);
					unserialize(f, time);
					e.add_point(p_t(n, time));
				}
			}
		}
	}

	~time_estimator_database() {}

	void save() {
		std::string tmp=tpie::tempname::tpie_name("",dir_name);
		ofstream f;
		f.open(tmp.c_str(), ifstream::binary | ifstream::out);
		if (!f.is_open()) {
			log_error() << "Failed to store time estimation database: Could not create temporary file ``" << tmp.c_str() << "''" << std::endl;
			return;
		}
		
		{
			using tpie::serialize;
			serialize(f, (uint64_t)0x2767e5399eada7d4ull);
			serialize(f, (size_t)db.size());
			for(db_type::iterator i=db.begin(); i != db.end(); ++i) {
				serialize(f, (size_t)i->first);
				serialize(f, (size_t)i->second.count);
				for (p_t * j=i->second.begin(); j != i->second.end(); ++j) {
					serialize(f, (stream_size_type)j->first);
					serialize(f, (time_type)j->second);
				}
			}
		}
		f.close();
		try {
			atomic_rename(tmp, dir_name+file_name);
		} catch(const std::runtime_error & e) {
			log_error() << "Failed to store time estimation database: " << e.what() << std::endl;
		}
	}
	
	time_type estimate(size_t id, stream_size_type n, double & confidence) {
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
				log_debug() << "In time estimation, first was 0." << std::endl;
				confidence=0.0;
				return -1; 
			}
			confidence = std::min(1.3 / (1.0 + std::log(double(n / l->first))/std::log(2.0)), 1.0);
			return (l->second*n)/l->first;
		}
		
		p_t p0(0,0);
		if (l != e.begin()) p0 = *(l-1);
		stream_size_type w=(l->first-p0.first);
		stream_size_type x=(n - p0.first);
		confidence=1.0;
		return p0.second * (w - x) / w + l->second * (x/w);
	}
};

static time_estimator_database * db = 0;

} //annonymous namespace
#endif //TPIE_EXECUTION_TIME_PREDICTOR
namespace tpie {

void init_execution_time_db() {
	#ifdef TPIE_EXECUTION_TIME_PREDICTOR
	if (db) return;
	db = new time_estimator_database();
	db->load();
	#endif
}

void finish_execution_time_db() {
	#ifdef TPIE_EXECUTION_TIME_PREDICTOR
	if (!db) return;
	db->save();
	delete db;
	db = 0;
	#endif
}

execution_time_predictor::execution_time_predictor(const std::string & id): 
	m_id(std::hash<std::string>()(id)),
	m_estimate(-1), m_confidence(1), m_pause_time_at_start(0)
#ifndef TPIE_NDEBUG
	,m_name(id)
#endif //TPIE_NDEBUG
 {}

execution_time_predictor::~execution_time_predictor() {
}

time_type execution_time_predictor::estimate_execution_time(stream_size_type n, double & confidence) {
#ifndef TPIE_EXECUTION_TIME_PREDICTOR
		confidence=0.0;
		return -1;
#else
	if (m_id == std::hash<std::string>()("")) {
		confidence=0.0;
		return -1;
	}
	time_type v=db->estimate(m_id, n, confidence);
#ifndef TPIE_NDEBUG
	if (v == static_cast<memory_size_type>(-1))
		log_debug() << "No database entry for " << m_name << " (" << m_id << ")" << std::endl;
#endif
	return v;
#endif
}

void execution_time_predictor::start_execution(stream_size_type n) {
    m_n = n;
    m_estimate = estimate_execution_time(n, m_confidence);
    m_start_time = std::chrono::steady_clock::now();
	m_pause_time_at_start = s_pause_time;
}

time_type execution_time_predictor::end_execution() {
	if (m_id == std::hash<std::string>()("") || !s_store_times || std::uncaught_exceptions()) return 0;
	time_type t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_start_time).count();
	t -= (s_pause_time - m_pause_time_at_start);
	#ifdef TPIE_EXECUTION_TIME_PREDICTOR
	entry & e = db->db[m_id];
	e.add_point( p_t(m_n, t) );
	#endif
	m_start_time = std::chrono::time_point<std::chrono::steady_clock>();
	return t;
}

std::string execution_time_predictor::estimate_remaining_time(double progress) {
	#ifdef TPIE_EXECUTION_TIME_PREDICTOR
	double time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_start_time).count();
	time -= static_cast<double>(s_pause_time - m_pause_time_at_start);

	double a = m_confidence * (1.0 - progress);
	double b = (1.0-m_confidence) * (1.0 - progress) + progress;

	double t2 = (progress < 0.00001)?0:time/progress;
	if (m_confidence * a + progress * b < 0.2) return "Estimating";
	double estimate = static_cast<double>(m_estimate) * a  + t2 * b;

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
	#else
	return "No estimation";
	#endif
}

void execution_time_predictor::start_pause() {
	s_start_pause_time = std::chrono::steady_clock::now();
}

void execution_time_predictor::end_pause() {
	s_pause_time += static_cast<memory_size_type>(
		std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - s_start_pause_time).count());
}

void execution_time_predictor::disable_time_storing() {
	s_store_times = false;
}

time_type execution_time_predictor::s_pause_time = 0;
std::chrono::time_point<std::chrono::steady_clock> execution_time_predictor::s_start_pause_time;
bool execution_time_predictor::s_store_times = true;

};


