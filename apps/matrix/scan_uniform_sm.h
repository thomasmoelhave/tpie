// Copyright (c) 1995 Darren Vengroff
//
// File: scan_uniform_sm.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/6/95
//
// $Id: scan_uniform_sm.h,v 1.7 2004-08-12 15:15:11 jan Exp $
//
#ifndef _TPIE_APPS_SCAN_UNIFORM_SM_H
#define _TPIE_APPS_SCAN_UNIFORM_SM_H

#include "sparse_matrix.h"

namespace tpie {

    namespace apps {
	
	class scan_uniform_sm : public ami::scan_object {
	private:
	    TPIE_OS_OFFSET r,c, rmax, cmax;
	    double d;

	public:
	    scan_uniform_sm(TPIE_OS_OFFSET rows, TPIE_OS_OFFSET cols,
			    double density, int seed);
	    virtual ~scan_uniform_sm(void);

	    ami::err initialize(void);
	    ami::err operate(sm_elem<double> *out, ami::SCAN_FLAG *sf);
	};

    }  //  namespace apps

}  //  namespace tpie

#endif // _TPIE_APPS_SCAN_UNIFORM_SM_H 
