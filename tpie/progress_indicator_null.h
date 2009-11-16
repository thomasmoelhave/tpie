// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
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

#ifndef _TPIE_PROGRESS_INDICATOR_NULL_H
#define _TPIE_PROGRESS_INDICATOR_NULL_H

#include <tpie/portability.h>
#include <tpie/progress_indicator_base.h>

namespace tpie {

///////////////////////////////////////////////
///
/// \class progress_indicator_null 
/// \brief a dummy progress indicator that produces no output
///
///////////////////////////////////////////////
class progress_indicator_null : public progress_indicator_base {

public:
    progress_indicator_null (const std::string& title, 
			     const std::string& description, 
			     TPIE_OS_OFFSET minRange, 
			     TPIE_OS_OFFSET maxRange, 
			     TPIE_OS_OFFSET stepValue) :  
	progress_indicator_base(title, description, minRange, maxRange, stepValue) {
    }

    virtual ~progress_indicator_null() { /*Do nothing*/ }
    
    ////////////////////////////////////////////////////////////////////
    ///
    ///  Reset the counter. The current position is reset to the
    ///  lower bound of the counting range.
    ///
    ////////////////////////////////////////////////////////////////////

    virtual void reset() { }

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Advance the indicator to the end and print an (optional)
    ///  message that is followed by a newline.
    ///
    ///  \param  text  The message to be printed at the end of the
    ///                indicator.
    ///
    ////////////////////////////////////////////////////////////////////

    virtual void done(const std::string& text = std::string()) { }

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Set the lower bound of the counting range. This method
    ///  also implies a reset of the counter. In order to be able
    ///  to set the lower bound independent of setting the upper bound,
    ///  no range checking is done.
    ///
    ///  \param  minRange  The new lower bound.
    ///
    ////////////////////////////////////////////////////////////////////

    virtual void set_min_range(TPIE_OS_OFFSET /*minRange*/) { }

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Set the upper bound of the counting range. This method
    ///  also implies a reset of the counter. In order to be able
    ///  to set the uper bound independent of setting the lower bound,
    ///  no range checking is done.
    ///
    ///  \param  maxRange  The new upper bound.
    ///
    ////////////////////////////////////////////////////////////////////

    virtual void set_max_range(TPIE_OS_OFFSET /*maxRange*/) { }

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Set the increment by which the counter is advanced upon each
    ///  call to step(). In order to be able to reset the counter,
    ///  no range checking is done.
    ///
    ///  \param  stepValue  The incerement.
    ///
    ////////////////////////////////////////////////////////////////////

    virtual void set_step_value(TPIE_OS_OFFSET /*stepValue*/) { }
  
    ////////////////////////////////////////////////////////////////////
    ///
    ///  Set the title of a new task to be monitored. The terminal
    ///  line will be newline'd, and the title will be followed by a
    ///  newline as well.
    ///
    ///  \param  title  The title of the new task to be monitored.
    ///
    ////////////////////////////////////////////////////////////////////

    virtual void set_title(const std::string& /*title*/)  { }

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Set the description of the task currently being monitored.
    ///  Invoking this method will clear the terminal line.
    ///
    ///  \param  description  The decription of the task being monitored.
    ///
    ////////////////////////////////////////////////////////////////////

    virtual void set_description(const std::string& /*description*/) { };

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Display the indicator.
    ///
    ////////////////////////////////////////////////////////////////////

    virtual void refresh() { };
};

} //tpie namespace

#endif //_TPIE_PROGRESS_INDICATOR_NULL_H
