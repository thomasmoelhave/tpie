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
/// \file compressed/position.h
///////////////////////////////////////////////////////////////////////////////

#include <tpie/compressed/predeclare.h>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief  POD object indicating the position of an item in a stream.
///
/// A stream position is the tuple
/// `(read_offset, block_item_index, block_number, offset)`.
///
/// The compressed block containing the item starts at byte read_offset+8.
/// The size of the compressed block is in `[read_offset, read_offset+8)`.
/// In the logical block, the item is at index `block_item_index`.
/// The block has the index `block_number` in the stream,
/// and the global item offset is `offset`.
///
/// Thus, the first item in the stream has the position tuple `(0, 0, 0, 0)`.
///////////////////////////////////////////////////////////////////////////////
class stream_position {
private:
	template <typename T>
	friend class compressed_stream;

	uint64_t m_readOffset;
	uint64_t m_blockNumber;
	uint64_t m_offset;
	uint32_t m_blockItemIndex;

public:
	stream_position()
		: m_offset(std::numeric_limits<uint64_t>::max())
	{
	}

private:
	stream_position(stream_size_type readOffset,
					memory_size_type blockItemIndex,
					stream_size_type blockNumber,
					stream_size_type offset)
		: m_readOffset(readOffset)
		, m_blockNumber(blockNumber)
		, m_offset(offset)
		, m_blockItemIndex(static_cast<uint32_t>(blockItemIndex))
	{
		if (m_blockItemIndex != blockItemIndex)
			throw exception("stream_position: Block item index out of bounds");
	}

	stream_size_type read_offset() const {
		return m_readOffset;
	}

	memory_size_type block_item_index() const {
		return m_blockItemIndex;
	}

	stream_size_type block_number() const {
		return m_blockNumber;
	}

	stream_size_type offset() const {
		return m_offset;
	}

	void advance_items(memory_size_type offset) {
		m_blockItemIndex += offset;
		m_offset += offset;
	}

	void advance_item() {
		advance_items(1);
	}

public:
	bool operator==(const stream_position & other) const {
		return m_readOffset == other.m_readOffset
			&& m_blockNumber == other.m_blockNumber
			&& m_offset == other.m_offset
			&& m_blockItemIndex == other.m_blockItemIndex;
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
