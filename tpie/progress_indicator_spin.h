#ifndef _TPIE_PROGRESS_INDICATOR_SPIN_H
#define _TPIE_PROGRESS_INDICATOR_SPIN_H

#include <portability.h>
#include <algorithm>

#include <progress_indicator_terminal.h>

namespace tpie {

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
	    progress_indicator_terminal(title, description, minRange, maxRange, stepValue), m_symbols(NULL), m_numberOfStates(0), m_state(0) {
	    m_numberOfStates = 4;
	    m_symbols = new char[m_numberOfStates+2];
	    m_symbols[0] = '|';
	    m_symbols[1] = '/';
	    m_symbols[2] = '-';
	    m_symbols[3] = '\\';
	    m_symbols[4] = 'X';
	    m_symbols[5] = '\0';
	}

	progress_indicator_spin(const progress_indicator_spin& other) : 
	    progress_indicator_terminal(other), m_symbols(NULL), m_numberOfStates(0), m_state(0) {
	    *this = other;
	}

	progress_indicator_spin& operator=(const progress_indicator_spin& other) {
	    if (this != &other) {

		progress_indicator_terminal::operator=(other);

		m_numberOfStates = other.m_numberOfStates;
		m_state          = other.m_state;
	    
		delete[] m_symbols;

		m_symbols = new char[m_numberOfStates+2];
		memcpy(m_symbols, other.m_symbols, m_numberOfStates+2);
	    }
	    return *this;
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

	virtual void refresh() {
	    m_state = ++m_state % m_numberOfStates;

	    //  Use the last symbol for indicating "done".
	    if (m_current == m_maxRange) m_state = m_numberOfStates;

	    //  Go to the beginning of the line and print the description.
	    cout << "\r" << m_description << " " << m_symbols[m_state] << flush;
	
	}

    protected:

	/**  The characters used for the spinning indicator.  */
	char* m_symbols;

	/**  The number of characters used for the spinning indicator.  */
	unsigned short m_numberOfStates;

	/**  The current character used for the spinning indicator.  */
	unsigned short m_state;


    private:
	progress_indicator_spin();
    };

}  //  tpie namespace

#endif // _TPIE_PROGRESS_INDICATOR_SPIN_H
