#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include <string>

/* a basic struct storing a 3D segment. */
struct segment_t {
	double x1, y1, x2, y2;
	float z1, z2;
};

/* Make a dummy segment with given x1,y1, but setting other coordinates to 0. */
struct segment_t make_segment(double x1, double y1){
	struct segment_t s;
	s.x1=x1; s.y1=y1;
	s.x2=s.y2=s.z1=s.z2=0.f;
	return s;
}

/* Print x1, y1 coords of segment. */
void print_item(const struct segment_t& item){
  std::cout << "  (" << item.x1 << ", " << item.y1 << ")" << std::endl;
}

/* Write n segment items to tpie::file_stream with given output file name fname.
 * Overwrites any previous file. */
void writestream(const std::string& fname, size_t n){

	tpie::file_stream<struct segment_t> out;
	out.open(fname, tpie::open::write_only);
	for(size_t i=0; i<n; i++){
		struct segment_t item = make_segment(i, i*2.);
		print_item(item);
		out.write(item);
	}
}

/* Read all segments from a tpie::file_stream with file name given by fname and
 * prints results. */
void readstream(const std::string& fname){

	tpie::file_stream<struct segment_t> in;
	in.open(fname, tpie::open::read_only);

  std::cout << "  size of stream " << in.size() << std::endl;

	while (in.can_read()) {
		segment_t item = in.read();
		print_item(item);
	}
}

/* Copy infile name to outfile name. Assumes infile is a valid tpie::file_stream
 * and overwrites outfile. */
void copystream(const std::string & infile,
		const std::string & outfile) {
	tpie::file_stream<struct segment_t> in;
	tpie::file_stream<struct segment_t> out;

	in.open(infile);
	out.open(outfile, tpie::open::write_only);
	while (in.can_read()) {
		segment_t item = in.read();
		out.write(item);
	}
}

/* Set up TPIE, write to a file, copy to another, read from the latter, and then
   shut down TPIE again. */
int main(int /*argc*/, char ** /*argv*/) {
	size_t memory_limit_mb = 50;

	/* start up TPIE subsystems, set memory limit */
	tpie::tpie_init();
	tpie::get_memory_manager().set_limit(memory_limit_mb*1024*1024);

	std::string a = "test.in";
	std::string b = "test.out";

  std::cout << "Writing '" << a << "'" << std::endl;
	writestream(a, 10);

  std::cout << std::endl << "Copying '" << a << "' to '" << b << std::endl;
	copystream(a, b);

  std::cout << std::endl << "Reading '" << b << "'" << std::endl;
	readstream(b);

	/* wrap up */
	tpie::tpie_finish();

	return 0;
}
