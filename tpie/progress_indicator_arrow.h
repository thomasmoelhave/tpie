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

///////////////////////////////////////////////////////////////////////////////
/// \file progress_indicator_arrow.h  Indicate progress by expanding an arrow
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_PROGRESS_INDICATOR_ARROW_H
#define _TPIE_PROGRESS_INDICATOR_ARROW_H

#include <algorithm>
#include <iostream>
#include <tpie/progress_indicator_terminal.h>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \class progress_indicator_arrow
/// A class that indicates the progress by expanding an arrow.
///////////////////////////////////////////////////////////////////////////////
    class progress_indicator_arrow : public progress_indicator_terminal {
	private:
		progress_indicator_arrow(const progress_indicator_arrow& other);
    public:

	///////////////////////////////////////////////////////////////////////////
	/// Initializes the indicator.
	///
	/// \param  title  The title of the progress indicator.
	/// \param  range  The number of times we call step
	///////////////////////////////////////////////////////////////////////////
	progress_indicator_arrow(const char * title, stream_size_type range, std::ostream & os = std::cout) :
	    progress_indicator_terminal(title, range) , m_indicatorLength(0), m_progress(0), m_os(os) {
	    m_indicatorLength = 110;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Set the maximum length of the indicator. The length is enforced
	/// to be an integer in [2,60].
	///
	/// \param  indicatorLength  The maximum length of the indicator.
	///////////////////////////////////////////////////////////////////////////
	void set_indicator_length(int indicatorLength) {
	    m_indicatorLength = std::max(2, std::min(60, indicatorLength));
	}

	///////////////////////////////////////////////////////////////////////////
	/// Reset the current state of the indicator and its current length
	///////////////////////////////////////////////////////////////////////////
	virtual void reset() {
	    m_current  = 0;
	    m_progress = 0;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Display the indicator.
	///////////////////////////////////////////////////////////////////////////
	virtual void refresh() {
	    //  Compute the relative length of the arrow.
		//std::cout << "refresh " << m_description << std::endl;

		stream_size_type l = m_indicatorLength - 12  - m_title.size();
	    stream_size_type progress = (m_range) ? 
			l * (m_current)/(m_range) : 0; 

		
	    //  Make sure that the first item gets printed.
	    //if (progress == 0) progress = 1;
	
	    //  Only print stuff to std::cout if the indicator needs to be updated.
	    //if (progress > m_progress) {

			//  Don't print the last item.
			if (progress >= l) progress = l -1;
			
			//  Go to the beginning of the line and print the description.
			m_os << '\r' << m_title << " [";
			
			//  Extend the arrow.
			
			m_os << std::string(progress, '=');
			m_os << '>';
			
			//  Print blank space.
			m_os << std::string(l-progress-1, ' ');
			m_os << "] ";
			
			//  Print either a percentage sign or the maximum range.
			display_percentage();
			
			m_os << ' ' << estimated_remaining_time();
			m_os << std::flush;
			m_progress = progress;
			//}
	}

    protected:

	/** The maximal length of the indicator */
	stream_size_type m_indicatorLength;

	/** The current length of the indicator */
	stream_size_type m_progress;

	/** ostream on which to display the progress indicator */
	std::ostream & m_os;

    private:

	progress_indicator_arrow();
    };

}

#endif // _TPIE_PROGRESS_INDICATOR_ARROW
