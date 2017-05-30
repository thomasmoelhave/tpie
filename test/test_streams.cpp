#include <iostream>
#include <fstream>
#include <chrono>

#include <tpie/tpie.h>
#include <tpie/uncompressed_stream.h>
#include <tpie/file_stream.h>
#include <tpie/serialization_stream.h>

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

	if (item_type == 0) {
		if (compression) {
			file_stream<int> f;
		} else {
			uncompressed_stream<int> f;
		}
	} else if (item_type == 1) {
		assert(!compression);
		serialization_writer f1;
		serialization_reader f2;
		serialization_reverse_writer f3;
		serialization_reverse_reader f4;
	} else if (item_type == 2) {
		if (compression) {

		} else {

		}
	}

	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> duration = end - start;

	std::cerr << "Duration: " << duration.count() << "s\n";

	tpie_finish();
}
