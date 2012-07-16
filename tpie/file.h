// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, 2010, 2012, The TPIE development team
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
#ifndef _TPIE_FILE_H
#define _TPIE_FILE_H

///////////////////////////////////////////////////////////////////////////////
/// \file file.h Streams that support substreams.
///////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
//Yes we know you do not support throw(stream_exception)
#pragma warning( disable: 4290 ) 
#endif //_MSC_VER

#include <limits>
#include <tpie/file_base.h>
namespace tpie {


#ifdef _MSC_VER
#pragma warning( disable: 4200 )
#endif //_MSC_VER

///////////////////////////////////////////////////////////////////////////////
/// \brief Central file abstraction.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class file: public file_base {
public:
	/** Type of items stored in the file. */
 	typedef T item_type;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate the memory usage of a file.
	///////////////////////////////////////////////////////////////////////////
	static inline memory_size_type memory_usage(bool includeDefaultFileAccessor=true) {
		memory_size_type x = sizeof(file);
		if (includeDefaultFileAccessor)
			x += default_file_accessor::memory_usage();
		return x;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct a file object with the given block factor and file
	/// accessor.
	/// \param blockFactor The relative size of a block compared to the 
	/// default. To find the block factor corresponding to an absolute
	/// block size, use file_base::calculate_block_factor.
	/// \param fileAccessor The file accessor to use, if none is supplied a
	/// default will be used.
	///////////////////////////////////////////////////////////////////////////
	file(double blockFactor=1.0,
		 file_accessor::file_accessor * fileAccessor=NULL):
		file_base(sizeof(T), blockFactor, fileAccessor) {};

	///////////////////////////////////////////////////////////////////////////
	/// \brief Central stream abstraction. Conceptually compatible with
	/// \ref file_stream.
	///////////////////////////////////////////////////////////////////////////
 	class stream: public file_base::stream {
	public:
		/** Type of items stored in the stream. */
		typedef T item_type;
		/** Type of underlying file object. */
		typedef file file_type;
	private:
		/** Type of block. */
		typedef typename file::block_t block_t;
	public:
		///////////////////////////////////////////////////////////////////////
		/// \brief Calculate the memory usage of a stream.
		///////////////////////////////////////////////////////////////////////
		inline static memory_size_type memory_usage(double blockFactor=1.0) {
			return sizeof(stream) + block_size(blockFactor) +  sizeof(block_t);
		}

		stream(file_type & file, stream_size_type offset=0):
			file_base::stream(file, offset) {}


		///////////////////////////////////////////////////////////////////////
		/// \brief Read a mutable item from the stream.
		///
		/// Don't use this method. Instead, use \ref file<T>::stream::read().
		///
		/// \copydetails file<T>::stream::read()
		///////////////////////////////////////////////////////////////////////
 		inline item_type & read_mutable() {
			assert(m_file.m_open);
			if (m_index >= m_block->size) {
				update_block();
				if (offset() >= m_file.size()) {
					throw end_of_stream_exception();
				}
			}
			return reinterpret_cast<T*>(m_block->data)[m_index++];
		}

		///////////////////////////////////////////////////////////////////////
		/// \brief Read an item from the stream.
		///
		/// Read current item from the stream, and increment the offset by one
		/// item.
		///
		/// This will throw an end_of_stream_exception if there are no more
		/// items left in the stream.
		///
		/// To ensure that no exception is thrown, check that can_read()
		/// returns true.
		///
		/// \returns The item read from the stream.
		///////////////////////////////////////////////////////////////////////
 		inline const item_type & read() {
			return read_mutable();
		}

		///////////////////////////////////////////////////////////////////////
		/// \brief Read an item from the stream.
		///
		/// Decrement the offset by one, and read current item from the stream.
		///
		/// This will throw an end_of_stream_exception if there are no more
		/// items left in the stream.
		///
		/// To ensure that no exception is thrown, check that can_read_back()
		/// returns true.
		///
		/// \returns The item read from the stream.
		///////////////////////////////////////////////////////////////////////
		inline const item_type & read_back() {
			assert(m_file.m_open);
			seek(-1, current);
			const item_type & i = read();
			seek(-1, current);
			return i;
		}

		/////////////////////////////////////////////////////////////////////////
		/// \brief Write an item to the stream.
		///
		/// \param item The item to write to the stream.
		/////////////////////////////////////////////////////////////////////////
 		inline void write(const item_type& item) throw(stream_exception) {
			assert(m_file.m_open);
#ifndef NDEBUG
			if (!m_file.is_writable())
				throw io_exception("Cannot write to read only stream");
#endif
			if (m_index >= block_items()) update_block();
			reinterpret_cast<T*>(m_block->data)[m_index++] = item;
			write_update();
		}

		/////////////////////////////////////////////////////////////////////////
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
		/////////////////////////////////////////////////////////////////////////
		template <typename IT>
		inline void write(const IT & start, const IT & end) {
			assert(m_file.m_open);
			IT i = start;
			while (i != end) {
				if (m_index >= block_items()) update_block();

				IT blockmax = i + (block_items()-m_index);

				T * dest = reinterpret_cast<T*>(m_block->data) + m_index;

				IT till = std::min(end, blockmax);

				std::copy(i, till, dest);

				m_index += till - i;
				write_update();
				i = till;
			}
		}

		///////////////////////////////////////////////////////////////////////
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
		///////////////////////////////////////////////////////////////////////
		template <typename IT>
		inline void read(const IT & start, const IT & end) {
			assert(m_file.m_open);
			IT i = start;
			while (i != end) {
				if (m_index >= block_items()) {
					// check to make sure we have enough items in the stream
					stream_size_type offs = offset();
					if (offs >= m_file.size()
						|| offs + (end-i) > m_file.size()) {

						throw end_of_stream_exception();
					}

					// fetch next block from disk
					update_block();
				}

				T * src = reinterpret_cast<T*>(m_block->data) + m_index;

				// either read the rest of the block or until `end'
				memory_size_type count = std::min(block_items()-m_index, static_cast<memory_size_type>(end-i));

				std::copy(src, src + count, i);

				// advance output iterator
				i += count;

				// advance input position
				m_index += count;
			}
		}
 	};
};
}
#endif //_TPIE_FILE_H
