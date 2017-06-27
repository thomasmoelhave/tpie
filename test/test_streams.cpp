#include <tpie/tpie.h>
#include <tpie/uncompressed_stream.h>
#include <tpie/file_stream.h>
#include <tpie/serialization_stream.h>

using namespace tpie;

constexpr size_t block_size() { return FILE_STREAM_BLOCK_SIZE; }

#define TEST_DIR "/hdd/tmp/tpie_old_speed_test/"
#define TEST_OLD_STREAMS

#include <compressed_stream_test/speed_test_common.h>

int main(int argc, char ** argv) {
	speed_test_init(argc, argv);

	if (cmd_options.readahead) skip();

	tpie_init();

	auto start = std::chrono::steady_clock::now();

	switch (cmd_options.item_type) {
	case 0:
		if (cmd_options.compression) {
			run_test<int_generator, file_stream<int>>();
			file_stream<int> f;
		} else {
			run_test<int_generator, uncompressed_stream<int>>();
			uncompressed_stream<int> f;
		}
		break;
	case 1: {
		skip();
		if (cmd_options.compression) skip();
		serialization_writer f1;
		serialization_reader f2;
		serialization_reverse_writer f3;
		serialization_reverse_reader f4;
		break;
	}
	case 2:
		if (cmd_options.compression) {
			run_test<keyed_generator, file_stream<keyed_generator::keyed_struct>>();
		} else {
			run_test<keyed_generator, uncompressed_stream<keyed_generator::keyed_struct>>();
		}
		break;
	default:
		die("item_type out of range");
	}

	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> duration = end - start;

	std::cerr << "Duration: " << duration.count() << "s\n";

	tpie_finish();
}
