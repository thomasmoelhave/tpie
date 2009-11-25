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

#error "This file is for documentation only"
namespace tpie {
namespace stream {
namespace concepts {

///////////////////////////////////////////////////////////////////////////
/// \file concepts.h Defines concepts used by the TPIE stream subsystem
/////////////////////////////////////////////////////////////////////////// 

///////////////////////////////////////////////////////////////////////////
/// \namespace tpie::stream::concepts Namespace of stream concepts
/////////////////////////////////////////////////////////////////////////// 

///////////////////////////////////////////////////////////////////////////
/// \class block_transfer_engine
/// \brief Defines the concept of a block transfer engine as used by the fd_file class
/// A bte must implement the block_transfer_engine_interface and the memory_calculatable_interface
/////////////////////////////////////////////////////////////////////////// 


///////////////////////////////////////////////////////////////////////////
/// \class file_stream
/// \brief Defines the concept of a stream in a file
/// A stream must implement file_interface::stream, and memory_calculatable_interface
/////////////////////////////////////////////////////////////////////////// 

///////////////////////////////////////////////////////////////////////////
/// \brief Interface that must be implemented inorder to support the file concept.
///
/// All implementation will take at least the following template parameters
/// \tparam T The type of the items contained in the file
/// \tparam canRead Indicate that we want to be able to read from the file.
/// \tparam canWrite Indicate that we want to be able to write to the file.
/// \tparam blockSizeMultiplyer The file will be opened with the globally configured block size multiplied with this parameter.
///
/// The following policies will imply when opening a file
/// \li canRead && !canWrite: The file is opened for reading, an exception will be throw if it does not exist.
/// \li !canRead && !canWrite: The file is opened for writing, it is created if it does not exist, and truncated otherwize.
/// \li canRead && canWrite: The file is opened for reading and writing, it is created if it does not exist.
/// \li !canRead && !canWrite: An exception will be thrown
/// 
/////////////////////////////////////////////////////////////////////////// 
template <typedef T, bool canRead, bool canWrite, float blockSizeMultiplyer=1.0> 
class file_interface {
public:
	//////////////////////////////////////////////////////////////////////////////////
	/// Defines the type of the items in the file.
	//////////////////////////////////////////////////////////////////////////////////
	typedef T item_type;

	//////////////////////////////////////////////////////////////////////////////////
	/// \brief Construct a new file object
	/// The read/write-ness of the file is defined in template paramaters to the given
	/// file implementation.
	/// \param typeMagic An integer describing the type of items the stream contains
	//////////////////////////////////////////////////////////////////////////////////
	file_interface(boost::uint64_t typeMagic=0);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Open a given file
	/// You cannot construct streams of the file, before open is called.
	/// All streams must be destructed before close is called.
	/// \param path The path of the file to open
	/////////////////////////////////////////////////////////////////////////// 
	void open(const std::string & path);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Open a new temporery file
    /////////////////////////////////////////////////////////////////////////// 	
	void open();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Close a open file
	/// All streams associated with the file object must be destructed before
	/// close is called.  
	/// close is automaticly called by the destructor.
	///////////////////////////////////////////////////////////////////////////
	void close();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the number of items in the file
	///////////////////////////////////////////////////////////////////////////
	offset_type size() const;


	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the path of the currently open file
	///////////////////////////////////////////////////////////////////////////
	const std::string & path() const;


	///////////////////////////////////////////////////////////////////////////
	/// \brief Interface that must be implemented inorder to support the file_stream concept.
	/////////////////////////////////////////////////////////////////////////// 
	class stream {
	public:
		/////////////////////////////////////////////////////////////////////////// 
		/// The type of the file object the stream can be associated with
		///////////////////////////////////////////////////////////////////////////
		typedef file_interface file_type;

		/////////////////////////////////////////////////////////////////////////// 
		/// The type of items in the file
		///////////////////////////////////////////////////////////////////////////
		typedef T item_type;
		
		///////////////////////////////////////////////////////////////////////////
		/// Create a stream into a file
		/// \param file The file to create a stream into.
		/// \param offset The initial offset in the file of the stream
		///////////////////////////////////////////////////////////////////////////
		stream(file_type & file, offset_type offset=0);
		
		/////////////////////////////////////////////////////////////////////////// 
		/// Change the offset in the file of the stream
		/// \param offset The new offset in the stream.
		///////////////////////////////////////////////////////////////////////////
		void seek(offset_type offset=0);

		///////////////////////////////////////////////////////////////////////////
		/// Return the current offset of the stream in the file
		///////////////////////////////////////////////////////////////////////////
		offset_type offset() const;

		///////////////////////////////////////////////////////////////////////////
		/// Indicate if there are more elements to read in the stream after the current
		/// offset, this is logicaly eqvivalent to offset() != size(), but is probably
		/// faster.
		/// \return true if there are more items to read.
		///////////////////////////////////////////////////////////////////////////
		bool has_more() const;

		///////////////////////////////////////////////////////////////////////////
		/// Return the number of items in the file
		///////////////////////////////////////////////////////////////////////////
		offset_type size() const;

		///////////////////////////////////////////////////////////////////////////
		/// Read an element from the current offset in the file and incrument it by one
		/// \return the item read
		///////////////////////////////////////////////////////////////////////////
		const item_type & read();
		
		///////////////////////////////////////////////////////////////////////////
		/// Write an element to the current offset in the file (possibly extending its size), and
		/// increment the current offset by one
		/// \param item The item to write.
		///////////////////////////////////////////////////////////////////////////
		void write(const item_type & item);
		
		///////////////////////////////////////////////////////////////////////////
		/// Read a bunch of items and store then in the iterator
		/// This is logically equivalent to
		/// \code
		/// size_type count=0;
		/// for(iterator_type i=start; i != end && stream.has_more(); ++i) {
		///   *i = stream.read();
        ///   ++count;
		/// }
		/// return count;
		/// \endcode
		/// But it might be faster.
		/// \return The number of items read
		///////////////////////////////////////////////////////////////////////////
		template <typename iterator_type>
		size_type read(const iterator_type & start, const iterator_type & end);

		///////////////////////////////////////////////////////////////////////////
		/// Write a bunch of items to the stream
		/// This is logically equivalent to
		/// \code
		/// for(iterator_type i=start; i != end && stream.has_more(); ++i)
		///   stream.write(*i);
		/// \endcode
		/// But it might be faster.
		///////////////////////////////////////////////////////////////////////////
		template <typename iterator_type>
		void write(const iterator_type & start, const iterator_type & end);
	};
};


///////////////////////////////////////////////////////////////////////////
/// \class file
/// Defines the concept of a file
/// A file must implement file_interface, and memory_calculatable_interface
/////////////////////////////////////////////////////////////////////////// 
}
}
}

