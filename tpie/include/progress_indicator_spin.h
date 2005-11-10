#ifndef _PROGRESS_INDICATOR_SPIN_H
#define _PROGRESS_INDICATOR_SPIN_H

#include <portability.h>
#include <algorithm>

#include <progress_indicator_terminal.h>

///////////////////////////////////////////////////////////////////
///
///  \class progress_indicator_spin
///
///  A class that indicates the progress by a spinning cross.
///
///  \author The TPIE Project
///
///////////////////////////////////////////////////////////////////

class progress_indicator_spin : public progress_indicator_terminal {

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

    progress_indicator_spin(const char* title, 
			     const char* description, 
			     TPIE_OS_OFFSET minRange, 
			     TPIE_OS_OFFSET maxRange, 
			     TPIE_OS_OFFSET stepValue) : 
	progress_indicator_terminal(title, description, minRange, maxRange, stepValue), state(0) {
	numberOfStates = 4;
	m_symbols = new char[numberOfStates+2];
	m_symbols[0] = '|';
	m_symbols[1] = '/';
	m_symbols[2] = '-';
	m_symbols[3] = '\\';
	m_symbols[4] = 'X';
	m_symbols[5] = '\0';
    }
    
    ////////////////////////////////////////////////////////////////////
    ///
    ///  The destructor. Nothing is done.
    ///
    ////////////////////////////////////////////////////////////////////

    virtual ~progress_indicator_spin() {
	delete [] m_symbols;
    };
    
    ////////////////////////////////////////////////////////////////////
    ///
    ///  Display the indicator.
    ///
    ////////////////////////////////////////////////////////////////////

    void refresh() {
	state = ++state % numberOfStates;

	//  Use the last symbol for indicating "done".
	if (m_current == m_maxRange) state = numberOfStates;

	//  Go to the beginning of the line and print the description.
	cout << "\r" << m_description << " " << m_symbols[state] << flush;
	
    }

protected:

    /**  The characters used for the spinning indicator.  */
    char* m_symbols;

    /**  The number of characters used for the spinning indicator.  */
    unsigned short numberOfStates;

    /**  The current character used for the spinning indicator.  */
    unsigned short state;


private:
    progress_indicator_spin();
};

#endif // _PROGRESS_INDICATOR_SPIN_H
