#ifndef _TPIE_BIT_H
#define _TPIE_BIT_H

// Get definitions for working with Unix and Windows
#include <portability.h>

#include <iostream>

namespace tpie {

// A bit with two operarators, addition (= XOR) and multiplication (=
// AND).
    class bit {

    private:
	char data;

    public:
	bit(void);
	bit(bool);
	bit(int);
	bit(long int);
	~bit(void);
	
	operator bool(void);
	operator int(void);
	operator long int(void);
	
	bit operator+=(bit rhs);
	bit operator*=(bit rhs);
	
	friend bit operator+(bit op1, bit op2);
	friend bit operator*(bit op1, bit op2);
	
	friend std::ostream &operator<<(std::ostream &s, bit b);
    };

}  //  tpie namespace

#endif // _TPIE_BIT_H 
