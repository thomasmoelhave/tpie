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
							 TPIE_OS_OFFSET minRange, 
							 TPIE_OS_OFFSET maxRange, 
							 TPIE_OS_OFFSET stepValue) : 
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
	    TPIE_OS_OFFSET progress = m_indicatorLength * 
		(m_current-m_minRange)/(m_maxRange-m_minRange); 

	    //  Make sure that the first item gets printed.
	    if (progress == 0) progress = 1;
	
	    //  Only print stuff to std::cout if the indicator needs to be updated.
	    if (progress > m_progress) {

		//  Don't print the last item.
		if (progress == m_indicatorLength) progress--;

		//  Go to the beginning of the line and print the description.
		std::cout << "\r" << m_description << " [";
	    
		//  Extend the arrow.
		for(TPIE_OS_OFFSET i = 0; i < progress; i++) std::cout << "=";
		std::cout << ">";

		//  Print blank space.
		for(TPIE_OS_OFFSET i = progress+1; i < m_indicatorLength; i++) std::cout << " ";
		std::cout << "] ";

		//  Print either a percentage sign or the maximum range.
		display_percentage();

		std::cout << std::flush;
		m_progress = progress;
	    }
	}

    protected:

	/** The maximal length of the indicator */
	TPIE_OS_OFFSET m_indicatorLength;

	/** The current length of the indicator */
	TPIE_OS_OFFSET m_progress;

    private:

  ////////////////////////////////////////////////////////////////////
  ///  Empty constructor.
  ////////////////////////////////////////////////////////////////////
  progress_indicator_arrow();
    };

}

#endif // _TPIE_PROGRESS_INDICATOR_ARROW
