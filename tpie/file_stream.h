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
#ifndef __TPIE_FILE_STREAM_H__
#define __TPIE_FILE_STREAM_H__
#include <tpie/file.h>
////////////////////////////////////////////////////////////////////////////
/// \file file_stream.h
/// \brief Implement simple class aggregating both a file and a file::stream
////////////////////////////////////////////////////////////////////////////

namespace tpie {

////////////////////////////////////////////////////////////////////////////
/// \brief Simple class aggregating both as file and a file::stream
///
/// A file stream basicly supports every operation a file or
/// a file::stream stream supports. This is used to access a file
/// io-efficiently, and is the direct replacement of the old
/// ami::stream
///
/// \tparam T The type of items stored in the stream
////////////////////////////////////////////////////////////////////////////
template <typename T>
class file_stream {
public:
	/** The type of the items stored in the stream */
	typedef T item_type;
	/** The type of file object that is used */
	typedef file<T> file_type;
	/** The type of file::stream object that is used */
	typedef typename file<T>::stream stream_type;
	
	/////////////////////////////////////////////////////////////////////////
	/// \brief Construct a new file_stream
	/// 
	/// \param blockFactor The relative size of a block compared to the 
	/// default
	/// \param fileAccessor The file accessor to use, if none is supplied a
	/// default will be used
	/////////////////////////////////////////////////////////////////////////
	inline file_stream(float blockFactor=1.0, 
					   file_accessor::file_accessor * fileAccessor=NULL)
		throw(stream_exception) : 
		m_file(blockFactor, fileAccessor), m_stream(m_file, 0)  {};

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::open
	/// \sa file_base::open
	/////////////////////////////////////////////////////////////////////////
	inline void open(const std::string & path, file_base::access_type accessType=file_base::read_write, memory_size_type user_data_size=0) {
		m_file.open(path, accessType, user_data_size);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::read_user_data
	/// \sa file_base::read_user_data
	/////////////////////////////////////////////////////////////////////////
	template <typename TT>
	void read_user_data(TT & data) throw (stream_exception) {
		m_file.read_user_data(data);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::write_user_data
	/// \sa file_base::write_user_data
	/////////////////////////////////////////////////////////////////////////
	template <typename TT>
	void write_user_data(const TT & data) {
		m_file.write_user_data(data);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Close the fileand release resources
	///
	/// This will close the file and resources used by buffers and such
	/////////////////////////////////////////////////////////////////////////
	inline void close() throw(stream_exception) {
		m_stream.free();
		m_file.close();
	}
	
	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file<T>::stream::write(const item_type & item)
	/// \sa file<T>::stream::write(const item_type & item)
	/////////////////////////////////////////////////////////////////////////
	inline void write(const item_type & item) throw(stream_exception) {
		m_stream.write(item);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file<T>::stream::write(const IT & start, const IT & end)
	/// \sa file<T>::stream::write(const IT & start, const IT & end)
	/////////////////////////////////////////////////////////////////////////
	template <typename IT>
	inline void write(const IT & start, const IT & end) throw(stream_exception) {
		m_stream.write(start, end);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file<T>::stream::read()
	/// \sa file<T>::stream::read()
	/////////////////////////////////////////////////////////////////////////
	inline item_type & read() throw(stream_exception) {
		return m_stream.read();
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file<T>::stream::read(const IT & start, const IT & end)
	/// \sa file<T>::stream::read(const IT & start, const IT & end)
	/////////////////////////////////////////////////////////////////////////
	template <typename IT>
	inline void read(const IT & start, const IT & end) throw(stream_exception) {
		m_stream.read(start, end);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::stream::offset()
	/// \sa file_base::stream::offset()
	/////////////////////////////////////////////////////////////////////////
	inline stream_size_type offset() const throw() {
		return m_stream.offset();
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::path()
	/// \sa file_base::path()
	/////////////////////////////////////////////////////////////////////////
	inline const std::string & path() const throw() {
		return m_file.path();
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::size()
	/// \sa file_base::size()
	/////////////////////////////////////////////////////////////////////////
	inline stream_size_type size() const throw() {
		return m_file.size();
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::stream::size()
	/// \sa file_base::stream::size()
	/////////////////////////////////////////////////////////////////////////
	inline bool has_more() const throw() {
		return m_stream.has_more();
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::stream::seek()
	/// \sa file_base::stream::seek()
	/////////////////////////////////////////////////////////////////////////
	inline void seek(stream_offset_type offset, 
					 file_base::offset_type whence=file_base::beginning) 
		throw (stream_exception) {
		return m_stream.seek(offset, whence);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::truncate()
	/// \sa file_base::truecate()
	///
	/// Note that when using a file_stream the stream will automaticly be
	/// seeked back if it is beond the new end of the file. 
	/////////////////////////////////////////////////////////////////////////
	inline void truncate(stream_size_type size) {
		if (offset() > size) 
			seek(size);
		m_file.truncate(size);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Calculate the amount of memory used by count file_streams
	///
	/// \param count The number of file_streams, to concider.
	/// \param blockFactor The block factor you promice to pass to open
	/// \param includeDefaultFileAccessor Unless you are supplieng your own
	/// file accessor to open leave this to be true
	/// \returns The amount of memory maximaly used by the count file_streams
	/////////////////////////////////////////////////////////////////////////
	inline static memory_size_type memory_usage(
		memory_size_type count=1, 
		float blockFactor=1.0,
		bool includeDefaultFileAccessor=true) throw() {
		return file_type::memory_usage(count, includeDefaultFileAccessor) 
			+ stream_type::memory_usage(count, blockFactor) 
			+ sizeof(file_stream)*count;
	}

	//////////////////////////////////////////////////////////////////////////
	/// \brief Return the underleing file
	///
	/// \returns The underleing file
	//////////////////////////////////////////////////////////////////////////
	inline file_type & get_file() throw() {
		return m_file;
	}

	//////////////////////////////////////////////////////////////////////////
	/// \brief Return the underleing stream
	///
	/// \returns The underleing stream
	//////////////////////////////////////////////////////////////////////////
	inline stream_type & get_stream() throw() {
		return m_stream;
	}
private:
	file_type m_file;
	stream_type m_stream;

};
}

#endif //__TPIE_FILE_STREAM_H__
