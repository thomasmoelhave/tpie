// Copyright (c) 1994 Darren Erik Vengroff
//
// File: quicksort.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 9/28/94
//
// A basic implementation of quicksort for use in core by AMI_sort() on
// streams or substreams that are small enough.
//
// $Id: quicksort.h,v 1.1 1994-09-29 13:13:15 darrenv Exp $
//
#ifndef _QUICKSORT_H
#define _QUICKSORT_H

template<class T>
void partition(T *data, size_t len, size_t &partition);

template<class T>
void quick_sort(T *data, size_t len)
{
    // On return from partition(), everything at or below this index
    // will be less that or equal to everything above it.
    // Furthermore, it will not be 0 since this will leave us to
    // recurse on the whole array again.
    
    size_t part_index;
    
    if (len <= 1) {
        return;
    }
    
    partition(data, len, part_index);

    tp_assert((part_index > 0) || (part_index < len - 1),
              "About to recurse on the same exact array.");

    if (part_index > 0) {
        quick_sort(data, part_index + 1);
    }
    if (len - part_index > 2) {
        quick_sort(data + part_index + 1, len - part_index - 1);
    }
}

template<class T>
inline T min(const T &t1, const T &t2) { return (t1<t2) ? t1 : t2; }

template<class T>
inline T max(const T &t1, const T &t2) { return (t1<t2) ? t2 : t1; }

template<class T>
void partition(T *data, size_t len, size_t &part_index)
{
    T t0, t1, t2, tpart;
    T *p, *q;

    // Try to get a good partition value and avoid being bitten by already
    // sorted input.
    
    t0 = data[0];
    t1 = data[len / 2];
    t2 = data[len - 1];

    tpart = min(t0, max(t1, t2));

    // Walk through the array and partition them.

    for (p = data, q = data + len - 1; ; ) {
        while ((p < q) && !(*q < tpart)) {
            q--;
        }
        // while ((p < q) && !(tpart < *p)) {
        while ((p < q) && (*p < tpart)) {
            p++;
        }

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


#endif // _QUICKSORT_H 
