// Copyright (c) 1994 Darren Erik Vengroff
//
// File: binary_merge_sort.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 8/29/94
//



static char binary_merge_sort_id[] = "$Id: binary_merge_sort.cpp,v 1.4 1999-11-02 17:04:25 tavi Exp $";

// If you actually want to see all the ints in the stream.
//#define REPORT_RESULTS

#define BTE_MMB_LOGICAL_BLOCKSIZE_FACTOR 32
#define TEST_SIZE 1024 * 1024 

#define MIN_RECURSE_LEN 1024 * 256

#include <stdlib.h>
#include <iostream.h>

// Use logs.
#define TPL_LOGGING 1
#include <tpie_log.h>

// Use the single BTE stream version of AMI streams.
#define AMI_IMP_SINGLE

// Pick a version of BTE streams.
#define BTE_IMP_MMB
//#define BTE_IMP_STDIO
//#define BTE_IMP_UFS

// Define it all.
#include <ami.h>

#ifdef REPORT_RESULTS
#include <ami_scan_utils.h>
#endif //REPORT_RESULTS

extern "C" void srandom(unsigned int);
extern "C" long int random(void);

// A scan object to generate random integers.
class random_scan : AMI_scan_object {
private:
    unsigned int max, remaining;
public:
    random_scan(unsigned int count = 1000, int seed = 17);
    virtual ~random_scan(void);
    AMI_err initialize(void);
    AMI_err operate(int *out1, AMI_SCAN_FLAG *sf);
};

random_scan::random_scan(unsigned int count, int seed) :
max(count), remaining(count)
{
    LOG_APP_DEBUG("random_scan seed = ");
    LOG_APP_DEBUG(seed);
    LOG_APP_DEBUG('\n');

    srandom(seed);
}

random_scan::~random_scan(void)
{
}


AMI_err random_scan::initialize(void)
{
    remaining = max;

    return AMI_ERROR_NO_ERROR;
};

AMI_err random_scan::operate(int *out1, AMI_SCAN_FLAG *sf)
{
    if (*sf = remaining--) {
        *out1 = random();
        return AMI_SCAN_CONTINUE;
    } else {
        return AMI_SCAN_DONE;
    }
};


// A merge object to merge two sorted streams
template<class T> class less_merge : AMI_merge_base<T> {
public:
    unsigned long int called;
    
    AMI_err initialize(arity_t arity, T **in, AMI_merge_flag *taken_flags);
    AMI_err operate(const T **in, AMI_merge_flag *taken_flags, T *out);
};

template<class T>
AMI_err less_merge<T>::initialize(arity_t arity, T **in,
                                  AMI_merge_flag *taken_flags)
{
    called = 0;
    
    if (arity != 2) {
        return AMI_ERROR_OBJECT_INITIALIZATION;
    }

    taken_flags[0] = taken_flags[1] = 0;
    
    return AMI_ERROR_NO_ERROR;
};



template<class T>
AMI_err less_merge<T>::operate(const T **in,
                               AMI_merge_flag *taken_flags,
                               T *out)
{
    called++;

    if ((in[0] != NULL) && (in[1] != NULL)) {

        if (*in[0] < *in[1]) {
            *out = *in[0];
            taken_flags[0] = 1;
            taken_flags[1] = 0;
        } else {
            *out = *in[1];
            taken_flags[1] = 1;
            taken_flags[0] = 0;
        }
        return AMI_MERGE_OUTPUT;
    }

    if (in[0] == NULL) {
        if (in[1] == NULL) {
            return AMI_MERGE_DONE;
        } else {
            *out = *in[1];
            taken_flags[1] = 1;
            taken_flags[0] = 0;
            return AMI_MERGE_OUTPUT;
        }
    }

    tp_assert((in[0] != NULL) && (in[1] == NULL), "Confused about input.");

    *out = *in[0];
    taken_flags[0] = 1;
    taken_flags[1] = 0;

    return AMI_MERGE_OUTPUT;

};


template <class T>
AMI_err binary_divide_and_conquer(AMI_STREAM<T> *instream,
                                  AMI_STREAM<T> *outstream,
                                  AMI_merge_base<T> *merge_obj,
                                  AMI_err (*mm_operate)(T *, off_t))
{
    AMI_err ae;
    off_t len;

    // How long is the stream?
    len = instream->stream_len();

    if ((ae = instream->seek(0)) != AMI_ERROR_NO_ERROR) {
        return ae;
    }
    if ((ae = outstream->seek(0)) != AMI_ERROR_NO_ERROR) {
        return ae;
    }
    
    // This can be made more efficient with a better stopping
    // condition, like when a stream can all fit in main memory.
    if (len <= MIN_RECURSE_LEN) {
        T *main_mem_stream;
        off_t len1;

        if ((main_mem_stream = new T[len]) == NULL) {
            return AMI_ERROR_MM_ERROR;
        };

        if ((ae = instream->read_array(main_mem_stream, &len1)) !=
            AMI_ERROR_NO_ERROR) {
            return ae;
        }

        tp_assert(len1 = len, "Did not read the right amount.");
        
        if ((ae = (*mm_operate)(main_mem_stream, len)) !=
            AMI_ERROR_NO_ERROR) {
            return ae;
        }

        if ((ae = outstream->write_array(main_mem_stream, len)) !=
            AMI_ERROR_NO_ERROR) {
            return ae;
        }
        
        delete main_mem_stream;

        return AMI_ERROR_NO_ERROR;
    }
    
    // Divide the stream in half.
    AMI_STREAM<T> *sub0, *sub1;

    if ((ae = instream->new_substream(AMI_READ_STREAM, 0, (len - 1)/2,
                                      (AMI_base_stream<T> **)&sub0)) !=
                                      AMI_ERROR_NO_ERROR) {
        return ae;
    }
    
    if ((ae = instream->new_substream(AMI_READ_STREAM, ((len - 1)/2) + 1,
                                      len-1,
                                      (AMI_base_stream<T> **)&sub1)) !=
                                      AMI_ERROR_NO_ERROR) {
        return ae;
    }

    tp_assert((sub0->stream_len() + sub1->stream_len()) == len,
              "Substream lengths don't add up properly." <<
              "\n\tsub0->stream_len() = " << sub0->stream_len() <<
              "\n\tsub1->stream_len() = " << sub1->stream_len() <<
              "\n\tlen = " << len);
              
    // Recurse on each half.
    
    AMI_STREAM<T> rec0, rec1;

    if ((ae = binary_divide_and_conquer(sub0, &rec0, merge_obj,
                                        mm_operate)) != AMI_ERROR_NO_ERROR) {
        return ae;
    }
    
    tp_assert(rec0.stream_len() == sub0->stream_len(),
              "Stream 0 length changed in recursion." <<
              "\n\trec0.stream_len() = " << rec0.stream_len() <<
              "\n\tsub0->stream_len() = " << sub0->stream_len());
    
    if ((ae = binary_divide_and_conquer(sub1, &rec1, merge_obj,
                                        mm_operate)) != AMI_ERROR_NO_ERROR) {
        return ae;
    }

    tp_assert(rec1.stream_len() == sub1->stream_len(),
              "Stream 1 length changed in recursion." <<
              "\n\trec1.stream_len() = " << rec1.stream_len() <<
              "\n\tsub1->stream_len() = " << sub1->stream_len());

    tp_assert((rec0.stream_len() + rec1.stream_len()) == len,
              "Recursive stream lengths don't add up properly." <<
              "\n\trec0.stream_len() = " << rec0.stream_len() <<
              "\n\trec1.stream_len() = " << rec1.stream_len() <<
              "\n\tlen = " << len);              
              
    // Merge them.

    AMI_base_stream<T> *abs[2];

    abs[0] = &rec0;
    abs[1] = &rec1;

    return AMI_merge((pp_AMI_bs<T>)abs, (arity_t)2,
                     (AMI_base_stream<T> *)outstream,
                     merge_obj);
}

extern "C" int int_cmp(const void *p1, const void *p2);
    
AMI_err sort_ints_in_main_mem(int *data, off_t len)
{
    qsort(data, len, sizeof(int), int_cmp);
    return AMI_ERROR_NO_ERROR;
}


int main(int argc, char **argv)
{
    AMI_err ae;

    AMI_STREAM<int> amis0;
    AMI_STREAM<int> amis1;
        
    // Write some ints.
    random_scan rnds(TEST_SIZE);
    
    ae = AMI_scan(&rnds, (AMI_base_stream<int> *)&amis0);

    cout << "Wrote the random values.\n";

    cout << "Stream length = " << amis0.stream_len() << '\n';

#ifdef REPORT_RESULTS    
    cxx_ostream_scan<int> rpts;
    
    ae = AMI_scan((AMI_base_stream<int> *)&amis0, &rpts);
#endif // REPORT_RESULTS
    
    less_merge<int> lm;

    binary_divide_and_conquer(&amis0,
                              &amis1,
                              (AMI_merge_base<int> *)&lm,
                              sort_ints_in_main_mem);

    cout << "Sorted them.\n";

    cout << "Sorted stream length = " << amis1.stream_len() << '\n';

#ifdef REPORT_RESULTS    
    ae = AMI_scan((AMI_base_stream<int> *)&amis1, &rpts);    
#endif // REPORT_RESULTS

    cout << '\n';
    
    return 0;
}
