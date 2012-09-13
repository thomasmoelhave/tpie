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
#include <tpie/pipelining/pipe_segment.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/memory.h>

namespace tpie {

namespace pipelining {

template <typename dest_t>
struct ostream_logger_t : public pipe_segment {
	typedef typename dest_t::item_type item_type;

	inline ostream_logger_t(const dest_t & dest, std::ostream & log) : dest(dest), log(log), begun(false), ended(false) {
		add_push_destination(dest);
		set_name("Log", PRIORITY_INSIGNIFICANT);
	}
	virtual void begin() /*override*/ {
		pipe_segment::begin();
		begun = true;
	}
	virtual void end() /*override*/ {
		pipe_segment::end();
		ended = true;
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
	typedef typename dest_t::item_type item_type;

	inline identity_t(const dest_t & dest) : dest(dest) {
		add_push_destination(dest);
		set_name("Identity", PRIORITY_INSIGNIFICANT);
	}

	inline void push(const item_type & item) {
		dest.push(item);
	}
private:
	dest_t dest;
};

inline pipe_middle<factory_0<identity_t> > identity() {
	return pipe_middle<factory_0<identity_t> >();
}

template <typename source_t>
struct pull_identity_t : public pipe_segment {
	typedef typename source_t::item_type item_type;

	inline pull_identity_t(const source_t & source) : source(source) {
		add_pull_destination(source);
		set_name("Identity", PRIORITY_INSIGNIFICANT);
	}

	inline item_type pull() {
		return source.pull();
	}

	inline bool can_pull() {
		return source.can_pull();
	}

private:
	source_t source;
};

template <typename T>
struct dummydest_t : public pipe_segment {
	dummydest_t() : buffer(new T()) {}

	typedef T item_type;
	boost::shared_ptr<T> buffer;
	inline void push(const T & el) {
		*buffer = el;
	}
	inline T pull() {
		return *buffer;
	}
};

template <typename pushfact_t>
struct push_to_pull {

	template <typename source_t>
	struct puller_t : public pipe_segment {

		typedef typename source_t::item_type item_type;
		typedef typename pushfact_t::template generated<dummydest_t<item_type> >::type pusher_t;

		source_t source;
		dummydest_t<item_type> dummydest;
		pusher_t pusher;

		inline puller_t(const source_t & source, const pushfact_t & pushfact)
			: source(source)
			, pusher(pushfact.construct(dummydest))
		{
			add_pull_destination(source);
			add_push_destination(pusher);
		}

		inline item_type pull() {
			pusher.push(source.pull());
			return dummydest.pull();
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
		typedef typename dest_t::item_type item_type;
		typedef typename pullfact_t::template generated<dummydest_t<item_type> >::type puller_t;

		dest_t dest;
		dummydest_t<item_type> dummydest;
		puller_t puller;

		inline pusher_t(const dest_t & dest, const pullfact_t & pullfact)
			: dest(dest)
			, puller(pullfact.construct(dummydest))
		{
			add_push_destination(dest);
			add_pull_destination(puller);
		}

		inline void push(const item_type & item) {
			dummydest.push(item);
			dest.push(puller.pull());
		}

	};
};

inline pullpipe_middle<factory_1<push_to_pull<factory_0<identity_t> >::puller_t, factory_0<identity_t> > > pull_identity() {
	return factory_1<push_to_pull<factory_0<identity_t> >::puller_t, factory_0<identity_t> >(factory_0<identity_t>());
}

inline
pipe_middle<factory_1<
	pull_to_push<factory_0<pull_identity_t> >::pusher_t,
	factory_0<pull_identity_t>
> >
alt_identity() {
	return factory_1<
		pull_to_push<factory_0<pull_identity_t> >::pusher_t,
		factory_0<pull_identity_t>
	>(factory_0<pull_identity_t>());
}

template <typename T>
struct bitbucket_t : public pipe_segment {
	typedef T item_type;

	inline void push(const T &) {
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
		typedef typename dest_t::item_type item_type;

		inline type(const dest_t & dest, const fact2_t & fact2) : dest(dest), dest2(fact2.construct()) {
			add_push_destination(dest);
			add_push_destination(dest2);
			set_name("Fork", PRIORITY_INSIGNIFICANT);
		}

		inline void push(const item_type & item) {
			dest.push(item);
			dest2.push(item);
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

template <typename T>
struct null_sink_t: public pipe_segment {
	typedef T item_type;
	void push(const T &) {}
};

template <typename T>
inline pipe_end<termfactory_0<null_sink_t<T> > >
null_sink() {return termfactory_0<null_sink_t<T> >();}

template <template <typename dest_t> class Fact>
pipe_begin<factory_0<Fact> > make_pipe_begin_0() {
	return pipe_begin<factory_0<Fact> >(factory_0<Fact>());
}

template <template <typename dest_t> class Fact>
pipe_middle<factory_0<Fact> > make_pipe_middle_0() {
	return pipe_middle<factory_0<Fact> >(factory_0<Fact>());
}

template <typename Fact>
pipe_end<termfactory_0<Fact> > make_pipe_end_0() {
	return pipe_end<termfactory_0<Fact> >(termfactory_0<Fact>());
}

}

}

#endif
