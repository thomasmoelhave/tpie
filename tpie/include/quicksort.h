// Copyright (c) 1994 Darren Erik Vengroff
//
// File: quicksort.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 9/28/94
//
// A basic implementation of quicksort for use in core by AMI_sort() on
// streams or substreams that are small enough.
//
// $Id: quicksort.h,v 1.2 1994-10-04 19:09:47 darrenv Exp $
//
#ifndef _QUICKSORT_H
#define _QUICKSORT_H


template<class T>
void partition(T *data, size_t len, size_t &partition,
               int (*cmp)(CONST T&, CONST T&));

template<class T>
void quick_sort(T *data, size_t len,
               int (*cmp)(CONST T&, CONST T&),
                size_t min_file_len = 2)
{
    // On return from partition(), everything at or below this index
    // will be less that or equal to everything above it.
    // Furthermore, it will not be 0 since this will leave us to
    // recurse on the whole array again.
    
    size_t part_index;
    
    if (len < min_file_len) {
        return;
    }
    
    partition(data, len, part_index, cmp);

    quick_sort(data, part_index + 1, cmp);
    quick_sort(data + part_index + 1, len - part_index - 1, cmp);
}

template<class T>
void partition(T *data, size_t len, size_t &part_index,
               int (*cmp)(CONST T&, CONST T&))
{
    T *ptpart, tpart;
    T *p, *q;
    T t0;

    // Try to get a good partition value and avoid being bitten by already
    // sorted input.

    ptpart = data + (random() % len);

    tpart = *ptpart;
    *ptpart = data[0];
    data[0] = tpart;

    // Walk through the array and partition them.

    for (p = data - 1, q = data + len; ; ) {

        do {
            q--;
        } while (cmp(*q, tpart) > 0);
        do {
            p++;
        } while (cmp(*p, tpart) < 0);

        if (p < q) {
            t0 = *p;
            *p = *q;
            *q = t0;
        } else {
            part_index = q - data;            
            break;
        }
    }
}


template<class T>
void insertion_sort(T *data, size_t len,
                    int (*cmp)(CONST T&, CONST T&));

template<class T>
void quicker_sort(T *data, size_t len,
                  int (*cmp)(CONST T&, CONST T&),
                  size_t min_file_len = 10)
{
    quick_sort(data, len, cmp, min_file_len);
    insertion_sort(data, len, cmp);
}

template<class T>
void insertion_sort(T *data, size_t len,
                    int (*cmp)(CONST T&, CONST T&))
{
    T *p, *q, test;

    for (p = data + 1; p < data + len; p++) {
        for (q = p - 1, test = *p; cmp(*q, test) > 0; q--) {
            *(q+1) = *q;
            if (q == data) {
                break;
            }
        }
        *(q+1) = test;
    }
}


#endif // _QUICKSORT_H 
