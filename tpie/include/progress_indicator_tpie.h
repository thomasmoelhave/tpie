#ifndef _PROGRESS_INDICATOR_TPIE_H
#define _PROGRESS_INDICATOR_TPIE_H

#include <portability.h>
#include <algorithm>

#include <progress_indicator_arrow.h>

///////////////////////////////////////////////////////////////////
///
///  \class progress_indicator_tpie
///
///  A class that indicates the progress by expanding the
///  word tpie.
///
///  \author The TPIE Project
///
///////////////////////////////////////////////////////////////////

class progress_indicator_tpie : public progress_indicator_arrow {

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

    progress_indicator_tpie(const char* title, 
			    const char* description, 
			    TPIE_OS_OFFSET minRange, 
			    TPIE_OS_OFFSET maxRange, 
			    TPIE_OS_OFFSET stepValue) : 
	progress_indicator_arrow(title, description, minRange, maxRange, stepValue) {
	m_progress = 0;
    }
    
    ////////////////////////////////////////////////////////////////////
    ///
    ///  The destructor. Nothing is done.
    ///
    ////////////////////////////////////////////////////////////////////

    virtual ~progress_indicator_tpie() {
	// Do nothing.
    };

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
	    cout << "\r" << m_description << " [t";
	    
	    //  Extend the e's.
	    for(TPIE_OS_OFFSET i = 0; i < progress; i++) cout << "e";
	    cout << "pie";

	    //  Print blank space.
	    for(TPIE_OS_OFFSET i = progress+1; i < m_indicatorLength; i++) cout << " ";
	    cout << "] ";

	    //  Print either a percentage sign or the maximum range.
	    display_percentage();

	    cout << flush;
	    m_progress = progress;
	}
    }
    
private:
    progress_indicator_tpie();
};

#endif // _PROGRESS_INDICATOR_TPIE
