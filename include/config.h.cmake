#ifndef _CONFIG_H
#define _CONFIG_H

// Define if asyncronous I/O is avaialable.
#cmakedefine HAVE_LIBAIO


#cmakedefine HAVE_UNISTD_H
#cmakedefine HAVE_SYS_UNISTD_H

#if defined (HAVE_UNISTD_H)
#include <unistd.h>
#elif defined(HAVE_SYS_UNISTD_H)
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

#endif // _CONFIG_H 
