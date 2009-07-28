// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

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
