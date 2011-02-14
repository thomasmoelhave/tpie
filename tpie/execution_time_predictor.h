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
#include <tpie/portability.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include <tpie/util.h>
#ifndef __TPIE_EXECUTION_TIME_PREDICTOR_H__
#define __TPIE_EXECUTION_TIME_PREDICTOR_H__

namespace tpie {

class unique_id_type {
public:
    inline unique_id_type & operator << (const std::type_info & type) {
		ss << type.name() << ";"; return *this;
    }
    
    template <typename T>
    inline unique_id_type & operator<<(const T & x) {
		ss << typeid(T).name() << ":" << x << ";"; return *this;
    }
	
	inline std::string operator()() {return ss.str();}
private:
	std::stringstream ss;
};


class execution_time_predictor {
public:
	execution_time_predictor(const std::string & id=std::string());
	~execution_time_predictor();
	TPIE_OS_OFFSET estimate_execution_time(TPIE_OS_OFFSET n, double & confidence);
	void start_execution(TPIE_OS_OFFSET n);
	TPIE_OS_OFFSET end_execution();
	std::string estimate_remaining_time(double progress);

	static void start_pause();
	static void end_pause();
	static void disable_time_storing();

	//Used by fractional_time_perdictor
	//TPIE_OS_OFFSET m_aux1;
	//double m_aux2;
private:
	size_t m_id;
	boost::posix_time::ptime m_start_time;
	TPIE_OS_OFFSET m_estimate;
	double m_confidence;
	TPIE_OS_OFFSET m_n;
	TPIE_OS_OFFSET m_pause_time_at_start;

#ifndef NDEBUG
	std::string m_name;
#endif

	static TPIE_OS_OFFSET s_pause_time;
	static boost::posix_time::ptime s_start_pause_time;
	static bool s_store_times;
};

} //namespace tpie

#endif //__TPIE_EXECUTION_TIME_PREDICTOR_H__

