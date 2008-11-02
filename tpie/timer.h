#ifndef _TPIE_TIMER_H
#define _TPIE_TIMER_H

///////////////////////////////////////////////////////////////////////////
/// \file timer.h
/// General definition of a virtual \ref timer class; is realized by
/// \ref cpu_timer.
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

namespace tpie {

    ///////////////////////////////////////////////////////////////////////////
    /// A virtual \ref timer class; is realized by \ref cpu_timer.
    ///////////////////////////////////////////////////////////////////////////
    class timer {
	
    public:
	virtual void start(void) = 0;
	virtual void stop(void) = 0;
	virtual void reset(void) = 0;
	virtual ~timer() {} ;
    };

}

#endif // _TPIE_TIMER_H 
