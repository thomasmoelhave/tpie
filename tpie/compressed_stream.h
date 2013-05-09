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

#ifndef TPIE_COMPRESSED_STREAM_H
#define TPIE_COMPRESSED_STREAM_H

///////////////////////////////////////////////////////////////////////////////
/// \file compressed_stream.h
///////////////////////////////////////////////////////////////////////////////

#include <snappy.h>
#include <tpie/array.h>
#include <tpie/tempname.h>
#include <tpie/file_base_crtp.h>
#include <tpie/file_stream_base.h>
#include <tpie/file_accessor/byte_stream_accessor.h>

namespace tpie {

class byte_stream_accessor_holder {
protected:
	typedef tpie::file_accessor::byte_stream_accessor<tpie::default_raw_file_accessor> byte_stream_accessor_t;
	byte_stream_accessor_t * m_byteStreamAccessor;

	byte_stream_accessor_holder(file_accessor::file_accessor *& fileAccessor)
		: m_byteStreamAccessor(new byte_stream_accessor_t())
	{
		if (fileAccessor != NULL) {
			log_warning() << "Non-null file accessor supplied to compressed_stream;"
				<< " ignored." << std::endl;
		}
		fileAccessor = m_byteStreamAccessor;
	}

	~byte_stream_accessor_holder() {
		delete m_byteStreamAccessor;
	}
};

class compressed_stream_base : public file_base_crtp<compressed_stream_base> {
	typedef file_base_crtp<compressed_stream_base> p_t;
	friend class file_base_crtp<compressed_stream_base>;

protected:

	struct seek_state {
		enum type {
			none,
			beginning,
			end
		};
	};

	struct buffer_state {
		enum type {
			write_only,
			read_only
		};
	};

	compressed_stream_base(memory_size_type itemSize,
						   double blockFactor,
						   file_accessor::file_accessor * fileAccessor)
		: file_base_crtp(itemSize, blockFactor, fileAccessor)
		, m_bufferDirty(false)
	{
	}

	virtual void flush_block() = 0;

	void close() {
		if (m_bufferDirty) {
			flush_block();
		}
		p_t::close();
	}

	virtual void post_open() = 0;

	void open_inner(const std::string & path,
					access_type accessType,
					memory_size_type userDataSize,
					cache_hint cacheHint)
	{
		p_t::open_inner(path, accessType, userDataSize, cacheHint);

		this->post_open();
	}

	bool m_bufferDirty;

	stream_size_type size() const;

};

namespace ami {
	template <typename T> class cstream;
}

template <typename T>
class compressed_stream :
	public byte_stream_accessor_holder,
	public compressed_stream_base
{
	using compressed_stream_base::seek_state;
	using compressed_stream_base::buffer_state;

	friend class ami::cstream<T>;

	static const file_stream_base::offset_type beginning = file_stream_base::beginning;
	static const file_stream_base::offset_type end = file_stream_base::end;

public:
	typedef T item_type;
	typedef file_stream_base::offset_type offset_type;

	compressed_stream(double blockFactor=1.0,
					  file_accessor::file_accessor * fileAccessor=NULL)
		: byte_stream_accessor_holder(fileAccessor)
		, compressed_stream_base(sizeof(T), blockFactor, fileAccessor)
		, m_seekState(seek_state::beginning)
		, m_bufferState(buffer_state::write_only)
		, m_offset(0)
	{
	}

	~compressed_stream() {
		this->close();
	}

	virtual void post_open() override {
		m_buffer.resize(this->block_size());
		m_nextItem = m_buffer.begin();
		m_lastItem = m_buffer.begin();

		seek(0);
	}

	void seek(stream_offset_type offset, offset_type whence=beginning) {
		assert(this->is_open());
		if (whence == beginning && offset == 0) {
			m_seekState = seek_state::beginning;
			if (m_fileAccessor->size() > 0)
				m_bufferState = buffer_state::read_only;
			else
				m_bufferState = buffer_state::write_only;
			m_offset = 0;
		} else if (whence == end && offset == 0) {
			m_seekState = seek_state::end;
			m_bufferState = buffer_state::write_only;
			m_offset = size();
		} else {
			throw stream_exception("Random seeks are not supported");
		}
	}

	stream_size_type offset() const {
		return m_offset;
	}

	stream_size_type size() const {
		return m_fileAccessor->size();
	}

	void truncate(stream_size_type offset) {
		if (offset == size()) return;
		if (offset != 0)
			throw stream_exception("Arbitrary truncate is not supported");
		m_fileAccessor->truncate(0);
		seek(0);
	}

private:
	const T & read_ref() {
		if (m_seekState != seek_state::none) perform_seek();
		if (!can_read()) throw stream_exception("!can_read()");
		if (m_nextItem == m_lastItem) read_next_block();
		return *m_nextItem++;
	}

public:
	T read() {
		return read_ref();
	}

	void read(T * const a, T * const b) {
		for (T * i = a; i != b; ++i) *i = read();
	}

	bool can_read() {
		if (!(this->m_open
			  && m_bufferState == buffer_state::read_only
			  && m_buffer.size() != 0))
			return false;

		if (m_nextItem != m_lastItem)
			return true;

		if (m_nextBlockSize != 0)
			return true;

		if ((m_nextReadOffset == 0 || m_seekState == seek_state::beginning)
			&& m_fileAccessor->size() > 0)
			return true;

		return false;
	}

	void write(const T & item) {
		if (m_seekState != seek_state::none) perform_seek();
		if (m_bufferState != buffer_state::write_only)
			throw stream_exception("Non-appending write attempted");
		if (m_nextItem == m_buffer.end()) {
			flush_block();
			m_nextItem = m_buffer.begin();
		}
		*m_nextItem++ = item;
		this->m_bufferDirty = true;
	}

	void write(const T * const a, const T * const b) {
		for (const T * i = a; i != b; ++i) write(*i);
	}

protected:
	void perform_seek() {
		if (m_seekState == seek_state::none)
			return;

		if (this->m_bufferDirty) {
			flush_block();
		}

		if (m_seekState == seek_state::beginning
			&& !m_byteStreamAccessor->empty())
		{
			m_nextReadOffset = 0;
			read_next_block();
			m_bufferState = buffer_state::read_only;
		} else {
			m_bufferState = buffer_state::write_only;
		}
		m_nextItem = m_buffer.begin();
		m_seekState = seek_state::none;
	}

	virtual void flush_block() override {
		memory_size_type blockItems = m_nextItem - m_buffer.begin();
		size_t inputLength = sizeof(T) * blockItems;
		memory_size_type blockSize = snappy::MaxCompressedLength(inputLength);
		array<char> scratch(sizeof(blockSize) + blockSize);
		snappy::RawCompress(reinterpret_cast<const char *>(m_buffer.get()),
							inputLength,
							scratch.get() + sizeof(blockSize),
							&blockSize);
		*reinterpret_cast<memory_size_type *>(scratch.get()) = blockSize;
		m_byteStreamAccessor->append(scratch.get(), sizeof(blockSize) + blockSize);
		m_byteStreamAccessor->increase_size(blockItems);
		this->m_bufferDirty = false;
	}

	void read_next_block() {
		if (m_nextReadOffset == 0) {
			m_byteStreamAccessor->read(0, &m_nextBlockSize, sizeof(m_nextBlockSize));
			m_nextReadOffset += sizeof(m_nextBlockSize);
			if (m_nextBlockSize == 0)
				throw stream_exception("Internal error; the block size was unexpectedly zero");
		} else if (m_nextBlockSize == 0) {
			throw end_of_stream_exception();
		}
		array<char> scratch(m_nextBlockSize + sizeof(m_nextBlockSize));
		memory_size_type nRead = m_byteStreamAccessor->read(m_nextReadOffset, scratch.get(), scratch.size());
		if (nRead == scratch.size()) {
			m_nextReadOffset += scratch.size();
			m_nextBlockSize = *(reinterpret_cast<memory_size_type *>(scratch.get() + scratch.size())-1);
		} else if (nRead == scratch.size() - sizeof(m_nextBlockSize)) {
			m_nextReadOffset += m_nextBlockSize;
			m_nextBlockSize = 0;
		}
		size_t uncompressedLength;
		if (!snappy::GetUncompressedLength(scratch.get(),
										   scratch.size() - sizeof(m_nextBlockSize),
										   &uncompressedLength))
			throw stream_exception("Internal error; snappy::GetUncompressedLength failed");
		if (uncompressedLength > m_buffer.size() * sizeof(T))
			throw stream_exception("Internal error; snappy::GetUncompressedLength exceeds the block size");
		snappy::RawUncompress(scratch.get(),
							  scratch.size() - sizeof(m_nextBlockSize),
							  reinterpret_cast<char *>(m_buffer.get()));
		m_nextItem = m_buffer.begin();
		m_lastItem = m_nextItem + (uncompressedLength / sizeof(T));
		m_bufferState = buffer_state::read_only;
	}

private:
	seek_state::type m_seekState;

	buffer_state::type m_bufferState;

	array<T> m_buffer;

	/** Next item in buffer to read/write. */
	typename array<T>::iterator m_nextItem;
	/** In read mode only: End of readable buffer. */
	typename array<T>::iterator m_lastItem;

	/** If nextReadOffset is zero, the next block to read is the first block and its size is not known.
	 * In that case, the size of the first block is the first eight bytes, and the first block begins
	 * after those eight bytes.
	 * If nextReadOffset and nextBlockSize are both non-zero, the next block begins at the given offset
	 * and has the given size.
	 * Otherwise, if nextReadOffset is non-zero and nextBlockSize is zero, we have reached the end of
	 * the stream.
	 */
	stream_size_type m_nextReadOffset;
	stream_size_type m_nextBlockSize;

	stream_size_type m_offset;
};

} // namespace tpie

#endif // TPIE_COMPRESSED_STREAM_H
