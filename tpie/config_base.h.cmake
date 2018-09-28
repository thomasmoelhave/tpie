#ifndef TPIE_CONFIG_BASE_H
#define TPIE_CONFIG_BASE_H

#cmakedefine TPIE_DEPRECATED_WARNINGS
#cmakedefine TPL_LOGGING 1
#cmakedefine TPIE_NDEBUG

#ifdef WIN32
	//disable windows crt security and deprecation warnings
	#define _CRT_SECURE_NO_DEPRECATE 
	#define _CRT_NONSTDC_NO_DEPRECATE
	#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _MSC_VER
	// recent visual studio versions gives heaps of compilation
	// warnings about security issues with fopen/strcpy and the like
	// this disables these warnings.
	#pragma warning(disable : 4996)

	// We know that visual studio does not know what to do with throw() 
	// but realy dont care
	#pragma warning(disable : 4290)

	// We know that we are casting ints to bool
	#pragma warning(disable : 4800)
#endif

#ifdef _MSC_VER
#define tpie_unreachable() __assume(0)
#else
#define tpie_unreachable() __builtin_unreachable()
#endif

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX  //ensures that the windows min/max macros are not defined 
#endif
#endif

#endif // _CONFIG_H 
