// Copyright (c) 1995 Darren Erik Vengroff
//
// File: ami_key.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 3/12/95
//
// $Id: ami_key.h,v 1.3 2005-11-18 12:29:00 jan Exp $
//
#ifndef _TPIE_AMI_KEY_H
#define _TPIE_AMI_KEY_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

namespace tpie {
    
    namespace ami {
	
// CHECK THIS
// Temporary until the configuration script is edited to determine word
// size.
#define UINT32 unsigned long
	
	
// Radix keys are unsigned 32 bit integers.
	typedef UINT32 kb_key;
	
#define KEY_MAX 0x80000000
#define KEY_MIN 0
	
// A range of keys.  A stream having this range of keys is guarantted
// to have no keys < min and no keys >= max.
	class key_range {
	public:
	    key_range();
	    key_range(kb_key min_key, kb_key max_key);
	    
	    kb_key get_min() const { 
		return m_min; 
	    }
	    
	    void put_min(kb_key min_key) {
		m_min = min_key;
	    }
	    
	    kb_key get_max() const { 
		return m_max; 
	    }
	    
	    void put_max(kb_key max_key) {
		m_max = max_key;
	    }
	    
	private:
	    kb_key m_min;
	    kb_key m_max;
	};
	

    }  //  ami namespace

}  //  tpie namespace

#endif // _TPIE_AMI_KEY_H 
