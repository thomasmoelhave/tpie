// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino=(0 :
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

#include <iostream>
#include <tpie/file_stream.h>
#include "unit/common.h"
#include <cstdlib>
#include <string>
#include <sstream>

static void usage(int exitcode = -1) {
	std::cout << "Parameters: -t <type> [stream1 [stream2 ...]]" << std::endl;
	if (exitcode >= 0) exit(exitcode);
}

template <typename T>
static inline void output_item(const T & item) {
	std::cout << item << '\n';
}

template <>
inline void output_item<char>(const char & item) {
	std::cout << item;
}

template <typename T>
static int read_files(std::vector<std::string> files) {
	int result = 0;
	for (std::vector<std::string>::iterator i = files.begin(); i != files.end(); ++i) {
		std::string filename = *i;
		tpie::file_stream<T> fs;
		try {
			fs.open(filename, tpie::file_base::read);
		} catch (const tpie::stream_exception & e) {
			std::cerr << "Couldn't open " << filename << ": " << e.what() << std::endl;
			result = 1;
			continue;
		}
		while (fs.can_read()) {
			output_item(fs.read());
		}
		fs.close();
	}
	return result;
}

template <typename T>
static int write_file(std::string filename, double blockFactor) {
	tpie::file_stream<T> fs(blockFactor);
	try {
		fs.open(filename);
	} catch (const tpie::stream_exception & e) {
		std::cerr << "Couldn't open " << filename << ": " << e.what() << std::endl;
		return 1;
	}
	T el;
	while (std::cin >> el) {
		fs.write(el);
	}
	fs.close();
	return 0;
}

template <typename base_t>
struct parameter_parser_base {
	inline parameter_parser_base(int argc, char ** argv)
		: m_blockSize(0), m_shouldWrite(false), argc(argc), argv(argv) {
	}

	int parse(int offset = 0) {
		if (argc <= offset) return finish();
		std::string arg(argv[offset]);
		if (arg == "-b") {
			std::stringstream(argv[offset+1]) >> m_blockSize;
			return parse(offset+2);
		}
		if (arg == "-o") {
			m_outputFile = argv[offset+1];
			m_shouldWrite = true;
			return parse(offset+2);
		}
		if (arg == "-t") {
			return handle_type_parameter(offset+1);
		}
		return handle_input_file(offset);
	}

protected:

	size_t m_blockSize;
	bool m_shouldWrite;
	std::string m_outputFile;
	std::vector<std::string> m_inputFiles;

	int argc;
	char ** argv;

private:

	int finish() {
		return static_cast<base_t*>(this)->finish();
	}

	int handle_type_parameter(int offset) {
		return static_cast<base_t*>(this)->handle_type_parameter(offset);
	}

	int handle_input_file(int offset) {
		return static_cast<base_t*>(this)->handle_input_file(offset);
	}
};

template <typename T>
struct parameter_parser : public parameter_parser_base<parameter_parser<T> > {
	int handle_type_parameter(int /*offset*/) {
		usage();
		return 1;
	}

	int handle_input_file(int offset) {
		std::string arg(this->argv[offset]);
		this->m_inputFiles.push_back(arg);
		return this->parse(offset+1);
	}

	int finish() {
		double blockFactor = this->m_blockSize ? (double) (2 << 20) / this->m_blockSize : 1.0;
		if (this->m_shouldWrite) {
			return write_file<T>(this->m_outputFile, blockFactor);
		} else {
			return read_files<T>(this->m_inputFiles);
		}
	}
};

struct parameter_parser_notype : public parameter_parser_base<parameter_parser_notype> {
	inline parameter_parser_notype(int argc, char ** argv)
		: parameter_parser_base(argc, argv) {
	}

	int finish() {
		usage();
		return 1;
	}

	int handle_input_file(int offset) {
		std::string arg(argv[offset]);
		this->m_inputFiles.push_back(arg);
		return parse(offset+1);
	}

	int handle_type_parameter(int offset) {
		std::string arg(argv[offset]);
		std::stringstream types;

#define trytype(target) \
			types << #target << '\n';\
			if (arg == #target)\
				return parameter_parser<target>(reinterpret_cast<parameter_parser<target> &>(*this)).parse(offset+1)

		trytype(size_t);
		trytype(char);
		usage();
		std::cout << "Accepted types:\n" << types.str() << std::flush;
		return 1;
	}
};

template <typename T>
static int use_type(int argc, char ** argv) {
	parameter_parser<T> p(argc, argv);
	return p.parse();
}

int main(int argc, char ** argv) {
	tpie_initer _;
	parameter_parser_notype(argc-1, argv+1).parse();
	return EXIT_FAILURE;
}
