#include <iostream>
#include <tpie/file_stream.h>
#include "unit/common.h"
#include <cstdlib>
#include <string>
#include <sstream>

static void usage() {
	std::cout << "Parameters: [type] [stream1 [stream2 ...]]" << std::endl;
	exit(1);
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
static int read_files(int argc, char ** argv) {
	int result = 0;
	for (int i = 0; i < argc; ++i) {
		std::string filename(argv[i]);
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

template <typename T>
static int use_type(int argc, char ** argv) {
	size_t blockSize = 0;
	std::string output; bool should_write = false;
	while (argc) {
		std::string arg(argv[0]);
		if (arg == "-b") {
			--argc, ++argv;
			std::stringstream(argv[0]) >> blockSize;
		} else if (arg == "-o") {
			--argc, ++argv;
			output = argv[0];
			should_write = true;
		} else break;
		--argc, ++argv;
	}
	double blockFactor = blockSize ? (double) (2 << 20) / blockSize : 1.0;
	if (should_write) {
		return write_file<T>(output, blockFactor);
	} else {
		return read_files<T>(argc, argv);
	}
}

#define trytype(target) do { if (type != #target) break; return use_type<target>(argc, argv); } while (0)

int main(int argc, char ** argv) {
	tpie_initer _;
	if (argc < 2) usage();
	std::string type(argv[1]);
	argc -= 2, argv += 2;
	trytype(size_t);
	trytype(char);
	usage();
	return 1;
}
