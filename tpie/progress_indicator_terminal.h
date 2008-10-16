#ifndef _TPIE_PROGRESS_INDICATOR_TERMINAL_H
#define _TPIE_PROGRESS_INDICATOR_TERMINAL_H

#include <portability.h>

#include <progress_indicator_base.h>

namespace tpie {

///////////////////////////////////////////////////////////////////
///
///  \class progress_indicator_terminal
///
///  A class that indicates the progress by a simple counter that
///  is printed to the terminal.
///
///  \author The TPIE Project
///
///////////////////////////////////////////////////////////////////

    class progress_indicator_terminal : public progress_indicator_base {

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

	progress_indicator_terminal(const char* title, 
				    const char* description, 
				    TPIE_OS_OFFSET minRange, 
				    TPIE_OS_OFFSET maxRange, 
				    TPIE_OS_OFFSET stepValue) : 
	    progress_indicator_base(title, description, minRange, maxRange, stepValue), m_title(NULL), m_description(NULL) {
	    m_title = new char[strlen(title)+1];
	    strcpy(m_title, title);

	    m_description = new char[strlen(description)+1];
	    strcpy(m_description, description);
	}

	progress_indicator_terminal(const progress_indicator_terminal& other) : 
	    progress_indicator_base(other), m_title(NULL), m_description(NULL) {
	    *this = other;
	}

	progress_indicator_terminal& operator=(const progress_indicator_terminal& other) {
	    if (this != &other) {

		progress_indicator_base::operator=(other);

		delete [] m_title;
		delete [] m_description;

		m_title = new char[strlen(other.m_title)+1];
		strcpy(m_title, other.m_title);
	    
		m_description = new char[strlen(other.m_description)+1];
		strcpy(m_description, other.m_description);
	    }
	    return *this;
	}

	////////////////////////////////////////////////////////////////////
	///
	///  The destructor. Free the space allocated for title/description.
	///
	////////////////////////////////////////////////////////////////////

	virtual ~progress_indicator_terminal() {
	    delete [] m_title;
	    delete [] m_description;
	};
    
	////////////////////////////////////////////////////////////////////
	///
	///  Advance the indicator to the end and print an (optional)
	///  message that is followed by a newline.
	///
	///  \param  text  The message to be printed at the end of the
	///                indicator.
	///
	////////////////////////////////////////////////////////////////////

	void done(const char* text = NULL) {
	    m_current = m_maxRange;
	    refresh();
	    if (text) {
		cout << " " << text;
	    }
	    cout << endl;
	}

	////////////////////////////////////////////////////////////////////
	///
	///  Reset the counter. The current position is reset to the
	///  lower bound of the counting range.
	///
	////////////////////////////////////////////////////////////////////

	virtual void reset() {
	    m_current = m_minRange;
	}

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

	void set_min_range(TPIE_OS_OFFSET minRange) {
	    m_minRange = minRange;
	    reset();
	}

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

	void set_max_range(TPIE_OS_OFFSET maxRange) {
	    m_maxRange = maxRange;
	    reset();
	}

	////////////////////////////////////////////////////////////////////
	///
	///  Set the increment by which the counter is advanced upon each
	///  call to step(). In order to be able to reset the counter,
	///  no range checking is done.
	///
	///  \param  stepValue  The incerement.
	///
	////////////////////////////////////////////////////////////////////

	void set_step_value(TPIE_OS_OFFSET stepValue) {
	    m_stepValue = stepValue;
	}
  
	////////////////////////////////////////////////////////////////////
	///
	///  Set the title of a new task to be monitored. The terminal
	///  line will be newline'd, and the title will be followed by a
	///  newline as well.
	///
	///  \param  title  The title of the new task to be monitored.
	///
	////////////////////////////////////////////////////////////////////

	void set_title(const char* title) {
	    delete[] m_title;

	    m_title = new char[strlen(title)+1];
	    strcpy(m_title, title);
	    cout << endl << title << endl;
	}

	////////////////////////////////////////////////////////////////////
	///
	///  Set the description of the task currently being monitored.
	///  Invoking this method will clear the terminal line.
	///
	///  \param  description  The decription of the task being monitored.
	///
	////////////////////////////////////////////////////////////////////

	void set_description(const char* description) {
	    delete[] m_description;

	    m_description = new char[strlen(description)+1];
	    strcpy(m_description, description);
	    cout << "\r";
	    for (int i = 0; i < 78; i++) cout << " ";
	    cout << "\r" << description << flush;
	}

	////////////////////////////////////////////////////////////////////
	///
	///  Display the indicator.
	///
	////////////////////////////////////////////////////////////////////

	virtual void refresh() {
	    cout << "\r" << m_description << " ";
	    display_percentage();
	    cout << flush;
	}

    protected:


	////////////////////////////////////////////////////////////////////
	///
	///  Compute and print the percentage or step count.
	///
	////////////////////////////////////////////////////////////////////

	void display_percentage() {
	    if (m_percentageUnit) {
		cout << setw(6) << setiosflags(ios::fixed) << setprecision(2) 
		     << ((static_cast<double>(m_current) * 100.0) / 
			 static_cast<double>(m_percentageUnit))
		     << "%";
	    }
	    else {
		cout << m_current << "/" << m_maxRange-m_minRange;
	    }
	}

	/**  A string holding the description of the title */
	char* m_title;

	/**  A string holding the description of the current task */
	char* m_description;

    private:
	progress_indicator_terminal();
    };

}  //  tpie namespace

#endif // _PROGRESS_INDICATOR_TERMINAL
