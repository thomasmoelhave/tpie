// Copyright (c) 1994 Darren Erik Vengroff
//
// File: test_ami_merge.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 6/2/94
//

static char test_ami_merge_id[] = "$Id: test_ami_merge.cpp,v 1.1 1994-06-03 13:47:27 dev Exp $";


#include <iostream.h>

// Use logs.
#define TPL_LOGGING 1
#include <tpie_log.h>

// Use the single BTE stream version of AMI streams.
#define AMI_IMP_SINGLE

// Use the MMB version of BTE streams.
//#define BTE_IMP_MMB
#define BTE_IMP_STDIO

// Define it all.
#include <ami.h>


// A scan object to generate output.
template<int MAX> class count_scan : AMI_scan_object {
private:
    int ii;
public:    
    AMI_err initialize(void);
    AMI_err operate(int *out1, AMI_SCAN_FLAG *sf);
};

template<int MAX>
AMI_err count_scan<MAX>::initialize(void)
{
    ii = 0;
    return AMI_ERROR_NO_ERROR;
};

template<int MAX>
AMI_err count_scan<MAX>::operate(int *out1, AMI_SCAN_FLAG *sf)
{
    *out1 = ii++;
    return (*sf = (ii <= MAX)) ? AMI_SCAN_CONTINUE : AMI_SCAN_DONE;
};


// A scan object to square numeric types.
template<class T> class square_scan : AMI_scan_object {
public:    
    AMI_err initialize(void);
    AMI_err operate(const T &in, AMI_SCAN_FLAG *sfin,
                    T *out, AMI_SCAN_FLAG *sfout);
};

template<class T>
AMI_err square_scan<T>::initialize(void)
{
    return AMI_ERROR_NO_ERROR;
};

template<class T>
AMI_err square_scan<T>::operate(const T &in, AMI_SCAN_FLAG *sfin,
                                T *out, AMI_SCAN_FLAG *sfout)
{
    if (*sfout = *sfin) {
        *out = in * in;
        return AMI_SCAN_CONTINUE;
    } else {
        return AMI_SCAN_DONE;
    }
};


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
    AMI_err initialize(arity_t arity, T **in, AMI_merge_flag *taken_flags);
    AMI_err operate(const T **in, AMI_merge_flag *taken_flags, T *out);
};

template<class T>
AMI_err interleave_merge<T>::initialize(arity_t arity, T **in,
                                        AMI_merge_flag *taken_flags)
{
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
AMI_err interleave_merge<T>::operate(const T **in, AMI_merge_flag *taken_flags,
                                     T *out)
{
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
};

int main(int argc, char **argv);

int main(int argc, char **argv)
{
#if TPL_LOGGING
    *tpl << setthreshold(TP_LOG_DATA_ERROR);
#endif

    AMI_err ae;

    // Write some ints.
    {
        count_scan<400000> cs;
    
        BTE_STREAM<int> btes("./TEST_SCAN0", BTE_WRITE_STREAM);
        AMI_STREAM<int> amis((BTE_base_stream<int> *)&btes);

        ae = AMI_scan(&cs, &amis);
    }

    cout << "Wrote them.\n";

    // Square them.
    {
        square_scan<int> ss;
        
        BTE_STREAM<int> bters("./TEST_SCAN0", BTE_READ_STREAM);
        AMI_STREAM<int> amirs((BTE_base_stream<int> *)&bters);

        BTE_STREAM<int> btews("./TEST_SCAN1", BTE_WRITE_STREAM);
        AMI_STREAM<int> amiws((BTE_base_stream<int> *)&btews);
        
        ae = AMI_scan(&amirs, &ss, &amiws);
    }
    
    cout << "Squared them.\n";

    // Interleave the streams.
    {
        interleave_merge<int> im;

        arity_t arity = 2;
        
        AMI_STREAM<int> *amirs[2];

        BTE_STREAM<int> bters0("./TEST_SCAN0", BTE_READ_STREAM);
        amirs[0] = new AMI_STREAM<int>((BTE_base_stream<int> *)&bters0);

        BTE_STREAM<int> bters1("./TEST_SCAN1", BTE_READ_STREAM);
        amirs[1] = new AMI_STREAM<int>((BTE_base_stream<int> *)&bters1);
        
        BTE_STREAM<int> btews("./TEST_MERGE1", BTE_WRITE_STREAM);
        AMI_STREAM<int> amiws((BTE_base_stream<int> *)&btews);
        
        ae = AMI_merge(amirs, arity, &amiws, &im);
    }

    cout << "Interleaved them.\n";
    return 0;
}



