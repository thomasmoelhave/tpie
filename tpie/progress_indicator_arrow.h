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

#ifndef _TPIE_PROGRESS_INDICATOR_ARROW_H
#define _TPIE_PROGRESS_INDICATOR_ARROW_H

#include <tpie/portability.h>
#include <algorithm>

#include <tpie/progress_indicator_terminal.h>

namespace tpie {

///////////////////////////////////////////////////////////////////
///
///  \class progress_indicator_arrow
///
///  A class that indicates the progress by expanding an arrow.
///
///  \author The TPIE Project
///
///////////////////////////////////////////////////////////////////

    class progress_indicator_arrow : public progress_indicator_terminal {

    public:

	////////////////////////////////////////////////////////////////////
	///
	///  Initializes the indicator.
	///
	///  \param  title        The title of the progress indicator.
	///  \param  description  A text to be printed in front of the 
	///                       indicator.
	///  \param  minRange     The lower bound of the range.
	///  \param  maxRange     The upper bound of the range.
	///  \param  stepValue    The increment for each step.
	///
	////////////////////////////////////////////////////////////////////

	progress_indicator_arrow(const std::string& title, 
							 const std::string& description, 
							 stream_offset_type minRange, 
							 stream_offset_type maxRange, 
							 stream_offset_type stepValue) : 
	    progress_indicator_terminal(title, description, minRange, maxRange, stepValue), m_indicatorLength(0), m_progress(0) {
	    m_indicatorLength = 40;
	}
    
  ////////////////////////////////////////////////////////////////////
  ///  Copy-constructor.
  ////////////////////////////////////////////////////////////////////

	progress_indicator_arrow(const progress_indicator_arrow& other) : 
	    progress_indicator_terminal(other), m_indicatorLength(40), m_progress(0) {
	    *this = other;
	}

  ////////////////////////////////////////////////////////////////////
  ///  Assignment operator.
  ////////////////////////////////////////////////////////////////////

	progress_indicator_arrow& operator=(const progress_indicator_arrow& other) {
	    if (this != &other) {

		progress_indicator_terminal::operator=(other);

		m_indicatorLength = other.m_indicatorLength;
		m_progress        = other.m_progress;
	    }
	    return *this;
	}

	////////////////////////////////////////////////////////////////////
	///
	///  The destructor. Nothing is done.
	///
	////////////////////////////////////////////////////////////////////

	virtual ~progress_indicator_arrow() {
	    // Do nothing.
	};
    
	////////////////////////////////////////////////////////////////////
	///
	///  Set the maximum length of the indicator. The length is enforced
	///  to be an integer in [2,60].
	///
	///  \param  indicatorLength  The maximum length of the indicator.
	///
	////////////////////////////////////////////////////////////////////

	void set_indicator_length(int indicatorLength) {
	    m_indicatorLength = std::max(2, std::min(60, indicatorLength));
	}

	////////////////////////////////////////////////////////////////////
	///
	///  Reset the current state of the indicator and its current length
	///
	////////////////////////////////////////////////////////////////////

	virtual void reset() {
	    m_current  = m_minRange;
	    m_progress = 0;
	}

	////////////////////////////////////////////////////////////////////
	///
	///  Display the indicator.
	///
	////////////////////////////////////////////////////////////////////

	virtual void refresh() {
	    //  Compute the relative length of the arrow.
	    stream_offset_type progress = (m_maxRange != m_minRange ) ? 
			m_indicatorLength * (m_current-m_minRange)/(m_maxRange-m_minRange) : 0; 

	    //  Make sure that the first item gets printed.
	    if (progress == 0) progress = 1;
	
	    //  Only print stuff to std::cout if the indicator needs to be updated.
	    if (progress > m_progress) {

		//  Don't print the last item.
		if (progress == m_indicatorLength) progress--;

		//  Go to the beginning of the line and print the description.
		std::cout << "\r" << m_description << " [";
	    
		//  Extend the arrow.
		for(stream_offset_type i = 0; i < progress; i++) std::cout << "=";
		std::cout << ">";

		//  Print blank space.
		for(stream_offset_type i = progress+1; i < m_indicatorLength; i++) std::cout << " ";
		std::cout << "] ";

		//  Print either a percentage sign or the maximum range.
		display_percentage();

		std::cout << std::flush;
		m_progress = progress;
	    }
	}

    protected:

	/** The maximal length of the indicator */
	stream_offset_type m_indicatorLength;

	/** The current length of the indicator */
	stream_offset_type m_progress;

    private:

  ////////////////////////////////////////////////////////////////////
  ///  Empty constructor.
  ////////////////////////////////////////////////////////////////////
  progress_indicator_arrow();
    };

}

#endif // _TPIE_PROGRESS_INDICATOR_ARROW
