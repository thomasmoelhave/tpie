// -*- C++ -*-
//
//  Description:     declarations for Hilbert values
//  Created:         02.02.1998
//  Author:          Jan Vahrenhold
//  mail:            jan.vahrenhold@math.uni-muenster.de
//  $Id: hilbert.h,v 1.2 2004-02-05 17:53:53 jan Exp $
//  Copyright (C) 1998 by  
// 
//  Jan Vahrenhold
//  Westfaelische Wilhelms-Universitaet Muenster
//  Institut fuer Informatik
//  Einsteinstr. 62
//  D-48149 Muenster
//  GERMANY
//

#ifndef HILBERT_H
#define HILBERT_H

#include <tpie/portability.h>
#include <tpie/stream.h>
using namespace tpie;
using namespace tpie::ami;


TPIE_OS_LONGLONG computeHilbertValue(TPIE_OS_LONGLONG x, TPIE_OS_LONGLONG y, TPIE_OS_LONGLONG side);

#endif
