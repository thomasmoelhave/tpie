// Copyright (c) 1995 Darren Vengroff
//
// File: nas_is.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/24/95
//

#include <portability.h>

#include <versions.h>
VERSION(nas_is_cpp,"$Id: nas_is.cpp,v 1.8 2003-09-12 01:52:20 tavi Exp $");

// Benchmark constants.
#define IMAX 10
#define COUNT (1048576/16)
#define BMAX (32768/16)
#define NAS_A (double(1220703125.0))
#define NAS_S (double(314159265.0))

#define TWO_TO_23 (double(2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2))
#define TWO_TO_46 (TWO_TO_23 * TWO_TO_23)

#define TWO_TO_MINUS_23 (1.0 / TWO_TO_23)
#define TWO_TO_MINUS_46 (1.0 / TWO_TO_46)

#define INPUT_FILENAME "K.8"

// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

// Define it all.
#include <ami.h>

// Utitlities for ascii output.
#include <ami_scan_utils.h>

#include <ami_key.h>

#include <cpu_timer.h>


static unsigned int bmax = BMAX;


// This is the structure we will do our computation on.

class key_triple {
public:
    static int input_index;
    static key_range input_range;
    unsigned int kv;
    unsigned int i;
    unsigned int r;
    key_triple() {};
    key_triple(unsigned int key_value, unsigned int index, unsigned int rank) :
            kv(key_value),
            i(index),
            r(rank) 
    {
    };
};

int key_triple::input_index;
key_range key_triple::input_range;


// Comparison operators for merge sort.

inline bool operator<(const key_triple &a, const key_triple &b)
{
    return a.kv < b.kv;
}
inline bool operator>(const key_triple &a, const key_triple &b)
{
    return a.kv > b.kv;
}

// These are access functions that grab the two different fields we use
// to sort key_triples.
inline unsigned int key_value(const key_triple &kt) { return kt.kv; }
inline unsigned int index(const key_triple &kt) { return kt.i; }

// These two functions produce a key value based on the CDF at the
// value of the actual key.  The CDF is simulated with either the sum of
// two or four uniform discrete random variables.

inline double key_cdf2(const key_triple &kt)
{
    register double dk = 2.0 * double(kt.kv) / double(bmax-1);
    register double cdf;
    
    if (dk <= 1.0) {
        cdf = dk*dk/2;
    } else {
        cdf = -(dk*dk)/2.0 + 2.0 * dk - 1.0;
    }

    return cdf * double(bmax-1);
}

inline double key_cdf4(const key_triple &kt)
{
    register double dk = 4.0 * double(kt.kv) / double(bmax-1);
    register double cdf;
    
    if (dk <= 2.0) {        
        if (dk <= 1.0) {
            cdf = (dk*dk*dk*dk)/24.0;
        } else {
            cdf = (-3.0*dk*dk*dk*dk + 16.0*dk*dk*dk -
                   24.0*dk*dk + 16.0*dk - 4.0) / 24.0;
        }
    } else {        
        if (dk <= 3.0) {
            cdf = (3.0*dk*dk*dk*dk - 32.0*dk*dk*dk +
                   120.0*dk*dk - 176.0*dk + 92.0) / 24.0;            
        } else {
            cdf = 1.0 - (4.0-dk)*(4.0-dk)*(4.0-dk)*(4.0-dk)/24.0;
        }
    }

    return cdf * double(bmax-1);
}


    
// We need two ways to sort key_triples, one by key value and the
// other by initial position.

#define KB_KEY key_value
#include <ami_kb_sort.h>
#undef KB_KEY

#define KB_KEY index
#include <ami_kb_sort.h>
#undef KB_KEY


// We also need to sort based on distribution information.

#define KB_KEY key_cdf2
#include <ami_kb_sort.h>
#undef KB_KEY

#define KB_KEY key_cdf4
#include <ami_kb_sort.h>
#undef KB_KEY


// We read and write key_triples simply as key values, since the other
// fields are just used internally.  When we read, we set the indeces
// to ascending integers.  If we set key_triple::input_index = 0
// before we start an input scan, this sets the index of every item to
// it's position in the stream.  We also set the range of the keys we
// have read implicitly when reading them.  Before this is done, we simply
// initialize key_triple::input_range to { KEY_MAX, KEY_MIN }.

ostream &operator<<(ostream& s, const key_triple &kt)
{
    return s << kt.kv;
};

istream &operator>>(istream& s, key_triple &kt)
{
    if (s >> kt.kv) {
        kt.i = kt.input_index++;
        if (kt.kv < kt.input_range.min) {
            kt.input_range.min = kt.kv;
        }
        if (kt.kv >= kt.input_range.max) {
            kt.input_range.max = kt.kv + 1;
        }
    }
    
    return s;
};


// The scan managemnt object that writes the random numbers into the keys.
// Code stolen from nas_ep.cpp

class scan_nas_psuedo_rand : AMI_scan_object {
private:
    // The seed.
    double s;

    // The last value output.
    double x;

    // A cache for the multiplicative factor a.
    double a1, a2;
    
    unsigned int max, remaining;

    unsigned int bmax;
public:
    scan_nas_psuedo_rand(double seed = NAS_S,
                         unsigned int count = COUNT,
                         double a = NAS_A,
                         unsigned long bm = BMAX);
                         
    virtual ~scan_nas_psuedo_rand(void);
    AMI_err initialize(void);
    inline AMI_err operate(key_triple *out, AMI_SCAN_FLAG *sf);
};


scan_nas_psuedo_rand::scan_nas_psuedo_rand(double seed,
                                           unsigned int count,
                                           double a, unsigned long bm) :
                                                   s(seed),
                                                   max(count),
                                                   bmax(bm)
{
        a1 = floor(TWO_TO_MINUS_23 * a);
        a2 = a - TWO_TO_23 * a1;    
}


scan_nas_psuedo_rand::~scan_nas_psuedo_rand()
{
}

AMI_err scan_nas_psuedo_rand::initialize(void)
{
    x = s;
    remaining = max;

    key_triple::input_index = 0;
    
    return AMI_ERROR_NO_ERROR;
}

inline AMI_err scan_nas_psuedo_rand::operate(key_triple *out, AMI_SCAN_FLAG *sf)
{
    register double b1, b2;
    register double t1, t2, t3, t4, t5;

    register double dev_sum;

    unsigned int out_key_val;

    unsigned int ii;
        
    if ((*sf = remaining--)) {

        // Generate and add up four random deviates.

        dev_sum = 0;
        
        for (ii = 4; ii--; ) {
            b1 = floor(TWO_TO_MINUS_23 * x);
            b2 = x - TWO_TO_23 * b1;

            t1 = a1*b2 + a2*b1;
            t2 = floor(TWO_TO_MINUS_23 * t1);
            t3 = t1 - TWO_TO_23 * t2;
            t4 = TWO_TO_23 * t3 + a2*b2;
            t5 = floor(TWO_TO_MINUS_46 * t4);
            
            x = t4 - TWO_TO_46 * t5;
            
            dev_sum += x * TWO_TO_MINUS_46;
        }

        out_key_val = (unsigned int)(bmax * dev_sum / 4);
        
        if (out_key_val > key_triple::input_range.max) {
            key_triple::input_range.max = out_key_val;
        } 
        if (out_key_val < key_triple::input_range.min) {
            key_triple::input_range.min = out_key_val;
        } 

        *out = key_triple(out_key_val,key_triple::input_index++,0);

        return AMI_SCAN_CONTINUE;

    } else {
        return AMI_SCAN_DONE;
    }
}




// A scan management object for placing ranks on a stream of ordered objects.

class scan_rank : AMI_scan_object {
private:
    int curr_rank;
public:
    AMI_err initialize()
    {
        curr_rank = 0;
        return AMI_ERROR_NO_ERROR;
    };
    inline AMI_err operate(const key_triple &in, AMI_SCAN_FLAG *sfin,
                           key_triple *out, AMI_SCAN_FLAG *sfout)
    {
        if ((*sfout = *sfin)) {
            *out = in;
            out->r = curr_rank++;
            return AMI_SCAN_CONTINUE;
        } else {
            return AMI_SCAN_DONE;
        }
    }
};

static unsigned int imax = IMAX;

static double nas_s = NAS_S;
static double nas_a = NAS_A;

static char def_srf[] = "oss.txt";
static char def_rrf[] = "osr.txt";

static char *sorted_results_filename = def_srf;
static char *rand_results_filename = def_rrf;

static bool report_results_random = false;
static bool report_results_sorted = false;

//static bool sort_again = false;

//static bool use_operator = false;

static bool kb_sort = true;

static unsigned int use_cdf = 0;


static const char as_opts[] = "R:S:rskI:B:C:";
void parse_app_opt(char c, char *optarg)
{
    switch (c) {
        case 'R':
            rand_results_filename = optarg;
        case 'r':
            report_results_random = true;
            break;
        case 'S':
            sorted_results_filename = optarg;
        case 's':
            report_results_sorted = true;
            break;
        case 'k':
            kb_sort = !kb_sort;
            break;
        case 'B':
            istrstream(optarg,strlen(optarg)) >> bmax;            
            break;
        case 'I':
            istrstream(optarg,strlen(optarg)) >> imax;            
            break;
        case 'C':
            istrstream(optarg,strlen(optarg)) >> use_cdf;            
            break;            
    }
}


AMI_err modify_keys(AMI_STREAM<key_triple> &keys, unsigned int iteration,
                    unsigned int imax, unsigned int bmax)
{
    AMI_err ae;
    
    keys.seek(iteration);
    ae = keys.write_item(key_triple(iteration, 0, 0));
    if (ae != AMI_ERROR_NO_ERROR) {
        return ae;
    }

    keys.seek(iteration+imax);
    ae = keys.write_item(key_triple(bmax - iteration, 0, 0));
    if (ae != AMI_ERROR_NO_ERROR) {
        return ae;
    }
    
    return AMI_ERROR_NO_ERROR;
}

int main(int argc, char **argv)
{

    AMI_err ae;

    cpu_timer cput;

    parse_args(argc,argv,as_opts,parse_app_opt);

#if 0    
    key_triple kt;

    kt.kv = bmax / 4;
    cout << key_cdf2(kt) << ' ';
    
    kt.kv = bmax / 2;
    cout << key_cdf2(kt) << ' ';
    
    kt.kv = 3 * bmax / 4;
    cout << key_cdf2(kt) << ' ';
    
    return 0;
#endif
    
    if (verbose) {
      cout << "test_mm_size = " << test_mm_size << "." << endl;
    } else {
        cout.precision(15);
        cout << test_mm_size << ' ' << test_size << ' ' << kb_sort << ' '
             << use_cdf << ' ' << imax << ' ' << bmax << ' ' << nas_a << ' '
             << nas_s << ' ';
    }
    
    // Set the amount of main memory:
    MM_manager.set_memory_limit (test_mm_size);

    // Stream of keys.  At the beginning of each iteration the keys
    // are here.

    AMI_STREAM<key_triple> keys;

    // A stream of keys sorted by value.

    AMI_STREAM<key_triple> keys_by_value;
    
    // A stream of keys with ranks assigned to them.

    AMI_STREAM<key_triple> ranked_keys;

    // Ranges of keys and indeces for key bucket sorting.
    
    key_range k_range, i_range;

    scan_rank sr;

    key_triple::input_index = 0;
    key_triple::input_range = key_range(KEY_MAX, KEY_MIN);
        
#if 0    
    // Read the input keys.

    {
        ifstream ifs(input_filename);
        cxx_istream_scan<key_triple> read_input(&ifs);
    
        ae = AMI_scan(&read_input, &keys);

    }
    
#else
    
    // Generate the input keys using the liner congurential method.

    {
        scan_nas_psuedo_rand snpr(nas_s, test_size, nas_a, bmax);
        
        ae = AMI_scan(&snpr, &keys);
    }

#endif

    k_range = key_triple::input_range;
    i_range = key_range(0, key_triple::input_index);

    if (verbose) {
        cout << "Input stream length = " << keys.stream_len() << endl;
        cout << "Key range = (" << k_range.min << ", " <<
	  k_range.max << ")" << endl;
    }

    // Part of the point of this benchmark is that we can't make any
    // assumptions about the key range, even though we know it is sort
    // of normal.

    k_range.min = 0;
    k_range.max = bmax;    
    
    // Streams for reporting random vand/or sorted values to ascii
    // streams.
    
    ofstream *oss;
    cxx_ostream_scan<key_triple> *rpts = NULL;
    ofstream *osr;
    cxx_ostream_scan<key_triple> *rptr = NULL;
    
    if (report_results_random) {
        osr = new ofstream(rand_results_filename);
        rptr = new cxx_ostream_scan<key_triple>(osr);
    }
    
    if (report_results_sorted) {
        oss = new ofstream(sorted_results_filename);
        rpts = new cxx_ostream_scan<key_triple>(oss);
    }
    
    if (report_results_random) {
        ae = AMI_scan(&keys, rptr);
    }

    cput.reset();
    cput.start();
    
    // The main loop over imax iterations.
    
    for (unsigned int ii = 1; ii <= imax; ii++) {

        if (verbose) {
            cout << "Iteration " << ii << endl;
        }
        
        // Modify the key sequence.

        ae = modify_keys(keys, ii, imax, bmax);
        
        // Sort by key value.
        
        keys_by_value.seek(0);
        if (kb_sort) {
            if (use_cdf == 4) {
                ae = AMI_kb_sort_key_cdf4(keys, keys_by_value, k_range);
            } else if (use_cdf == 2) {
                ae = AMI_kb_sort_key_cdf2(keys, keys_by_value, k_range);
            } else {
                ae = AMI_kb_sort_key_value(keys, keys_by_value, k_range);
            }
        } else {
            ae = AMI_sort(&keys, &keys_by_value);
        }
        
        if (verbose) {
            cout << "Sorted by value; stream len = " <<
                keys_by_value.stream_len() << endl;
        }

        // Put in the ranks.

        ae = AMI_scan(&keys_by_value, &sr, &ranked_keys);
        
        if (verbose) {
            cout << "Assigned ranks; stream len = " <<
                ranked_keys.stream_len() << endl;
        }

        // Sort back into initial order, using the initial index as
        // the sorting key.

        keys.seek(0);
        ae = AMI_kb_sort_index(ranked_keys, keys, i_range);
        
        if (verbose) {
            cout << "Sorted by index; stream len = " <<
                keys.stream_len() << endl;
        }

#if 0        
        // Perform verification test.

        if (partial_verify) {
            ae = partial_verification();
        }
#endif
        
    }

    cput.stop();

    cout << cput << endl;

    if (report_results_sorted) {
        ae = AMI_scan(&keys, rpts);
    }


#if 0    
    // Perform the total verification test.

    ae = AMI_scan();
#endif
    
    return 0;
}



