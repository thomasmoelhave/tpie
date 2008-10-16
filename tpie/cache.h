// Copyright (C) 2001 Octavian Procopiuc
//
// File:    ami_cache.h
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: ami_cache.h,v 1.10 2004-08-12 12:35:30 jan Exp $
//
// Declaration and definition of AMI_CACHE_MANAGER
// implementation(s).
//

#ifndef _TPIE_AMI_CACHE_H
#define _TPIE_AMI_CACHE_H

#include <cache_base.h>
#include <cache_lru.h>

namespace tpie {
    
    namespace ami {
	
// The only implementation is AMI_cache_manager_lru.
#define CACHE_MANAGER cache_manager_lru

    }  //  ami namespace

}  //  tpie namespace


#endif // _TPIE_AMI_CACHE_H
