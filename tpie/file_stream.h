// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, 2011, The TPIE development team
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
#ifndef __TPIE_FILE_STREAM_H__
#define __TPIE_FILE_STREAM_H__
#include <tpie/tempname.h>
#include <tpie/file.h>
#include <tpie/memory.h>
#include <tpie/file_base.h>
///////////////////////////////////////////////////////////////////////////////
/// \file file_stream.h
/// \brief Simple class acting both as a tpie::file and a
/// tpie::file::stream.
///////////////////////////////////////////////////////////////////////////////

namespace tpie {


///////////////////////////////////////////////////////////////////////////////
/// \brief Simple class acting both as \ref file and a file::stream.
///
/// A file_stream basically supports every operation a \ref file or a
/// file::stream supports. This is used to access a file I/O-efficiently, and
/// is the direct replacement of the old ami::stream.
///
/// \tparam T The type of items stored in the stream.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class file_stream: public file_stream_base {
public:
	/** The type of the items stored in the stream */
	typedef T item_type;

	/////////////////////////////////////////////////////////////////////////
	/// \brief Construct a new file_stream.
	/// 
	/// \copydetails tpie::file::file(double blockFactor, file_accessor::file_accessor * fileAccessor)
	/////////////////////////////////////////////////////////////////////////
	inline file_stream(double blockFactor=1.0, 
					   file_accessor::file_accessor * fileAccessor=NULL):
		file_stream_base(blockFactor, fileAccessor, sizeof(item_type) ) {};

	
	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file<T>::stream::write(const item_type & item)
	/// \sa file<T>::stream::write(const item_type & item)
	/////////////////////////////////////////////////////////////////////////
	inline void write(const item_type & item) throw(stream_exception) {
		assert(m_open);
#ifndef NDEBUG
		if (!is_writable())
			throw io_exception("Cannot write to read only stream");
#endif
		if (m_index >= m_blockItems) update_block();
		reinterpret_cast<item_type*>(m_block.data)[m_index++] = item;
		write_update();
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file<T>::stream::write(const IT & start, const IT & end)
	/// \sa file<T>::stream::write(const IT & start, const IT & end)
	/////////////////////////////////////////////////////////////////////////
	template <typename IT>
	inline void write(const IT & start, const IT & end) throw(stream_exception) {
		assert(m_open);
		IT i = start;
		while (i != end) {
			if (m_index >= m_blockItems) update_block();

			size_t streamRemaining = end - i;
			size_t blockRemaining = m_blockItems-m_index;

			IT till = (blockRemaining < streamRemaining) ? (i + blockRemaining) : end;

			T * dest = reinterpret_cast<item_type*>(m_block.data) + m_index;

			std::copy(i, till, dest);

			m_index += static_cast<memory_size_type>(till - i);
			write_update();
			i = till;
		}
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file<T>::stream::read()
	/// \sa file<T>::stream::read()
	/////////////////////////////////////////////////////////////////////////
	inline const item_type & read() throw(stream_exception) {
		assert(m_open);
		if (m_index >= m_block.size) {
			update_block();
			if (offset() >= size()) {
				throw end_of_stream_exception();
			}
		}
		return reinterpret_cast<item_type*>(m_block.data)[m_index++];
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file<T>::stream::read(const IT & start, const IT & end)
	/// \sa file<T>::stream::read(const IT & start, const IT & end)
	/////////////////////////////////////////////////////////////////////////
	template <typename IT>
	inline void read(const IT & start, const IT & end) throw(stream_exception) {
		assert(m_open);
		IT i = start;
		while (i != end) {
			if (m_index >= m_blockItems) {
				// check to make sure we have enough items in the stream
				stream_size_type offs = offset();
				if (offs >= size()
					|| offs + (end-i) > size()) {

					throw end_of_stream_exception();
				}

				// fetch next block from disk
				update_block();
			}

			T * src = reinterpret_cast<item_type*>(m_block.data) + m_index;

			// either read the rest of the block or until `end'
			memory_size_type count = std::min(m_blockItems-m_index, static_cast<memory_size_type>(end-i));

			std::copy(src, src + count, i);

			// advance output iterator
			i += count;

			// advance input position
			m_index += count;
		}
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file<T>::stream::read_back()
	/// \sa file<T>::stream::read_back()
	/////////////////////////////////////////////////////////////////////////
	inline const item_type & read_back() throw(stream_exception) {
		assert(m_open);
		seek(-1, file_base::current);
		const item_type & i = read();
		seek(-1, file_base::current);
		return i;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate the amount of memory used by a single file_stream.
	///
	/// \param blockFactor The block factor you pass to open.
	/// \param includeDefaultFileAccessor Unless you are supplying your own
	/// file accessor to open, leave this to be true.
	/// \returns The amount of memory maximally used by the count file_streams.
	///////////////////////////////////////////////////////////////////////////
	inline static memory_size_type memory_usage(
		float blockFactor=1.0,
		bool includeDefaultFileAccessor=true) throw() {
		// TODO
		memory_size_type x = sizeof(file_stream);
		x += block_memory_usage(blockFactor); // allocated in constructor
		if (includeDefaultFileAccessor)
			x += default_file_accessor::memory_usage();
		return x;
	}

	void swap(file_stream<T> & other) {
		file_stream_base::swap(other);
	}
};

} // namespace tpie

namespace std {

///////////////////////////////////////////////////////////////////////////////
/// \brief Enable std::swapping two tpie::file_streams.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
void swap(tpie::file_stream<T> & a, tpie::file_stream<T> & b) {
	a.swap(b);
}

} // namespace std

#endif //__TPIE_FILE_STREAM_H__
