// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, The TPIE development team
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
#ifndef __tpie_file_accessor_file_accossor_h__
#define __tpie_file_accessor_file_accossor_h__
#include <tpie/types.h>
#include <tpie/stream_header.h>

///////////////////////////////////////////////////////////////////////////////
/// \file file_accessor.h Declare base class of file accessors.
///////////////////////////////////////////////////////////////////////////////

namespace tpie {
namespace file_accessor {

///////////////////////////////////////////////////////////////////////////////
/// \brief Base class of file accessors. A file accessor encapsulates file
/// descriptors and the act of reading and writing headers and user data.
/// It does not handle serialization and unserialization of data, however it
/// does convert between physical byte offsets and logical item offsets
/// transparently.
///////////////////////////////////////////////////////////////////////////////
class file_accessor {
protected:
	stream_size_type m_size;
	memory_size_type m_userDataSize;
	memory_size_type m_itemSize;
	memory_size_type m_blockSize;
	memory_size_type m_blockItems;
	std::string m_path;

	inline memory_size_type boundary() { return 4096; }
	inline memory_size_type align_to_boundary(memory_size_type z) { return (z+boundary()-1)/boundary()*boundary(); }
	inline memory_size_type header_size() { return align_to_boundary(sizeof(stream_header_t)+m_userDataSize); }
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Open file given by path.
	/// \param path Path of file to open.
	/// \param read Whether we should be able to read from the file.
	/// \param write Whether we should be able to write to the file.
	/// \param itemSize Size of items in bytes.
	/// \param userDataSize Size of user data in bytes.
	///////////////////////////////////////////////////////////////////////////
	virtual void open(const std::string & path, 
					  bool read, 
					  bool write, 
					  memory_size_type itemSize,
					  memory_size_type blockSize,
					  memory_size_type userDataSize) = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Close whatever file is currently open.
	///////////////////////////////////////////////////////////////////////////
	virtual void close() = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Read item data into an arbitrary byte buffer.
	/// The size of the buffer must be at least itemCount * itemSize.
	/// \param data Pointer to writable buffer in which to write the read data.
	/// \param blockNumber Logical block offset from which to begin reading.
	/// \param itemCount Number of items to read.
	///////////////////////////////////////////////////////////////////////////
	virtual memory_size_type read_block(void * data, stream_size_type blockNumber, stream_size_type itemCount) = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Write item data from memory into file.
	/// \param data The buffer from which to write data.
	/// \param blockNumber Logical block offset at which to begin writing.
	/// \param itemCount Number of items to write.
	///////////////////////////////////////////////////////////////////////////
	virtual void write_block(const void * data, stream_size_type blockNumber, stream_size_type itemCount) = 0; 

	///////////////////////////////////////////////////////////////////////////
	/// \brief Read user data from file into buffer.
	/// \param data Writable buffer in which to write user data from file.
	///////////////////////////////////////////////////////////////////////////
	virtual void read_user_data(void * data) = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Write user data to file.
	/// \param data User data to write.
	///////////////////////////////////////////////////////////////////////////
	virtual void write_user_data(const void * data) = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Truncate file to given number of items.
	/// \param size New number of items in file.
	///////////////////////////////////////////////////////////////////////////
	virtual void truncate(stream_size_type size) = 0;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Destroy file accessor.
	///////////////////////////////////////////////////////////////////////////
	virtual ~file_accessor() {}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return number of items in file.
	///////////////////////////////////////////////////////////////////////////
	inline stream_size_type size() const {return m_size;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return path to file.
	///////////////////////////////////////////////////////////////////////////
	inline const std::string & path() const {return m_path;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return size of user data.
	///////////////////////////////////////////////////////////////////////////
	inline memory_size_type user_data_size() const {return m_userDataSize;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Validate consistency of given header struct.
	///////////////////////////////////////////////////////////////////////////
	void validate_header(const stream_header_t & header);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Populate stream header.
	///////////////////////////////////////////////////////////////////////////
	void fill_header(stream_header_t & header, bool clean);
};

}
}
#endif //__tpie_file_accessor_file_accossor_h__
