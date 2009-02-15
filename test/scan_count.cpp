// A scan object to generate a stream of intergers in ascending order.

#include "app_config.h"
#include "scan_count.h"

ami::err scan_count::initialize(void) {

    called = 0;
    ii = 0;
    
    return ami::NO_ERROR;
};

scan_count::scan_count(TPIE_OS_OFFSET max) :
        maximum(max),
        ii(0) {

    //  No code in this constructor
};

ami::err scan_count::operate(TPIE_OS_OFFSET *out1, ami::SCAN_FLAG *sf) {
    called++;
    *out1 = ++ii;
    return (*sf = (ii <= maximum)) ? ami::SCAN_CONTINUE : ami::SCAN_DONE;
};

