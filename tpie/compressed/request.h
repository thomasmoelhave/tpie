// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
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

#ifndef TPIE_COMPRESSED_REQUEST_H
#define TPIE_COMPRESSED_REQUEST_H

///////////////////////////////////////////////////////////////////////////////
/// \file compressed/request.h
///////////////////////////////////////////////////////////////////////////////

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <tpie/file_accessor/file_accessor.h>
#include <tpie/file_accessor/byte_stream_accessor.h>
#include <tpie/compressed/predeclare.h>

namespace tpie {

// The response object is used to relay information back from the compressor
// thread to the stream class.
// The compressor mutex must be acquired before any compressor_response method is called.
// Each method is annotated with a (request kind, caller)-comment,
// identifying whether the method relates to a read request or a write request,
// and whether the stream object (main thread) or the compressor thread should call it.
class compressor_response {
public:
	compressor_response()
		: m_blockNumber(std::numeric_limits<stream_size_type>::max())
		, m_readOffset(0)
		, m_blockSize(0)
		, m_done(false)
		, m_endOfStream(false)
		, m_nextReadOffset(0)
		, m_nextBlockSize(0)
	{
	}

	// any, stream
	void wait(compressor_thread_lock & lock);

	// any, stream
	void initiate_request() {
		m_done = m_endOfStream = false;
		m_nextReadOffset = m_nextBlockSize = 0;
	}

	// write, stream
	void clear_block_info() {
		m_blockNumber = std::numeric_limits<stream_size_type>::max();
	}

	// write, thread -- must have lock!
	void set_block_info(stream_size_type blockNumber,
						stream_size_type readOffset,
						stream_size_type blockSize)
	{
		if (m_blockNumber != std::numeric_limits<stream_size_type>::max()
			&& blockNumber < m_blockNumber)
		{
			//log_debug() << "set_block_info(blockNumber=" << blockNumber
			//	<< ", readOffset=" << readOffset << ", blockSize=" << blockSize << "): "
			//	<< "We already know the size of block " << m_blockNumber << std::endl;
		} else {
			//log_debug() << "set_block_info(blockNumber=" << blockNumber
			//	<< ", readOffset=" << readOffset << ", blockSize=" << blockSize << "): "
			//	<< "Previous was " << m_blockNumber << std::endl;
			m_blockNumber = blockNumber;
			m_readOffset = readOffset;
			m_blockSize = blockSize;
			m_changed.notify_all();
		}
	}

	// write, stream
	bool has_block_info(stream_size_type blockNumber)
	{
		if (m_blockNumber == std::numeric_limits<stream_size_type>::max())
			return false;

		if (blockNumber < m_blockNumber) {
			std::stringstream ss;
			ss << "Wanted block number " << blockNumber << ", but recalled was " << m_blockNumber;
			throw exception(ss.str());
		}

		if (blockNumber == m_blockNumber)
			return true;
		else // blockNumber > m_blockNumber
			return false;
	}

	// write, stream
	stream_size_type get_block_size(stream_size_type blockNumber)
	{
		if (!has_block_info(blockNumber))
			throw exception("get_block_size: !has_block_info");

		return m_blockSize;
	}

	// write, stream
	stream_size_type get_read_offset(stream_size_type blockNumber) {
		if (!has_block_info(blockNumber))
			throw exception("get_read_offset: !has_block_info");

		return m_readOffset;
	}

	// read, thread
	void set_end_of_stream() {
		m_done = true;
		m_endOfStream = true;
		m_changed.notify_all();
	}

	// read, stream
	bool done() {
		return m_done;
	}

	// read, stream
	bool end_of_stream() {
		return m_endOfStream;
	}

	// read, stream
	stream_size_type next_read_offset() {
		return m_nextReadOffset;
	}

	// read, stream
	stream_size_type next_block_size() {
		return m_nextBlockSize;
	}

	// read, thread
	void set_next_block(stream_size_type offset,
						stream_size_type size)
	{
		m_done = true;
		m_nextReadOffset = offset;
		m_nextBlockSize = size;
		m_changed.notify_all();
	}

private:
	boost::condition_variable m_changed;

	// Information about the write
	stream_size_type m_blockNumber;
	stream_size_type m_readOffset;
	stream_size_type m_blockSize;

	// Information about the read
	bool m_done;
	bool m_endOfStream;
	stream_size_type m_nextReadOffset;
	stream_size_type m_nextBlockSize;
};

#ifdef __GNUC__
class __attribute__((__may_alias__)) request_base;
class __attribute__((__may_alias__)) read_request;
class __attribute__((__may_alias__)) write_request;
#endif // __GNUC__

class request_base {
protected:
	request_base(compressor_response * response)
		: m_response(response)
	{
	}

public:
	void initiate_request() {
		m_response->initiate_request();
	}

protected:
	compressor_response * m_response;
};

class read_request : public request_base {
public:
	typedef boost::shared_ptr<compressor_buffer> buffer_t;
	typedef file_accessor::byte_stream_accessor<default_raw_file_accessor> file_accessor_t;
	typedef boost::condition_variable condition_t;

	read_request(buffer_t buffer,
				 file_accessor_t * fileAccessor,
				 stream_size_type readOffset,
				 stream_size_type blockSize,
				 compressor_response * response)
		: request_base(response)
		, m_buffer(buffer)
		, m_fileAccessor(fileAccessor)
		, m_readOffset(readOffset)
		, m_blockSize(blockSize)
	{
	}

	buffer_t buffer() {
		return m_buffer;
	}

	void set_end_of_stream() {
		m_response->set_end_of_stream();
	}

	file_accessor_t & file_accessor() {
		return *m_fileAccessor;
	}

	stream_size_type read_offset() {
		return m_readOffset;
	}

	stream_size_type block_size() {
		return m_blockSize;
	}

	void set_next_block(stream_size_type offset,
						stream_size_type size)
	{
		m_response->set_next_block(offset, size);
	}

private:
	buffer_t m_buffer;
	file_accessor_t * m_fileAccessor;
	/** If readOffset is zero, the next block to read is the first block and its size is not known.
	 * In that case, the size of the first block is the first eight bytes, and the first block begins
	 * after those eight bytes.
	 * If readOffset and blockSize are both non-zero, the next block begins at the given offset
	 * and has the given size.
	 * Otherwise, if readOffset is non-zero and blockSize is zero, we have reached the end of
	 * the stream.
	 */
	const stream_size_type m_readOffset;
	const stream_size_type m_blockSize;
};

class write_request : public request_base {
public:
	typedef boost::shared_ptr<compressor_buffer> buffer_t;
	typedef file_accessor::byte_stream_accessor<default_raw_file_accessor> file_accessor_t;

	write_request(const buffer_t & buffer,
				  file_accessor_t * fileAccessor,
				  stream_size_type writeOffset,
				  memory_size_type blockItems,
				  stream_size_type blockNumber,
				  compressor_response * response)
		: request_base(response)
		, m_buffer(buffer)
		, m_fileAccessor(fileAccessor)
		, m_writeOffset(writeOffset)
		, m_blockItems(blockItems)
		, m_blockNumber(blockNumber)
	{
	}

	file_accessor_t & file_accessor() {
		return *m_fileAccessor;
	}

	buffer_t buffer() {
		return m_buffer;
	}

	memory_size_type block_items() {
		return m_blockItems;
	}

	bool should_append() {
		return m_writeOffset == std::numeric_limits<stream_size_type>::max();
	}

	stream_size_type write_offset() {
		return m_writeOffset;
	}

	// must have lock!
	void set_block_info(stream_size_type readOffset,
						stream_size_type blockSize)
	{
		m_response->set_block_info(m_blockNumber, readOffset, blockSize);
	}

private:
	buffer_t m_buffer;
	file_accessor_t * m_fileAccessor;
	const stream_size_type m_writeOffset;
	const memory_size_type m_blockItems;
	const stream_size_type m_blockNumber;
};

class compressor_request_kind {
public:
	enum type {
		NONE,
		READ,
		WRITE
	};

private:
	compressor_request_kind() /*= delete*/;
	compressor_request_kind(const compressor_request_kind &) /*= delete*/;
	~compressor_request_kind() /*= delete*/;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  Tagged union containing either a read_request or a write_request.
///
/// In C++11, this can be implemented more elegantly since the C++11 standard
/// allows unions with member types having constructors and destructors.
/// In C++03, we have to emulate this with a char buffer that is large enough.
///
/// To turn the tagged union into a T, call set_T(ctor params) which returns a
/// reference to T. When the tagged union is a T (check with kind()), use
/// get_T() to get the reference to T.
///////////////////////////////////////////////////////////////////////////////
class compressor_request {
public:
	compressor_request()
		: m_kind(compressor_request_kind::NONE)
	{
	}

	~compressor_request() {
		destruct();
	}

	compressor_request(const compressor_request & other)
		: m_kind(compressor_request_kind::NONE)
	{
		switch (other.kind()) {
			case compressor_request_kind::NONE:
				break;
			case compressor_request_kind::READ:
				set_read_request(other.get_read_request());
				break;
			case compressor_request_kind::WRITE:
				set_write_request(other.get_write_request());
				break;
		}
	}

	read_request & set_read_request(const read_request::buffer_t & buffer,
									read_request::file_accessor_t * fileAccessor,
									stream_size_type readOffset,
									stream_size_type blockSize,
									compressor_response * response)
	{
		destruct();
		m_kind = compressor_request_kind::READ;
		return *new (m_payload) read_request(buffer, fileAccessor, readOffset, blockSize, response);
	}

	read_request & set_read_request(const read_request & other) {
		destruct();
		m_kind = compressor_request_kind::READ;
		return *new (m_payload) read_request(other);
	}

	write_request & set_write_request(const write_request::buffer_t & buffer,
									  write_request::file_accessor_t * fileAccessor,
									  stream_size_type writeOffset,
									  memory_size_type blockItems,
									  stream_size_type blockNumber,
									  compressor_response * response)
	{
		destruct();
		m_kind = compressor_request_kind::WRITE;
		return *new (m_payload) write_request(buffer, fileAccessor, writeOffset,
											  blockItems, blockNumber, response);
	}

	write_request & set_write_request(const write_request & other) {
		destruct();
		m_kind = compressor_request_kind::WRITE;
		return *new (m_payload) write_request(other);
	}

	// Precondition: kind() == READ
	read_request & get_read_request() {
		return *reinterpret_cast<read_request *>(m_payload);
	}

	// Precondition: kind() == READ
	const read_request & get_read_request() const {
		return *reinterpret_cast<const read_request *>(m_payload);
	}

	// Precondition: kind() == WRITE
	write_request & get_write_request() {
		return *reinterpret_cast<write_request *>(m_payload);
	}

	// Precondition: kind() == WRITE
	const write_request & get_write_request() const {
		return *reinterpret_cast<const write_request *>(m_payload);
	}

	// Precondition: kind() != NONE
	request_base & get_request_base() {
		return *reinterpret_cast<request_base *>(m_payload);
	}

	// Precondition: kind() != NONE
	const request_base & get_request_base() const {
		return *reinterpret_cast<const request_base *>(m_payload);
	}

	compressor_request_kind::type kind() const {
		return m_kind;
	}

private:
	void destruct() {
		switch (m_kind) {
			case compressor_request_kind::NONE:
				break;
			case compressor_request_kind::READ:
				get_read_request().~read_request();
				break;
			case compressor_request_kind::WRITE:
				get_write_request().~write_request();
				break;
		}
		m_kind = compressor_request_kind::NONE;
	}

	compressor_request_kind::type m_kind;

	char m_payload[sizeof(read_request) < sizeof(write_request) ? sizeof(write_request) : sizeof(read_request)];
};

} // namespace tpie

#endif // TPIE_COMPRESSED_REQUEST_H
