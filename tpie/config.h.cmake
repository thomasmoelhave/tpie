#ifndef _CONFIG_H
#define _CONFIG_H

#cmakedefine TPIE_HAVE_UNISTD_H
#cmakedefine TPIE_HAVE_SYS_UNISTD_H

#cmakedefine TPIE_USE_EXCEPTIONS

#if defined (TPIE_HAVE_UNISTD_H)
#include <unistd.h>
#elif defined(TPIE_HAVE_SYS_UNISTD_H)
#include <sys/unistd.h>
#endif

// On Solaris, _SC_PAGE_SIZE is called _SC_PAGE_SIZE.  Here's a quick
// fix.
#if !defined(_SC_PAGE_SIZE) && defined(_SC_PAGESIZE)
#define _SC_PAGE_SIZE _SC_PAGESIZE
#endif

#cmakedefine TPL_LOGGING 1
#cmakedefine DEBUG_ASSERTIONS 1

#include <string>
namespace tpie {
const std::string tempDir = "${TEMP_DIR}";
} 

// recent visual studio versions gives heaps of compilation
// warnings about security issues with fopen/strcpy and the like
// this disables these warnings.
#ifdef WIN32
	//disable windows crt security and deprecation warnings
	#define _CRT_SECURE_NO_DEPRECATE 
	#define _CRT_NONSTDC_NO_DEPRECATE
	#define _CRT_SECURE_NO_WARNINGS
	#pragma warning(disable : 4996) 
#endif

#endif // _CONFIG_H 
