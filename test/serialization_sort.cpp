// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :

#include <sstream>
#include <tpie/pipelining.h>
#include <tpie/serialization_sorter.h>
#include "ref_ptr.h"
#include <tpie/tpie_log.h>

namespace tp = tpie::pipelining;

namespace application {

enum tagtype {
	point,
	not_point
};

struct item : public tpie::ref_cnt {
	tagtype t;
	double px, py;
};

template <typename D>
void serialize(D & dst, const item & x) {
	using tpie::serialize;
	serialize(dst, (unsigned char) x.t);
	serialize(dst, x.px);
	serialize(dst, x.py);
}

template <typename S>
void unserialize(S & src, item & x) {
	using tpie::unserialize;
	unsigned char t;
	unserialize(src, t);
	x.t = (tagtype) t;
	unserialize(src, x.px);
	unserialize(src, x.py);
}

typedef tpie::ref_ptr<item> item_ptr;

class item_cmp_y {
public:
	typedef item_ptr first_argument_type;
	typedef item_ptr second_argument_type;
	typedef bool result_type;

	bool operator()(const item_ptr & lhs, const item_ptr & rhs) const {
		return lhs->py < rhs->py;
	}
};

template <typename dest_t>
class item_generator_type : public tp::node {
public:
	item_generator_type(dest_t && dest, size_t n)
		: dest(std::move(dest))
		, n(n)
	{
	}

	virtual void go() override {
		item x;
		x.t = point;
		for (size_t i = 0; i < n; ++i) {
			x.px = i;
			x.py = n-i;
			dest.push(tpie::mkref(tpie::tpie_new<item>(x)));
		}
	}

private:
	dest_t dest;
	size_t n;
};

typedef tp::pipe_begin<tp::factory_1<item_generator_type, size_t> > item_generator;

class item_consumer_type : public tp::node {
public:
	item_consumer_type() {
	}

	void push(item_ptr) {
		// throw it on the ground
	}
};

typedef tp::pipe_end<tp::termfactory_0<item_consumer_type> > item_consumer;

} // namespace application

int main(int argc, char ** argv) {
	size_t n = 1000;
	if (argc > 1) std::stringstream(argv[1]) >> n;
	tpie::tpie_init();
	tpie::get_memory_manager().set_limit(20*1024*1024);
	{
		tpie::stderr_log_target lt(tpie::LOG_DEBUG);
		tpie::get_log().add_target(&lt);
		{
			tp::pipeline p
				= application::item_generator(n)
				| tp::serialization_sort(application::item_cmp_y())
				| application::item_consumer();
			p.plot(std::cout);
			p();
		}
		tpie::get_log().remove_target(&lt);
	}
	tpie::tpie_finish();
	std::cout << "Whaaaat" << std::endl;
	return 0;
}
