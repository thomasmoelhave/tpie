// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008,2012, The TPIE development team
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

// The tpie_stats class for recording statistics. The parameter C is
// the number of statistics to be recorded.

#include <tpie/stats.h>

namespace tpie {
	
	static stream_size_type temp_file_usage=0;
	static stream_size_type bytes_read=0;
	static stream_size_type bytes_written=0;

	stream_size_type get_tempfile_usage() {
		return temp_file_usage;
	}

	void increment_temp_file_usage(stream_offset_type delta) {
		stream_offset_type x=temp_file_usage+delta;
		if (x < 0) temp_file_usage=0;
		temp_file_usage=x;
	}

	stream_size_type get_bytes_read() {
		return bytes_read;
	}

	stream_size_type get_bytes_written() {
		return bytes_written;
	}

	void increment_bytes_read(stream_size_type delta) {
		bytes_read += delta;
	}
	
	void increment_bytes_written(stream_size_type delta) {
		bytes_written += delta;
	}
}  //  tpie namespace

