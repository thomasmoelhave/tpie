#include <iostream>
#include <fstream>
#include <chrono>

#include <tpie/tpie.h>

using namespace tpie;

constexpr size_t block_size() { return FILE_STREAM_BLOCK_SIZE; }

#define TEST_DIR "/hdd/tmp/tpie_old_speed_test/"

size_t file_size = 1ull * 1024 * 1024 * 1024;
size_t blocks = file_size / block_size();

std::vector<std::string> words;

int main(int argc, char ** argv) {
	if (argc != 6) {
		std::cerr << "Usage: " << argv[0] << " compression readahead item_type test setup\n";
		return EXIT_FAILURE;
	}
	bool compression = (bool)std::atoi(argv[1]);
	bool readahead = (bool)std::atoi(argv[2]);
	int item_type = std::atoi(argv[3]);
	int test = std::atoi(argv[4]);
	bool setup = (bool)std::atoi(argv[5]);

	std::cerr << "Test info:\n"
			  << "  Block size:  " << block_size() << "\n"
			  << "  Compression: " << compression << "\n"
			  << "  Readahead:   " << readahead << "\n"
			  << "  Item type:   " << item_type << "\n"
			  << "  Test:        " << test << "\n"
			  << "  Action:      " << (setup? "Setup": "Run test") << "\n";

	{
		std::ifstream word_stream("/usr/share/dict/words");
		std::string w;
		while (word_stream >> w) {
			words.push_back(w);
		}
	}

	tpie_init();

	auto start = std::chrono::steady_clock::now();

	/*
	switch (item_type) {
	case 0:
		run_test<int, file_stream<int>>(setup, test, oflags);
		break;
	case 1:
		run_test<string_generator, serialized_file_stream<std::string>>(setup, test, oflags);
		break;
	case 2:
		run_test<keyed_generator, file_stream<keyed_generator::keyed_struct>>(setup, test, oflags);
		break;
	default:
		die("item_type out of range");
	}*/

	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> duration = end - start;

	std::cerr << "Duration: " << duration.count() << "s\n";

	tpie_finish();
}
