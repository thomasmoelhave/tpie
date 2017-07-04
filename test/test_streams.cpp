#include <tpie/tpie.h>
#include <tpie/uncompressed_stream.h>
#include <tpie/file_stream.h>
#include <tpie/serialization_stream.h>

using namespace tpie;

constexpr size_t block_size() { return FILE_STREAM_BLOCK_SIZE; }

#define TEST_DIR "/hdd/tmp/tpie_old_speed_test/"
#define TEST_OLD_STREAMS

#include <compressed_stream_test/speed_test_common.h>

template <typename T>
struct serialization_adapter {
	serialization_writer writer;
	serialization_reader reader;
	serialization_reverse_reader reverse_reader;

	static const int end = -1;

	std::string current_path;
	bool opened[3] = {};

	~serialization_adapter() {
		if (opened[0]) writer.close();
		if (opened[1]) reader.close();
		if (opened[2]) reverse_reader.close();
	}

	template <typename S>
	void ensure_open(S & s) {
		void * p = &s;
		int i = p == &writer? 0: (p == &reader? 1: 2);
        if (!opened[i]) {
			s.open(current_path);
			opened[i] = true;
		}
	}

	void open(std::string path, access_type, size_t, cache_hint) {
		current_path = path;
	}

	void write(T v) {
		writer.serialize(v);
	}

	template <typename IT>
	void write(IT a, IT b) {
		writer.serialize(a, b);
	}

	T read() {
		T v;
		reader.unserialize(v);
		return v;
	}

	bool can_read() {
		return reader.can_read();
	}

	T read_back() {
		T v;
		reverse_reader.unserialize(v);
		return v;
	}

	void seek(int, int) {}
};


template <typename T>
void ensure_open_write(serialization_adapter<T> & fs) {
	fs.ensure_open(fs.writer);
}

template <typename T>
void ensure_open_read(serialization_adapter<T> & fs) {
	fs.ensure_open(fs.reader);
}

template <typename T>
void ensure_open_read_back(serialization_adapter<T> & fs) {
	fs.ensure_open(fs.reverse_reader);
}

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
		if (cmd_options.compression) skip();
        run_test<string_generator, serialization_adapter<std::string>>();
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
