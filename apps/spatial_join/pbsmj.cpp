// Copyright (c) 1997 Octavian Procopiuc
//
// File: pbsmj.cpp
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 03/30/97
// Last Modified: 01/28/99
//
// $Id: pbsmj.cpp,v 1.1 2003-11-21 17:01:09 tavi Exp $
//
// Rectangle intersection using the alg. of Patel and DeWitt.
//

#include <iostream>
#include <fstream>
using std::ifstream;
using std::ofstream;
using std::istream;
using std::ostream;
using std::cerr;
using std::cout;
using std::endl;
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

#include "app_config.h"
#include <ami.h>
#include "rectangle.h"
#include "striped.h"
#include <ami_scan_utils.h>
#include "parse_args.h"
#include "intertree.h"
#include "joinlog.h"

#ifdef USE_STL_SORT
#include <algorithm>
#endif

bool verbose = false;
size_t test_mm_size = 64*1024*1024; // Default mem. size.
int random_seed = 17;
 


#define MAX_NAME_LEN 128

// Sweeping algorithms:
#define ORIGINAL	1
#define STRIPED		2
#define TREE		3

static char def_if_red[] = "rectangles.red";
static char def_if_blue[] = "rectangles.blue";
static char def_of[] = "output";

static char *input_filename_red = def_if_red;
static char *input_filename_blue = def_if_blue;
static char *output_filename = def_of;

extern int register_new;
static int part_count;

// Default sweeping algorithm.
static int algorithm = ORIGINAL;

static const char as_opts[] = "R:B:p:a:o:h";
void parse_app_opt(char c, char *optarg)
{
    switch (c) {
    case 'R':
      input_filename_red = optarg;
      break;
    case 'B':
      input_filename_blue = optarg;
      break;
    case 'o':
      output_filename = optarg;
      break;
    case 'p':
      part_count = atol(optarg);
      //istrstream(optarg,strlen(optarg)) >> part_count;
      break;
    case 'a':
      algorithm = atol(optarg);
      //istrstream(optarg,strlen(optarg)) >> algorithm;
      if ((algorithm != ORIGINAL) && (algorithm != STRIPED) && (algorithm != TREE)) {
	cerr << "Invalid algorithm choice.\n";
	exit(-1);
      }
      break;
    case 'h':
      cout << "Usage: pbsmjoin [ -R <red_input_file_name> ] "
	   << "[ -B <blue_input_file_name> ]\n" 
	   << "\t[ -o <output_file_name> ] [ -p <part_count> ]"
	   << "\t[ -m <memory_size> ] [ -a <algorithm> ]\n" 
	   << "Algorithm can be 1, 2, or 3:\n" 
	   << "\t1: ORIGINAL, 2: STRIPED, 3: TREE." << endl;
      exit(0);
    }
}

extern "C" {void UsageStart(); void UsageEnd(char *);}

// Scan management object for finding intersections using 
// interval trees.
class inter_rect : AMI_scan_object {
  InterTree *inter0;
  InterTree *inter1;
  randInfo rand;
  AMI_STREAM<pair_of_rectangles> *outStream;
  
public:
  unsigned long intersection_count;
  inter_rect(AMI_STREAM<pair_of_rectangles> *output, 
	     int randseed = random_seed): rand(randseed), outStream(output) 
  { 
    inter0 = NULL; 
    inter1 = NULL; 
  }

  AMI_err initialize(void) {
    if (inter0 != NULL)
      delete inter0;
    if (inter1 != NULL)
      delete inter1;
    inter0 = new InterTree(0);
    inter1 = new InterTree(1);
    intersection_count = 0;
    return AMI_ERROR_NO_ERROR;
  };
  AMI_err operate(const rectangle &in0, const rectangle &in1, AMI_SCAN_FLAG *sfin)
  {
    if (sfin[0] && sfin[1]) {
      if (in0.ylo < in1.ylo) { // consume in0
        sfin[1] = false;
        inter1->DeleteOld(in0.ylo);
        intersection_count += inter1->Search(in0, outStream);
        inter0->Insert(in0, &rand);
      } else {
        sfin[0] = false;
        inter0->DeleteOld(in1.ylo);
        intersection_count += inter0->Search(in1, outStream);
        inter1->Insert(in1, &rand);
      }
    } else if (!sfin[0]) {
      if (!sfin[1]) {
        return AMI_SCAN_DONE;
      } else {// we only have in1
        inter0->DeleteOld(in1.ylo);
        intersection_count += inter0->Search(in1, outStream);
      }
    } else { // we only have in0
      inter1->DeleteOld(in0.ylo);
      intersection_count += inter1->Search(in0, outStream);
    }
    return AMI_SCAN_CONTINUE;
  }

  // method to scan from an array.
  unsigned long inMemoryScan(rectangle* red_a, unsigned long red_l, 
		    rectangle* blue_a, unsigned long blue_l)
  {
    unsigned long red_i, blue_i;
    rectangle r;
    if (inter0 != NULL)
      delete inter0;
    if (inter1 != NULL)
      delete inter1;
    inter0 = new InterTree(0);
    inter1 = new InterTree(1);
    intersection_count = 0;
    red_i = blue_i = 0;

    while (true) {
      if (red_i < red_l && blue_i < blue_l) {
	if (red_a[red_i].ylo < blue_a[blue_i].ylo) {
	  r = red_a[red_i++];
	  inter1->DeleteOld(r.ylo);
	  intersection_count += inter1->Search(r, outStream);
	  inter0->Insert(r, &rand);
	} else {
	  r = blue_a[blue_i++];
	  inter0->DeleteOld(r.ylo);
	  intersection_count += inter0->Search(r, outStream);
	  inter1->Insert(r, &rand);
	}
      } else if (red_i >= red_l) {
	if (blue_i >= blue_l) {
	  break;
	} else {
	  r = blue_a[blue_i++];
	  inter0->DeleteOld(r.ylo);
	  intersection_count += inter0->Search(r, outStream);
	}
      } else {		// blue_i >= blue_l
	r = red_a[red_i++];
	inter1->DeleteOld(r.ylo);
	intersection_count += inter1->Search(r, outStream);
      }
    }
    delete inter0;
    delete inter1;
    return intersection_count;
  }
};

// Scan management object that distributes data in tiles.
class Distributor : AMI_scan_object {
  AMI_STREAM<rectangle> **red_parts;
  AMI_STREAM<rectangle> **blue_parts;
  int tile_count_row, tile_count_col, part_count;
  double  tile_height, tile_width;
  rectangle mbr, tile;
  bool *written;
public:
  Distributor(rectangle ambr, 
	      int tcr, int tcc, int pc) :mbr(ambr)
  {
    tile_count_row = tcr; tile_count_col = tcc;  part_count = pc;
    red_parts = new (AMI_STREAM<rectangle> *)[pc];
    blue_parts = new (AMI_STREAM<rectangle> *)[pc];
    tile_height = (double) (mbr.yhi - mbr.ylo) / tile_count_row;
    tile_width = (double) (mbr.xhi - mbr.xlo) / tile_count_col;
    written = new bool[part_count];
  }

  ~Distributor()
  {
    int i;
    delete [] written; 
    for (i = 0; i < part_count; i++) {
      delete red_parts[i];
      delete blue_parts[i];
    }
    delete [] red_parts;
    delete [] blue_parts;
  }

  AMI_err process(const rectangle &r, int isRed)
  {
    // round robin.
    
    int i, j, rowLow, colLow, rowHigh, colHigh;
    for (i=0; i<part_count; i++)
      written[i] = false;
    rowLow = (int) ceil((r.ylo - mbr.ylo) / tile_height) - 1;
    rowLow = (rowLow > 0) ? rowLow: 0;
    rowHigh = (int) floor((r.yhi - mbr.ylo) / tile_height);
    colLow = (int) ceil((r.xlo - mbr.xlo) / tile_width) - 1;
    colLow = (colLow > 0) ? colLow : 0;
    colHigh = (int) floor((r.xhi - mbr.xlo) / tile_width);
    for (i = rowLow; i <= rowHigh; i++) {
      //cout << "i:" << i << endl;
      for (j = i*tile_count_row+colLow; j <= i*tile_count_row+colHigh; j++) {
	//cout << "j: " << j << endl;
	if (!written[j % part_count]) {
	  written[j % part_count] = true;
	  if (isRed) 
	    red_parts[j % part_count]->write_item(r);
	  else
	    blue_parts[j % part_count]->write_item(r);
	}
      }
    }
    
    /*
    // new version, dumber. untested.
    int i, j;
    for (i=0; i<part_count; i++)
      written[i] = false;
    for (i = 0; i < tile_count_row; i++) {
      for (j = 0; j < tile_count_col; j++) {
	tile = rectangle(0, mbr.xlo + j*tile_width, mbr.ylo + i*tile_height,
			 mbr.xlo + (j+1)*tile_width, mbr.ylo + (i+1)*tile_height);
	if (tile.intersects(r) && !written[(j+i*tile_count_col) % part_count]) {
	  written[(j+i*tile_count_col) % part_count] = true;
	  if (isRed) 
	    red_parts[(j+i*tile_count_col) % part_count]->write_item(r);
	  else
	    blue_parts[(j+i*tile_count_col) % part_count]->write_item(r);
	}
      }
    }
    */
  }
    
  AMI_err initialize(void) 
  {
    int i;
    for (i = 0; i < part_count; i++) {
      red_parts[i] = new AMI_STREAM<rectangle>;
      red_parts[i]->persist(PERSIST_PERSISTENT);
      blue_parts[i] = new AMI_STREAM<rectangle>;
      blue_parts[i]->persist(PERSIST_PERSISTENT);
    }
    //cout << "Initialized distributor.\n";
    return AMI_ERROR_NO_ERROR; 
  }

  AMI_err operate(const rectangle &in0, const rectangle &in1, AMI_SCAN_FLAG *sfin)
  {
    //cout << "Entering operate\n";
    if (sfin[0] && sfin[1]) {
      process(in0, 1);	// red rect.
      //cout << "processed:" << in0 << endl;
      process(in1, 0);	// blue rect.
      //cout << "processed:" << in1 << endl;
    } else if (!sfin[0]) {
      if (sfin[1])
	process(in1, 0);
      else {
	return AMI_SCAN_DONE;
      }
    } else
      process(in0, 1);
    return AMI_SCAN_CONTINUE;
  }

  AMI_STREAM<rectangle> **get_red_parts() { return red_parts; }
  AMI_STREAM<rectangle> **get_blue_parts() { return blue_parts; }
};

// Scan management object that reads data from a stream into an array.
class Reader: AMI_scan_object {
protected:
  rectangle *array;
  unsigned long current;
public:
  Reader() 
  { 
    array = NULL;
  }

  void setArray(rectangle *a)
  {
    array = a;
  }

  AMI_err initialize(void)
  {
    current = 0;
    return AMI_ERROR_NO_ERROR;
  }

  AMI_err operate(const rectangle &in, AMI_SCAN_FLAG *sfin)
  {
    if (*sfin) {
      array[current++] = in;
      return AMI_SCAN_CONTINUE;
    } else 
      return AMI_SCAN_DONE;
  }
};

// Implements the original PBSM sweeping algorithms.
// Returns the number of intersections found.
unsigned long pseudoScan(rectangle* red_a, unsigned long red_l, 
			 rectangle* blue_a, unsigned long blue_l,
			 AMI_STREAM<pair_of_rectangles> *outStream) 
{
  rectangle rr, br;
  unsigned long i, red_st, blue_st, intersection_count;
  pair_of_rectangles intersection;
  red_st = blue_st = intersection_count = 0;
  while (red_st < red_l && blue_st < blue_l) {
    if (red_a[red_st].ylo < blue_a[blue_st].ylo) { //take the red one
      rr = red_a[red_st++];
      for (i = blue_st; i < blue_l; i++) {
	if ((br = blue_a[i]).ylo > rr.yhi)
	  break;
	if ((rr.xlo <= br.xlo && br.xlo <= rr.xhi) || 
	    (br.xlo <= rr.xlo && rr.xlo <= br.xhi)) {
	  intersection.first = rr.id; intersection.second = br.id; 
	  outStream->write_item(intersection);
	  intersection_count++;
	}
      }
    } else {
      br = blue_a[blue_st++];
      for (i = red_st; i < red_l; i++) {
	if ((rr = red_a[i]).ylo > br.yhi)
	  break;
	if ((rr.xlo <= br.xlo && br.xlo <= rr.xhi) || 
	    (br.xlo <= rr.xlo && rr.xlo <= br.xhi)) {
	  intersection.first = rr.id; intersection.second = br.id; 
	  outStream->write_item(intersection);
	  intersection_count++;
	}
      }
    }
  }
  return intersection_count;
}

// Try to get the mbr of a stream of rectangles
// using the ".mbr" meta file.
void getMbr(char *input_filename, rectangle *mbr) {

  // Add suffix ".mbr" to the input file name.
  char *mbr_filename = new char[strlen(input_filename)+5];
  strcpy(mbr_filename, input_filename);
  mbr_filename = strcat(mbr_filename, ".mbr");
  // Read the mbr.
  ifstream *mbr_file_stream = new ifstream(mbr_filename);
  if (!(*mbr_file_stream))
    cerr << "Error: couldn't open " << mbr_filename << endl;
  else 
    mbr_file_stream->read((char *) mbr, sizeof(rectangle));

  delete mbr_file_stream;
  delete [] mbr_filename;
}

int computePartitionCount(unsigned int rect_count) {
  int p;
  size_t sz_avail, sz_of_all_rects;
  sz_avail = MM_manager.memory_available();
  //  MM_manager.available(&sz_avail);
  long space_needed, prev_space_needed = 0;

  sz_of_all_rects = sizeof(rectangle) * (rect_count);
  for (p = 1; 1; p++) {
    space_needed = p*(sz_avail - 2*p*(MAX_NAME_LEN + 2*sizeof(char *)));
    if (space_needed <= prev_space_needed) { // went too far.
      p--;
      break;
    } else
      prev_space_needed = space_needed;
    if (space_needed > sz_of_all_rects)
      break;
  }
  return p;
}

// Sets things up, runs the distribution procedure, 
// and runs the apropriate join procedure.
void pbsmJoin()
{
  char **name_array_red, **name_array_blue;
  AMI_STREAM<rectangle> **red_parts, **blue_parts;
  AMI_STREAM<rectangle> *input_stream_red, *input_stream_blue;

  rectangle mbr_red, mbr_blue, mbr;
  Distributor *distributor;
  Reader *reader;
  size_t sz_avail;
  int tile_count_row, tile_count_col, i, p;
 
  cerr << "*******\tPBSM Join (" 
       << (algorithm==STRIPED?"STRIPED":(algorithm==ORIGINAL?"ORIGINAL":"TREE")) 
       << ")\t*******\n";
  cerr << "Memory size set to " << test_mm_size << " bytes.\n";

  // Create the input streams.
  input_stream_red = new AMI_STREAM<rectangle>(input_filename_red, AMI_READ_STREAM);
  input_stream_red->persist(PERSIST_PERSISTENT);
  input_stream_blue = new AMI_STREAM<rectangle>(input_filename_blue, AMI_READ_STREAM);
  input_stream_blue->persist(PERSIST_PERSISTENT);

  JoinLog jl("PBSM", 
	     input_filename_red,  input_stream_red->stream_len(),
	     input_filename_blue, input_stream_blue->stream_len());

  cerr << "Initial Stream length: " 
       << input_stream_red->stream_len()+input_stream_blue->stream_len() << endl;

  // Get the mbrs of the red and blue rectangles from the .mbr files.
  getMbr(input_filename_red, &mbr_red);
  getMbr(input_filename_blue, &mbr_blue);

  // Compute the mbr of the above 2 mbrs.
  mbr = rectangle(0, ((mbr_red.xlo < mbr_blue.xlo) ? mbr_red.xlo : mbr_blue.xlo),
		  ((mbr_red.ylo < mbr_blue.ylo) ? mbr_red.ylo : mbr_blue.ylo),
		  ((mbr_red.xhi > mbr_blue.xhi) ?  mbr_red.xhi : mbr_blue.xhi),
		  ((mbr_red.yhi > mbr_blue.yhi) ? mbr_red.yhi : mbr_blue.yhi));

  jl.UsageStart();
  
  tile_count_row  = 128;//32;
  tile_count_col  = 128;//32;
  sz_avail = MM_manager.memory_available();
  //MM_manager.available(&sz_avail);
  //cerr << "Before: " << sz_avail << endl;

  // Compute the # of partitions.
  part_count = computePartitionCount(input_stream_red->stream_len() +
				     input_stream_blue->stream_len());
  
  cerr << "Partitions: " << part_count << endl;
  //  cout << "Avail. Mem. Before Distribution: " << sz_avail << endl;
  if (part_count > 1) {

    distributor = new Distributor(mbr, tile_count_row, tile_count_col, part_count);
    
    AMI_scan(input_stream_red, input_stream_blue, distributor);

    // Get the names of the partition files.
    name_array_red = new (char *)[part_count];
    name_array_blue = new (char *)[part_count];
    red_parts = distributor->get_red_parts();
    blue_parts = distributor->get_blue_parts();
    for (i = 0; i < part_count; i++) {
      red_parts[i]->name(&name_array_red[i]);
      cerr << " r" << red_parts[i]->stream_len();
      blue_parts[i]->name(&name_array_blue[i]);
      cerr << " b" << blue_parts[i]->stream_len();
    }
    
    delete distributor;

  } else {
    red_parts = new (AMI_STREAM<rectangle> *);
    blue_parts = new (AMI_STREAM<rectangle> *);
    red_parts[0] = input_stream_red;
    blue_parts[0] = input_stream_blue;
  }
  sz_avail = MM_manager.memory_available();
  //  MM_manager.available(&sz_avail);
  cerr << "Finished distribution. Avail. mem: " << sz_avail << endl;

  // *******************************************************//

  rectangle *red_array, *blue_array;
  unsigned long red_length, blue_length;
  AMI_STREAM<pair_of_rectangles> *outStream;
  inter_rect *interObj;
  unsigned long intersection_count = 0;
 
  outStream = new AMI_STREAM<pair_of_rectangles>(output_filename);
  outStream->persist(PERSIST_PERSISTENT);

  if (algorithm == TREE)
    interObj = new inter_rect(outStream, 12345);
  
  if (algorithm == STRIPED) {
    cerr << "Striped PBSM disabled. Aborting.\n";
    return;
  }

  reader = new Reader;
  if (part_count > 1) {
    red_parts = new (AMI_STREAM<rectangle> *)[part_count];
    blue_parts = new (AMI_STREAM<rectangle> *)[part_count];
  }

  // Loop thru the partitions.
  for (i = 0; i < part_count; i++) {
    cerr << "loading partition " << i << endl;
    // Get the 2 streams for the ith partition.
    if (part_count > 1) {
      red_parts[i] = new AMI_STREAM<rectangle>(name_array_red[i]);
      blue_parts[i] = new AMI_STREAM<rectangle>(name_array_blue[i]);
      red_parts[i]->persist(PERSIST_DELETE);
      blue_parts[i]->persist(PERSIST_DELETE);
    }
    cerr << "opened the streams " << name_array_red[i] << " and " << name_array_blue[i] << endl;
    // Read red rectangles into red_array
    red_length = red_parts[i]->stream_len();
    red_array = new rectangle[red_length];
    reader->setArray(red_array);
    AMI_scan(red_parts[i], reader);
    delete red_parts[i];

    // Read blue rectangles into red_array
    blue_length = blue_parts[i]->stream_len();
    blue_array = new rectangle[blue_length];
    reader->setArray(blue_array);
    AMI_scan(blue_parts[i], reader);
    delete blue_parts[i];
    //cerr << "lengths: " << red_length << " " << blue_length << endl;
    // Sort the two arrays in memory.
#ifdef USE_STL_SORT    
    sort(red_array, red_array+red_length);
    sort(blue_array, blue_array+blue_length);
#else
    quick_sort_op(red_array, red_length);
    quick_sort_op(blue_array, blue_length);
#endif

    if (algorithm == ORIGINAL) {

      intersection_count += pseudoScan(red_array, red_length, 
				       blue_array, blue_length, outStream);

    } else if (algorithm == TREE) { 

      intersection_count += interObj->inMemoryScan(red_array, red_length, 
						   blue_array, blue_length);

    } else {
      // Should never reach this line.
      cerr << " Error: invalid algorithm value!\n";
    }

    delete [] red_array;
    delete [] blue_array;
  }
  
  delete reader;
  delete outStream;

  jl.UsageEnd("PBSM Intersection:", intersection_count);
  
  if (verbose) {
    cerr << "Intersection stream length: " 
	 << intersection_count << endl;
    cerr << "******\tDone PBSM Join\t******\n\n";
  }
}

int main(int argc, char **argv)
{
  //  register_new = 0;
  MM_manager.warn_memory_limit();
  verbose = true;
  part_count = 4;
  parse_args(argc,argv,as_opts,parse_app_opt);
  MM_manager.set_memory_limit(test_mm_size);
  pbsmJoin();
}
