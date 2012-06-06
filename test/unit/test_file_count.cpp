#include "common.h"
#include <tpie/array.h>
#include <tpie/file_accessor/file_accessor.h>
#include <tpie/file_count.h>
#include <tpie/file_stream.h>
#include <tpie/tempname.h>

using namespace tpie;

bool file_count_test() {
	temp_file tmp;
	memory_size_type avail = available_files();
	memory_size_type itemSize;
	memory_size_type blockSize;
	memory_size_type userDataSize = 0;
	{
		file_stream<int> src(1.0);
		src.open(tmp);
		blockSize = file<int>::block_size(1.0);
		itemSize = sizeof(int);
	}
	array<tpie::file_accessor::file_accessor> fs(avail+1);
	for (memory_size_type i = 0; i < avail; ++i) {
		try {
			fs[i].open(tmp.path(), true, false, itemSize, blockSize, userDataSize);
		} catch (tpie::exception & e) {
			std::cout << "After opening " << i << " files, got an unexcepted exception of type " << typeid(e).name() << std::endl;
			return false;
		}
	}
	std::cout << "Opened available_files() == " << avail << " files" << std::endl;
	try {
		fs[avail].open(tmp.path(), true, false, itemSize, blockSize, userDataSize);
	} catch (tpie::io_exception & e) {
		std::cout << "Opening another file yields an exception of type\n" << typeid(e).name() << " (" << e.what() << ")\nwhich is allowed per available_files()" << std::endl;
		return true;
	}
	std::cout << "available_files() is not a strict bound" << std::endl;
	return true;
}

int main(int argc, char ** argv) {
	unittests(argc, argv)
	.test<file_count_test>("basic")
	;
	return EXIT_FAILURE;
}
