// -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
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

///////////////////////////////////////////////////////////////////////////////
/// \file util.cpp  Miscellaneous utility functions - implementation
///////////////////////////////////////////////////////////////////////////////

#include <tpie/exception.h>
#include <tpie/util.h>
#include <stdexcept>
#include <cstdio>

namespace tpie {

void atomic_rename(const std::string & src, const std::string & dst) {
	// On Windows, we could use ReplaceFile here.
	// The MSDN article "Alternatives to using Transactional NTFS"
	// states that ReplaceFile should be used for
	// "atomically updating 'document-like' data", but the ReplaceFile docs
	// do not specify anywhere that atomicity is guaranteed.
	// From using DrStrace, it seems that POSIX rename is implemented
	// using NtSetInformationFile with a FILE_RENAME_INFORMATION structure,
	// which should be as good as ReplaceFile with regards to atomicity.
	// Furthermore, ReplaceFile does a lot of bookkeeping to maintain all sorts
	// of filesystem metadata which we do not need to maintain,
	// so we just use POSIX rename on both Linux and Windows.
	if (rename(src.c_str(), dst.c_str()) != 0)
		throw std::runtime_error("Atomic rename failed");
}

} // namespace tpie
