#ifndef _AMI_H
#define _AMI_H
///////////////////////////////////////////////////////////////////////////
/// \file ami.h
/// This include file inputs the various include files required to compile
/// an application program with TPIE.
/// This file should be included in every TPIE application program that uses the 
/// AMI-level interface. It in turn inputs the definitions for the 
/// AMI-level services of TPIE.  
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
/// \namespace tpie TPIE's namespace.
/// \namespace tpie::ami The namespace within TPIE for the Access
/// Method Interface elements.
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// Get a stream implementation.
#include <tpie/stream.h>

// Get templates for ami_scan().
#include <tpie/scan.h>

// Get templates for ami_merge().
#include <tpie/merge.h>

// Get templates for ami_sort().
#include <tpie/sort.h>

// Get templates for general permutation.
#include <tpie/gen_perm.h>

// Get templates for bit permuting.
//#include <bit_permute.h>

// Get a collection implementation.
#include <tpie/coll.h>

// Get a block implementation.
#include <tpie/block.h>

// Get templates for AMI_btree.
#include <tpie/btree.h>

#endif // _AMI_H 
