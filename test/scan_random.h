#ifndef _SCAN_RANDOM_H
#define _SCAN_RANDOM_H

#include <tpie/portability.h>
#include <tpie/scan.h>

using namespace tpie;

// A scan object to generate random integers.
class scan_random : ami::scan_object {
private:
    TPIE_OS_OFFSET m_max;
    TPIE_OS_OFFSET m_remaining;
public:
    scan_random(TPIE_OS_OFFSET count = 1000, int seed = 17);
    virtual ~scan_random(void);
    ami::err initialize(void);
    ami::err operate(int *out1, ami::SCAN_FLAG *sf);
};

#endif // _SCAN_RANDOM_H 
