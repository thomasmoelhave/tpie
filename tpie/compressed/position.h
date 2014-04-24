// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2013, The TPIE development team
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

#ifndef TPIE_COMPRESSED_POSITION_H
#define TPIE_COMPRESSED_POSITION_H

///////////////////////////////////////////////////////////////////////////////
/// \file compressed/position.h  Stream position indicator.
///////////////////////////////////////////////////////////////////////////////

#include <tpie/compressed/predeclare.h>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief  POD object indicating the position of an item in a stream.
///
/// A stream position is the tuple `(read_offset, offset)`.
///
/// For compressed streams, the stream block begins with a header at byte
/// position `read_offset`.  After the block header, the items follow.
///
/// For uncompressed streams, `read_offset` is zero and the item position
/// in the stream is determined solely by `offset`.
///
/// Thus, the first item in the stream has the position tuple `(0, 0)`.
///////////////////////////////////////////////////////////////////////////////
class stream_position {
private:
	friend class compressed_stream_base;
	template <typename T>
	friend class file_stream;

	uint64_t m_readOffset;
	uint64_t m_offset;

public:
	stream_position()
		: m_readOffset(std::numeric_limits<uint64_t>::max())
		, m_offset(std::numeric_limits<uint64_t>::max())
	{
	}

private:
	stream_position(stream_size_type readOffset,
					stream_size_type offset)
		: m_readOffset(readOffset)
		, m_offset(offset)
	{
	}

	stream_size_type read_offset() const {
		return m_readOffset;
	}

	stream_size_type offset() const {
		return m_offset;
	}

	void advance_items(memory_size_type offset) {
		m_offset += offset;
	}

	void advance_item() {
		advance_items(1);
	}

public:
	bool operator==(const stream_position & other) const {
		return m_readOffset == other.m_readOffset
			&& m_offset == other.m_offset;
	}

	bool operator!=(const stream_position & other) const {
		return !(*this == other);
	}

	bool operator<(const stream_position & other) const {
		return m_offset < other.m_offset;
	}
};

} // namespace tpie

#endif // TPIE_COMPRESSED_POSITION_H
