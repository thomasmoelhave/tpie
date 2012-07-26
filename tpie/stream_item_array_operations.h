// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012 The TPIE development team
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
/// \file stream_item_array_operations.h  Stream read() and write() on pairs of
/// input iterators
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_STREAM_ITEM_ARRAY_OPERATIONS_H__
#define __TPIE_STREAM_ITEM_ARRAY_OPERATIONS_H__

#include <tpie/exception.h>

namespace tpie {

struct stream_item_array_operations {
	///////////////////////////////////////////////////////////////////////////
	/// \brief Reads several items from the stream.
	///
	/// Implementation note: If your iterator type is efficiently copyable
	/// with std::copy, then this will also read efficiently from the
	/// internal TPIE buffer.
	///
	/// \tparam IT The type of Random Access Iterators used to supply the
	/// items.
	/// \param start Iterator to the first spot to write to.
	/// \param end Iterator past the last spot to write to.
	///
	/// \throws end_of_stream_exception If there are not enough elements in
	/// the stream to fill all the spots between start and end.
	///////////////////////////////////////////////////////////////////////////
	template <typename T, typename IT, typename Stream>
	static inline void read(Stream & stream, const IT & start, const IT & end) throw(stream_exception) {
		IT i = start;
		while (i != end) {
			if (stream.m_index >= stream.block_items()) {
				// check to make sure we have enough items in the stream
				stream_size_type offs = stream.offset();
				if (offs >= stream.size()
					|| offs + (end-i) > stream.size()) {

					throw end_of_stream_exception();
				}

				// fetch next block from disk
				stream.update_block();
			}

			T * src = reinterpret_cast<T*>(stream.__block().data) + stream.m_index;

			// either read the rest of the block or until `end'
			memory_size_type count = std::min(stream.block_items()-stream.m_index, static_cast<memory_size_type>(end-i));

			std::copy(src, src + count, i);

			// advance output iterator
			i += count;

			// advance input position
			stream.m_index += count;
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	/// \brief Write several items to the stream.
	///
	/// Implementation note: If your iterator type is efficiently copyable
	/// with std::copy, then this will also write efficiently into the
	/// internal TPIE buffer.
	///
	/// \tparam IT The type of Random Access Iterators used to supply the
	/// items.
	/// \param start Iterator to the first item to write.
	/// \param end Iterator past the last item to write.
	/////////////////////////////////////////////////////////////////////////////
	template <typename T, typename IT, typename Stream>
	static inline void write(Stream & stream, const IT & start, const IT & end) throw(stream_exception) {
		IT i = start;
		while (i != end) {
			if (stream.m_index >= stream.block_items()) stream.update_block();

			size_t streamRemaining = end - i;
			size_t blockRemaining = stream.block_items()-stream.m_index;

			IT till = (blockRemaining < streamRemaining) ? (i + blockRemaining) : end;

			T * dest = reinterpret_cast<T*>(stream.__block().data) + stream.m_index;

			std::copy(i, till, dest);

			stream.m_index += till - i;
			stream.write_update();
			i = till;
		}
	}
};

} // namespace tpie

#endif // __TPIE_STREAM_ITEM_ARRAY_OPERATIONS_H__
