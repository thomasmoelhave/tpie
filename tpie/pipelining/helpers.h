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

#ifndef __TPIE_PIPELINING_HELPERS_H__
#define __TPIE_PIPELINING_HELPERS_H__

#include <iostream>
#include <tpie/pipelining/core.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/memory.h>

namespace tpie {

namespace pipelining {

template <typename dest_t>
struct ostream_logger_t : public pipe_segment {
	///////////////////////////////////////////////////////////////////////////
	/// \brief Virtual dtor.
	///////////////////////////////////////////////////////////////////////////
	~ostream_logger_t() {}

	typedef typename dest_t::item_type item_type;

	inline ostream_logger_t(const dest_t & dest, std::ostream & log) : dest(dest), log(log), begun(false), ended(false) {
	}
	inline void begin() {
		begun = true;
		dest.begin();
	}
	inline void end() {
		ended = true;
		dest.end();
	}
	inline void push(const item_type & item) {
		if (!begun) {
			log << "WARNING: push() called before begin(). Calling begin on rest of pipeline." << std::endl;
			begin();
		}
		if (ended) {
			log << "WARNING: push() called after end()." << std::endl;
			ended = false;
		}
		log << "pushing " << item << std::endl;
		dest.push(item);
	}

	const pipe_segment * get_next() const {
		return &dest;
	}
private:
	dest_t dest;
	std::ostream & log;
	bool begun;
	bool ended;
};

inline pipe_middle<factory_1<ostream_logger_t, std::ostream &> >
cout_logger() {
	return factory_1<ostream_logger_t, std::ostream &>(std::cout);
}

template <typename dest_t>
struct identity_t : public pipe_segment {
	///////////////////////////////////////////////////////////////////////////
	/// \brief Virtual dtor.
	///////////////////////////////////////////////////////////////////////////
	~identity_t() {}

	typedef typename dest_t::item_type item_type;

	inline identity_t(const dest_t & dest) : dest(dest) {
	}

	inline void begin() {
		dest.begin();
	}

	inline void push(const item_type & item) {
		dest.push(item);
	}

	inline void end() {
		dest.end();
	}

	const pipe_segment * get_next() const {
		return &dest;
	}
private:
	dest_t dest;
};

inline pipe_middle<factory_0<identity_t> > identity() {
	return pipe_middle<factory_0<identity_t> >();
}

template <typename source_t>
struct pull_identity_t {
	typedef typename source_t::item_type item_type;

	inline pull_identity_t(const source_t & source) : source(source) {
	}

	inline void begin() {
		source.begin();
	}

	inline item_type pull() {
		return source.pull();
	}

	inline bool can_pull() {
		return source.can_pull();
	}

	inline void end() {
		source.end();
	}

private:
	source_t source;
};

template <typename T>
struct dummydest_t : public pipe_segment {
	///////////////////////////////////////////////////////////////////////////
	/// \brief Virtual dtor.
	///////////////////////////////////////////////////////////////////////////
	~dummydest_t() {}

	typedef T item_type;
	T & buffer;
	inline dummydest_t(T & buffer) : buffer(buffer) {
	}
	inline dummydest_t(const dummydest_t & other) : buffer(other.buffer) {
	}
	inline void begin() {
	}
	inline void end() {
	}
	inline void push(const T & el) {
		buffer = el;
	}
	inline T pull() {
		return buffer;
	}
	const pipe_segment * get_next() const { return 0; }
};

template <typename pushfact_t>
struct push_to_pull {

	template <typename source_t>
	struct puller_t {

		typedef typename source_t::item_type item_type;
		typedef typename pushfact_t::template generated<dummydest_t<item_type> >::type pusher_t;

		item_type buffer;
		source_t source;
		pushfact_t pushfact;
		dummydest_t<item_type> dummydest;
		auto_ptr<pusher_t> pusher;

		inline puller_t(const source_t & source, const pushfact_t & pushfact)
			: source(source)
			, pushfact(pushfact)
			, dummydest(buffer) {
		}

		inline puller_t(const puller_t & other)
			: buffer(other.buffer)
			, source(other.source)
			, dummydest(buffer) {
		}

		inline void begin() {
			pusher.reset(tpie_new<pusher_t>(pushfact.construct(dummydest)));
			pusher->begin();
			source.begin();
		}

		inline void end() {
			source.end();
			pusher->end();
			pusher.reset();
		}

		inline item_type pull() {
			pusher->push(source.pull());
			return buffer;
		}

		inline bool can_pull() {
			return source.can_pull();
		}

	};
};

template <typename pullfact_t>
struct pull_to_push {

	template <typename dest_t>
	struct pusher_t : public pipe_segment {
		///////////////////////////////////////////////////////////////////////
		/// \brief Virtual dtor.
		///////////////////////////////////////////////////////////////////////
		~pusher_t() {}

		typedef typename dest_t::item_type item_type;
		typedef typename pullfact_t::template generated<dummydest_t<item_type> >::type puller_t;

		item_type buffer;
		dest_t dest;
		pullfact_t pullfact;
		dummydest_t<item_type> dummydest;
		auto_ptr<puller_t> puller;

		inline pusher_t(const dest_t & dest, const pullfact_t & pullfact)
			: dest(dest)
			, pullfact(pullfact)
			, dummydest(buffer) {
		}

		inline pusher_t(const pusher_t & other)
			: buffer(other.buffer)
			, dest(other.dest)
			, dummydest(buffer) {
		}

		inline void begin() {
			puller.reset(tpie_new<puller_t>(pullfact.construct(dummydest)));
			puller->begin();
			dest.begin();
		}

		inline void end() {
			dest.end();
			puller->end();
			puller.reset();
		}

		inline void push(const item_type & item) {
			dummydest.push(item);
			puller->pull();
			dest.push(dummydest.pull());
		}

		const pipe_segment * get_next() const {
			return &dest;
		}

	};
};

inline pull_factory_1<push_to_pull<factory_0<identity_t> >::puller_t, factory_0<identity_t> > pull_identity() {
	return factory_0<identity_t>();
}

inline
pipe_middle<factory_1<
	pull_to_push<pull_factory_0<pull_identity_t> >::pusher_t,
	pull_factory_0<pull_identity_t>
> >
alt_identity() {
	return factory_1<
		pull_to_push<pull_factory_0<pull_identity_t> >::pusher_t,
		pull_factory_0<pull_identity_t>
	>(pull_factory_0<pull_identity_t>());
}

template <typename T>
struct bitbucket_t : public pipe_segment {
	///////////////////////////////////////////////////////////////////////////
	/// \brief Virtual dtor.
	///////////////////////////////////////////////////////////////////////////
	~bitbucket_t() {}

	typedef T item_type;

	inline void begin() {
	}

	inline void end() {
	}

	inline void push(const T &) {
	}

	const pipe_segment * get_next() const {
		return 0;
	}
};

template <typename T>
inline pipe_end<termfactory_0<bitbucket_t<T> > >
bitbucket(T) {
	return termfactory_0<bitbucket_t<T> >();
}

template <typename fact2_t>
struct fork_t {
	typedef typename fact2_t::generated_type dest2_t;

	template <typename dest_t>
	struct type : public pipe_segment {
		///////////////////////////////////////////////////////////////////////
		/// \brief Virtual dtor.
		///////////////////////////////////////////////////////////////////////
		~type() {}

		typedef typename dest_t::item_type item_type;

		inline type(const dest_t & dest, const fact2_t & fact2) : dest(dest), dest2(fact2.construct()) {
			std::cout << typeid(dest2_t).name() << std::endl;
		}

		inline void begin() {
			dest.begin();
			dest2.begin();
		}

		inline void push(const item_type & item) {
			dest.push(item);
			dest2.push(item);
		}

		inline void end() {
			dest.end();
			dest2.end();
		}

		const pipe_segment * get_next() const {
			return &dest;
		}

	private:
		dest_t dest;
		dest2_t dest2;
	};
};

template <typename fact_t>
inline pipe_middle<factory_1<fork_t<fact_t>::template type, const fact_t &> >
fork(const pipe_end<fact_t> & to) {
	return factory_1<fork_t<fact_t>::template type, const fact_t &>(to.factory);
}

}

}

#endif
