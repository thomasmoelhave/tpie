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
	serialization_reverse_writer reverse_writer;
	serialization_reader reader;
	serialization_reverse_reader reverse_reader;

	static const int end = -1;

	std::string current_path;
	int opened = -1;

	~serialization_adapter() {
		switch (opened) {
		case 0: writer.close(); break;
		case 1: reverse_writer.close(); break;
		case 2: reader.close(); break;
		case 3: reverse_reader.close(); break;
		}
	}

	template <typename S>
	void ensure_open(S & s) {
		void * sp = &s;

		int i = 0;
		for (void * p : std::initializer_list<void *>{&writer, &reverse_reader, &reader, &reverse_reader}) {
			if (p == sp) {
				break;
			}
			i++;
		}

		if (opened == -1) {
			s.open(current_path);
			opened = i;
		} else {
			assert(opened == i);
		}
	}

	void open(std::string path, open::type, size_t) {
		current_path = path;
	}

	void write(T v) {
		if (opened == 0) {
			writer.serialize(v);
		} else {
			reverse_writer.serialize(v);
		}
	}

	template <typename IT>
	void write(IT a, IT b) {
		if (opened == 0) {
			writer.serialize(a, b);
		} else {
			reverse_writer.serialize(a, b);
		}
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

	size_t size() {
		// reader and reverse_reader returns physical file size, not logical
		return no_file_size;
	}

	void seek(int, int) {}
};


template <typename T>
void ensure_open_write(serialization_adapter<T> & fs) {
	fs.ensure_open(fs.writer);
}

template <typename T>
void ensure_open_write_reverse(serialization_adapter<T> & fs) {
	fs.ensure_open(fs.reverse_writer);
}

template <typename T>
void ensure_open_read(serialization_adapter<T> & fs) {
	fs.ensure_open(fs.reader);
}

template <typename T>
void ensure_open_read_reverse(serialization_adapter<T> & fs) {
	fs.ensure_open(fs.reverse_reader);
}

int main(int argc, char ** argv) {
	speed_test_init(argc, argv);

	if (cmd_options.readahead) skip();

	tpie_init();

	auto start = std::chrono::steady_clock::now();

	switch (cmd_options.item_type) {
	case 0:
		run_test<int_generator, file_stream<long long>>();
		break;
	case 1: {
		if (cmd_options.compression) skip();
        run_test<string_generator, serialization_adapter<std::string>>();
		break;
	}
	case 2:
		run_test<keyed_generator, file_stream<keyed_generator::keyed_struct>>();
		break;
	default:
		die("item_type out of range");
	}

	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> duration = end - start;

	std::cerr << "Duration: " << duration.count() << "s\n";

	tpie_finish();
}
