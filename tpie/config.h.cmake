#ifndef TPIE_CONFIG_H
#define TPIE_CONFIG_H
#include <tpie/config_base.h>

#cmakedefine TPIE_HAVE_UNISTD_H
#cmakedefine TPIE_HAVE_SYS_UNISTD_H

#cmakedefine TPIE_PARALLEL_SORT

#if defined (TPIE_HAVE_UNISTD_H)
#include <unistd.h>
#elif defined(TPIE_HAVE_SYS_UNISTD_H)
#include <sys/unistd.h>
#endif

#cmakedefine TPIE_FRACTIONDB_DIR_INL
#ifdef TPIE_FRACTIONDB_DIR_INL
#undef TPIE_FRACTIONDB_DIR_INL
#define TPIE_FRACTIONDB_DIR_INL "${TPIE_FRACTIONDB_DIR_INL}"
#endif

#cmakedefine TPIE_HAS_SNAPPY
#cmakedefine TPIE_HAS_LZ4
#cmakedefine TPIE_HAS_ZSTD

// See https://github.com/lz4/lz4/pull/459
#if __cplusplus >= 201402
	#define LZ4_DISABLE_DEPRECATE_WARNINGS
#endif

#endif // TPIE_CONFIG_H 
