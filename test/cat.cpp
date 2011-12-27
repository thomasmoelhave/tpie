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
	std::cout << "Parameters: [type] [stream1 [stream2 ...]]" << std::endl;
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

struct null_t;

template <typename T = null_t>
struct parameter_parser {
	inline parameter_parser(int argc, char ** argv)
		: argc(argc), argv(argv) {
	}

	int parse(int offset = 0) {
		if (argc <= offset) return finish();
		std::string arg(argv[offset]);
		if (arg == "-b") {
			std::stringstream(argv[offset+1]) >> m_blockSize;
			return parse(offset+2);
		}
		if (arg == "-o") {
			m_outputFile = argv[1];
			m_shouldWrite = true;
			return parse(offset+2);
		}
		m_inputFiles.push_back(arg);
		return parse(offset+1);
	}

private:

	size_t m_blockSize;
	bool m_shouldWrite;
	std::string m_outputFile;
	std::vector<std::string> m_inputFiles;

	int argc;
	char ** argv;

	int finish() {
		double blockFactor = m_blockSize ? (double) (2 << 20) / m_blockSize : 1.0;
		if (m_shouldWrite) {
			return write_file<T>(m_outputFile, blockFactor);
		} else {
			return read_files<T>(m_inputFiles);
		}
	}
};

template <typename T>
static int use_type(int argc, char ** argv) {
	parameter_parser<T> p(argc, argv);
	return p.parse();
}

#define trytype(target) do { types << #target << '\n'; if (type != #target) break; return use_type<target>(argc, argv); } while (0)

int main(int argc, char ** argv) {
	tpie_initer _;
	std::string type = "";
	--argc, ++argv;
	if (argc >= 1) {
		type = argv[1];
		--argc, ++argv;
	}
	std::stringstream types;
	trytype(size_t);
	trytype(char);
	std::cout << "Recognized types:\n" << types.str() << std::flush;
	usage();
	return EXIT_FAILURE;
}
