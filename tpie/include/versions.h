// File: versions.h
// Created: 99/11/15

// This file defines a macro VERSION that creates a static variable __name whose
// contents contain the given string __id. This is intended to be used for creating
// RCS version identifiers as static data in object files and executables.

// The "compiler_fooler stuff creates a (small) self-referential structure that 
// prevents the compiler from warning that __name is never referenced.
// 	$Id: versions.h,v 1.2 1999-11-15 23:25:11 hutchins Exp $	

#ifndef _VERSIONS_H
#define _VERSIONS_H

// Be careful : keep on a single line
#define VERSION(__name,__id)  static char __name[] = __id;static struct __name##_compiler_fooler { char *pc; __name##_compiler_fooler *next;} the##__name##_compiler_fooler = { __name, & the##__name##_compiler_fooler};

#endif // _VERSIONS_H 
