#include <iostream>

#include <bit_permute.h>

using tpie::ami;

bit_perm_object::bit_perm_object(const bit_matrix &A,
				 const bit_matrix &c) :
    mA(A), mc(c) {
    //  No code in this constructor.
}

bit_perm_object::~bit_perm_object(void) {

    //  No code in this destructor.

}

bit_matrix bit_perm_object::A(void) {
    return mA;
}

bit_matrix bit_perm_object::c(void) {
    return mc;
}


