// Copyright (c) 1994 Darren Erik Vengroff
//
// File: test_ami_merge.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 6/2/94
//

static char test_ami_merge_id[] = "$Id: test_ami_merge.cpp,v 1.2 1994-09-22 15:18:15 darrenv Exp $";

#define BTE_MMB_LOGICAL_BLOCKSIZE_FACTOR 32
#define TEST_SIZE 1000

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


// A scan object to generate output.
class count_scan : AMI_scan_object {
private:
    int maximum;
public:
    int ii;
    unsigned long int called;

    count_scan(int max = 1000) : maximum(max), ii(0) {};
    AMI_err initialize(void);
    AMI_err operate(int *out1, AMI_SCAN_FLAG *sf);
};

AMI_err count_scan::initialize(void)
{
    called = 0;
    ii = 0;
    return AMI_ERROR_NO_ERROR;
};

AMI_err count_scan::operate(int *out1, AMI_SCAN_FLAG *sf)
{
    called++;
    *out1 = ++ii;
    return (*sf = (ii <= maximum)) ? AMI_SCAN_CONTINUE : AMI_SCAN_DONE;
};

// A dummy function to force g++ to actually define an appropriate
// version of AMI_scan() and not just declare it.
#if 0 // __GNUG__
static void _____dummy_cs(void) {
    count_scan *cs;
    AMI_base_stream<int> *abs;
    tp_assert(0, "We should never *EVER* call this dummy function; "
              "seg fault imminent.");
    AMI_err ae = AMI_scan(cs, abs);
    ae = ae;
}
#endif

// g++ 2.5.2 seems to misbehave on this, treating it only as a
// declaration, but not defining it based on the template from
// <ami_scan.h>.  The dummy function above is designed to force the
// code to actually be generated.
AMI_err AMI_scan(count_scan *, AMI_base_stream<int> *);


// A scan object to square numeric types.
template<class T> class square_scan : AMI_scan_object {
public:
    T ii;
    unsigned long int called;
    AMI_err initialize(void);
    AMI_err operate(const T &in, AMI_SCAN_FLAG *sfin,
                    T *out, AMI_SCAN_FLAG *sfout);
};

template<class T>
AMI_err square_scan<T>::initialize(void)
{
    ii = 0;
    called = 0;
    return AMI_ERROR_NO_ERROR;
};

template<class T>
AMI_err square_scan<T>::operate(const T &in, AMI_SCAN_FLAG *sfin,
                                T *out, AMI_SCAN_FLAG *sfout)
{
    called++;
    if (*sfout = *sfin) {
        ii = in;
        *out = in * in;
        return AMI_SCAN_CONTINUE;
    } else {
        return AMI_SCAN_DONE;
    }
};


// A dummy function to force g++ to actually define an appropriate
// version of AMI_scan() and not just declare it.  See the comment at
// the first dummy function above.
#if 0 // __GNUG__
static void _____dummy_ss(void) {
    square_scan<int> *ss;
    AMI_base_stream<int> *abs0, *abs1;
    tp_assert(0, "We should never *EVER* call this dummy function; "
              "seg fault imminent.");
    AMI_err ae = AMI_scan(abs0, ss, abs1);
    ae = ae;
}
#endif

AMI_err AMI_scan(AMI_base_stream<int> *,
                 square_scan<int> *,
                 AMI_base_stream<int> *);


// A merge object to interleave two streams.
template<class T> class interleave_merge : AMI_merge_base<T> {
private:
    T hold;

    // States are :
    //  1 - hold was read from in[0].
    //  2 - hold was read from in[1].
    // -1 - hold was read from in[0] and in[1] is empty.
    // -2 - hold was read from in[1] and in[0] is empty.
    //  0 - both in[0] and in[1] are empty.
    int state;

public:
    unsigned long int called;
    
    AMI_err initialize(arity_t arity, T **in, AMI_merge_flag *taken_flags);
    AMI_err operate(const T **in, AMI_merge_flag *taken_flags, T *out);
};

template<class T>
AMI_err interleave_merge<T>::initialize(arity_t arity, T **in,
                                        AMI_merge_flag *taken_flags)
{
    called = 0;
    
    if (arity != 2) {
        return AMI_ERROR_OBJECT_INITIALIZATION;
    }

    hold = *in[0];
    state = 1;
    taken_flags[0] = 1;
    taken_flags[1] = 0;

    return AMI_ERROR_NO_ERROR;
};



template<class T>
AMI_err interleave_merge<T>::operate(const T **in,
                                     AMI_merge_flag *taken_flags,
                                     T *out)
{
    called++;
    
    // This should actually be changed to interleave any number of
    // input streams, and use a mod operator on the state to determine
    // next state and which in[] to take from.
    
    switch (state) {
        case 1:
            *out = hold;
            if (in[1] == NULL) {
                if (in[0] == NULL) {                
                    state = 0;
                } else {
                    hold = *in[0];
                    taken_flags[0] = 1;
                    state = -1;
                }
            } else {
                hold = *in[1];
                taken_flags[1] = 1;
                state = 2;
            }
            return AMI_MERGE_OUTPUT;
        case 2:
            *out = hold;
            if (in[0] == NULL) {
                if (in[1] == NULL) {                
                    state = 0;
                } else {
                    hold = *in[1];
                    taken_flags[1] = 1;
                    state = -2;
                }
            } else {
                hold = *in[0];
                taken_flags[0] = 1;
                state = 1;
            }
            return AMI_MERGE_OUTPUT;
        case -1:
            *out = hold;
            if (in[0] == NULL) {                
                state = 0;
            } else {
                hold = *in[0];
                taken_flags[0] = 1;
                state = -1;
            }
            return AMI_MERGE_OUTPUT;
        case -2:
            *out = hold;
            if (in[1] == NULL) {                
                state = 0;
            } else {
                hold = *in[1];
                taken_flags[1] = 1;
                state = -2;
            }
            return AMI_MERGE_OUTPUT;
        case 0:
            return AMI_MERGE_DONE;
    }
    // Just to keep the compiler happy, since it does not like a
    // non-void function to end without returning.
    tp_assert(0, "Control should never reach this point.");
    return AMI_MERGE_DONE;
};


// A dummy function to force g++ to actually define an appropriate
// version of AMI_scan() and not just declare it.  See the comment at
// the first dummy function above.
#if  0 // __GNUG__
static void _____dummy_int(void) {
    interleave_merge<int> *im;
    arity_t arity;
    pp_AMI_bs<int> aabs;
    AMI_base_stream<int> *abs;
    tp_assert(0, "We should never *EVER* call this dummy function; "
              "seg fault imminent.");
    AMI_err ae = AMI_merge(aabs, arity, abs, im);
    ae = ae;
}
#endif

AMI_err AMI_merge(pp_AMI_bs<int>, arity_t,
                  AMI_base_stream<int> *, interleave_merge<int> *);



int main(int argc, char **argv);

int main(int argc, char **argv)
{
#if TPL_LOGGING
    *tpl << setthreshold(TP_LOG_DATA_ERROR);
#endif

    AMI_err ae;

    // Write some ints.
    {
        count_scan cs(TEST_SIZE);
    
        BTE_STREAM<int> btes("./TEST_SCAN0", BTE_WRITE_STREAM);
        AMI_STREAM<int> amis((BTE_base_stream<int> *)&btes);

        ae = AMI_scan(&cs, (AMI_base_stream<int> *)&amis);

        cout << "Wrote them; stopped (didn't write) with ii = "
             << cs.ii << ". operate() called " << cs.called << " times.\n";
    }


    // Square them.
    {
        square_scan<int> ss;
        
        BTE_STREAM<int> bters("./TEST_SCAN0", BTE_READ_STREAM);
        AMI_STREAM<int> amirs((BTE_base_stream<int> *)&bters);

        BTE_STREAM<int> btews("./TEST_SCAN1", BTE_WRITE_STREAM);
        AMI_STREAM<int> amiws((BTE_base_stream<int> *)&btews);
        
        ae = AMI_scan((AMI_base_stream<int> *)&amirs, &ss,
                      (AMI_base_stream<int> *)&amiws);

        cout << "Squared them; last squared was ii = "
             << ss.ii << ". operate() called " << ss.called << " times.\n";
    }
    

    // Interleave the streams.
    {
        interleave_merge<int> im;

        arity_t arity = 2;
        
        AMI_STREAM<int> *amirs[2];

        BTE_STREAM<int> bters0("./TEST_SCAN0", BTE_READ_STREAM);
        amirs[0] = new AMI_STREAM<int>((BTE_base_stream<int> *)&bters0);

        BTE_STREAM<int> bters1("./TEST_SCAN1", BTE_READ_STREAM);
        amirs[1] = new AMI_STREAM<int>((BTE_base_stream<int> *)&bters1);
        
        BTE_STREAM<int> btews("./TEST_MERGE0", BTE_WRITE_STREAM);
        AMI_STREAM<int> amiws((BTE_base_stream<int> *)&btews);

        ae = AMI_merge((pp_AMI_bs<int>)amirs, arity,
                       (AMI_base_stream<int> *)&amiws, &im);

        cout << "Interleaved them; operate() called " << im.called 
             << " times.\n";
    }

    // Divide the stream into two substreams, and interleave them.
    {
        interleave_merge<int> im;

        arity_t arity = 2;

        BTE_STREAM<int> bterss("./TEST_MERGE0", BTE_READ_STREAM);

        BTE_STREAM<int> btews("./TEST_MERGE1", BTE_WRITE_STREAM);
        AMI_STREAM<int> amiws((BTE_base_stream<int> *)&btews);

        AMI_base_stream<int> *amirs[2];

        AMI_STREAM<int> amirss(&bterss);
        
        ae = amirss.new_substream(AMI_READ_STREAM, 0, TEST_SIZE, &(amirs[0]));
        ae = amirss.new_substream(AMI_READ_STREAM, TEST_SIZE, 2*TEST_SIZE,
                                  &(amirs[1]));

        ae = AMI_merge((pp_AMI_bs<int>)amirs, arity,
                       (AMI_base_stream<int> *)&amiws, &im);

        cout << "Interleaved them; operate() called " << im.called 
             << " times.\n";
    }
    
    return 0;
}









