// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, The TPIE development team
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

#ifndef __TPIE_PIPELINING_FILE_STREAM_H__
#define __TPIE_PIPELINING_FILE_STREAM_H__

#include <tpie/file_stream.h>

#include <tpie/pipelining/pipe_segment.h>
#include <tpie/pipelining/factory_helpers.h>

namespace tpie {

namespace pipelining {

namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \class input_t
///
/// file_stream input generator.
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
struct input_t : public pipe_segment {
	typedef typename dest_t::item_type item_type;

	inline input_t(const dest_t & dest, file_stream<item_type> & fs) : dest(dest), fs(fs) {
		add_push_destination(dest);
		set_name("Read", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(fs.memory_usage());
	}

	virtual void begin() /*override*/ {
		pipe_segment::begin();
		if (fs.is_open()) {
			forward("items", fs.size());
		} else {
			forward("items", 0);
		}
		set_steps(fs.size());
	}

	virtual void go() /*override*/ {
		if (fs.is_open()) {
			while (fs.can_read()) {
				dest.push(fs.read());
				step();
			}
		}
	}

private:
	dest_t dest;
	file_stream<item_type> & fs;
};

///////////////////////////////////////////////////////////////////////////////
/// \class pull_input_t
///
/// file_stream pull input generator.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
struct pull_input_t : public pipe_segment {
	typedef T item_type;

	inline pull_input_t(file_stream<T> & fs) : fs(fs) {
		set_name("Read", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(fs.memory_usage());
	}

	virtual void begin() /*override*/ {
		forward("items", fs.size());
		set_steps(fs.size());
	}

	inline T pull() {
		step();
		return fs.read();
	}

	inline bool can_pull() {
		return fs.can_read();
	}

	file_stream<T> & fs;
};

///////////////////////////////////////////////////////////////////////////////
/// \class output_t
///
/// file_stream output terminator.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
struct output_t : public pipe_segment {
	typedef T item_type;

	inline output_t(file_stream<T> & fs) : fs(fs) {
		set_name("Write", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(fs.memory_usage());
	}

	inline void push(const T & item) {
		fs.write(item);
	}
private:
	file_stream<T> & fs;
};

///////////////////////////////////////////////////////////////////////////////
/// \class pull_output_t
///
/// file_stream output pull data source.
///////////////////////////////////////////////////////////////////////////////
template <typename source_t>
struct pull_output_t : public pipe_segment {
	typedef typename source_t::item_type item_type;

	inline pull_output_t(const source_t & source, file_stream<item_type> & fs) : source(source), fs(fs) {
		add_pull_destination(source);
		set_name("Write", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(fs.memory_usage());
	}

	virtual void go() /*override*/ {
		source.begin();
		while (source.can_pull()) {
			fs.write(source.pull());
		}
		source.end();
	}

	source_t source;
	file_stream<item_type> & fs;
};

template <typename T>
struct tee_t {
	template <typename dest_t>
	class type: public pipe_segment {
	public:
		typedef T item_type;
		type(const dest_t & dest, file_stream<item_type> & fs): fs(fs), dest(dest) {
			add_push_destination(dest);
			set_minimum_memory(fs.memory_usage());
		}

		void push(const item_type & i) {
			fs.write(i);
			dest.push(i);
		}
	private:
		file_stream<item_type> & fs;
		dest_t dest;
	};
};

} // namespace bits

template<typename T>
inline pipe_begin<factory_1<bits::input_t, file_stream<T> &> > input(file_stream<T> & fs) {
	return factory_1<bits::input_t, file_stream<T> &>(fs);
}

template<typename T>
inline pullpipe_begin<termfactory_1<bits::pull_input_t<T>, file_stream<T> &> > pull_input(file_stream<T> & fs) {
	return termfactory_1<bits::pull_input_t<T>, file_stream<T> &>(fs);
}

template <typename T>
inline pipe_end<termfactory_1<bits::output_t<T>, file_stream<T> &> > output(file_stream<T> & fs) {
	return termfactory_1<bits::output_t<T>, file_stream<T> &>(fs);
}

template<typename T>
inline pullpipe_end<factory_1<bits::pull_output_t, file_stream<T> &> > pull_output(file_stream<T> & fs) {
	return factory_1<bits::pull_output_t, file_stream<T> &>(fs);
}

template <typename T>
inline pipe_middle<factory_1<bits::tee_t<typename T::item_type>::template type, T &> >
tee(T & fs) {return factory_1<bits::tee_t<typename T::item_type>::template type, T &>(fs);}

} // namespace pipelining

} // namespace tpie
#endif
