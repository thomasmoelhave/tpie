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

// Copyright (c) 1994 Darren Vengroff
//
// File: ami_matrix_blocks.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/11/94
//
// $Id: ami_matrix_blocks.h,v 1.4 2004-08-12 12:35:30 jan Exp $
//
#ifndef _TPIE_APPS_MATRIX_BLOCKS_H
#define _TPIE_APPS_MATRIX_BLOCKS_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
// Get AMI_gen_perm_object.
#include <tpie/gen_perm_object.h>

namespace tpie {

    namespace apps {
	
	class perm_matrix_into_blocks : public ami::gen_perm_object {

	private:
	    stream_offset_type r,c,be;

	public:    
	    perm_matrix_into_blocks(stream_offset_type rows, stream_offset_type cols,
				    stream_offset_type block_extent);
	    virtual ~perm_matrix_into_blocks();
	    
	    ami::err initialize(stream_offset_type len);
	    stream_offset_type destination(stream_offset_type source);
	};
	
    }  //  namespace apps

}  //  namespace tpie

namespace tpie {

    namespace apps {
	
	class perm_matrix_outof_blocks : public ami::gen_perm_object {

	private:
	    stream_offset_type r,c,be;

	public:    
	    perm_matrix_outof_blocks(stream_offset_type rows, stream_offset_type cols,
				     stream_offset_type block_extent);
	    virtual ~perm_matrix_outof_blocks();

	    ami::err initialize(stream_offset_type len);
	    stream_offset_type destination(stream_offset_type source);
	};


    }  //  namespace apps

}  //  namespace tpie


#endif // _TPIE_APPS__MATRIX_BLOCKS_H 



