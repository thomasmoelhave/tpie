#ifndef _LIB_CONFIG_H
#define _LIB_CONFIG_H

#include <tpie/config.h>

// Use logs if requested.
#if TP_LOG_LIB
#define TPL_LOGGING 1
#endif
#include <tpie/tpie_log.h>

// Enable assertions if requested.
#if TP_ASSERT_LIB
#define DEBUG_ASSERTIONS 1
#endif
#include <tpie/tpie_assert.h>


#endif // _LIB_CONFIG_H 
 
