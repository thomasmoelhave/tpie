#ifndef _CONFIG_H
#define _CONFIG_H

// Define if asyncronous I/O is avaialable.
#cmakedefine TPIE_HAVE_LIBAIO

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

// Flags to enable or disable various features of the system.

#define TP_ASSERT_APPS 1
#define TP_ASSERT_LIB 1

#define TP_LOG_APPS 1
#define TP_LOG_LIB 1


// recent visual studio versions gives heaps of compilation
// warnings about security issues with fopen/strcpy and the like
// this disables these warnings.
#ifdef WIN32
	#define _CRT_SECURE_NO_DEPRECATE 1
	#define _CRT_SECURE_NO_WARNINGS 1 
#endif

#endif // _CONFIG_H 
