// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2014, The TPIE development team
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
/// \file block_collection Class to handle blocks of varying size on disk
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_BLOCKS_BLOCK_COLLECTION_H
#define _TPIE_BLOCKS_BLOCK_COLLECTION_H

#include <tpie/tpie.h>
#include <tpie/tpie_assert.h>
#include <tpie/file_accessor/file_accessor.h>
#include <tpie/blocks/block.h>
#include <tpie/blocks/freespace_collection.h>

namespace tpie {

namespace blocks {

/**
 * \brief A class to manage writing and reading of block to disk.
 */
class block_collection {
public:
	/**
	 * \brief Create a new non-open block collection
	 */
	block_collection()
	: m_open(false)
	, m_writeable(false)
	{}

	/**
	 * \brief Create a block collection
	 * \param fileName the file in which blocks are saved
	 * \param indicates whether the collection is readable
	 */
	block_collection(std::string fileName, bool writeable)
	: m_open(false)
	, m_writeable(false)
	{
		open(fileName, writeable);
	}

	~block_collection() {
		close();
	}

	/**
	 * \brief Returns whether the colleciton is open or not
	 */
	bool is_open() const {
		return m_open;
	}

	/**
	 * \brief Opens the block collection. If the collection is already open, it will first be closed.
	 * \param fileName the file in which blocks are saved
	 * \param indicates whether the collection is readable
	 */
	void open(std::string fileName, bool writeable) {
		close();

		if(writeable)
			m_accessor.open_rw_new(fileName);
		else
			m_accessor.open_ro(fileName);

		if(m_accessor.file_size_i() == 0) {
			m_collection.initial_configuration();
		}
		else {
			stream_size_type data_size;
			m_accessor.read_i(static_cast<void*>(&data_size), sizeof(stream_size_type));

			stream_size_type configuration_size = m_accessor.file_size_i() - sizeof(stream_size_type) - data_size;
			block configuration(configuration_size);

			m_accessor.seek_i(sizeof(stream_size_type) + data_size);
			m_accessor.read_i(static_cast<void*>(configuration.get()), configuration_size);
			m_collection.set_configuration(configuration);
		}

		m_writeable = writeable;
		m_open = true;
	}

	/**
	 * \brief Closes the block collection
	 */
	void close() {
		if(m_open) {
			stream_size_type data_size = m_collection.used_space();
			block configuration;
			m_collection.get_configuration(configuration);

			m_accessor.seek_i(0);
			m_accessor.write_i(static_cast<void*>(&data_size), sizeof(stream_size_type));
			m_accessor.seek_i(sizeof(stream_size_type) + data_size);
			m_accessor.write_i(static_cast<void*>(configuration.get()), configuration.size());

			m_accessor.close_i();

			m_open = false;
		}
	}

	/**
	 * \brief Allocates a new block
	 * \param size the minimum size needed given in bytes
	 * \return the handle of the new block
	 */
	block_handle get_free_block(stream_size_type size) {
		tp_assert(is_open(), "get_free_block(): the block collection is not open");
		tp_assert(m_writeable, "get_free_block(): the block collection is read only");

		return m_collection.alloc(size);
	}

	/**
	 * \brief frees a block
	 * \param handle the handle of the block to be freed
	 */
	void free_block(block_handle handle) {
		tp_assert(is_open(), "free_block(): the block collection is not open");
		tp_assert(m_writeable, "free_block(): the block collection is read only");

		m_collection.free(handle);

		if(m_accessor.file_size_i() > m_collection.used_space() + sizeof(stream_size_type)) {
			m_accessor.truncate_i(m_collection.used_space() + sizeof(stream_size_type));
		}
	}

	/**
	 * \brief Reads the content of a block from disk
	 * \param handle the handle of the block to read
	 * \b the block to store the content in
	 */
	void read_block(block_handle handle, block & b) {
		tp_assert(is_open(), "read_block(): the block collection is not open");

		b.resize(handle.size);

		m_accessor.seek_i(handle.position + sizeof(stream_size_type));
		m_accessor.read_i(static_cast<void*>(b.get()), handle.size);
	}

	/**
	 * \brief Writes the content of a block to disk
	 * \param handle the handle of the block to write
	 * \param b the block type in which the content is stored
	 */
	void write_block(block_handle handle, const block & b) {
		tp_assert(is_open(), "write_block(): the block collection is not open");
		tp_assert(m_writeable, "write_block(): the block collection is read only.");
		tp_assert(handle.size >= b.size(), "the given block is not large enough.");

		m_accessor.seek_i(handle.position + sizeof(stream_size_type));
		m_accessor.write_i(static_cast<const void*>(b.get()), b.size());
	}
private:
	bits::freespace_collection m_collection;
	tpie::file_accessor::raw_file_accessor m_accessor;

	bool m_open;
	bool m_writeable;
};

} // blocks namespace

}  //  tpie namespace

#endif // _TPIE_BLOCKS_BLOCK_COLLECTION_H
