// Copyright (C) 2001-2003 Octavian Procopiuc
//
// File:    datagen.cpp
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
//
// $Id: datagen.cpp,v 1.1 2003-09-25 17:45:29 tavi Exp $

#include <portability.h>

// For floor.
#include <math.h>

#include <vector>

// TPIE config: choose BTE, block size, etc.
#include "app_config.h"

// TPIE streams.
#include <ami_stream.h>
#include <ami_scan.h>
#include <ami_scan_utils.h>
#include <cpu_timer.h>

#include <ami_point.h>

// Default side length of the bounding rectangle for random points.
#define MBR_SIDE_LENGTH 100000000
// Max value returned by random().
#define MAX_RANDOM ((double)0x7fffffff)

// Input types.
#define INPUT_UNDEFINED 0
#define INPUT_ASCII     1
#define INPUT_RAW       2
#define INPUT_STREAM    3
#define INPUT_RANDOM    4
#define INPUT_GARBAGE   5

// Output types
#define OUTPUT_UNDEFINED 0
#define OUTPUT_ASCII     1
#define OUTPUT_RAW       2
#define OUTPUT_STREAM    3
#define OUTPUT_SORTED    4

// Distribution types.
#define UNIFORM  0
#define DIAGONAL 1
#define CLUSTERED 2

#if defined(COORDT_INT)
#  define COORDT int
#elif defined(COORDT_LONG)
#  define COORDT long
#elif defined(COORDT_FLOAT)
#  define COORDT float
#elif defined(COORDT_DOUBLE)
#  define COORDT double
#elif defined(COORDT_LONGLONG)
#  define COORDT long long
#else // Nothing defined.
#  define COORDT_INT
#  define COORDT int
#endif

#ifndef DIM
#  define DIM 2
#endif

#if defined(IDT_OFFT)
#  define IDT off_t
#elif defined(IDT_SIZET)
#  define IDT size_t
#elif defined(IDT_VOID)
#  define IDT void
#  define NOID
#elif defined(NOID)
#  define IDT void
#else
#  define IDT_SIZET
#  define IDT size_t
#endif


#ifdef NOID
typedef AMI_point<COORDT, DIM> point_t;
#else
typedef AMI_record<COORDT, IDT, DIM> point_t;
#endif

class cmp_eps {
  COORDT eps_; // epsilon.
public:
  cmp_eps(COORDT eps = COORDT(1)): eps_(eps) {}
  int compare(const point_t& p1, const point_t& p2) const {
    size_t j = 0;
    while ((j < DIM) && (floor(double(p1[j]/eps_)) == floor(double(p2[j]/eps_))))
      j++;
    if (j == DIM)
      return 0;
    else
      return (floor(double(p1[j]/eps_)) < floor(double(p2[j]/eps_))) ? -1: 1;
  }
};
    

void datagen_usage(char* argv0) {
  cerr << "Usage: \n" 
       << "  To generate random points (TPIE stream):\n\t" << argv0
       << " -ic <point_count> [-r x_lo y_lo x_hi y_hi] [-d U|D|C] -o <output_file_name>\n"
       << "  To generate points from ASCII file (TPIE stream):\n\t" << argv0
       << " -ia <ascii_file_name> [...]  [-r x_lo y_lo x_hi y_hi] -o <output_file_name>\n"
       << "  To generate boxes (ASCII):\n\t" << argv0
       << " -b <box_edge_as_%_of_MBR_edge> -ic <box_count> [-r x_lo y_lo x_hi y_hi] -o <output_file_name>\n"
       << "  To concatenate or extract from TPIE streams:\n\t" << argv0
       << " -if <stream_file_name> [...]  [-r x_lo y_lo x_hi y_hi] -o <output_file_name>\n"
       << "  To flush memory:\n\t" << argv0
       << " -f <memory_size_in_MB>\n"
       << "  To constrain TIGER points use '-T'\n"
    ;
}

istream& operator>>(istream& s, point_t& p) {
  for (int i = 0; i < DIM; i++)
    s >> p[i];
#ifndef NOID
  return s >> p.id();
#else
  return s;
#endif
}

template<class T>
class scan_cat: AMI_scan_object {
  T lop_;
  T hip_;
  FILE* rawout_;
public:
  scan_cat(const T& lop, const T& hip, FILE* rawout = NULL): lop_(lop), hip_(hip), rawout_(rawout) {}
  AMI_err initialize() { return AMI_ERROR_NO_ERROR; }
  AMI_err operate(const T &in, AMI_SCAN_FLAG *sfin, T* out, AMI_SCAN_FLAG *sfout) {
    if (*sfin) {
      *sfout = (lop_ < in) && (in < hip_);
      *out = in;
      return AMI_SCAN_CONTINUE;
    } else 
      return AMI_SCAN_DONE;
  }
  AMI_err operate(const T &in, AMI_SCAN_FLAG *sfin) {
    if (*sfin) {
      if ((lop_ < in) && (in < hip_)) {
	if (rawout_ != NULL)
	  fwrite(&in, sizeof(T), 1, rawout_);
      }
      return AMI_SCAN_CONTINUE;
    } else 
      return AMI_SCAN_DONE;
  }
};


int main(int argc, char **argv) {

  // Log debugging info from the application, but not from the library. 
  tpie_log_init(TPIE_LOG_APP_DEBUG); 
 
   MM_manager.ignore_memory_limit();

  int input_type = INPUT_UNDEFINED;
  int output_type = OUTPUT_UNDEFINED;
  char* base_file_name = NULL;
  vector<char*> inputs;
  char* input_file_name = NULL;
  int point_count = 10000;
  // The size of the box as a percentage of the max box (should be
  // between 0.0 and 100.0)
  double box_size = 0.0;
  int distribution = UNIFORM;
  // How many points in a chunk on the diagonal distribution.
  int diagonal_chunk = 1;
  int i = 1, j;
  char* buf = NULL;
  size_t sz;

  cerr << "Point: " << DIM << " "
#if defined(COORDT_INT)
       << "int"
#elif defined(COORDT_LONG)
       << "long"
#elif defined(COORDT_LONGLONG)
       << "long long"
#elif defined(COORDT_FLOAT)
       << "float"
#elif defined(COORDT_DOUBLE)
       << "double"
#endif
       << " coordinates plus "
#if defined(NOID)
       << "no"
#elif defined(IDT_OFFT)
       << "one off_t"
#elif defined(IDT_SIZET)
       << "one size_t"
#endif
       << " ID."
       << " Size of point: "
       << sizeof(point_t) 
       <<" bytes.\n";

  // All random points are in the box defined by these two points.
  point_t lop, hip;
  for (j = 0; j < DIM; j++) {
    lop[j] = COORDT(0);
    hip[j] = COORDT(MBR_SIDE_LENGTH);
  }

  // For epsilon-grid-order. 
  COORDT epsilon = COORDT(1);

  // Parse command-line arguments.
  i = 1;
  while (i < argc) {
    if (argv[i][0] != '-') {
      cerr << argv[0] << ": " << argv[i] << ": Unrecognized option.\n";
      datagen_usage(argv[0]);
      exit(1);
    }
    switch (argv[i][1]) {
    case 'e':
      epsilon = COORDT(atof(argv[++i]));
      break;
    case 'r':
      for (j = 0; j < DIM; j++)
	lop[j] = COORDT(atof(argv[++i]));
      for (j = 0; j < DIM; j++)
	hip[j] = COORDT(atof(argv[++i]));
      break;
    case 'T':
      // Constrain TIGER points.
      lop[0] = -128000000;
      lop[1] =   21000000;
      hip[0] =  -65000000;
      hip[1] =   50000000;
      break;
    case 'i':
      switch (argv[i][2]) {
      case 'r': 
      case 'c':
	point_count = atoi(argv[++i]);
	input_type = INPUT_RANDOM;
	break;
      case 'g': // garbage; just to check writing bandwidth.
	point_count = atoi(argv[++i]);
	input_type = INPUT_GARBAGE;
	break;	
      case 'u': // unsorted; useful for merging streams.
      case 'f':
      case 'a': // ascii files.
	input_type = (argv[i][2] == 'a' ? INPUT_ASCII: INPUT_STREAM);
	while ((i+1 < argc) && (argv[i+1][0] != '-')) {
	  input_file_name = new char[128];
	  strncpy(input_file_name, argv[++i], 128);
	  inputs.push_back(input_file_name);
	}
	break;
      default:
	cerr << argv[0] << ": " << argv[i] << ": Unrecognized option.\n";
	datagen_usage(argv[0]);
	exit(1);
	break;
      }
      break;
    case 'd':
      switch (argv[++i][0]) {
      case 'u':
      case 'U':
	distribution = UNIFORM;
	break;
      case 'd':
      case 'D':
	distribution = DIAGONAL;
	if (argv[i][1] != 0)
	  diagonal_chunk = atoi(&argv[i][1]);
	break;
      case 'c':
      case 'C':
	distribution = CLUSTERED;
	break;
      default:
	cerr << argv[0] << ": Unrecognized distribution.\n";
	datagen_usage(argv[0]);
	exit(1);
	break;
      }
      break;
    case 'O':
      output_type = OUTPUT_RAW;
      if (argv[i][2] == 'A')
	output_type = OUTPUT_ASCII;
      base_file_name = new char[128];
      strncpy(base_file_name, argv[++i], 128);
      break;
    case 'o': // set output file for points.
      output_type = OUTPUT_STREAM;
      base_file_name = new char[128];
      strncpy(base_file_name, argv[++i], 128);
      break;
    case 's':
      output_type = OUTPUT_SORTED;
      base_file_name = new char[128];
      strncpy(base_file_name, argv[++i], 128);
      break;
    case 'b': // set size of boxes.
      box_size = atof(argv[++i]); // a percentage.
      assert(box_size > 0.0 && box_size <= 100.0);
      break;
    case 'f': // flush cache.
      sz = atoi(argv[++i])*1024*1024;
      buf = new char[sz];
      for (j = 0; j < sz; j++) 
	// buf[j] = (j <= 64000000 ? j % 128: buf[int((TPIE_OS_RANDOM()/MAX_RANDOM)*64000000)]);
        buf[j] = j % 128;
      delete [] buf;
      exit(0);
      break;
    case 'R':
      {	    
	cpu_timer atimer;
	point_t *pp, p;
	atimer.start();
	AMI_STREAM<point_t> *is = 
	  new AMI_STREAM<point_t>(argv[++i], AMI_READ_STREAM);
	cout << "Reading " << is->stream_len() << " points..." << flush;
	while (is->read_item(&pp) == AMI_ERROR_NO_ERROR)
	  ;
	delete is;
	cout << "Done.\n";
	atimer.stop();
	cout << "Timings: " << atimer << "\n";
      }
      exit(0);
      break;
    default:
      cerr << argv[0] << ": " << argv[i] << ": Unrecognized option.\n";
      datagen_usage(argv[0]);
      exit(1);
      break;
    }
    i++;
  }

  point_t p;
  cpu_timer atimer;
  TPIE_OS_SRANDOM(time(NULL));

  if (output_type == OUTPUT_UNDEFINED) {
    cerr << argv[0] << ": " << "No output file specified.\n";
    datagen_usage(argv[0]);
    exit(1);
  }


  for (j = 0; j < DIM; j++)
    assert(hip[j] >= lop[j]);

  if (box_size != 0.0) {
    // Generate boxes.
    COORDT side_length[DIM];
    for (j = 0; j < DIM; j++)
      side_length[j] = COORDT((hip[j] - lop[j]) * (box_size/100.00));
    //size_t side_length_0 = size_t((hip[0] - lop[0]) * sqrt(box_size/100.0));
    //size_t side_length_1 = size_t((hip[1] - lop[1]) * sqrt(box_size/100.0));
    cerr << "Generating " << point_count << " boxes.\n";
    cerr << "Edge lengths: ";
    for (j = 0; j <DIM; j++)
      cerr << side_length[j] << " ";
    cerr << "\n";

    ofstream ofs(base_file_name);
    if (!ofs) {
      cerr << "Error opening output file " << base_file_name << ".\n";
      exit(1);
    }

    for (i = 0; i < point_count; i++) {
      for (j = 0; j < DIM; j++)
	p[j] = COORDT((TPIE_OS_RANDOM()/MAX_RANDOM) * (hip[j] - lop[j] - side_length[j]))
	  + lop[j];
      for (j = 0; j < DIM; j++)
	ofs << p[j] << " ";
      for (j = 0; j < DIM; j++)
	ofs << p[j] + side_length[j] << " ";
      ofs << "\n";
    }

    exit(0);
  }

  AMI_STREAM<point_t>* out_stream = NULL;
  FILE* out_raw = NULL;
  ofstream* out_cxxstr = NULL;
  if (output_type == OUTPUT_STREAM || output_type == OUTPUT_SORTED) {
    out_stream = new AMI_STREAM<point_t>(base_file_name);
    if (out_stream->status() == AMI_STREAM_STATUS_INVALID) {
      cerr << "Error opening output stream in file " 
	   << base_file_name << ".\n";
      cerr << "Aborting.\n";
      delete out_stream;
      exit(1);
    }
    out_stream->seek(out_stream->stream_len());
  } else if (output_type == OUTPUT_RAW) {
    // Open for writing at the end of file.
    out_raw = fopen(base_file_name, "a");
  } else if (output_type == OUTPUT_ASCII) {
    out_cxxstr = new ofstream(base_file_name, ios::app);
  }

  switch (input_type) {
  case INPUT_RANDOM:
    cout << "Writing " << point_count << " random points ";
    switch (distribution) {
    case UNIFORM:
      cout << "(uniform)\n";
      break;
    case DIAGONAL:
      cout << "(diagonal " << diagonal_chunk << ")\n";
      break;
    case CLUSTERED:
      cout << "(clustered)\n";
      break;
    }
    cout << "MBR: [";
    for (j = 0; j < DIM; j++)
      cout << lop[j] << ",";
    cout << "\b] [";
    for (j = 0; j < DIM; j++)
      cout << hip[j] << ",";
    cout << "\b]\n";

    TPIE_OS_SRANDOM(time(NULL));
    atimer.start();
    for (i = 0; i < point_count; i++) {
      switch (distribution) {
      case UNIFORM:
	for (j = 0; j < DIM; j++)
	  p[j] = COORDT((TPIE_OS_RANDOM()/MAX_RANDOM) * (hip[j] - lop[j])) + lop[j];
	break;
      case DIAGONAL:
	for (j = 0; j < DIM; j++) {
	  COORDT left_corner = int(i / diagonal_chunk) * diagonal_chunk;
	  p[j] = COORDT(left_corner * 10 + 
			diagonal_chunk * 10 * (TPIE_OS_RANDOM() / MAX_RANDOM));
	}
	break;
      case CLUSTERED:
	cerr << "CLUSTERED: Unimplemented case.\n";
	exit(1);
	break;
      }

      if (output_type == OUTPUT_STREAM)
	out_stream->write_item(p);
      else if (output_type == OUTPUT_RAW)
	fwrite(&p, sizeof(point_t), 1, out_raw);
      else if (output_type == OUTPUT_ASCII)
	*out_cxxstr << p << endl;
      else
	cerr << "Error: bad or unimplemented case.\n";
    }
    atimer.stop();
    cout << "Done.\n";
    cout << "Timings: " << atimer << "\n";
    break;
  case INPUT_GARBAGE:
    cout << "Writing " << point_count << " points..." << flush;
    atimer.start();
    for (i = 0; i < point_count; i++) {
      for (j = 0; j < DIM; j++)
	p[j] = i;
      out_stream->write_item(p);
    }
    atimer.stop();
    cout << "Done.\n";
    cout << "Timings: " << atimer << "\n";
    break;    
  case INPUT_STREAM:
    {
      int i;
      cout << "Reading TPIE input... " << flush;
      AMI_err err = AMI_ERROR_NO_ERROR;
      AMI_STREAM<point_t>* s;
      scan_cat<point_t> cat(lop, hip, out_raw);
      point_count = 0;
      for (i = 0; i < inputs.size() && err == AMI_ERROR_NO_ERROR; i++) {
	s = new AMI_STREAM<point_t>(inputs[i], AMI_READ_STREAM);
	if (s->status() == AMI_STREAM_STATUS_INVALID) {
	  cerr << "Error opening input stream in file " << inputs[i] << ".\n";
	} else {
	  point_count += s->stream_len();
	  if (output_type == OUTPUT_STREAM)
	    err = AMI_scan(s, &cat, out_stream);
	  else if (output_type == OUTPUT_RAW)
	    err = AMI_scan(s, &cat);
	  else if (output_type == OUTPUT_SORTED) {
#if 0
	    if (epsilon == 0)
	      cerr << "Error: epsilon is 0.\n";
	    else {
	      cmp_eps ce(epsilon);
	      if (AMI_sort(s, out_stream, &ce) != AMI_ERROR_NO_ERROR)
		cerr << "Error sorting stream.\n";
	    }
#else
	    cerr << "Error: This option is no longer supported\n";
#endif
	  } else
	    cerr << "Error: bad or unimplemented case.\n";

	  cout << inputs[i] << " " << flush;
	}

	delete s;
	delete [] inputs[i];
      }

      cout << "\nDone (" 
	   << (output_type == OUTPUT_RAW ? point_count: out_stream->stream_len()) << " points).\n";
      if (err != AMI_ERROR_NO_ERROR) {
	cerr << argv[0] << ": Error reading ascii input.\n";
	exit(1);
      }
    }
    break;
  case INPUT_ASCII:
    {
      int i;
      cout << "Reading ascii input... " << flush;
      AMI_err err = AMI_ERROR_NO_ERROR;
      for (i = 0; i < inputs.size() && err == AMI_ERROR_NO_ERROR; i++) {
	ifstream ifs(inputs[i]);
	if (!ifs) {
	  cerr << argv[0] << ": Error opening input file " << argv[i] << "\n";
	  exit(1); 
	}
	cxx_istream_scan<point_t> read_input(&ifs);
	// Read ascii input.
	err = AMI_scan(&read_input, out_stream);
	cout << inputs[i] << " " << flush;
	delete [] inputs[i];
      }
      point_count = out_stream->stream_len();
      cout << "\nDone (" << point_count << " points).\n";
      
      if (err != AMI_ERROR_NO_ERROR) {
	cerr << argv[0] << ": Error reading ascii input.\n";
	exit(1);
      }
    }
    break;

  default:
    break;
  }

  if (output_type == OUTPUT_STREAM || output_type == OUTPUT_SORTED) {
    out_stream->persist(PERSIST_PERSISTENT);
    delete out_stream;
  } else if (output_type == OUTPUT_RAW) {
    fclose(out_raw);
  } else if (output_type == OUTPUT_ASCII) {
    delete out_cxxstr;
  }

  delete [] base_file_name;
  return 0;
}
