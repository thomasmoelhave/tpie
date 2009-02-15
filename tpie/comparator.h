// Copyright 2008, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

#ifndef _COMPARATOR_H
#define _COMPARATOR_H

///////////////////////////////////////////////////////////////////////////
/// \file comparator.h
/// \internal 
/// Provides convertion between STL and TPIE comparisions.
/// Used in \ref internal_sort.h.
///////////////////////////////////////////////////////////////////////////


// Get definitions for working with Unix and Windows
#include <tpie/config.h>
#include <tpie/portability.h>

namespace tpie {
    
// In is unlikely that users will need to directly need these classes
// except to maybe upgrade old code with minimal changes. In the future it
// may be better to make TPIE's compare() method match the syntax of the
// STL operator ()
    
//Convert STL comparison object with operator() to a TPIE comparison
//object with a compare() function. 
    template<class T, class STLCMP>
    class STL2TPIE_cmp{
    private:
	STLCMP *isLess; //Class with STL comparison operator()
	
    public:
	STL2TPIE_cmp(STLCMP* _cmp) {isLess=_cmp; }
	//Do not use with applications that test if compare returns +1
	//Because it never does so.
	inline int compare(const T& left, const T& right){
	    if( (*isLess)(left, right) ){ return -1; }
	    else { return 0; }
	}
    };
    
    
//Convert a TPIE comparison object with a compare() function. 
//to STL comparison object with operator() 
    template<class T, class TPCMP>
    class TPIE2STL_cmp{
    private:
	TPCMP* cmpobj; //Class with TPIE comparison compare()
	
    public:
	TPIE2STL_cmp(TPCMP* cmp) {cmpobj=cmp;}
	inline bool operator()(const T& left, const T& right) const{
	    return (cmpobj->compare(left, right) < 0);
	}
    };
    
//Convert a class with a comparison operator <
//to a TPIE comparison object with a compare() function. 
    template<class T>
    class op2TPIE_cmp{
    public:
	op2TPIE_cmp(){};
	//Do not use with applications that test if compare returns +1
	//Because it never does so.
	inline int compare(const T& left, const T& right){
	    if( left < right ){ return -1; }
	    else { return 0; }
	}
    };
    
//Convert a class with a comparison operator <
//to an STL comparison object with a comparison operator ().
//Not implemented here. It is called less in <functional>, part of STL

}  //  tpie namespace

#endif // _COMPARATOR_H 
