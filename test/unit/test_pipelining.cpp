// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2011, The TPIE development team
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
#include <tpie/pipelining/graph.h>
#include <tpie/sysinfo.h>
#include <tpie/pipelining/virtual.h>

using namespace tpie;
using namespace tpie::pipelining;

typedef uint64_t test_t;

template <typename dest_t>
struct multiply_t : public pipe_segment {
	typedef test_t item_type;

	inline multiply_t(const dest_t & dest, uint64_t factor)
		: dest(dest)
		, factor(factor)
	{
		set_minimum_memory(17000000);
		add_push_destination(dest);
		set_name("Multiply");
	}

	virtual void begin() /*override*/ {
		pipe_segment::begin();
		log_debug() << "multiply begin with memory " << this->get_available_memory() << std::endl;
	}

	void push(const test_t & item) {
		dest.push(factor*item);
	}

	dest_t dest;
	uint64_t factor;
};

pipe_middle<factory_1<multiply_t, uint64_t> > multiply(uint64_t factor) {
	return factory_1<multiply_t, uint64_t>(factor);
}

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

void file_system_cleanup() {
	boost::filesystem::remove("input");
	boost::filesystem::remove("output");
}

bool file_stream_test() {
	file_system_cleanup();
	{
		file_stream<test_t> in;
		in.open("input");
		in.write(1);
		in.write(2);
		in.write(3);
	}
	{
		file_stream<test_t> in;
		in.open("input");
		file_stream<test_t> out;
		out.open("output");
		// p is actually an input_t<multiply_t<multiply_t<output_t<test_t> > > >
		pipeline p = (input(in) | multiply(3) | multiply(2) | output(out));
		p.plot(log_info());
		p();
	}
	{
		file_stream<test_t> out;
		out.open("output");
		if (6 != out.read()) return false;
		if (12 != out.read()) return false;
		if (18 != out.read()) return false;
	}
	return true;
}

bool file_stream_pull_test() {
	file_system_cleanup();
	{
		file_stream<test_t> in;
		in.open("input");
		in.write(1);
		in.write(2);
		in.write(3);
	}
	{
		file_stream<test_t> in;
		in.open("input");
		file_stream<test_t> out;
		out.open("output");
		pipeline p = (pull_input(in) | pull_identity() | pull_output(out));
		p.get_segment_map()->dump(log_info());
		p.plot(log_info());
		p();
	}
	{
		file_stream<test_t> out;
		out.open("output");
		if (1 != out.read()) return false;
		if (2 != out.read()) return false;
		if (3 != out.read()) return false;
	}
	return true;
}

bool file_stream_alt_push_test() {
	file_system_cleanup();
	{
		file_stream<test_t> in;
		in.open("input");
		in.write(1);
		in.write(2);
		in.write(3);
	}
	{
		file_stream<test_t> in;
		in.open("input");
		file_stream<test_t> out;
		out.open("output");
		pipeline p = (input(in) | alt_identity() | output(out));
		p.plot(log_info());
		p();
	}
	{
		file_stream<test_t> out;
		out.open("output");
		if (1 != out.read()) return false;
		if (2 != out.read()) return false;
		if (3 != out.read()) return false;
	}
	return true;
}

bool merge_test() {
	{
		file_stream<test_t> in;
		in.open("input");
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
		in.open("input");
		file_stream<test_t> out;
		out.open("output");
		std::vector<test_t> inputvector2 = inputvector;
		pipeline p = input_vector(inputvector) | merge(pull_input(in)) | output(out);
		p.plot(log_info());
		p();
	}
	{
		file_stream<test_t> in;
		in.open("output");
		pipeline p = input(in) | output_vector(outputvector);
		p.plot(log_info());
		p();
	}
	return check_test_vectors();
}

bool reverse_test() {
	pipeline p1 = input_vector(inputvector) | reverser() | output_vector(outputvector);
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

template <typename dest_t>
struct sequence_generator : public pipe_segment {
	typedef size_t item_type;

	inline sequence_generator(const dest_t & dest, size_t elements)
		: dest(dest)
		, elements(elements)
	{
		add_push_destination(dest);
		set_name("Generate integers", PRIORITY_INSIGNIFICANT);
	}

	virtual void begin() /*override*/ {
		pipe_segment::begin();
		forward("items", static_cast<stream_size_type>(elements));
		set_steps(elements);
	}

	virtual void go() /*override*/ {
		for (size_t i = elements; i > 0; --i) {
			dest.push(i);
			step();
		}
	}
private:
	dest_t dest;
	size_t elements;
};

struct sequence_verifier : public pipe_segment {
	typedef size_t item_type;

	inline sequence_verifier(size_t elements, bool & result)
		: elements(elements)
		, expect(1)
		, result(result)
		, bad(false)
	{
		result = false;
		set_name("Verify integers", PRIORITY_INSIGNIFICANT);
	}

	virtual void begin() /*override*/ {
		if (!can_fetch("items")) {
			log_error() << "Sorter did not forward number of items" << std::endl;
			bad = true;
		}
		result = false;
	}

	inline void push(size_t element) {
		if (element != expect) {
			(bad ? log_debug() : log_error()) << "Got " << element << ", expected " << expect << std::endl;
			bad = true;
		}
		result = false;
		++expect;
	}

	virtual void end() /*override*/ {
		if (can_fetch("items")
			&& static_cast<stream_size_type>(elements) != fetch<stream_size_type>("items")) {

			log_error() << "Sorter did not send as many items as promised" << std::endl;
			bad = true;
		}
		result = !bad;
	}

private:
	size_t elements;
	size_t expect;
	bool & result;
	bool bad;
};

bool sort_test(size_t elements) {
	bool result = false;
	pipeline p = make_pipe_begin_1<sequence_generator>(elements)
		| pipesort()
		| make_pipe_end_2<sequence_verifier, size_t, bool &>(elements, result);
	p.plot(log_info());
	p();
	return result;
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
	pipeline p = input_vector(inputvector) | ((pipesort() | pipesort()) | output_vector(outputvector));
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

bool memory_test() {
	pipeline p = input_vector(inputvector).memory(1.1) | multiply(3).memory(3.2) | multiply(2).memory(3.3) | output_vector(outputvector).memory(2.3);
	p();
	tpie::pipelining::bits::graph_traits g(*p.get_segment_map());
	double fractions = g.sum_memory();
	memory_size_type memory = g.sum_minimum_memory();
	log_info() << fractions << std::endl << memory << std::endl;
	double d = fractions-(1.1+3.2+3.3+2.3);
	return d*d < 0.0001;
}

bool fork_test() {
	expectvector = inputvector;
	pipeline p = input_vector(inputvector).name("Input vector") | fork(output_vector(outputvector)) | bitbucket<test_t>(0);
	p();
	return check_test_vectors();
}

template <typename dest_t>
struct buffer_node_t : public pipe_segment {
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
struct FF1 : public pipe_segment {
	dest_t dest;
	FF1(const dest_t & dest) : dest(dest) {
		add_push_destination(dest);
		set_name("FF1");
	}
	virtual void begin() /*override*/ {
		pipe_segment::begin();
		my_item i;
		i.v1 = 1;
		forward("my_item", i);
	}
	virtual void go() /*override*/ {
	}
};

template <typename dest_t>
struct FF2 : public pipe_segment {
	dest_t dest;
	FF2(const dest_t & dest) : dest(dest) {
		add_push_destination(dest);
		set_name("FF2");
	}
};

bool fetch_forward_result;

struct FF3 : public pipe_segment {
	FF3() {
		set_name("FF3");
	}
	virtual void begin() /*override*/ {
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
	p();
	return fetch_forward_result;
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

template <typename dest_t>
class multiplicative_inverter_type : public pipe_segment {
	dest_t dest;
	const size_t p;

public:
	typedef size_t item_type;

	multiplicative_inverter_type(const dest_t & dest, size_t p)
		: dest(dest)
		, p(p)
	{
		add_push_destination(dest);
	}

	void push(size_t n) {
		size_t i;
		for (i = 0; (i*n) % p != 1; ++i);
		dest.push(i);
	}
};

inline pipe_middle<factory_1<multiplicative_inverter_type, size_t> >
multiplicative_inverter(size_t p) {
	return factory_1<multiplicative_inverter_type, size_t>(p);
}

bool parallel_test(size_t modulo) {
	bool result = false;
	pipeline p = make_pipe_begin_1<sequence_generator>(modulo-1)
		| parallel(multiplicative_inverter(modulo))
		| pipesort()
		| make_pipe_end_2<sequence_verifier, size_t, bool &>(modulo-1, result);
	p.plot(log_info());
	p();
	return result;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
	.setup(setup_test_vectors)
	.setup(file_system_cleanup)
	.test(vector_multiply_test, "vector")
	.test(file_stream_test, "filestream")
	.test(file_stream_pull_test, "fspull")
	.test(file_stream_alt_push_test, "fsaltpush")
	.test(merge_test, "merge")
	.test(reverse_test, "reverse")
	.test(sort_test_small, "sort")
	.test(sort_test_large, "sortbig")
	.test(operator_test, "operators")
	.test(uniq_test, "uniq")
	.test(memory_test, "memory")
	.test(fork_test, "fork")
	.test(merger_memory_test, "merger_memory", "n", static_cast<size_t>(10))
	.test(fetch_forward_test, "fetch_forward")
	.test(virtual_test, "virtual")
	.test(parallel_test, "parallel", "modulo", static_cast<size_t>(524287))
	;
}
