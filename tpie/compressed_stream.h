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
#include <tpie/compressor_thread.h>

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

class stream_buffers {
public:
	typedef boost::shared_ptr<compressor_buffer> buffer_t;

	const static memory_size_type MAX_BUFFERS = 3;

	stream_buffers(memory_size_type blockSize)
		: m_bufferCount(0)
		, m_blockSize(blockSize)
	{
	}

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

class compressed_stream_base {
public:
	typedef boost::shared_ptr<compressor_buffer> buffer_t;

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
						   double blockFactor)
		: m_bufferDirty(false)
		, m_blockItems(block_size(blockFactor) / itemSize)
		, m_blockSize(block_size(blockFactor))
		, m_canRead(false)
		, m_canWrite(false)
		, m_open(false)
		, m_itemSize(itemSize)
		, m_ownedTempFile()
		, m_tempFile(0)
		, m_size(0)
		, m_buffers(m_blockSize)
	{
	}

	virtual void flush_block() = 0;

	virtual void post_open() = 0;

	void open_inner(const std::string & path,
					access_type accessType,
					memory_size_type userDataSize,
					cache_hint cacheHint)
	{
		if (userDataSize != 0)
			throw stream_exception("Compressed stream does not support user data");

		m_canRead = accessType == access_read || accessType == access_read_write;
		m_canWrite = accessType == access_write || accessType == access_read_write;
		m_byteStreamAccessor.open(path, m_canRead, m_canWrite, m_itemSize, m_blockSize, userDataSize, cacheHint);
		m_size = m_byteStreamAccessor.size();
		m_open = true;

		this->post_open();
	}

	stream_size_type size() const;

	compressor_thread & compressor() {
		return the_compressor_thread();
	}

public:
	bool is_readable() const throw() {
		return m_canRead;
	}

	bool is_writable() const throw() {
		return m_canWrite;
	}

	static memory_size_type block_size(double blockFactor) throw () {
		return static_cast<memory_size_type>(get_block_size() * blockFactor);
	}

	static double calculate_block_factor(memory_size_type blockSize) throw () {
		return (double)blockSize / (double)block_size(1.0);
	}

	static memory_size_type block_memory_usage(double blockFactor) {
		return block_size(blockFactor);
	}

	memory_size_type block_items() const {
		return m_blockItems;
	}

	memory_size_type block_size() const {
		return m_blockSize;
	}

	template <typename TT>
	void read_user_data(TT & /*data*/) {
		throw stream_exception("Compressed stream does not support user data");
	}

	memory_size_type read_user_data(void * /*data*/, memory_size_type /*count*/) {
		throw stream_exception("Compressed stream does not support user data");
	}

	template <typename TT>
	void write_user_data(const TT & /*data*/) {
		throw stream_exception("Compressed stream does not support user data");
	}

	void write_user_data(const void * /*data*/, memory_size_type /*count*/) {
		throw stream_exception("Compressed stream does not support user data");
	}

	memory_size_type user_data_size() const {
		return 0;
	}

	memory_size_type max_user_data_size() const {
		return 0;
	}

	const std::string & path() const {
		assert(m_open);
		return m_byteStreamAccessor.path();
	}

	void open(const std::string & path,
			  access_type accessType = access_read_write,
			  memory_size_type userDataSize = 0,
			  cache_hint cacheHint=access_sequential)
	{
		close();
		open_inner(path, accessType, userDataSize, cacheHint);
	}

	void open(memory_size_type userDataSize = 0,
			  cache_hint cacheHint = access_sequential)
	{
		close();
		m_ownedTempFile.reset(tpie_new<temp_file>());
		m_tempFile = m_ownedTempFile.get();
		open_inner(m_tempFile->path(), access_read_write, userDataSize, cacheHint);
	}

	void open(temp_file & file,
			  access_type accessType = access_read_write,
			  memory_size_type userDataSize = 0,
			  cache_hint cacheHint = access_sequential)
	{
		close();
		m_tempFile = &file;
		open_inner(m_tempFile->path(), accessType, userDataSize, cacheHint);
	}

	void close() {
		if (m_open) {
			if (m_bufferDirty) {
				flush_block();
			}
			m_buffer.reset();

			compressor_thread_lock l(compressor());
			finish_requests(l);

			m_byteStreamAccessor.close();
		}
		m_open = false;
		m_tempFile = NULL;
		m_ownedTempFile.reset();
	}

protected:
	void finish_requests(compressor_thread_lock & l) {
		m_buffers.clean();
		while (!m_buffers.empty()) {
			compressor().wait_for_request_done(l);
			m_buffers.clean();
		}
	}

public:
	bool is_open() const {
		return m_open;
	}

	stream_size_type file_size() const {
		return m_size;
	}

protected:
	bool m_bufferDirty;
	memory_size_type m_blockItems;
	memory_size_type m_blockSize;
	bool m_canRead;
	bool m_canWrite;
	bool m_open;
	memory_size_type m_itemSize;
	file_accessor::byte_stream_accessor<default_raw_file_accessor> m_byteStreamAccessor;
	tpie::auto_ptr<temp_file> m_ownedTempFile;
	temp_file * m_tempFile;
	stream_size_type m_size;
	buffer_t m_buffer;
	stream_buffers m_buffers;
};

namespace ami {
	template <typename T> class cstream;
}

template <typename T>
class compressed_stream : public compressed_stream_base {
	using compressed_stream_base::seek_state;
	using compressed_stream_base::buffer_state;

	friend class ami::cstream<T>;

	static const file_stream_base::offset_type beginning = file_stream_base::beginning;
	static const file_stream_base::offset_type end = file_stream_base::end;

public:
	typedef T item_type;
	typedef file_stream_base::offset_type offset_type;

	compressed_stream(double blockFactor=1.0)
		: compressed_stream_base(sizeof(T), blockFactor)
		, m_seekState(seek_state::beginning)
		, m_bufferState(buffer_state::write_only)
		, m_nextReadOffset(0)
		, m_nextBlockSize(0)
		, m_offset(0)
		, m_streamBlocks(0)
	{
	}

	~compressed_stream() {
		this->close();
	}

	virtual void post_open() override {
		seek(0);
	}

	void seek(stream_offset_type offset, offset_type whence=beginning) {
		assert(this->is_open());
		if (whence == beginning && offset == 0) {
			m_seekState = seek_state::beginning;
			if (size() > 0)
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
		stream_size_type sz = m_byteStreamAccessor.size();
		if (m_bufferDirty) sz += (m_nextItem - m_bufferBegin);
		return sz;
	}

	void truncate(stream_size_type offset) {
		if (offset == size()) return;
		if (offset != 0)
			throw stream_exception("Arbitrary truncate is not supported");

		// No need to flush block
		m_buffer.reset();
		compressor_thread_lock l(compressor());
		finish_requests(l);

		m_byteStreamAccessor.truncate(0);
		seek(0);
	}

private:
	const T & read_ref() {
		if (m_seekState != seek_state::none) perform_seek();
		if (!can_read()) throw stream_exception("!can_read()");
		if (m_nextItem == m_lastItem) {
			compressor_thread_lock l(compressor());
			read_next_block(l);
		}
		return *m_nextItem++;
	}

public:
	T read() {
		return read_ref();
	}

	template <typename IT>
	void read(IT const a, IT const b) {
		for (IT i = a; i != b; ++i) *i = read();
	}

	bool can_read() {
		if (!this->m_open)
			return false;

		if (m_nextReadOffset == 0 && m_byteStreamAccessor.size() > 0)
			return true;

		if (m_seekState != seek_state::none)
			perform_seek();

		if (m_bufferState != buffer_state::read_only)
			return false;

		if (m_nextItem != m_lastItem)
			return true;

		if (m_nextBlockSize != 0)
			return true;

		return false;
	}

	void write(const T & item) {
		if (m_seekState != seek_state::none) perform_seek();
		if (m_bufferState != buffer_state::write_only)
			throw stream_exception("Non-appending write attempted");
		if (m_nextItem == m_bufferEnd) {
			flush_block();
			m_nextItem = m_bufferBegin;
		}
		*m_nextItem++ = item;
		this->m_bufferDirty = true;
	}

	template <typename IT>
	void write(IT const a, IT const b) {
		for (IT i = a; i != b; ++i) write(*i);
	}

protected:
	void perform_seek() {
		if (m_seekState == seek_state::none)
			return;

		if (this->m_bufferDirty) {
			flush_block();
		}

		m_buffer.reset();
		compressor_thread_lock l(compressor());
		finish_requests(l);

		if (m_seekState == seek_state::beginning
			&& !m_byteStreamAccessor.empty())
		{
			m_nextReadOffset = 0;
			get_buffer(l, 0);
			read_next_block(l);
			m_bufferState = buffer_state::read_only;
		} else {
			get_buffer(l, m_streamBlocks++);
			m_bufferState = buffer_state::write_only;
			m_nextItem = m_bufferBegin;
		}
		m_seekState = seek_state::none;
	}

private:
	void get_buffer(compressor_thread_lock & l, stream_size_type blockNumber) {
		m_buffer = this->m_buffers.get_buffer(l, blockNumber);
		m_bufferBegin = reinterpret_cast<T *>(m_buffer->get());
		m_bufferEnd = m_bufferBegin + block_items();
	}

public:
	virtual void flush_block() override {
		memory_size_type blockItems = m_nextItem - m_bufferBegin;
		m_buffer->set_size(blockItems * sizeof(T));
		compressor_request r;
		r.set_write_request(m_buffer, &m_byteStreamAccessor, blockItems);
		compressor_thread_lock lock(compressor());
		compressor().request(r);
		get_buffer(lock, m_streamBlocks++);
		/*
		size_t inputLength = sizeof(T) * blockItems;
		//compressor().append_block(
		memory_size_type blockSize = snappy::MaxCompressedLength(inputLength);
		array<char> scratch(sizeof(blockSize) + blockSize);
		snappy::RawCompress(reinterpret_cast<const char *>(m_buffer.get()),
							inputLength,
							scratch.get() + sizeof(blockSize),
							&blockSize);
		*reinterpret_cast<memory_size_type *>(scratch.get()) = blockSize;
		m_byteStreamAccessor.append(scratch.get(), sizeof(blockSize) + blockSize);
		m_byteStreamAccessor.increase_size(blockItems);
		this->m_bufferDirty = false;
		*/
	}

	void read_next_block(compressor_thread_lock & lock) {
		compressor_request r;
		read_request & rr =
			r.set_read_request(m_buffer,
							   &m_byteStreamAccessor,
							   m_nextReadOffset,
							   m_nextBlockSize,
							   m_readComplete);

		compressor().request(r);
		while (!rr.done()) {
			rr.wait(lock);
		}
		if (rr.end_of_stream())
			throw end_of_stream_exception();

		m_nextReadOffset = rr.next_read_offset();
		m_nextBlockSize = rr.next_block_size();
		m_nextItem = m_bufferBegin;
		memory_size_type itemsRead = m_buffer->size() / sizeof(T);
		m_lastItem = m_bufferBegin + itemsRead;
		m_bufferState = buffer_state::read_only;

		/*
		if (m_nextReadOffset == 0) {
			m_byteStreamAccessor.read(0, &m_nextBlockSize, sizeof(m_nextBlockSize));
			m_nextReadOffset += sizeof(m_nextBlockSize);
			if (m_nextBlockSize == 0)
				throw stream_exception("Internal error; the block size was unexpectedly zero");
		} else if (m_nextBlockSize == 0) {
			throw end_of_stream_exception();
		}
		array<char> scratch(m_nextBlockSize + sizeof(m_nextBlockSize));
		memory_size_type nRead = m_byteStreamAccessor.read(m_nextReadOffset, scratch.get(), scratch.size());
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
		*/
	}

private:
	seek_state::type m_seekState;

	buffer_state::type m_bufferState;

	T * m_bufferBegin;
	T * m_bufferEnd;

	/** Next item in buffer to read/write. */
	T * m_nextItem;
	/** In read mode only: End of readable buffer. */
	T * m_lastItem;

	/** If nextReadOffset is zero,
	 * the next block to read is the first block and its size is not known.
	 * In that case, the size of the first block is the first eight bytes,
	 * and the first block begins after those eight bytes.
	 * If nextReadOffset and nextBlockSize are both non-zero,
	 * the next block begins at the given offset and has the given size.
	 * Otherwise, if nextReadOffset is non-zero and nextBlockSize is zero,
	 * we have reached the end of the stream.
	 */
	stream_size_type m_nextReadOffset;
	stream_size_type m_nextBlockSize;

	stream_size_type m_offset;

	stream_size_type m_streamBlocks;

	bool m_canReadAgain;

	/** Condition variable indicating a read request is complete. */
	read_request::condition_t m_readComplete;
};

} // namespace tpie

#endif // TPIE_COMPRESSED_STREAM_H
