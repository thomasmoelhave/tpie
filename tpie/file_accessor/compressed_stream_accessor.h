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

#ifndef TPIE_FILE_ACCESSOR_COMPRESSED_STREAM_ACCESSOR_H
#define TPIE_FILE_ACCESSOR_COMPRESSED_STREAM_ACCESSOR_H

#include <tpie/tpie_log.h>
#include <tpie/file_accessor/stream_accessor_base.h>

namespace tpie {
namespace file_accessor {

namespace bits {
	bool RawUncompress(const char* compressed, size_t compressed_length, char* uncompressed);
	bool GetUncompressedLength(const char* compressed, size_t compressed_length, size_t* result);
	size_t Compress(const char* input, size_t input_length, std::string* output);
}

template <typename file_accessor_t>
class compressed_stream_accessor : public stream_accessor_base<file_accessor_t> {
	stream_size_type m_nextInputOffset;
	stream_size_type m_nextBlock;

	stream_size_type seek(stream_size_type blockNumber) {
		stream_size_type loc = m_nextInputOffset;
		if (blockNumber == 0) {
			loc = this->header_size();
		} else if (blockNumber != m_nextBlock) {
			log_error() << "Next expected block is " << m_nextBlock
				<< " but " << blockNumber << " was requested." << std::endl;
			throw stream_exception("Random seeks are not supported");
		}
		// Here, we may seek beyond the file size.
		// However, lseek(2) specifies that the file will be padded with zeroes in this case,
		// and on Windows, the file is padded with arbitrary garbage (which is ok).
		this->m_fileAccessor.seek_i(loc);
		return loc;
	}

public:
	compressed_stream_accessor()
		: m_nextBlock(0)
	{
		m_nextInputOffset = this->header_size();
	}

	virtual memory_size_type read_block(void * data,
										stream_size_type blockNumber,
										memory_size_type itemCount) override
	{
		stream_size_type offset = blockNumber*this->block_items();
		if (offset + itemCount > this->size()) itemCount = static_cast<memory_size_type>(this->size() - offset);

		stream_size_type loc = seek(blockNumber);
		stream_size_type bytes;
		this->m_fileAccessor.read_i(&bytes, sizeof(stream_size_type));
		std::string compressed(bytes, '\0');
		this->m_fileAccessor.read_i(&compressed[0], bytes);
		size_t uncompressedLength;
		if (!bits::GetUncompressedLength(&compressed[0], bytes, &uncompressedLength)) {
			throw stream_exception("Snappy failed to GetUncompressedLength");
		}
		if (uncompressedLength > this->block_size()) {
			throw stream_exception("Snappy returned an uncompressed length greater than m_blockSize");
		}
		bits::RawUncompress(&compressed[0], bytes, reinterpret_cast<char *>(data));

		m_nextBlock = blockNumber + 1;
		m_nextInputOffset = loc + bytes + sizeof(bytes);

		return itemCount;
	}

	virtual void write_block(const void * data,
							 stream_size_type blockNumber,
							 memory_size_type itemCount) override
	{
		stream_size_type loc = seek(blockNumber);
		stream_size_type offset = blockNumber*this->block_items();
		memory_size_type z=itemCount*this->item_size();

		if (offset != this->size()) {
			throw stream_exception("Overwriting stream blocks is not supported");
		}

		{
			std::string compressed;
			bits::Compress(reinterpret_cast<const char *>(data), z, &compressed);
			stream_size_type l = compressed.size();
			this->m_fileAccessor.write_i(&l, sizeof(l));
			this->m_fileAccessor.write_i(&compressed[0], l);
			m_nextBlock = blockNumber + 1;
			m_nextInputOffset = loc + l + sizeof(l);
		}

		if (offset+itemCount > this->size()) this->set_size(offset+itemCount);
	}
};

} // namespace tpie
} // namespace file_accessor

#endif // TPIE_FILE_ACCESSOR_COMPRESSED_STREAM_ACCESSOR_H
