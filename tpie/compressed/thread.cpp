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

#include <queue>
#include <snappy.h>
#include <tpie/compressed/thread.h>
#include <tpie/compressed/request.h>
#include <tpie/compressed/buffer.h>

namespace tpie {

class compressor_thread::impl {
public:
	impl()
		: m_done(false)
	{
	}

	void stop(compressor_thread_lock & /*lock*/) {
		m_done = true;
		m_newRequest.notify_one();
	}

private:
	bool request_valid(const compressor_request & r) {
		switch (r.kind()) {
			case compressor_request_kind::NONE:
				return false;
			case compressor_request_kind::READ:
			case compressor_request_kind::WRITE:
				return true;
		}
		throw exception("Unknown request type");
	}

	bool request_pending() {
		return !m_requests.empty() || m_readAheadRequested;
	}

public:
	void run() {
		while (true) {
			compressor_thread_lock::lock_t lock(mutex());
			while (!m_done && !request_pending()) m_newRequest.wait(lock);
			if (!m_requests.empty()) {
				compressor_request r = m_requests.front();
				m_requests.pop();

				switch (r.kind()) {
					case compressor_request_kind::NONE:
						throw exception("Invalid request");
					case compressor_request_kind::READ:
						process_read_request(lock, r.get_read_request());
						break;
					case compressor_request_kind::WRITE:
						process_write_request(lock, r.get_write_request());
						break;
				}
			} else if (m_readAheadRequested) {
				process_read_ahead(lock);
			} else if (m_done) {
				break;
			} else {
				throw exception("compressor_thread: no reason not to wait");
			}

			m_requestDone.notify_all();
		}
	}

private:
	void process_read_request(compressor_thread_lock::lock_t & lock, read_request & rr) {
		if (!m_readAheadRequested
			&& m_readAhead.get() != 0
			&& m_readAhead->done()
			&& m_readAhead->compatible_parameters(rr))
		{
			log_debug() << "Use readahead at position " << m_readAhead->read_offset() << std::endl;
			rr.swap_result(*m_readAhead);
			m_readAhead.reset();
		} else {
			perform_read(lock, rr);
		}
		if (rr.next_block_size() != 0) {
			log_debug() << "Request readahead at position " << rr.next_read_offset() << std::endl;
			m_readAhead.reset(new read_request(rr.buffer(),
											   rr.buffer_source(),
											   &rr.file_accessor(),
											   rr.next_read_offset(),
											   rr.next_block_size(),
											   NULL));
			m_readAheadRequested = true;
			m_newRequest.notify_one();
		}
		rr.set_done();
		rr.notify();
	}

	void process_read_ahead(compressor_thread_lock::lock_t & lock) {
		log_debug() << "Perform readahead at position " << m_readAhead->read_offset() << std::endl;
		// Take a copy, because it might disappear during reading.
		read_request rr = *m_readAhead;
		perform_read(lock, rr);
		m_readAheadRequested = false;
	}

	void perform_read(compressor_thread_lock::lock_t & lock, read_request & rr) {
		compressor_thread_scoped_unlock unlocker(lock);

		stream_size_type blockSize = rr.block_size();
		stream_size_type readOffset = rr.read_offset();
		if (readOffset == 0) {
			memory_size_type nRead = rr.file_accessor().read(readOffset, &blockSize, sizeof(blockSize));
			if (nRead != sizeof(blockSize)) {
				rr.set_end_of_stream();
				return;
			}
			readOffset += sizeof(blockSize);
		}
		if (blockSize == 0) {
			throw exception("Block size was unexpectedly zero");
		}
		array<char> scratch(blockSize + sizeof(blockSize));
		memory_size_type nRead = rr.file_accessor().read(readOffset, scratch.get(), blockSize + sizeof(blockSize));
		if (nRead == blockSize + sizeof(blockSize)) {
			// This might be unaligned! Watch out.
			rr.set_next_block_size(*(reinterpret_cast<memory_size_type *>(scratch.get() + blockSize)));
			rr.set_next_read_offset(readOffset + blockSize + sizeof(blockSize));
		} else if (nRead == blockSize) {
			rr.set_next_block_size(0);
			rr.set_next_read_offset(readOffset + blockSize);
		} else {
			rr.set_end_of_stream();
			return;
		}

		size_t uncompressedLength;
		if (!snappy::GetUncompressedLength(scratch.get(),
										   blockSize,
										   &uncompressedLength))
			throw stream_exception("Internal error; snappy::GetUncompressedLength failed");
		if (uncompressedLength > rr.buffer()->capacity()) {
			log_error() << uncompressedLength << ' ' << rr.buffer()->capacity() << std::endl;
			throw stream_exception("Internal error; snappy::GetUncompressedLength exceeds the block size");
		}
		rr.buffer()->set_size(uncompressedLength);
		snappy::RawUncompress(scratch.get(),
							  blockSize,
							  reinterpret_cast<char *>(rr.buffer()->get()));
	}

	void process_write_request(compressor_thread_lock::lock_t & lock, write_request & wr) {
		compressor_thread_scoped_unlock unlocker(lock);

		size_t inputLength = wr.buffer()->size();
		memory_size_type blockSize = snappy::MaxCompressedLength(inputLength);
		array<char> scratch(sizeof(blockSize) + blockSize);
		snappy::RawCompress(reinterpret_cast<const char *>(wr.buffer()->get()),
							inputLength,
							scratch.get() + sizeof(blockSize),
							&blockSize);
		*reinterpret_cast<memory_size_type *>(scratch.get()) = blockSize;
		wr.file_accessor().append(scratch.get(), sizeof(blockSize) + blockSize);
		wr.file_accessor().increase_size(wr.block_items());
	}

public:
	mutex_t & mutex() {
		return m_mutex;
	}

	void request(const compressor_request & r) {
		if (!request_valid(r))
			throw exception("Invalid request");

		m_requests.push(r);
		m_newRequest.notify_one();
	}

	void wait_for_request_done(compressor_thread_lock & l) {
		m_requestDone.wait(l.get_lock());
	}

	void free_held_buffers(compressor_thread_lock & l, stream_buffers & bufferSource) {
		if (m_readAhead.get() != 0
			&& &m_readAhead->buffer_source() == &bufferSource)
		{
			m_readAhead.reset();
			m_readAheadRequested = false;
		}
	}

private:
	mutex_t m_mutex;
	std::queue<compressor_request> m_requests;
	boost::condition_variable m_newRequest;
	boost::condition_variable m_requestDone;
	bool m_done;

	std::auto_ptr<read_request> m_readAhead;
	bool m_readAheadRequested;
};

} // namespace tpie

namespace {

tpie::compressor_thread the_compressor_thread;
boost::thread the_compressor_thread_handle;
bool compressor_thread_already_finished = false;

void run_the_compressor_thread() {
	the_compressor_thread.run();
}

} // unnamed namespace

namespace tpie {

compressor_thread & the_compressor_thread() {
	return ::the_compressor_thread;
}

void init_compressor() {
	if (the_compressor_thread_handle != boost::thread()) {
		log_debug() << "Attempted to initiate compressor thread twice" << std::endl;
		return;
	}
	boost::thread t(run_the_compressor_thread);
	the_compressor_thread_handle.swap(t);
	compressor_thread_already_finished = false;
}

void finish_compressor() {
	if (the_compressor_thread_handle == boost::thread()) {
		if (compressor_thread_already_finished) {
			log_debug() << "Compressor thread already finished" << std::endl;
		} else {
			log_debug() << "Attempted to finish compressor thread that was never initiated" << std::endl;
		}
		return;
	}
	{
		compressor_thread_lock lock(the_compressor_thread());
		the_compressor_thread().stop(lock);
	}
	boost::thread t;
	the_compressor_thread_handle.swap(t);
	compressor_thread_already_finished = true;
}

compressor_thread::compressor_thread()
	: pimpl(new impl)
{
}

compressor_thread::~compressor_thread() {
	delete pimpl;
}

compressor_thread::mutex_t & compressor_thread::mutex() {
	return pimpl->mutex();
}

void compressor_thread::request(compressor_request & r) {
	pimpl->request(r);
}

void compressor_thread::run() {
	pimpl->run();
}

void compressor_thread::wait_for_request_done(compressor_thread_lock & l) {
	pimpl->wait_for_request_done(l);
}

void compressor_thread::free_held_buffers(compressor_thread_lock & l, stream_buffers & bufferSource) {
	pimpl->free_held_buffers(l, bufferSource);
}

void compressor_thread::stop(compressor_thread_lock & lock) {
	pimpl->stop(lock);
}

}
