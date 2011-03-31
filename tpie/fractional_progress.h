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

#define TPIE_FSI __FILE__,__FUNCTION__

namespace tpie {

void init_fraction_db();
void finish_fraction_db();

class fractional_progress;

class fractional_subindicator: public progress_indicator_subindicator {
public:
	fractional_subindicator(fractional_progress & fp,
							const char * id,
							const char * file,
							const char * function,
							TPIE_OS_OFFSET n,
							const char * crumb=0,
							bool display_subcrumbs=true,
							bool enabled=true);

	~fractional_subindicator();
	virtual void init(TPIE_OS_OFFSET range);
	virtual void done();
private:
#ifndef NDEBUG
	bool m_init_called;
	bool m_done_called;
#endif
	double m_fraction;
	TPIE_OS_OFFSET m_estimate;
	double m_confidence;
	TPIE_OS_OFFSET m_n;
	fractional_progress & m_fp;
	execution_time_predictor m_predict;

#ifdef TPIE_FRACTION_STATS
	std::string m_stat;
#endif
	friend class fractional_progress;
};

class fractional_progress {
public:
	fractional_progress(progress_indicator_base * pi);
	~fractional_progress();
	void done();
	void init();
	unique_id_type & id();

#ifndef NDEBUG
	bool m_init_called;
#endif

private:
	double get_fraction(fractional_subindicator & sub);

	void add_sub_indicator(fractional_subindicator & sub);
	progress_indicator_base * m_pi;
	bool m_add_state;
#ifndef NDEBUG
	bool m_done_called;
#endif
	double m_confidence;
	
	unique_id_type m_id;
	double m_total_sum;
	TPIE_OS_OFFSET m_time_sum;
	double m_timed_sum;

#ifdef TPIE_FRACTION_STATS
	void stat(std::string, TPIE_OS_OFFSET, TPIE_OS_OFFSET);
	std::vector< std::pair<std::string, std::pair<TPIE_OS_OFFSET, TPIE_OS_OFFSET> > > m_stat;
#endif
	friend class fractional_subindicator;
};

}
#endif //__TPIE_FRACTIONAL_PROGRESS__

