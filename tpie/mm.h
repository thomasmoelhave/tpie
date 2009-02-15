#ifndef _TPIE_MM_H
#define _TPIE_MM_H

// Get the base class, enums, etc...
#include <tpie/mm_base.h>

///////////////////////////////////////////////////////////////////////////
/// \file mm.h Provides means to choose and set a specific memory
/// management to use within TPIE.
/// For now only single address space memory management is supported
/// (through the class MM_Register).
///////////////////////////////////////////////////////////////////////////

// Get an implementation definition..

#ifdef MM_IMP_REGISTER
#include <tpie/mm_manager.h>
#else
#error No MM implementation selected.
#endif

#endif // _TPIE_MM_H 
