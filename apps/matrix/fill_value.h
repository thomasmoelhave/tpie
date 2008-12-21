// Copyright (c) 1995 Darren Vengroff
//
// File: fill_value.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/6/95
//
// $Id: fill_value.h,v 1.5 2005-11-17 17:07:40 jan Exp $
//
#ifndef _TPIE_APPS_FILL_VALUE_H
#define _TPIE_APPS_FILL_VALUE_H

#include <tpie/portability.h>

#include "matrix_fill.h"

namespace tpie {

    namespace apps {
	
	template<class T>
	class fill_value : public matrix_filler<T> {
	    
	private:
	    T value;
	public:
	    
	    fill_value() : value() {};

	    void set_value(const T &v) {
		value = v;
	    };

	    ami::err initialize(TPIE_OS_OFFSET /*rows*/, TPIE_OS_OFFSET /*cols*/) {
		return ami::NO_ERROR;
	    };
	    
	    T element(TPIE_OS_OFFSET /*row*/, TPIE_OS_OFFSET /*col*/) {
		return value;
	    };
	};

    }  //  namespace apps

}  //  namespace tpie

#endif // _TPIE_APPS_FILL_VALUE_H 
