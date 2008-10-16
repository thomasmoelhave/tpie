// Copyright (C) 2003 Octavian Procopiuc
//
// File:    test_correctness.cpp
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// An extensive test suite for TPIE functionality.
//
// $Id: test_correctness.cpp,v 1.11 2006-01-17 23:31:26 jan Exp $
//

#include <portability.h>

// TPIE configuration: choose BTE, block size, etc.
#include "app_config.h"

#include <config.h>

#if (!defined(BTE_STREAM_IMP_UFS) && !defined(BTE_STREAM_IMP_MMAP) && !defined(BTE_STREAM_IMP_STDIO))
#  define BTE_STREAM_IMP_UFS
#endif

#ifndef BTE_STREAM_UFS_BLOCK_FACTOR
#  define BTE_STREAM_UFS_BLOCK_FACTOR 32
#endif
#ifndef BTE_STREAM_MMAP_BLOCK_FACTOR
#  define BTE_STREAM_MMAP_BLOCK_FACTOR 32
#endif

// Use logs if requested.
#if TP_LOG_APPS
#  define TPL_LOGGING 1
#endif
#include <tpie_log.h>


// TPIE core classes and functions.
#include <ami.h>
#include <scan_utils.h>

// The getopts() function for reading command-line arguments.
#include "getopts.h"
// The scan_random class for generating random ints.
#include "scan_universal.h"


// Number of spaces to indent messages written during a test.
#define INDENT 2

// Status of a test.
enum status_t {
    EMPTY = 0,
    PASS,
    FAIL,
    SKIP,
    NA
};

// Dummy type for streams.
template <int sz>
struct foo_t {
    char el[sz];
};
const foo_t<40> thefoo = { "  This space for rent. Cheap.          " };


struct options opts[] = {
    { 1,  "memory", "Sets the TPIE memory limit.", "m", 1 },
    { 10, "stream", "Tests AMI_STREAM", NULL, 0 },
    { 20, "scan", "Tests AMI_scan", NULL, 0 },
    { 25, "scan-cxx", "Tests AMI_scan with C++ streams", NULL, 0 },
    //  { 31, "sort-op-int-int", "Test standard sorting (using integers and operator \"<\")", NULL, 0 },
    //  { 32, "sort-op-100b-int", "Test AMI_sort (using 40-byte elements, integers as keys, and operator \"<\")", NULL, 0 },
    //  { 33, "sort-op-100b-100b", "Test AMI_sort (using 100-byte elements, and operator \"<\")", NULL, 0 },
    //  { 34, "sort-cl-int-int", "Test AMI_sort (using integers and comparison class)", NULL, 0 },
    //  { 35, "sort-op-100b-int", "Test AMI_sort (using 40-byte elements, integers as keys, and comparison class)", NULL, 0 },
    //  { 36, "sort-op-100b-100b", "Test AMI_sort (using 40-byte elements, and comparison class)", NULL, 0 },
    { 30, "sort", "Tests AMI_sort", NULL, 0 },
    { 40, "large-file", "Tests large files (> 2GB)", NULL, 0 },
    { 0,  NULL, NULL, NULL, 0 }
};

// The test functions.
int test_stream();
int test_scan();
int test_scan_cxx();
int test_sort();
int test_large_files();
// Print current configuration options.
void print_cfg();



///////////////////////////////////////////////////
////////////////    main()     ////////////////////
///////////////////////////////////////////////////

int main(int argc, char **argv) {
    char *args;
    int idx;
    int fail = 0;

    // Log debugging info from the application, but not from the library. 
    tpie_log_init(TPIE_LOG_APP_DEBUG); 
 
    MM_manager.set_memory_limit(40*1024*1024);
    MM_manager.enforce_memory_limit();

    if (argc == 1) {
	getopts_usage(argv[0], opts);
	exit(0);
    }

    print_cfg();

    while ((idx = getopts(argc, argv, opts, &args)) != 0) {
	switch(idx) {
	case 1:  
	{
	    int mem_limit = atoi(args);
	    if (mem_limit < (2*1024*1024) )
		fprintf(stdout, "Attempting to set memory limit too low (min is 2MB). Did not change.\n");
	    else {
		fprintf(stdout, "Setting memory limit to %d KB.\n", mem_limit/1024);
		MM_manager.set_memory_limit(mem_limit);
	    }
	}
	break;
	case 10: fail += test_stream(); break;
	case 20: fail += test_scan(); break;
	case 25: fail += test_scan_cxx(); break;
	case 30: fail += test_sort(); break;
	case 40: fail += test_large_files(); break;
	default: break;
	}

	free(args);
    }

    if (fail)
	fprintf(stdout, "One or more sub-tests failed. See the log for more details.\n");
    else
	fprintf(stdout, "All test have completed successfully. See the log for more info.\n");

    return fail;
}



/////////////////////////////////////////////////////////
// Auxiliary: print_msg(), print_status(), print_cfg() //
/////////////////////////////////////////////////////////

void print_msg(const char* msg, int indent = 0) {
    if (msg == NULL)
		return;

    int len = static_cast<int>(strlen(msg));
	if (indent < 0) {
		for (int i=0; i<len; i++) fprintf(stdout, "\b");
	} else {
		for (int i=0; i<indent; i++) fprintf(stdout, " ");
	}
    fprintf(stdout, msg);
    TP_LOG_APP_DEBUG(">>-");
    TP_LOG_APP_DEBUG(msg);
    TP_LOG_APP_DEBUG("\n");
    if (indent >= 0) {
		int current_pos = indent + len;
		for (int i=current_pos; i<73; i++) fprintf(stdout, " ");
	}
    fflush(stdout);
}

void print_status(status_t status) {
    switch (status) {
    case EMPTY: 
	fprintf(stdout, "\n"); 
	TP_LOG_APP_DEBUG(">>-\n");
	break;
    case SKIP:  
	fprintf(stdout, " [SKIP]\n"); 
	TP_LOG_APP_DEBUG(">>-[SKIP]\n");
	break;
    case PASS:  
	// This prints in green, when possible.
#ifdef _WIN32
	fprintf(stdout, "[PASS]\n"); 
#else
	fprintf(stdout, " \033[1;32m[PASS]\033[0m\n"); 
#endif
	TP_LOG_APP_DEBUG(">>-[PASS]\n");
	break;
    case FAIL:  
	// This prints in red, when possible.
#ifdef _WIN32
	fprintf(stdout, "[FAIL]\n"); 
#else
	fprintf(stdout, " \033[1;31m[FAIL]\033[0m\n"); 
#endif
	TP_LOG_APP_DEBUG(">>-[FAIL]\n");
	break;
    case NA:    
	fprintf(stdout, " [N/Av]\n"); 
	TP_LOG_APP_DEBUG(">>-[N/Av]\n");
	break;
    }
}

void print_cfg() {
    fprintf(stdout, "TPIE Configuration\n");
    fprintf(stdout, "    Stream BTE: ");
#if defined(BTE_STREAM_IMP_UFS)
    fprintf(stdout, "UFS (%d)", BTE_STREAM_UFS_BLOCK_FACTOR);
#elif defined(BTE_STREAM_IMP_MMAP)
    fprintf(stdout, "MMAP (%d)", BTE_STREAM_MMAP_BLOCK_FACTOR);
#elif defined(BTE_STREAM_IMP_STDIO)
    fprintf(stdout, "STDIO");
#elif defined(BTE_STREAM_IMP_USER_DEFINED)
    fprintf(stdout, "USER_DEFINED");
#else
    fprintf(stdout, "UNSPECIFIED");
#endif
    fprintf(stdout, "\n");
    fprintf(stdout, "    TPIE Memory limit: %d KB\n", MM_manager.memory_limit()/1024);
#if TP_LOG_APPS
    fprintf(stdout, "    Logging in file %s\n", tpie_log_name());
#else
    fprintf(stdout, "    Logging is OFF\n");
#endif
    fprintf(stdout, "\n");
}




////////////////////////////////////////////////////////
//////////////     test_stream()      //////////////////
////////////////////////////////////////////////////////

int test_stream() {
    static bool been_here = false;
    status_t status = EMPTY;
    AMI_STREAM<foo_t<40> >* s;
    int failed = 0;
    AMI_err err;
    char *fn  = new char[strlen(TMP_DIR)+strlen("tpie00.stream")+1];
    strcpy(fn,TMP_DIR);
    strcpy(fn+strlen(fn),"tpie00.stream");

    char *pfn = NULL; // Pointer to a file name.
    foo_t<40> afoo = thefoo;
    foo_t<40> *pafoo;
    int i;
    struct stat buf;

    print_msg("Testing AMI_STREAM creation and destruction", 0);
    if (been_here) {
	print_status(SKIP);
	return 0;
    }

    been_here = true;
    print_status(EMPTY);


    //////// Part 1: temporary stream.       //////////

    print_msg("Creating temporary stream (calling op. new)", INDENT);
    s = new AMI_STREAM<foo_t<40> >;
    status = (s != NULL && s->is_valid() ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++;

    if (status != FAIL) {

	print_msg("Checking stream_len() (should return 0)", INDENT);
	status = (s->stream_len() == 0 ? PASS: FAIL);
	print_status(status); if (status == FAIL) failed++;
  
	print_msg("Inquiring file name: ", INDENT);
	err = s->name(&pfn);
	status = (err != AMI_ERROR_NO_ERROR || pfn == NULL || strlen(pfn) == 0 ? FAIL: PASS);
	if (pfn != NULL) print_msg(pfn, -1);
	print_status(status); if (status == FAIL) failed++;

	print_msg("Checking persist() (should return PERSIST_DELETE)", INDENT);
	status = (s->persist() == PERSIST_DELETE ? PASS: FAIL);
	print_status(status); if (status == FAIL) failed++;    
    
	print_msg("Checking write_item() (writing 1M items)", INDENT);
	err = AMI_ERROR_NO_ERROR;
	for (i=0; i<1000000; i++) {
	    afoo.el[0] = i % 128;
	    afoo.el[38] = (i+5) % 128;
	    if ((err = s->write_item(afoo)) != AMI_ERROR_NO_ERROR)
		break;
	}
	status = (err == AMI_ERROR_NO_ERROR && s->stream_len() == 1000000 ? PASS: FAIL);
	print_status(status); if (status == FAIL) failed++; 

	print_msg("Checking seek() (seeking to illegal position 1000001)", INDENT);
	err = s->seek(1000001);
	status = (err == AMI_ERROR_NO_ERROR ? FAIL: PASS);
	print_status(status); if (status == FAIL) failed++; 

	print_msg("Checking seek() (seeking to position 50000)", INDENT);
	err = s->seek(50000);
	status = (err == AMI_ERROR_NO_ERROR ? PASS: FAIL);
	print_status(status); if (status == FAIL) failed++; 

	print_msg("Checking read_item() (reading 10k items from current pos)", INDENT);
	for (i=50000; i<60000; i++) {
	    err = s->read_item(&pafoo);
	    if (err != AMI_ERROR_NO_ERROR || pafoo->el[0] != (i%128) || pafoo->el[38] != ((i+5)%128))
		break;
	}
	status = (err == AMI_ERROR_NO_ERROR && i == 60000 ? PASS: FAIL);
	print_status(status); if (status == FAIL) failed++; 

	print_msg("Checking tell() (should return 60000)", INDENT);
	status = (s->tell() == 60000 ? PASS: FAIL);
	print_status(status); if (status == FAIL) failed++; 

	print_msg("Checking truncate() (to 50000)", INDENT);
	err = s->truncate(50000);
	status = (err == AMI_ERROR_NO_ERROR && s->stream_len() == 50000 ? PASS: FAIL);
	print_status(status); if (status == FAIL) failed++; 

	print_msg("Checking tell() again (should return 50000)", INDENT);
	status = (s->tell() == 50000 ? PASS: FAIL);
	print_status(status); if (status == FAIL) failed++; 

    }
 
    print_msg("Destroying temp stream (file should be removed) (calling op. delete)", INDENT);
    delete s;
    status = (stat(pfn, &buf) == -1 && errno == ENOENT ? PASS: FAIL);
    delete pfn;
    print_status(status); if (status == FAIL) failed++;


    print_status(EMPTY); // New line.

    //////// Part 2: named stream.           //////////

    print_msg("Creating named writable stream (calling op. new)", INDENT);
    // Make sure there's no old file lingering around.
    unlink(fn);
    s = new AMI_STREAM<foo_t<40> >(fn);
    status = (s != NULL && s->is_valid() ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++;

    if (status != FAIL) {

	print_msg("Inquiring file name; should be called", INDENT);
	print_msg(fn, -1);
	err = s->name(&pfn);
	status = (err != AMI_ERROR_NO_ERROR || pfn == NULL || strcmp(pfn, fn) != 0 ? FAIL: PASS);
	if (pfn != NULL) print_msg(pfn, -1);
	print_status(status); if (status == FAIL) failed++;

	print_msg("Checking persist() (should return PERSIST_PERSISTENT)", INDENT);
	status = (s->persist() == PERSIST_PERSISTENT ? PASS: FAIL);
	print_status(status); if (status == FAIL) failed++;    
 
	print_msg("Checking write_item() (writing 1M items)", INDENT);
	err = AMI_ERROR_NO_ERROR;
	for (i=0; i<1000000; i++) {
	    afoo.el[0] = i % 128;
	    afoo.el[38] = (i+5) % 128;
	    if ((err = s->write_item(afoo)) != AMI_ERROR_NO_ERROR) break;
	}
	status = (err == AMI_ERROR_NO_ERROR && s->stream_len() == 1000000 ? PASS: FAIL);
	print_status(status); if (status == FAIL) failed++; 

	print_msg("Checking illegal read_item() at current pos", INDENT);
	err = s->read_item(&pafoo);
	status = (err == AMI_ERROR_END_OF_STREAM ? PASS: FAIL);
	print_status(status); if (status == FAIL) failed++;

    }

    print_msg("Closing named stream (file should NOT be removed) (calling op. delete)", INDENT);
    delete s;
    status = (stat(pfn, &buf) == 0 ? PASS: FAIL);
    delete pfn;
    print_status(status); if (status == FAIL) failed++;

    print_msg("Reopening named stream read-only (calling op. new)", INDENT);
    s = new AMI_STREAM<foo_t<40> >(fn, AMI_READ_STREAM);
    status = (s != NULL && s->is_valid() ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++;  
  
    if (status != FAIL) {

	print_msg("Checking stream_len() (should return 1M)", INDENT);
	status = (s->stream_len() == 1000000 ? PASS: FAIL);
	print_status(status); if (status == FAIL) failed++;

	print_msg("Checking illegal write_item() in read-only stream", INDENT);
	err = s->write_item(afoo);
	status = (err == AMI_ERROR_NO_ERROR || s->stream_len() != 1000000 ? FAIL: PASS);
	print_status(status); if (status == FAIL) failed++;
    
    }

    print_msg("Closing named stream (file should NOT be removed) (calling op. delete)", INDENT);
    delete s;
    status = (stat(fn, &buf) == 0 ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++;  


    print_msg("Reopening named stream for reading and writing (calling op. new)", INDENT);
    s = new AMI_STREAM<foo_t<40> >(fn, AMI_READ_WRITE_STREAM);
    status = (s != NULL && s->is_valid() ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++;  
  
    if (status != FAIL) {

	print_msg("Calling persist(PERSIST_DELETE) to set persistency", INDENT);
	s->persist(PERSIST_DELETE);
	print_status(PASS);

    }

    print_msg("Destroying named stream (file should be removed) (calling op. delete)", INDENT);
    delete s;
    status = (stat(fn, &buf) == -1 && errno == ENOENT ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++;
  
    print_status(EMPTY);

    delete[] fn;

    return (failed ? 1: 0);
}




////////////////////////////////////////////////////////
//////////////      test_sort()       //////////////////
////////////////////////////////////////////////////////


int test_sort() {
    static bool been_here = false;
    status_t status = EMPTY;
    int failed = 0;
    int i;
    AMI_err err;
    scan_universal<40> so(1000000, 47);
    AMI_STREAM< ifoo_t<40> > *ps[2];

    // Print the test heading.
    print_msg("Testing AMI_sort", 0);
    if (been_here) {
	print_status(SKIP);
	return 0;
    }
    been_here = true;
    print_status(EMPTY); // New line.

    print_msg("Preliminary: Initializing temporary streams.", INDENT);
    for (i = 0; i < 2; i++) {
	ps[i] = new AMI_STREAM< ifoo_t<40> >;
	if (!ps[i]->is_valid()) {
	    status = FAIL;
	    break;
	} else
	    status = PASS;
    }
    print_status(status); if (status == FAIL) { failed++; status = SKIP; }


    print_msg("Preliminary: Generating stream with 1m 40-byte items", INDENT);
    if (status != SKIP) {
	err = AMI_scan(&so, ps[0]);
	status = (err == AMI_ERROR_NO_ERROR && ps[0]->stream_len() == 1000000 ? PASS: FAIL);
    }
    print_status(status); if (status == FAIL) { failed++; status = SKIP; }


    print_msg("Running AMI_sort on 1m 40-byte-item stream (key:int, comp:op)", INDENT);
    if (status != SKIP) {
	err = AMI_sort(ps[0], ps[1]);
	status = (err == AMI_ERROR_NO_ERROR && 
		  ps[1]->stream_len() == ps[0]->stream_len()
		  ? PASS: FAIL);
    }
    print_status(status); if (status == FAIL) failed++;
  
  
    print_msg("Running AMI_scan to verify sorted order", INDENT);
    if (status != SKIP) {
	err = AMI_scan(ps[1], &so);
	status = (err == AMI_ERROR_NO_ERROR && 
		  so.switches() == 0 
		  ? PASS: FAIL);
	TP_LOG_APP_DEBUG_ID("Number of switches:");
	TP_LOG_APP_DEBUG_ID(so.switches());
    }
    print_status(status); if (status == FAIL) failed++;
  

    print_msg("Running AMI_sort on 1m 40-byte-item stream (key:int, comp:cl)", INDENT);
    if (status != SKIP) {
	ifoo_t<40> comparison_obj;
	ps[1]->truncate(0);
	err = AMI_sort(ps[0], ps[1], &comparison_obj);
	status = (err == AMI_ERROR_NO_ERROR && ps[1]->stream_len() == 1000000 ? PASS: FAIL);
    }
    print_status(status); if (status == FAIL) failed++;
    

    print_msg("Running AMI_scan to verify sorted order", INDENT);
    if (status != SKIP) {
	err = AMI_scan(ps[1], &so);
	status = (err == AMI_ERROR_NO_ERROR && 
		  ps[1]->stream_len() == 1000000 && 
		  so.switches() == 0 
		  ? PASS: FAIL);
	TP_LOG_APP_DEBUG_ID("Number of switches:");
	TP_LOG_APP_DEBUG_ID(so.switches());
    }
    print_status(status); if (status == FAIL) failed++;
  
    for (i = 0; i < 2; i++)
	delete ps[i];

    print_status(EMPTY); // New line.
    return (failed ? 1: 0);
}



////////////////////////////////////////////////////////
//////////////    test_scan_cxx()     //////////////////
////////////////////////////////////////////////////////

// Define a << operator for pairs of ints.
ostream& operator<<(ostream& os, const pair<int,int>& item) {
    return os << item.first << " " << item.second << "\n";
}

// Define a << operator for pairs of ints.
istream& operator>>(istream& is, pair<int,int>& item) {
    return is >> item.first >> item.second;
}

int test_scan_cxx() {
    static bool been_here = false;
    status_t status = EMPTY;
    int failed = 0;
    AMI_err err;
    int i;
    AMI_STREAM< pair<int,int> >* ts;

    char *fns  = new char[strlen(TMP_DIR)+strlen("tpie00.stream")+1];
    strcpy(fns,TMP_DIR);
    strcpy(fns+strlen(fns),"tpie00.stream");

    char *fnt0  = new char[strlen(TMP_DIR)+strlen("tpie00.txt")+1];
    strcpy(fnt0,TMP_DIR);
    strcpy(fnt0+strlen(fnt0),"tpie00.txt");

    char *fnt1  = new char[strlen(TMP_DIR)+strlen("tpie01.txt")+1];
    strcpy(fnt1,TMP_DIR);
    strcpy(fnt1+strlen(fnt1),"tpie01.txt");

    // Print the test heading.
    print_msg("Testing AMI_scan with C++ streams", 0);
    if (been_here) {
	print_status(SKIP);
	return 0;
    }
    been_here = true;
    print_status(EMPTY); // New line.

    print_msg("Creating an ASCII file with 5m pairs of integers", INDENT);
    if (status != SKIP) {
	unlink(fnt0);
	unlink(fns);
	ofstream xos;
	xos.open(fnt0);
	if (!xos) {
	    TP_LOG_APP_DEBUG_ID("Could not open C++ stream for writing to tpie00.txt");
	    status = FAIL;
	} else {
	    for (i = 0; i < 5000000; i++) {
		xos << TPIE_OS_RANDOM() << " " << TPIE_OS_RANDOM() << "\n";
	    }
	}
	xos.close();
    }
    print_status(status); if (status == FAIL) { failed++; status = SKIP; }


    print_msg("Running AMI_scan with cxx_istream_scan", INDENT);
    if (status != SKIP) {
	ifstream xis;    
	xis.open(fnt0);
	if (!xis) {
	    TP_LOG_APP_DEBUG_ID("Could not open C++ stream for reading from tpie00.txt");
	    status = FAIL;
	}
	cxx_istream_scan< pair<int,int> > so(&xis);
	ts = new AMI_STREAM< pair<int,int> >(fns);
	if (!ts->is_valid()) {
	    TP_LOG_APP_DEBUG_ID("Could not open TPIE stream for writing in tpie00.stream");
	    status = FAIL;
	} 
	if (status != FAIL) {
	    err = AMI_scan(&so, ts);
	    TP_LOG_APP_DEBUG_ID("Length of TPIE stream in tpie00.stream:");
	    TP_LOG_APP_DEBUG_ID(ts->stream_len());
	    status = (err == AMI_ERROR_NO_ERROR && ts->stream_len() == 5000000 ? PASS: FAIL);
	}
	xis.close();
	delete ts;
    }
    print_status(status); if (status == FAIL) { failed++; status = SKIP; }


    print_msg("Running AMI_scan with cxx_ostream_scan", INDENT);
    if (status != SKIP) {
	ofstream xos;
	xos.open(fnt1);
	if (!xos) {
	    TP_LOG_APP_DEBUG_ID("Could not open C++ stream for writing to tpie01.txt");
	    status = FAIL;
	}
	cxx_ostream_scan< pair<int,int> > so(&xos);
	ts = new AMI_STREAM< pair<int,int> >(fns, AMI_READ_STREAM);
	if (!ts->is_valid() || ts->stream_len() != 5000000) {
	    TP_LOG_APP_DEBUG_ID("Error while re-opening stream from tpie00.stream");
	    status = FAIL;
	}
	if (status != FAIL) {
	    err = AMI_scan(ts, &so);
	    status = (err == AMI_ERROR_NO_ERROR ? PASS: FAIL);
	}
	xos.close();
	delete ts;
    }
    print_status(status); if (status == FAIL) failed++;
  
    unlink(fns); delete[] fns;
    unlink(fnt0); delete[] fnt0;
    unlink(fnt1); delete[] fnt1;
    print_status(EMPTY); // New line.
    return (failed ? 1: 0);
}




////////////////////////////////////////////////////////
//////////////      test_scan()       //////////////////
////////////////////////////////////////////////////////

int test_scan() {
    static bool been_here = false;
    status_t status = EMPTY;
    int failed = 0;
//  AMI_STREAM<foo_t<40> > *s;
    AMI_STREAM<int> *ps[9];
    AMI_err err;
    int i;
    scan_universal<40> so(10000000, 43); // scan object.


    print_msg("Testing AMI_scan", 0);
    if (been_here) {
	print_status(SKIP);
	return 0;
    }
    been_here = true;
    print_status(EMPTY); // New line.


    print_msg("Initializing temporary streams.", INDENT);
    for (i = 0; i < 9; i++) {
	ps[i] = new AMI_STREAM<int>;
	if (!ps[i]->is_valid()) {
	    status = FAIL;
	    break;
	}
    }
    print_status(status);
    if (status == FAIL) { failed++; status = SKIP; }

  
    print_msg("Running AMI_scan with 0 in and 1 out (10m random integers)", INDENT);
    if (status != SKIP) {
	err = AMI_scan(&so, ps[0]);
	status = (err == AMI_ERROR_NO_ERROR && ps[0]->stream_len() == 10000000 ? PASS: FAIL);
    }
    print_status(status); if (status == FAIL) failed++;

    print_msg("Running AMI_scan with 1 in and 0 out (counting switches)", INDENT);
    if (status != SKIP) {
	err = AMI_scan(ps[0], &so);
	status = (err == AMI_ERROR_NO_ERROR && ps[0]->stream_len() == 10000000 ? PASS: FAIL);
	TP_LOG_APP_DEBUG_ID("Number of switches:");
	TP_LOG_APP_DEBUG_ID(so.switches());
    }
    print_status(status); if (status == FAIL) failed++;

  
    print_msg("Running AMI_scan with 1 in and 1 out (halving each of 10m integers)", INDENT);
    if (status != SKIP) {
	err = AMI_scan(ps[0], &so, ps[1]);
	status = (err == AMI_ERROR_NO_ERROR && ps[0]->stream_len() == 10000000 && ps[1]->stream_len() == 10000000 ? PASS: FAIL);
    }
    print_status(status); if (status == FAIL) failed++;

  
    print_msg("Checking streams integrity and current pos (should be 10000000)", INDENT);
    if (status != SKIP) {
	status = (ps[0]->is_valid() && ps[0]->stream_len() == 10000000 && ps[0]->tell() == 10000000  && ps[1]->tell() == 10000000 ? PASS: FAIL);
	TP_LOG_APP_DEBUG_ID("Current position in input stream:");
	TP_LOG_APP_DEBUG_ID(ps[0]->tell());
	TP_LOG_APP_DEBUG_ID("Current position in output stream:");
	TP_LOG_APP_DEBUG_ID(ps[1]->tell());
    }
    print_status(status); if (status == FAIL) failed++;
  

    print_msg("Running AMI_scan with 2 in and 1 out (min of 10m pairs of integers)", INDENT);
    if (status != SKIP) {
	err = AMI_scan(ps[0], ps[1], &so, ps[2]);
	status = (err == AMI_ERROR_NO_ERROR && ps[0]->stream_len() == 10000000 && ps[1]->stream_len() == 10000000 && ps[2]->stream_len() == 10000000 ? PASS: FAIL);
    }
    print_status(status); if (status == FAIL) failed++;


    print_msg("Same as above, with non-equal-size inputs: 9m and 10m  ", INDENT);
    if (status != SKIP) {
	err = ps[0]->truncate(9000000);
	err = AMI_scan(ps[0], ps[1], &so, ps[3]);
	status = (err == AMI_ERROR_NO_ERROR && ps[0]->stream_len() == 9000000 && ps[1]->stream_len() == 10000000 && ps[3]->stream_len() == 10000000 ? PASS: FAIL);
    }
    print_status(status); if (status == FAIL) failed++;

    print_msg("Running AMI_scan with 2 in and 2 out (off-synch: even and odd)", INDENT);
    if (status != SKIP) {
	err = AMI_scan(ps[0], ps[1], &so, ps[4], ps[5]);
	status = (err == AMI_ERROR_NO_ERROR && ps[0]->stream_len() == 9000000 && ps[1]->stream_len() == 10000000 && ps[4]->stream_len() + ps[5]->stream_len() == 19000000 ? PASS: FAIL);
	TP_LOG_APP_DEBUG_ID("Length of \"even\" stream: ");
	TP_LOG_APP_DEBUG_ID(ps[4]->stream_len());
	TP_LOG_APP_DEBUG_ID("Length of \"odd\" stream: ");
	TP_LOG_APP_DEBUG_ID(ps[5]->stream_len());
	TP_LOG_APP_DEBUG_ID("The sum should be 19000000.");
    }
    print_status(status); if (status == FAIL) failed++;


    print_msg("Running AMI_scan with 1 in and 0 out to verify \"even\" stream", INDENT);
    if (status != SKIP) {
	err = AMI_scan(ps[4], &so);
	status = (err == AMI_ERROR_NO_ERROR && so.even() == ps[4]->stream_len() && so.odd() == 0 ? PASS: FAIL);
    }
    print_status(status); if (status == FAIL) failed++;


    print_msg("Running AMI_scan with 1 in and 0 out to verify \"odd\" stream", INDENT);
    if (status != SKIP) {
	err = AMI_scan(ps[5], &so);
	status = (err == AMI_ERROR_NO_ERROR && so.odd() == ps[5]->stream_len() && so.even() == 0 ? PASS: FAIL);
    }
    print_status(status); if (status == FAIL) failed++;


    print_msg("Running AMI_scan with 4 in and 3 out (avg, min, max)", INDENT);
    if (status != SKIP) {
	ps[3]->truncate(2000000);
	err = AMI_scan(ps[0], ps[1], ps[2], ps[3], &so, ps[6], ps[7], ps[8]);
	status = (err == AMI_ERROR_NO_ERROR && ps[6]->stream_len() == 10000000 && ps[7]->stream_len() == 10000000 && ps[8]->stream_len() == 10000000 ? PASS: FAIL);
    }
    print_status(status); if (status == FAIL) failed++;

	
    print_msg("Running AMI_scan illegally with non-valid in-stream", INDENT);
    if (status != SKIP) {
	AMI_STREAM<int> *psn = new AMI_STREAM<int>("/glkdjldas");
	err = AMI_scan(psn, &so);
	status = (err == AMI_ERROR_NO_ERROR && !psn->is_valid() ? FAIL: PASS);
	delete psn;
    }
    print_status(status); if (status == FAIL) failed++;


    print_msg("Running AMI_scan illegally with non-valid out-stream", INDENT);
    if (status != SKIP) {
	AMI_STREAM<int> *psn = new AMI_STREAM<int>("/glkdjldas");
	err = AMI_scan(ps[2], &so, psn);
	status = (err == AMI_ERROR_NO_ERROR && !psn->is_valid() ? FAIL: PASS);
	delete psn;
    }
    print_status(status); if (status == FAIL) failed++;


    print_msg("Running AMI_scan illegally with read-only out-stream", INDENT);
    if (status != SKIP) {
	AMI_STREAM<int> *psn = new AMI_STREAM<int>;
	psn->persist(PERSIST_PERSISTENT);
	char *fn;
	psn->name(&fn);
	delete psn;
	psn = new AMI_STREAM<int>(fn, AMI_READ_STREAM);
	if (!psn->is_valid())
	    status = FAIL;
	else {
	    err = AMI_scan(ps[3], &so, psn);
	    status = (err == AMI_ERROR_NO_ERROR ? FAIL: PASS);
	}
	delete psn;
	// open it again, to delete the file.
	psn = new AMI_STREAM<int>(fn);
	psn->persist(PERSIST_DELETE);
	delete psn;
	delete fn;
    }
    print_status(status); if (status == FAIL) failed++;

    for (i = 0; i < 9; i++)
	delete ps[i];

    print_status(EMPTY); // New line.

    return (failed ? 1: 0);
}


////////////////////////////////////////////////////////
////////////     test_large_file()      ////////////////
////////////////////////////////////////////////////////

int test_large_files() {
    static bool been_here = false;
    status_t status = EMPTY;
    AMI_STREAM<TPIE_OS_OFFSET>* s;
    int failed = 0;
    AMI_err err;

    TPIE_OS_OFFSET i;
    TPIE_OS_OFFSET myLargeNumber = 
	static_cast<TPIE_OS_OFFSET>(3) * 
	static_cast<TPIE_OS_OFFSET>(1024) * 
	static_cast<TPIE_OS_OFFSET>(1024) * 
	static_cast<TPIE_OS_OFFSET>(1024) / 
	static_cast<TPIE_OS_OFFSET>(sizeof(TPIE_OS_OFFSET));

    char *fn  = new char[strlen(TMP_DIR)+strlen("tpie_large.stream")+1];
    strcpy(fn,TMP_DIR);
    strcpy(fn+strlen(fn),"tpie_large.stream");

    print_msg("Testing large file support", 0);
    if (been_here) {
	print_status(SKIP);
	return 0;
    }

    been_here = true;
    print_status(EMPTY);


    //////// Part 1: temporary stream.       //////////

    print_msg("Creating temporary stream (calling op. new)", INDENT);
    s = new AMI_STREAM<TPIE_OS_OFFSET>("large.stream");
    status = (s != NULL && s->is_valid() ? PASS: FAIL);
    s->persist(PERSIST_PERSISTENT);
    print_status(status); if (status == FAIL) failed++;

    print_status(EMPTY); // New line.

    if (status != FAIL) {
	err = AMI_ERROR_NO_ERROR;

	print_msg("Truncating temporary stream ", INDENT);
	err = s->truncate(myLargeNumber);
	status = (err == AMI_ERROR_NO_ERROR && s->stream_len() == myLargeNumber ? PASS: FAIL);
	print_status(status); if (status == FAIL) failed++; 

	cerr << "Writing data";
	err = s->seek(0);
	TPIE_OS_OFFSET mbcount = 0;
	TPIE_OS_OFFSET mbcount100 = 0;
	TPIE_OS_OFFSET gbcount = 0;
	for (i=0; i< myLargeNumber; i++) {
	    if ((err = s->write_item(i)) != AMI_ERROR_NO_ERROR) break; 
	    mbcount++;
	    if (mbcount > 1024 * 1024 / sizeof(TPIE_OS_OFFSET)) {
		mbcount100++;
		mbcount = 0;
		if (mbcount100 == 100) {
		    cerr << ".";
		    mbcount100 = 0;
		    gbcount++;
		    if (gbcount == 10) {
			cerr << "|";
			gbcount = 0;
		    }
		}
	    }
	}

	cerr << endl;
	status = (err == AMI_ERROR_NO_ERROR && s->stream_len() == myLargeNumber ? PASS: FAIL);
	print_status(status); if (status == FAIL) failed++; 
    }

    print_msg("Deleting temporary stream (calling op. delete)", INDENT);
    delete s;   
    print_status(status); if (status == FAIL) failed++; 

    return (failed ? 1: 0);
}
