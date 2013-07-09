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

#ifndef TPIE_COMPRESSED_BUFFER_H
#define TPIE_COMPRESSED_BUFFER_H

///////////////////////////////////////////////////////////////////////////////
/// \file compressed/buffer.h
///////////////////////////////////////////////////////////////////////////////

#include <tpie/array.h>

namespace tpie {

class compressor_buffer {
private:
	typedef array<char> storage_t;

	storage_t m_storage;
	memory_size_type m_size;

public:
	compressor_buffer(memory_size_type capacity)
		: m_storage(capacity)
		, m_size(0)
	{
	}

	~compressor_buffer() {
	}

	char * get() {
		return m_storage.get();
	}

	const char * get() const {
		return m_storage.get();
	}

	memory_size_type size() const {
		return m_size;
	}

	memory_size_type capacity() const {
		return m_storage.size();
	}

	void set_size(memory_size_type size) {
		m_size = size;
	}

	void set_capacity(memory_size_type capacity) {
		m_storage.resize(capacity);
		m_size = 0;
	}
};

class stream_buffers {
public:
	typedef boost::shared_ptr<compressor_buffer> buffer_t;

	const static memory_size_type MAX_BUFFERS = 3;

	stream_buffers(memory_size_type blockSize)
		: m_bufferCount(0)
		, m_blockSize(blockSize)
	{
	}

	///////////////////////////////////////////////////////////////////////////
	/// Exception guarantee: nothrow
	///////////////////////////////////////////////////////////////////////////
	buffer_t get_buffer(compressor_thread_lock & lock, stream_size_type blockNumber) {
		if (m_buffers.size() >= MAX_BUFFERS) {
			// First, search for the buffer in the map.
			buffermapit target = m_buffers.find(blockNumber);
			if (target != m_buffers.end()) return target->second;

			// If not found, wait for a free buffer to become available.
			buffer_t b;
			while (true) {
				buffermapit i = m_buffers.begin();
				while (i != m_buffers.end() && !i->second.unique()) ++i;
				if (i == m_buffers.end()) {
					compressor().wait_for_request_done(lock);
					continue;
				} else {
					b.swap(i->second);
					m_buffers.erase(i);
					break;
				}
			}

			m_buffers.insert(std::make_pair(blockNumber, b));
			return b;
		} else {
			// First, search for the buffer in the map.
			std::pair<buffermapit, bool> res
				= m_buffers.insert(std::make_pair(blockNumber, buffer_t()));
			buffermapit & target = res.first;
			bool & inserted = res.second;
			if (!inserted) return target->second;

			// If not found, find a free buffer and place it in target->second.

			// target->second is the only buffer in the map with use_count() == 0.
			// If a buffer in the map has use_count() == 1 (that is, unique() == true),
			// that means only our map (and nobody else) refers to the buffer,
			// so it is free to be reused.
			buffermapit i = m_buffers.begin();
			while (i != m_buffers.end() && !i->second.unique()) ++i;

			if (i == m_buffers.end()) {
				// No free found: allocate new buffer.
				target->second.reset(new buffer_t::element_type(block_size()));
				++m_bufferCount;
			} else {
				// Free found: reuse buffer.
				target->second.swap(i->second);
				m_buffers.erase(i);
			}

			return target->second;
		}
	}

	bool empty() const {
		return m_buffers.empty();
	}

	void clean() {
		buffermapit i = m_buffers.begin();
		while (i != m_buffers.end()) {
			buffermapit j = i++;
			if (j->second.unique() || j->second == buffer_t()) {
				m_buffers.erase(j);
			}
		}
	}

private:
	compressor_thread & compressor() {
		return the_compressor_thread();
	}

	memory_size_type block_size() const {
		return m_blockSize;
	}

	memory_size_type m_bufferCount;
	memory_size_type m_blockSize;

	typedef std::map<stream_size_type, buffer_t> buffermap_t;
	typedef buffermap_t::iterator buffermapit;
	buffermap_t m_buffers;
};

} // namespace tpie

#endif // TPIE_COMPRESSED_BUFFER_H
