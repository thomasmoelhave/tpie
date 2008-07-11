//
//  Description:     source for computing Hilbert values
//  Created:         02.02.1998
//  Author:          Jan Vahrenhold
//  mail:            jan.vahrenhold@math.uni-muenster.de
//  $Id: hilbert.cpp,v 1.2 2004-02-05 17:53:53 jan Exp $
//  Copyright (C) 1998 by  
// 
//  Jan Vahrenhold
//  Westfaelische Wilhelms-Universitaet Muenster
//  Institut fuer Informatik
//  Einsteinstr. 62
//  D-48149 Muenster
//  GERMANY
//

#include "hilbert.h"
#include <assert.h>

//
//  This code is taken from:
//
//  Jagadish, H.V.: "Linear Clustering of Objects with Multiple Attributes", 
//  in: Proceedings of the 1990 ACM SIGMOD International Conference on 
//  Management of Data (1990), 332-342.
//

int HILBERTrotation_table[4] = {3, 0, 0, 1};
int HILBERTsense_table[4] = {-1, 1, 1, -1};
int HILBERTquad_table[4][2][2] = { {{0,1},{3,2}},
				   {{1,2},{0,3}},
				   {{2,3},{1,0}},
				   {{3,0},{2,1}} };

TPIE_OS_LONGLONG computeHilbertValue(TPIE_OS_LONGLONG x, TPIE_OS_LONGLONG y, TPIE_OS_LONGLONG side) {

    assert(0 <= x);
    assert(0 <= y);
    assert(x <= side);
    assert(y <= side);
    assert(side >= 0);

    int              rotation = 0;
    int              sense    = 1;
    TPIE_OS_LONGLONG num      = 0;
    TPIE_OS_LONGLONG k        = 0;
    TPIE_OS_LONGLONG xbit     = 0;
    TPIE_OS_LONGLONG ybit     = 0;
    int              quad     = 0;
    
    for(k = side/2; k > 0; k = k/2) {
	xbit = x/k;
	ybit = y/k;
	x -= k*xbit;
	y -= k*ybit;
	quad = HILBERTquad_table[rotation][xbit][ybit];
	num += (sense == -1) ? k*k*(3-quad) : k*k*quad;
	rotation += HILBERTrotation_table[quad];
	if (rotation >= 4) rotation -= 4;
	sense *= HILBERTsense_table[quad];	
    }
    return num;
}

//  End (Jagadish)
