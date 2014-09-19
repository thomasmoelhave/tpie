// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2011, 2012, 2013 The TPIE development team
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

#include "common.h"
#include <tpie/pipelining.h>
#include <tpie/file_stream.h>
#include <boost/filesystem.hpp>
#include <algorithm>
#include <tpie/sysinfo.h>
#include <tpie/pipelining/virtual.h>
#include <tpie/progress_indicator_arrow.h>

using namespace tpie;
using namespace tpie::pipelining;

typedef uint64_t test_t;

template <typename dest_t>
struct multiply_t : public node {
	typedef test_t item_type;

	inline multiply_t(const dest_t & dest, uint64_t factor)
		: dest(dest)
		, factor(factor)
	{
		set_minimum_memory(17000000);
		add_push_destination(dest);
	}

	virtual void begin() override {
		node::begin();
		log_debug() << "multiply begin with memory " << this->get_available_memory() << std::endl;
	}

	void push(const test_t & item) {
		dest.push(factor*item);
	}

	dest_t dest;
	uint64_t factor;
};

typedef pipe_middle<factory_1<multiply_t, uint64_t> > multiply;

std::vector<test_t> inputvector;
std::vector<test_t> expectvector;
std::vector<test_t> outputvector;

void setup_test_vectors() {
	inputvector.resize(0); expectvector.resize(0); outputvector.resize(0);
	inputvector.resize(20); expectvector.resize(20);
	for (size_t i = 0; i < 20; ++i) {
		inputvector[i] = i;
		expectvector[i] = i*6;
	}
}

bool check_test_vectors() {
	if (outputvector != expectvector) {
		log_error() << "Output vector does not match expect vector\n"
			<< "Expected: " << std::flush;
		std::vector<test_t>::iterator expectit = expectvector.begin();
		while (expectit != expectvector.end()) {
			log_error()<< *expectit << ' ';
			++expectit;
		}
		log_error()<< '\n'
			<< "Output:   " << std::flush;
		std::vector<test_t>::iterator outputit = outputvector.begin();
		while (outputit != outputvector.end()) {
			log_error()<< *outputit << ' ';
			++outputit;
		}
		log_error()<< std::endl;
		return false;
	}
	return true;
}

bool vector_multiply_test() {
	pipeline p = input_vector(inputvector) | multiply(3) | multiply(2) | output_vector(outputvector);
	p.plot(log_info());
	p();
	return check_test_vectors();
}

bool file_stream_test(stream_size_type items) {
	tpie::temp_file input_file;
	tpie::temp_file output_file;
	{
		file_stream<test_t> in;
		in.open(input_file.path());
		for (stream_size_type i = 0; i < items; ++i) {
			in.write(i);
		}
	}
	{
		file_stream<test_t> in;
		in.open(input_file.path());
		file_stream<test_t> out;
		out.open(output_file.path());
		// p is actually an input_t<multiply_t<multiply_t<output_t<test_t> > > >
		pipeline p = (input(in) | multiply(3) | multiply(2) | output(out));
		p.plot(log_info());
		p();
	}
	{
		file_stream<test_t> out;
		out.open(output_file.path());
		for (stream_size_type i = 0; i < items; ++i) {
			if (i*6 != out.read()) return false;
		}
	}
	return true;
}

bool file_stream_pull_test() {
	tpie::temp_file input_file;
	tpie::temp_file output_file;
	{
		file_stream<test_t> in;
		in.open(input_file.path());
		in.write(1);
		in.write(2);
		in.write(3);
	}
	{
		file_stream<test_t> in;
		in.open(input_file.path());
		file_stream<test_t> out;
		out.open(output_file.path());
		pipeline p = (pull_input(in) | pull_identity() | pull_output(out));
		p.get_node_map()->dump(log_info());
		p.plot(log_info());
		p();
	}
	{
		file_stream<test_t> out;
		out.open(output_file.path());
		if (1 != out.read()) return false;
		if (2 != out.read()) return false;
		if (3 != out.read()) return false;
	}
	return true;
}

bool file_stream_alt_push_test() {
	tpie::temp_file input_file;
	tpie::temp_file output_file;
	{
		file_stream<test_t> in;
		in.open(input_file.path());
		in.write(1);
		in.write(2);
		in.write(3);
	}
	{
		file_stream<test_t> in;
		in.open(input_file.path());
		file_stream<test_t> out;
		out.open(output_file.path());
		pipeline p = (input(in) | alt_identity() | output(out));
		p.plot(log_info());
		p();
	}
	{
		file_stream<test_t> out;
		out.open(output_file.path());
		if (1 != out.read()) return false;
		if (2 != out.read()) return false;
		if (3 != out.read()) return false;
	}
	return true;
}

bool merge_test() {
	tpie::temp_file input_file;
	tpie::temp_file output_file;
	{
		file_stream<test_t> in;
		in.open(input_file.path());
		pipeline p = input_vector(inputvector) | output(in);
		p.plot(log_info());
		p();
	}
	expectvector.resize(2*inputvector.size());
	for (int i = 0, j = 0, l = inputvector.size(); i < l; ++i) {
		expectvector[j++] = inputvector[i];
		expectvector[j++] = inputvector[i];
	}
	{
		file_stream<test_t> in;
		in.open(input_file.path());
		file_stream<test_t> out;
		out.open(output_file.path());
		std::vector<test_t> inputvector2 = inputvector;
		pipeline p = input_vector(inputvector) | merge(pull_input(in)) | output(out);
		p.plot(log_info());
		p();
	}
	{
		file_stream<test_t> in;
		in.open(output_file.path());
		pipeline p = input(in) | output_vector(outputvector);
		p.plot(log_info());
		p();
	}
	return check_test_vectors();
}

bool reverse_test() {
	pipeline p1 = input_vector(inputvector) | reverser() | output_vector(outputvector);
	p1.plot_full(log_info());
	p1();
	expectvector = inputvector;
	std::reverse(expectvector.begin(), expectvector.end());

	//reverser<test_t> r(inputvector.size());
	//pipeline p1 = input_vector(inputvector) | r.sink();
	//pipeline p2 = r.source() | output_vector(outputvector);
	//p1.plot(log_info());
	//p1();

	return check_test_vectors();
}

bool internal_reverse_test() {
	pipeline p1 = input_vector(inputvector) | reverser() | output_vector(outputvector);
	p1();
	expectvector = inputvector;
	std::reverse(expectvector.begin(), expectvector.end());

	return check_test_vectors();
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Sums the two components of the input pair
///////////////////////////////////////////////////////////////////////////////

template <typename factory_type>
class add_pairs_type {
public:
	template <typename dest_t>
	class type : public tpie::pipelining::node {
	public:
		dest_t dest;
		typename factory_type::constructed_type pullSource;
		typedef uint64_t item_type;

		type(const dest_t & dest, const factory_type & factory)
		: dest(dest)
		, pullSource(factory.construct())
		{
			add_push_destination(dest);
			add_pull_source(pullSource);
		}

		void push(const item_type &item) {
			dest.push(item + pullSource.pull());
		}
	};
};

template <typename pipe_type>
tpie::pipelining::pipe_middle<tpie::pipelining::tempfactory_1<add_pairs_type<typename pipe_type::factory_type>, typename pipe_type::factory_type> >
add_pairs(const pipe_type &pipe) {
	return tpie::pipelining::tempfactory_1<add_pairs_type<typename pipe_type::factory_type>, typename pipe_type::factory_type>(pipe.factory);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief templated test for the passive reversers
///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool templated_passive_reverse_test(size_t n) {
	std::vector<test_t> input;
	std::vector<test_t> output;

	for(size_t i = 0; i < n; ++i)
		input.push_back(i);

	T p_reverser;
	pipeline p = input_vector(input)
	| fork(p_reverser.input())
	| buffer()
	| add_pairs(p_reverser.output())
	| output_vector(output);

	p();

	TEST_ENSURE_EQUALITY(output.size(), n, "The passive reverser didn't output the correct number of elements");
	for(std::vector<test_t>::iterator i = output.begin(); i != output.end(); ++i)
		TEST_ENSURE_EQUALITY(*i, n-1, "The passive reverser did not output in the correct order");
	return true;
}

bool internal_passive_reverse_test(size_t n) {
	return templated_passive_reverse_test<passive_reverser<test_t> >(n);
}

bool passive_reverse_test(size_t n) {
	return templated_passive_reverse_test<internal_passive_reverser<test_t> >(n);
}

template <typename dest_t>
struct sequence_generator_type : public node {
	typedef size_t item_type;

	sequence_generator_type(const dest_t & dest, size_t elements, bool reverse)
		: dest(dest)
		, elements(elements)
		, reverse(reverse)
	{
		add_push_destination(dest);
	}

	virtual void propagate() override {
		forward("items", static_cast<stream_size_type>(elements));
		set_steps(elements);
	}

	virtual void go() override {
		if (reverse) {
			for (size_t i = elements; i > 0; --i) {
				dest.push(i);
				step();
			}
		} else {
			for (size_t i = 1; i <= elements; ++i) {
				dest.push(i);
				step();
			}
		}
	}
private:
	dest_t dest;
	size_t elements;
	bool reverse;
};

typedef pipe_begin<factory_2<sequence_generator_type, size_t, bool> >
	sequence_generator;

struct sequence_verifier_type : public node {
	typedef size_t item_type;

	sequence_verifier_type(size_t elements, bool * result)
		: elements(elements)
		, expect(1)
		, result(result)
		, bad(false)
	{
		*result = false;
	}

	virtual void propagate() override {
		if (!can_fetch("items")) {
			log_error() << "Sorter did not forward number of items" << std::endl;
			bad = true;
		}
		*result = false;
	}

	inline void push(size_t element) {
		if (element != expect) {
			(bad ? log_debug() : log_error()) << "Got " << element << ", expected " << expect << std::endl;
			bad = true;
		}
		*result = false;
		++expect;
	}

	virtual void end() override {
		if (can_fetch("items")
			&& static_cast<stream_size_type>(elements) != fetch<stream_size_type>("items")) {

			log_error() << "Sorter did not send as many items as promised" << std::endl;
			bad = true;
		}
		*result = !bad;
	}

private:
	size_t elements;
	size_t expect;
	bool * result;
	bool bad;
};

typedef pipe_end<termfactory_2<sequence_verifier_type, size_t, bool *> >
	sequence_verifier;

bool sort_test(size_t elements) {
	bool result = false;
	pipeline p = sequence_generator(elements, true)
		| sort().name("Test")
		| sequence_verifier(elements, &result);
	p.plot(log_info());
	p();
	return result;
}

bool sort_test_trivial() {
	TEST_ENSURE(sort_test(0), "Cannot sort 0 elements");
	TEST_ENSURE(sort_test(1), "Cannot sort 1 element");
	TEST_ENSURE(sort_test(2), "Cannot sort 2 elements");
	return true;
}

bool sort_test_small() {
	return sort_test(20);
}

bool sort_test_large() {
	return sort_test(300*1024);
}

// This tests that pipe_middle | pipe_middle -> pipe_middle,
// and that pipe_middle | pipe_end -> pipe_end.
// The other tests already test that pipe_begin | pipe_middle -> pipe_middle,
// and that pipe_begin | pipe_end -> pipeline.
bool operator_test() {
	expectvector = inputvector;
	std::reverse(inputvector.begin(), inputvector.end());
	pipeline p = input_vector(inputvector) | ((sort() | sort()) | output_vector(outputvector));
	p.plot(log_info());
	p();
	return check_test_vectors();
}

bool uniq_test() {
	const size_t n = 5;
	inputvector.resize(n*(n+1)/2);
	expectvector.resize(n);
	size_t k = 0;
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = 0; j <= i; ++j) {
			inputvector[k++] = i;
		}
		expectvector[i] = i;
	}
	pipeline p = input_vector(inputvector) | pipeuniq() | output_vector(outputvector);
	p.plot(log_info());
	p();
	return check_test_vectors();
}

struct memtest {
	size_t totalMemory;
	size_t minMem1;
	size_t maxMem1;
	size_t minMem2;
	size_t maxMem2;
	double frac1;
	double frac2;

	size_t assigned1;
	size_t assigned2;
};

template <typename dest_t>
class memtest_1 : public node {
	dest_t dest;
	memtest & settings;

public:
	memtest_1(const dest_t & dest, memtest & settings)
		: dest(dest)
		, settings(settings)
	{
		add_push_destination(dest);
	}

	void prepare() {
		set_minimum_memory(settings.minMem1);
		if (settings.maxMem1 > 0)
			set_maximum_memory(settings.maxMem1);
		set_memory_fraction(settings.frac1);
	}

	virtual void set_available_memory(memory_size_type m) override {
		node::set_available_memory(m);
		settings.assigned1 = m;
	}

	virtual void go() override {
	}
};

class memtest_2 : public node {
	memtest & settings;

public:
	memtest_2(memtest & settings)
		: settings(settings)
	{
	}

	void prepare() {
		set_minimum_memory(settings.minMem2);
		if (settings.maxMem2 > 0)
			set_maximum_memory(settings.maxMem2);
		set_memory_fraction(settings.frac2);
	}

	virtual void set_available_memory(memory_size_type m) override {
		node::set_available_memory(m);
		settings.assigned2 = m;
	}
};

bool memory_test(memtest settings) {
	if (settings.minMem1 + settings.minMem2 > settings.totalMemory) {
		throw tpie::exception("Memory requirements too high");
	}

	const memory_size_type NO_MEM = std::numeric_limits<memory_size_type>::max();
	settings.assigned1 = settings.assigned2 = NO_MEM;

	progress_indicator_null pi;

	pipeline p =
		make_pipe_begin_1<memtest_1, memtest &>(settings)
		| make_pipe_end_1<memtest_2, memtest &>(settings);
	p(0, pi, settings.totalMemory);

	log_debug() << "totalMemory " << settings.totalMemory << '\n'
	            << "minMem1     " << settings.minMem1 << '\n'
	            << "maxMem1     " << settings.maxMem1 << '\n'
	            << "minMem2     " << settings.minMem2 << '\n'
	            << "maxMem2     " << settings.maxMem2 << '\n'
	            << "frac1       " << settings.frac1 << '\n'
	            << "frac2       " << settings.frac2 << '\n'
	            << "assigned1   " << settings.assigned1 << '\n'
	            << "assigned2   " << settings.assigned2 << std::endl;

	if (settings.assigned1 == NO_MEM || settings.assigned2 == NO_MEM) {
		log_error() << "No memory assigned" << std::endl;
		return false;
	}

	if (settings.assigned1 + settings.assigned2 > settings.totalMemory) {
		log_error() << "Too much memory assigned" << std::endl;
		return false;
	}

	if (settings.assigned1 < settings.minMem1 || settings.assigned2 < settings.minMem2) {
		log_error() << "Too little memory assigned" << std::endl;
		return false;
	}

	if ((settings.maxMem1 != 0 && settings.assigned1 > settings.maxMem1)
		|| (settings.maxMem2 != 0 && settings.assigned2 > settings.maxMem2)) {
		log_error() << "Too much memory assigned" << std::endl;
		return false;
	}

	const double EPS = 1e-9;
	const size_t min1 = settings.minMem1;
	const size_t max1 = (settings.maxMem1 == 0) ? settings.totalMemory : settings.maxMem1;
	const size_t min2 = settings.minMem2;
	const size_t max2 = (settings.maxMem2 == 0) ? settings.totalMemory : settings.maxMem2;
	const size_t m1 = settings.assigned1;
	const size_t m2 = settings.assigned2;
	const double f1 = settings.frac1;
	const double f2 = settings.frac2;
	if ((min1 < m1 && m1 < max1) && (min2 < m2 && m2 < max2)
		&& abs(m1 * f2 - m2 * f1) > EPS)
	{
		log_error() << "Fractions not honored" << std::endl;
		return false;
	}

	return true;
}

void memory_test_shorthand(teststream & ts, size_t totalMemory, size_t minMem1, size_t maxMem1, size_t minMem2, size_t maxMem2, double frac1, double frac2) {
	ts << "(" << totalMemory << ", " << minMem1 << ", " << maxMem1 << ", " << minMem2 << ", " << maxMem2 << ", " << frac1 << ", " << frac2 << ")";
	memtest settings;
	settings.totalMemory = totalMemory;
	settings.minMem1 = minMem1;
	settings.maxMem1 = maxMem1;
	settings.minMem2 = minMem2;
	settings.maxMem2 = maxMem2;
	settings.frac1 = frac1;
	settings.frac2 = frac2;
	ts << result(memory_test(settings));
}

void memory_test_multi(teststream & ts) {
	//                        total   min1   max1   min2   max2  frac1  frac2
	memory_test_shorthand(ts,  2000,     0,     0,     0,     0,   1.0,   1.0);
	memory_test_shorthand(ts,  2000,   800,     0,   800,     0,   1.0,   1.0);
	memory_test_shorthand(ts,  4000,  1000,     0,  1000,     0,   0.0,   0.0);
	memory_test_shorthand(ts,  2000,     0,     0,     0,     0,   0.0,   1.0);
	memory_test_shorthand(ts,  2000,   500,     0,     0,     0,   0.0,   1.0);
	memory_test_shorthand(ts,  2000,   500,   700,     0,     0,   1.0,   1.0);
	memory_test_shorthand(ts,  2000,     0,   700,     0,   500,   1.0,   1.0);
	memory_test_shorthand(ts,  2000,     0,  2000,     0,  2000,   1.0,   1.0);
	memory_test_shorthand(ts,  2000,   200,  2000,     0,  2000,   1.0,   1.0);
}

bool fork_test() {
	expectvector = inputvector;
	pipeline p = input_vector(inputvector).name("Input vector") | fork(output_vector(outputvector)) | bitbucket<test_t>(0);
	p();
	return check_test_vectors();
}

template <typename dest_t>
struct buffer_node_t : public node {
	typedef typename dest_t::item_type item_type;

	inline buffer_node_t(const dest_t & dest)
		: dest(dest)
	{
		add_dependency(dest);
	}

	inline void push(const item_type & item) {
		dest.push(item);
	}

	dest_t dest;
};

inline pipe_middle<factory_0<buffer_node_t> >
buffer_node() {
	return pipe_middle<factory_0<buffer_node_t> >();
}

struct merger_memory : public memory_test {
	typedef int test_t;
	size_t n;
	array<file_stream<test_t> > inputs;
	merger<test_t, std::less<test_t> > m;

	inline merger_memory(size_t n)
		: n(n)
		, m(std::less<test_t>())
	{
		inputs.resize(n);
		for (size_t i = 0; i < n; ++i) {
			inputs[i].open();
			inputs[i].write(static_cast<test_t>(n-i));
			inputs[i].seek(0);
		}
	}

	virtual void alloc() {
		m.reset(inputs, 1);
	}

	virtual void free() {
		m.reset();
	}

	virtual void use() {
		test_t prev = m.pull();
		for (size_t i = 1; i < n; ++i) {
			test_t it = m.pull();
			if (prev > it) {
				log_error() << "Merger returns items out of order in memory test" << std::endl;
			}
			prev = it;
		}
	}

	virtual size_type claimed_size() {
		return static_cast<size_type>(merger<test_t, std::less<test_t> >::memory_usage(n));
	}
};

bool merger_memory_test(size_t n) {
	merger_memory m(n);
	return m();
}

struct my_item {
	my_item() : v1(42), v2(9001) {}
	short v1;
	int v2;
};

template <typename dest_t>
struct FF1 : public node {
	dest_t dest;
	FF1(const dest_t & dest) : dest(dest) {
		add_push_destination(dest);
	}
	virtual void propagate() override {
		my_item i;
		i.v1 = 1;
		forward("my_item", i);
	}
	virtual void go() override {
	}
};

template <typename dest_t>
struct FF2 : public node {
	dest_t dest;
	FF2(const dest_t & dest) : dest(dest) {
		add_push_destination(dest);
	}
};

bool fetch_forward_result;

struct FF3 : public node {
	FF3() {
	}
	virtual void propagate() override {
		if (!can_fetch("my_item")) {
			log_error() << "Cannot fetch my_item" << std::endl;
			fetch_forward_result = false;
			return;
		}
		my_item i = fetch<my_item>("my_item");
		if (i.v1 != 1) {
			log_error() << "Wrong answer" << std::endl;
			fetch_forward_result = false;
			return;
		}
	}
};

bool fetch_forward_test() {
	fetch_forward_result = true;
	pipeline p = make_pipe_begin_0<FF1>()
		| make_pipe_middle_0<FF2>()
		| make_pipe_end_0<FF3>()
		;
	p.plot(log_info());
	p.forward<int>("test", 42);
	p();
	if (!fetch_forward_result) return false;
	if (p.fetch<int>("test") != 42) {
		log_error() << "Something went wrong" << std::endl;
		return false;
	}
	return true;
}

// Assume that dest_t::item_type is a reference type.
// Push a dereferenced zero pointer to the destination.
template <typename dest_t>
class push_zero_t : public node {
	dest_t dest;
public:
	typedef typename dest_t::item_type item_type;
private:
	// Type of pointer to dereference.
	typedef typename boost::remove_reference<item_type>::type * ptr_type;
public:
	push_zero_t(const dest_t & dest)
		: dest(dest)
	{
		add_push_destination(dest);
	}

	virtual void go() /*override*/ {
		dest.push(*static_cast<ptr_type>(0));
	}
};

pipe_begin<factory_0<push_zero_t> >
push_zero() {
	return factory_0<push_zero_t>();
}

bool virtual_test() {
	pipeline p = virtual_chunk_begin<test_t>(input_vector(inputvector))
		| virtual_chunk<test_t, test_t>(multiply(3) | multiply(2))
		| virtual_chunk<test_t, test_t>()
		| virtual_chunk_end<test_t>(output_vector(outputvector));
	p.plot(log_info());
	p();
	return check_test_vectors();
}

bool virtual_fork_test() {
	pipeline p = virtual_chunk_begin<test_t>(input_vector(inputvector))
		| vfork(virtual_chunk_end<test_t>(output_vector(outputvector)))
		| virtual_chunk_end<test_t>(output_vector(outputvector));
	p.plot_full(log_info());
	p();
	expectvector.resize(inputvector.size() * 2);
	for (size_t i = 0; i < inputvector.size(); ++i) {
		expectvector[2*i] = expectvector[2*i+1] = inputvector[i];
	}
	return check_test_vectors();
}

struct prepare_result {
	prepare_result()
		: t(0)
	{
	}

	memory_size_type memWanted1;
	memory_size_type memWanted2;
	memory_size_type memWanted3;

	memory_size_type memGotten1;
	memory_size_type memGotten2;
	memory_size_type memGotten3;

	size_t t;
	size_t prep1;
	size_t prep2;
	size_t prep3;
	size_t propagate1;
	size_t propagate2;
	size_t propagate3;
	size_t begin1;
	size_t begin2;
	size_t begin3;
	size_t end1;
	size_t end2;
	size_t end3;
};

std::ostream & operator<<(std::ostream & os, const prepare_result & r) {
	return os
		<< "memWanted1: " << r.memWanted1 << '\n'
		<< "memWanted2: " << r.memWanted2 << '\n'
		<< "memWanted3: " << r.memWanted3 << "\n\n"
		<< "memGotten1: " << r.memGotten1 << '\n'
		<< "memGotten2: " << r.memGotten2 << '\n'
		<< "memGotten3: " << r.memGotten3 << "\n\n"
		<< "t:          " << r.t << '\n'
		<< "prep1:      " << r.prep1 << '\n'
		<< "prep2:      " << r.prep2 << '\n'
		<< "prep3:      " << r.prep3 << '\n'
		<< "begin1:     " << r.begin1 << '\n'
		<< "begin2:     " << r.begin2 << '\n'
		<< "begin3:     " << r.begin3 << '\n'
		<< "end1:       " << r.end1 << '\n'
		<< "end2:       " << r.end2 << '\n'
		<< "end3:       " << r.end3 << '\n'
		;
}

template <typename dest_t>
class prepare_begin_type : public node {
	dest_t dest;
	prepare_result & r;
public:
	typedef void * item_type;

	prepare_begin_type(dest_t dest, prepare_result & r)
		: dest(dest)
		, r(r)
	{
		add_push_destination(dest);
	}

	virtual void prepare() override {
		log_debug() << "Prepare 1" << std::endl;
		r.prep1 = r.t++;
		set_minimum_memory(r.memWanted1);
		forward("t", r.t);
	}

	virtual void propagate() override {
		log_debug() << "Propagate 1" << std::endl;
		r.propagate1 = r.t++;
		r.memGotten1 = get_available_memory();
		forward("t", r.t);
	}

	virtual void begin() override {
		log_debug() << "Begin 1" << std::endl;
		r.begin1 = r.t++;
	}

	virtual void go() override {
		// We don't test go()/push() in this unit test.
	}

	virtual void set_available_memory(memory_size_type mem) override {
		node::set_available_memory(mem);
		log_debug() << "Begin memory " << mem << std::endl;
	}

	virtual void end() override {
		r.end1 = r.t++;
	}
};

inline pipe_begin<factory_1<prepare_begin_type, prepare_result &> >
prepare_begin(prepare_result & r) {
	return factory_1<prepare_begin_type, prepare_result &>(r);
}

template <typename dest_t>
class prepare_middle_type : public node {
	dest_t dest;
	prepare_result & r;
public:
	typedef void * item_type;

	prepare_middle_type(dest_t dest, prepare_result & r)
		: dest(dest)
		, r(r)
	{
		add_push_destination(dest);
	}

	virtual void prepare() override {
		log_debug() << "Prepare 2" << std::endl;
		if (!can_fetch("t")) {
			log_error() << "Couldn't fetch time variable in middle::prepare" << std::endl;
		} else if (fetch<size_t>("t") != r.t) {
			log_error() << "Time is wrong" << std::endl;
		}
		r.prep2 = r.t++;
		set_minimum_memory(r.memWanted2);
		forward("t", r.t);
	}

	virtual void propagate() override {
		log_debug() << "Propagate 2" << std::endl;
		if (!can_fetch("t")) {
			log_error() << "Couldn't fetch time variable in middle::propagate" << std::endl;
		} else if (fetch<size_t>("t") != r.t) {
			log_error() << "Time is wrong" << std::endl;
		}
		r.propagate2 = r.t++;
		r.memGotten2 = get_available_memory();
		forward("t", r.t);
	}

	virtual void begin() override {
		log_debug() << "Begin 2" << std::endl;
		r.begin2 = r.t++;
	}

	virtual void end() override {
		r.end2 = r.t++;
	}
};

inline pipe_middle<factory_1<prepare_middle_type, prepare_result &> >
prepare_middle(prepare_result & r) {
	return factory_1<prepare_middle_type, prepare_result &>(r);
}

class prepare_end_type : public node {
	prepare_result & r;
public:
	typedef void * item_type;

	prepare_end_type(prepare_result & r)
		: r(r)
	{
	}

	virtual void prepare() override {
		log_debug() << "Prepare 3" << std::endl;
		if (!can_fetch("t")) {
			log_error() << "Couldn't fetch time variable in end::prepare" << std::endl;
		} else if (fetch<size_t>("t") != r.t) {
			log_error() << "Time is wrong" << std::endl;
		}
		r.prep3 = r.t++;
		set_minimum_memory(r.memWanted3);
	}

	virtual void propagate() override {
		log_debug() << "Propagate 3" << std::endl;
		if (!can_fetch("t")) {
			log_error() << "Couldn't fetch time variable in end::propagate" << std::endl;
		} else if (fetch<size_t>("t") != r.t) {
			log_error() << "Time is wrong" << std::endl;
		}
		r.propagate3 = r.t++;
		r.memGotten3 = get_available_memory();
	}

	virtual void begin() override {
		log_debug() << "Begin 3" << std::endl;
		r.begin3 = r.t++;
	}

	virtual void end() override {
		r.end3 = r.t++;
	}
};

inline pipe_end<termfactory_1<prepare_end_type, prepare_result &> >
prepare_end(prepare_result & r) {
	return termfactory_1<prepare_end_type, prepare_result &>(r);
}

bool prepare_test() {
	prepare_result r;
	r.memWanted1 = 23;
	r.memWanted2 = 45;
	r.memWanted3 = 67;

	pipeline p = prepare_begin(r)
		| prepare_middle(r)
		| prepare_end(r);
	p();
	log_debug() << r << std::endl;
	TEST_ENSURE(r.prep1      == 0,  "Prep 1 time is wrong");
	TEST_ENSURE(r.prep2      == 1,  "Prep 2 time is wrong");
	TEST_ENSURE(r.prep3      == 2,  "Prep 3 time is wrong");
	TEST_ENSURE(r.propagate1 == 3,  "Propagate 1 time is wrong");
	TEST_ENSURE(r.propagate2 == 4,  "Propagate 2 time is wrong");
	TEST_ENSURE(r.propagate3 == 5,  "Propagate 3 time is wrong");
	TEST_ENSURE(r.begin3     == 6,  "Begin 3 time is wrong");
	TEST_ENSURE(r.begin2     == 7,  "Begin 2 time is wrong");
	TEST_ENSURE(r.begin1     == 8,  "Begin 1 time is wrong");
	TEST_ENSURE(r.end1       == 9,  "End 1 time is wrong");
	TEST_ENSURE(r.end2       == 10, "End 2 time is wrong");
	TEST_ENSURE(r.end3       == 11, "End 3 time is wrong");
	TEST_ENSURE(r.t          == 12, "Time is wrong after execution");

	TEST_ENSURE(r.memGotten1 == r.memWanted1, "Memory assigned to 1 is wrong");
	TEST_ENSURE(r.memGotten2 == r.memWanted2, "Memory assigned to 2 is wrong");
	TEST_ENSURE(r.memGotten3 == r.memWanted3, "Memory assigned to 3 is wrong");

	return true;
}

namespace end_time {

struct result {
	size_t t;

	size_t end1;
	size_t end2;

	friend std::ostream & operator<<(std::ostream & os, result & r) {
		return os
			<< "end1 = " << r.end1 << '\n'
			<< "end2 = " << r.end2 << '\n'
			<< "t    = " << r.t << '\n'
			<< std::endl;
	}
};

class begin_type : public node {
	result & r;

public:
	begin_type(result & r) : r(r) {
	}

	virtual void end() override {
		r.end1 = r.t++;
	}
};

pullpipe_begin<termfactory_1<begin_type, result &> >
inline begin(result & r) {
	return termfactory_1<begin_type, result &>(r);
}

template <typename dest_t>
class end_type : public node {
	result & r;
	dest_t dest;

public:
	end_type(dest_t dest, result & r) : r(r), dest(dest) {
		add_pull_source(dest);
	}

	virtual void go() override {
	}

	virtual void end() override {
		r.end2 = r.t++;
	}
};

pullpipe_end<factory_1<end_type, result &> >
inline end(result & r) {
	return factory_1<end_type, result &>(r);
}

bool test() {
	result r;
	r.t = 0;
	pipeline p = begin(r) | end(r);
	p.plot(log_info());
	p();
	log_debug() << r;
	TEST_ENSURE(r.end2 == 0, "End 2 time wrong");
	TEST_ENSURE(r.end1 == 1, "End 1 time wrong");
	TEST_ENSURE(r.t    == 2, "Time wrong");
	return true;
}

} // namespace end_time

bool pull_iterator_test() {
	outputvector.resize(inputvector.size());
	expectvector = inputvector;
	pipeline p =
		pull_input_iterator(inputvector.begin(), inputvector.end())
		| pull_output_iterator(outputvector.begin());
	p.plot(log_info());
	p();
	return check_test_vectors();
}

bool push_iterator_test() {
	outputvector.resize(inputvector.size());
	expectvector = inputvector;
	pipeline p =
		push_input_iterator(inputvector.begin(), inputvector.end())
		| push_output_iterator(outputvector.begin());
	p.plot(log_info());
	p();
	return check_test_vectors();
}

template <typename dest_t>
class multiplicative_inverter_type : public node {
	dest_t dest;
	const size_t p;

public:
	typedef size_t item_type;

	multiplicative_inverter_type(const dest_t & dest, size_t p)
		: dest(dest)
		, p(p)
	{
		add_push_destination(dest);
		set_steps(p);
	}

	void push(size_t n) {
		size_t i;
		for (i = 0; (i*n) % p != 1; ++i);
		dest.push(i);
		step();
	}
};

inline pipe_middle<factory_1<multiplicative_inverter_type, size_t> >
multiplicative_inverter(size_t p) {
	return factory_1<multiplicative_inverter_type, size_t>(p);
}

bool parallel_test(size_t modulo) {
	bool result = false;
	pipeline p = sequence_generator(modulo-1, true)
		| parallel(multiplicative_inverter(modulo))
		| sort()
		| sequence_verifier(modulo-1, &result);
	p.plot(log_info());
	tpie::progress_indicator_arrow pi("Parallel", 1);
	p(modulo-1, pi);
	return result;
}

bool parallel_ordered_test(size_t modulo) {
	bool result = false;
	pipeline p = sequence_generator(modulo-1, false)
		| parallel(multiplicative_inverter(modulo) | multiplicative_inverter(modulo), maintain_order)
		| sequence_verifier(modulo-1, &result);
	p.plot(log_info());
	tpie::progress_indicator_arrow pi("Parallel", 1);
	p(modulo-1, pi);
	return result;
}

template <typename dest_t>
class Monotonic : public node {
	dest_t dest;
	test_t sum;
	test_t chunkSize;
public:
	typedef test_t item_type;
	Monotonic(const dest_t & dest, test_t sum, test_t chunkSize)
		: dest(dest)
		, sum(sum)
		, chunkSize(chunkSize)
	{
		add_push_destination(dest);
	}

	virtual void go() override {
		while (sum > chunkSize) {
			dest.push(chunkSize);
			sum -= chunkSize;
		}
		if (sum > 0) {
			dest.push(sum);
			sum = 0;
		}
	}
};

pipe_begin<factory_2<Monotonic, test_t, test_t> >
monotonic(test_t sum, test_t chunkSize) {
	return factory_2<Monotonic, test_t, test_t>(sum, chunkSize);
}

template <typename dest_t>
class Splitter : public node {
	dest_t dest;
public:
	typedef test_t item_type;
	Splitter(const dest_t & dest)
		: dest(dest)
	{
		add_push_destination(dest);
	}

	void push(test_t item) {
		while (item > 0) {
			dest.push(1);
			--item;
		}
	}
};

pipe_middle<factory_0<Splitter> >
splitter() {
	return factory_0<Splitter>();
}

class Summer : public node {
	test_t & result;
public:
	typedef test_t item_type;
	Summer(test_t & result)
		: result(result)
	{
	}

	void push(test_t item) {
		result += item;
	}
};

pipe_end<termfactory_1<Summer, test_t &> >
summer(test_t & result) {
	return termfactory_1<Summer, test_t &>(result);
}

bool parallel_multiple_test() {
	test_t sumInput = 1000;
	test_t sumOutput = 0;
	pipeline p = monotonic(sumInput, 5) | parallel(splitter()) | summer(sumOutput);
	p.plot();
	p();
	if (sumInput != sumOutput) {
		log_error() << "Expected sum " << sumInput << ", got " << sumOutput << std::endl;
		return false;
	} else {
		return true;
	}
}

template <typename dest_t>
class buffering_accumulator_type : public node {
	dest_t dest;
	test_t inputs;

public:
	static const test_t bufferSize = 8;

	typedef test_t item_type;

	buffering_accumulator_type(dest_t dest)
		: dest(dest)
		, inputs(0)
	{
		add_push_destination(dest);
	}

	void push(test_t item) {
		inputs += item;
		if (inputs >= bufferSize) flush_buffer();
	}

	virtual void end() override {
		if (inputs > 0) flush_buffer();
	}

private:
	void flush_buffer() {
		while (inputs > 0) {
			dest.push(1);
			--inputs;
		}
	}
};

pipe_middle<factory_0<buffering_accumulator_type> >
buffering_accumulator() {
	return factory_0<buffering_accumulator_type>();
}

bool parallel_own_buffer_test() {
	test_t sumInput = 64;
	test_t sumOutput = 0;
	pipeline p =
		monotonic(sumInput, 1)
		| parallel(buffering_accumulator(), arbitrary_order, 1, 2)
		| summer(sumOutput);
	p.plot();
	p();
	if (sumInput != sumOutput) {
		log_error() << "Expected sum " << sumInput << ", got " << sumOutput << std::endl;
		return false;
	} else {
		return true;
	}
}

template <typename dest_t>
class noop_initiator_type : public node {
	dest_t dest;
public:
	noop_initiator_type(const dest_t & dest)
		: dest(dest)
	{
		add_push_destination(dest);
	}

	virtual void go() override {
		// noop
	}
};

pipe_begin<factory_0<noop_initiator_type> >
noop_initiator() {
	return factory_0<noop_initiator_type>();
}

template <typename dest_t>
class push_in_end_type : public node {
	dest_t dest;
public:
	typedef test_t item_type;

	push_in_end_type(const dest_t & dest)
		: dest(dest)
	{
		add_push_destination(dest);
	}

	void push(item_type) {
	}

	virtual void end() override {
		for (test_t i = 0; i < 100; ++i) {
			dest.push(1);
		}
	}
};

pipe_middle<factory_0<push_in_end_type> >
push_in_end() {
	return factory_0<push_in_end_type>();
}

bool parallel_push_in_end_test() {
	test_t sumOutput = 0;
	pipeline p =
		noop_initiator()
		| parallel(push_in_end(), arbitrary_order, 1, 10)
		| summer(sumOutput);
	p.plot(log_info());
	p();
	if (sumOutput != 100) {
		log_error() << "Wrong result, expected 100, got " << sumOutput << std::endl;
		return false;
	}
	return true;
}

template <typename dest_t>
class step_begin_type : public node {
	dest_t dest;
	static const size_t items = 256*1024*1024;

public:
	typedef typename dest_t::item_type item_type;

	step_begin_type(dest_t dest)
		: dest(dest)
	{
		add_push_destination(dest);
	}

	virtual void propagate() override {
		forward<stream_size_type>("items", items);
	}

	virtual void go() override {
		for (size_t i = 0; i < items; ++i) {
			dest.push(item_type());
		}
	}
};

pipe_begin<factory_0<step_begin_type> >
step_begin() {
	return factory_0<step_begin_type>();
}

template <typename dest_t>
class step_middle_type : public node {
	dest_t dest;

public:
	typedef typename dest_t::item_type item_type;

	step_middle_type(dest_t dest)
		: dest(dest)
	{
		add_push_destination(dest);
	}

	virtual void propagate() override {
		if (!can_fetch("items")) throw tpie::exception("Cannot fetch items");
		set_steps(fetch<stream_size_type>("items"));
	}

	void push(item_type i) {
		step();
		dest.push(i);
	}
};

pipe_middle<factory_0<step_middle_type> >
step_middle() {
	return factory_0<step_middle_type>();
}

class step_end_type : public node {
public:
	typedef size_t item_type;

	void push(item_type) {
	}
};

pipe_end<termfactory_0<step_end_type> >
step_end() {
	return termfactory_0<step_end_type>();
}

bool parallel_step_test() {
	pipeline p = step_begin() | parallel(step_middle()) | step_end();
	progress_indicator_arrow pi("Test", 0);
	p(get_memory_manager().available(), pi);
	return true;
}

class virtual_cref_item_type_test_helper {
public:
	template <typename In, typename Expect>
	class t {
		typedef typename tpie::pipelining::bits::maybe_add_const_ref<In>::type Out;
	public:
		typedef typename boost::enable_if<boost::is_same<Out, Expect>, int>::type u;
	};
};

bool virtual_cref_item_type_test() {
	typedef virtual_cref_item_type_test_helper t;
	t::t<int, const int &>::u t1 = 1;
	t::t<int *, int *>::u t2 = 2;
	t::t<int &, int &>::u t3 = 3;
	t::t<const int *, const int *>::u t4 = 4;
	t::t<const int &, const int &>::u t5 = 5;
	return t1 + t2 + t3 + t4 + t5 > 0;
}

class nocopy_tag;

static const nocopy_tag & nocopy = *static_cast<const nocopy_tag *>(0);

class node_map_tester : public node {
	friend class node_map_tester_factory;

	std::auto_ptr<node_map_tester> dest;

public:
	node_map_tester() {
		set_name("Node map tester leaf");
	}

	node_map_tester(const node_map_tester & copy)
		: node(copy)
	{
		if (copy.dest.get())
			dest.reset(new node_map_tester(*copy.dest));
	}

	node_map_tester(node_map_tester & copy, const nocopy_tag &)
		: dest(new node_map_tester(copy))
	{
		set_name("Node map tester non-leaf");
	}

	void add(std::vector<node_map_tester *> & v) {
		v.push_back(this);
		if (dest.get()) dest->add(v);
	}

	virtual void go() override {
		// Nothing to do.
	}
};

class node_map_tester_factory : public factory_base {
	size_t nodes;
	std::string edges;

public:
	typedef node_map_tester constructed_type;

	node_map_tester_factory(size_t nodes, const std::string & edges)
		: nodes(nodes)
		, edges(edges)
	{
		if (edges.size() != nodes*nodes) throw std::invalid_argument("edges has wrong size");
	}

	node_map_tester construct() const {
		std::vector<node_map_tester *> nodes;
		std::auto_ptr<node_map_tester> node;
		for (size_t i = 0; i < this->nodes; ++i) {
			node.reset(node.get() ? new node_map_tester(*node, nocopy) : new node_map_tester());
			this->init_node(*node);
		}
		node->add(nodes);
		for (size_t i = 0, idx = 0; i < this->nodes; ++i) {
			for (size_t j = 0; j < this->nodes; ++j, ++idx) {
				switch (edges[idx]) {
					case '.':
						break;
					case '>':
						log_debug() << i << " pushes to " << j << std::endl;
						nodes[i]->add_push_destination(*nodes[j]);
						break;
					case '<':
						log_debug() << i << " pulls from " << j << std::endl;
						nodes[i]->add_pull_source(*nodes[j]);
						break;
					case '-':
						nodes[i]->add_dependency(*nodes[j]);
						break;
					default:
						throw std::invalid_argument("Bad char");
				}
			}
		}
		return *node;
	}
};

bool node_map_test(size_t nodes, bool hasInitiator, const std::string & edges) {
	node_map_tester_factory fact(nodes, edges);
	pipeline p =
		tpie::pipelining::bits::pipeline_impl<node_map_tester_factory>(fact);
	p.plot(log_info());
	try {
		p();
	} catch (const no_initiator_node &) {
		return !hasInitiator;
	}
	return hasInitiator;
}

void node_map_multi_test(teststream & ts) {
	ts << "push_basic" << result
		(node_map_test
		 (4, true,
		  ".>.."
		  "..>."
		  "...>"
		  "...."));
	ts << "pull_basic" << result
		(node_map_test
		 (4, true,
		  "...."
		  "<..."
		  ".<.."
		  "..<."));
	ts << "phase_basic" << result
		(node_map_test
		 (3, true,
		  "..."
		  "-.."
		  ".-."));
	ts << "self_push" << result
		(node_map_test
		 (1, false,
		  ">"));

	ts << "actor_cycle" << result
		(node_map_test
		 (2, false,
		  ".>"
		  "<."));
	ts << "item_cycle" << result
		(node_map_test
		 (3, true,
		  ".><"
		  "..>"
		  "..."));
}

bool join_test() {
	std::vector<int> i(10);
	std::vector<int> o;

	for (int j = 0; j < 10; ++j) {
		i[j] = j;
	}

	join<int> j;
	pipeline p1 = input_vector(i) | j.sink();
	pipeline p2 = input_vector(i) | j.sink();
	pipeline p3 = j.source() | output_vector(o);

	p3.plot(log_info());
	p3();

	if (o.size() != 20) {
		log_error() << "Wrong output size " << o.size() << " expected 20" << std::endl;
		return false;
	}

	for (int i = 0; i < 20; ++i) {
		if (o[i] == i % 10) continue;
		log_error() << "Wrong output item got " << o[i] << " expected " << i%10 << std::endl;
		return false;
	}
	return true;
}

bool copy_ctor_test() {
	std::vector<int> i(10);
	std::vector<int> j;
	pipeline p1 = input_vector(i) | output_vector(j);
	pipeline p2 = p1;
	p2();
	return true;
}

struct datastructuretest {
	size_t totalMemory;
	size_t minMem1;
	size_t maxMem1;
	size_t minMem2;
	size_t maxMem2;
	double frac1;
	double frac2;

	size_t assigned1;
	size_t assigned2;
};

template <typename dest_t>
class datastructuretest_1 : public node {
	dest_t dest;
	datastructuretest & settings;

public:
	datastructuretest_1(const dest_t & dest, datastructuretest & settings)
		: dest(dest)
		, settings(settings)
	{
		add_push_destination(dest);
	}

	void prepare() {
		register_datastructure_usage("datastructure1", settings.frac1);
		register_datastructure_usage("datastructure2", settings.frac2);
		if (settings.maxMem1 > 0)
			set_datastructure_memory_limits("datastructure1", settings.minMem1, settings.maxMem1);
		else
			set_datastructure_memory_limits("datastructure1", settings.minMem1);

		if (settings.maxMem2 > 0)
			set_datastructure_memory_limits("datastructure2", settings.minMem2, settings.maxMem2);
		else
			set_datastructure_memory_limits("datastructure2", settings.minMem2);
	}

	 virtual void go() {

	 }
};

class datastructuretest_2 : public node {
	datastructuretest & settings;

public:
	datastructuretest_2(datastructuretest & settings)
		: settings(settings)
	{
	}

	void prepare() {
		register_datastructure_usage("datastructure1", settings.frac1);
		register_datastructure_usage("datastructure2", settings.frac2);
	}

	virtual void propagate() {
		settings.assigned1 = get_datastructure_memory("datastructure1");
		settings.assigned2 = get_datastructure_memory("datastructure2");
	}
};

bool datastructure_test(datastructuretest settings) {
	if (settings.minMem1 + settings.minMem2 > settings.totalMemory) {
		throw tpie::exception("Memory requirements too high");
	}

	const memory_size_type NO_MEM = std::numeric_limits<memory_size_type>::max();
	settings.assigned1 = settings.assigned2 = NO_MEM;

	progress_indicator_null pi;

	pipeline p =
		make_pipe_begin_1<datastructuretest_1, datastructuretest &>(settings)
		| make_pipe_end_1<datastructuretest_2, datastructuretest &>(settings);
	p(0, pi, settings.totalMemory);

	log_debug() << "totalMemory " << settings.totalMemory << '\n'
	            << "minMem1     " << settings.minMem1 << '\n'
	            << "maxMem1     " << settings.maxMem1 << '\n'
	            << "minMem2     " << settings.minMem2 << '\n'
	            << "maxMem2     " << settings.maxMem2 << '\n'
	            << "frac1       " << settings.frac1 << '\n'
	            << "frac2       " << settings.frac2 << '\n'
	            << "assigned1   " << settings.assigned1 << '\n'
	            << "assigned2   " << settings.assigned2 << std::endl;

	if (settings.assigned1 == NO_MEM || settings.assigned2 == NO_MEM) {
		log_error() << "No memory assigned" << std::endl;
		return false;
	}

	if (settings.assigned1 + settings.assigned2 > settings.totalMemory) {
		log_error() << "Too much memory assigned" << std::endl;
		return false;
	}

	if (settings.assigned1 < settings.minMem1 || settings.assigned2 < settings.minMem2) {
		log_error() << "Too little memory assigned" << std::endl;
		return false;
	}

	if ((settings.maxMem1 != 0 && settings.assigned1 > settings.maxMem1)
		|| (settings.maxMem2 != 0 && settings.assigned2 > settings.maxMem2)) {
		log_error() << "Too much memory assigned" << std::endl;
		return false;
	}

	const double EPS = 1e-9;
	const size_t min1 = settings.minMem1;
	const size_t max1 = (settings.maxMem1 == 0) ? settings.totalMemory : settings.maxMem1;
	const size_t min2 = settings.minMem2;
	const size_t max2 = (settings.maxMem2 == 0) ? settings.totalMemory : settings.maxMem2;
	const size_t m1 = settings.assigned1;
	const size_t m2 = settings.assigned2;
	const double f1 = settings.frac1;
	const double f2 = settings.frac2;
	if ((min1 < m1 && m1 < max1) && (min2 < m2 && m2 < max2)
		&& abs(m1 * f2 - m2 * f1) > EPS)
	{
		log_error() << "Fractions not honored" << std::endl;
		return false;
	}

	return true;
}

void datastructure_test_shorthand(teststream & ts, size_t totalMemory, size_t minMem1, size_t maxMem1, size_t minMem2, size_t maxMem2, double frac1, double frac2) {
	ts << "(" << totalMemory << ", " << minMem1 << ", " << maxMem1 << ", " << minMem2 << ", " << maxMem2 << ", " << frac1 << ", " << frac2 << ")";
	datastructuretest settings;
	settings.totalMemory = totalMemory;
	settings.minMem1 = minMem1;
	settings.maxMem1 = maxMem1;
	settings.minMem2 = minMem2;
	settings.maxMem2 = maxMem2;
	settings.frac1 = frac1;
	settings.frac2 = frac2;
	ts << result(datastructure_test(settings));
}

void datastructure_test_multi(teststream & ts) {
	//                        total   min1   max1   min2   max2  frac1  frac2
	datastructure_test_shorthand(ts,  2000,     0,     0,     0,     0,   1.0,   1.0);
	datastructure_test_shorthand(ts,  2000,   800,     0,   800,     0,   1.0,   1.0);
	datastructure_test_shorthand(ts,  4000,  1000,     0,  1000,     0,   0.0,   0.0);
	datastructure_test_shorthand(ts,  2000,     0,     0,     0,     0,   0.0,   1.0);
	datastructure_test_shorthand(ts,  2000,   500,     0,     0,     0,   0.0,   1.0);
	datastructure_test_shorthand(ts,  2000,   500,   700,     0,     0,   1.0,   1.0);
	datastructure_test_shorthand(ts,  2000,     0,   700,     0,   500,   1.0,   1.0);
	datastructure_test_shorthand(ts,  2000,     0,  2000,     0,  2000,   1.0,   1.0);
	datastructure_test_shorthand(ts,  2000,   200,  2000,     0,  2000,   1.0,   1.0);
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
	.setup(setup_test_vectors)
	.test(vector_multiply_test, "vector")
	.test(file_stream_test, "filestream", "n", static_cast<stream_size_type>(3))
	.test(file_stream_pull_test, "fspull")
	.test(file_stream_alt_push_test, "fsaltpush")
	.test(merge_test, "merge")
	.test(reverse_test, "reverse")
	.test(internal_reverse_test, "internal_reverse")
	.test(passive_reverse_test, "passive_reverse", "n", static_cast<size_t>(50000))
	.test(internal_passive_reverse_test, "internal_passive_reverse", "n", static_cast<size_t>(50000))
	.test(sort_test_trivial, "sorttrivial")
	.test(sort_test_small, "sort")
	.test(sort_test_large, "sortbig")
	.test(operator_test, "operators")
	.test(uniq_test, "uniq")
	.multi_test(memory_test_multi, "memory")
	.test(fork_test, "fork")
	.test(merger_memory_test, "merger_memory", "n", static_cast<size_t>(10))
	.test(fetch_forward_test, "fetch_forward")
	.test(virtual_test, "virtual")
	.test(virtual_fork_test, "virtual_fork")
	.test(virtual_cref_item_type_test, "virtual_cref_item_type")
	.test(prepare_test, "prepare")
	.test(end_time::test, "end_time")
	.test(pull_iterator_test, "pull_iterator")
	.test(push_iterator_test, "push_iterator")
	.test(parallel_test, "parallel", "modulo", static_cast<size_t>(20011))
	.test(parallel_ordered_test, "parallel_ordered", "modulo", static_cast<size_t>(20011))
	.test(parallel_step_test, "parallel_step")
	.test(parallel_multiple_test, "parallel_multiple")
	.test(parallel_own_buffer_test, "parallel_own_buffer")
	.test(parallel_push_in_end_test, "parallel_push_in_end")
	.test(join_test, "join")
	.multi_test(node_map_multi_test, "node_map")
	.test(copy_ctor_test, "copy_ctor")
	.multi_test(datastructure_test_multi, "datastructures")
	;
}
