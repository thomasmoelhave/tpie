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
	inline void set_title(const std::string&) {}
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

}
#endif //__TPIE_FRACTIONAL_PROGRESS__

