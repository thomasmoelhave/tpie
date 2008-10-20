//
// File: ami_device.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 8/22/93
//

#include "lib_config.h"

#include <cstdlib>
#include <string.h>

#include <tpie/err.h>
#include <tpie/device.h>

using namespace tpie;

//ami::device::device(void) : argc(0), argv(NULL) {
//    TP_LOG_DEBUG_ID("In device(void).");
//}
//
//
//ami::device::device(unsigned int count, char **strings) : argc(count), argv(NULL)
//{
//    char *s, *t;
//
//    if (argc) {
//        argv = new char*[argc];
//
//        while (count--) {
//            argv[count] = new char[strlen(strings[count]) + 1];
//            // for (s = strings[count], t = argv[count]; *t++ = *s++; )
//	    // [tavi] modified to avoid warning.
//	    for (s = strings[count], t = argv[count]; *t; *t++ = *s++)
//                ;
//        }
//                     
//    }
//}
//
//ami::device::device(const ami::device& other) : argc(0), argv(NULL) {
//    *this = other;
//}
//
//ami::device::~device(void) {
//    dispose_contents();
//}
//
//
//ami::device& ami::device::operator=(const ami::device& other) {
//
//    if (this != &other) {
//
//	char *s, *t;
//	
//	unsigned int count = other.argc;
//	argc = other.argc;;
//
//	if (argc) {
//	    argv = new char*[argc];
//	    
//	    while (count--) {
//		argv[count] = new char[strlen(other.argv[count]) + 1];
//		// for (s = strings[count], t = argv[count]; *t++ = *s++; )
//		// [tavi] modified to avoid warning.
//		for (s = other.argv[count], t = argv[count]; *t; *t++ = *s++)
//		    ;
//	    }
//	    
//	} else {
//	    argv = NULL;
//	}
//    }
//    return *this;
//};
//
//const char * ami::device::operator[](unsigned int index)
//{
//    if (argv)
//	return argv[index];
//
//    return NULL;
//}
//
//unsigned int ami::device::arity() {
//    return argc;
//}
//
//void ami::device::dispose_contents(void) {
//    if (argc) {
//        while (argc--) {
//            delete argv[argc];
//        }
//
//        tp_assert((argv != NULL), "Nonzero argc and NULL argv.");
//
//        delete [] argv;
//    }
//}
//
//ami::err ami::device::set_to_path(const char *path) {
//    const char *s, *t;
//    unsigned int ii;
//
//    dispose_contents();
//
//    // Count the components
//    for (argc = 1, s = path; *s; s++) {
//        if (*s == '|') {
//            argc++;
//        }
//    }
//                
//    argv = new char*[argc];
//
//    // copy the components one by one.  t points to the start of the 
//    // current component and s is used to scan to the end of it.
//
//    for (ii = 0, s = t = path; ii < argc; ii++, t = ++s) {
//        // Move past the current component.
//        while (*s && (*s != '|')) 
//            s++;
//
//        tp_assert(((*s == '|') || (ii == argc - 1)),
//                  "Path ended before all components found.");
//        
//        // Copy the current component.
//        argv[ii] = new char[s - t + 1];
//        strncpy(argv[ii], t, s - t);
//        argv[ii][s - t] = '\0';
//
//		// make sure there is no trailing /
//		for(TPIE_OS_LONGLONG i=s-t-1; i && (argv[ii][i]) == '/'; i--) {
//		  argv[ii][i] = '\0';
//		}
//		tp_assert(strlen(argv[ii]) > 0, "non-null path specified");
//		tp_assert(strlen(argv[ii]) == 1 ||
//				  argv[ii][strlen(argv[ii])] != '/', "no / suffix");
//    }
//
//    return ami::NO_ERROR;
//}
//
//
//ami::err ami::device::read_environment(const char *name) {
//    char *env_value = getenv(name);
//    
//    if (env_value == NULL) {
//        return ami::ENV_UNDEFINED;
//    }
//
//    return set_to_path(env_value);
//}

