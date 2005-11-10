#ifndef _PROGRESS_INDICATOR_ARROW_H
#define _PROGRESS_INDICATOR_ARROW_H

#include <portability.h>
#include <algorithm>

#include <progress_indicator_terminal.h>

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

    progress_indicator_arrow(const char* title, 
			     const char* description, 
			     TPIE_OS_OFFSET minRange, 
			     TPIE_OS_OFFSET maxRange, 
			     TPIE_OS_OFFSET stepValue) : 
	progress_indicator_terminal(title, description, minRange, maxRange, stepValue) {
	m_indicatorLength = 40;
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
	m_indicatorLength = max(2, min(60, indicatorLength));
    }

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Reset the current state of the indicator and its current length
    ///
    ////////////////////////////////////////////////////////////////////

    void reset() {
	m_current  = m_minRange;
	m_progress = 0;
    }

    ////////////////////////////////////////////////////////////////////
    ///
    ///  Display the indicator.
    ///
    ////////////////////////////////////////////////////////////////////

    void refresh() {
	//  Compute the relative length of the arrow.
	TPIE_OS_OFFSET progress = m_indicatorLength * 
	    (m_current-m_minRange)/(m_maxRange-m_minRange); 

	//  Make sure that the first item gets printed.
	if (progress == 0) progress = 1;
	
	//  Only print stuff to cout if the indicator needs to be updated.
	if (progress > m_progress) {

	    //  Don't print the last item.
	    if (progress == m_indicatorLength) progress--;

	    //  Go to the beginning of the line and print the description.
	    cout << "\r" << m_description << " [";
	    
	    //  Extend the arrow.
	    for(TPIE_OS_OFFSET i = 0; i < progress; i++) cout << "=";
	    cout << ">";

	    //  Print blank space.
	    for(TPIE_OS_OFFSET i = progress+1; i < m_indicatorLength; i++) cout << " ";
	    cout << "] ";

	    //  Print either a percentage sign or the maximum range.
	    display_percentage();

	    cout << flush;
	    m_progress = progress;
	}
    }

protected:


    //* The current length of the indicator */
    TPIE_OS_OFFSET m_progress;
    //* The maximal length of the indicator */
    TPIE_OS_OFFSET m_indicatorLength;

private:
    progress_indicator_arrow();
};

#endif // _PROGRESS_INDICATOR_ARROW
