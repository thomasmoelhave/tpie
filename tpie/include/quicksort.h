// Copyright (c) 1994 Darren Erik Vengroff
//
// File: quicksort.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 9/28/94
//
// A basic implementation of quicksort for use in core by AMI_sort() on
// streams or substreams that are small enough.
//
// $Id: quicksort.h,v 1.16 1999-02-17 19:59:33 natsev Exp $
//
#ifndef _QUICKSORT_H
#define _QUICKSORT_H

//extern "C" long int random(void);

#include <comparator.h>

#define CONST const


//A simple class that facilitates doing key sorting followed 
//by in-memory permuting to sort items in-memory. This is 
//particularly useful when key size is much smaller than 
//item size. Note that using this requires that the class Key
//have the comparison operators defined appropriately.

template<class Key>
class qsort_item {
  public:
  Key keyval;
  unsigned int source;

  friend int operator==(const qsort_item &x, const qsort_item &y)
             {return  (x.keyval ==  y.keyval);}

  friend int operator!=(const qsort_item &x, const qsort_item &y)
             {return  (x.keyval !=  y.keyval);}    

  friend int operator<=(const qsort_item &x, const qsort_item &y)
             {return  (x.keyval <=  y.keyval);}

  friend int operator>=(const qsort_item &x, const qsort_item &y)
             {return  (x.keyval >=  y.keyval);}

  friend int operator<(const qsort_item &x, const qsort_item &y)
             {return  (x.keyval <  y.keyval);}

  friend int operator>(const qsort_item &x, const qsort_item &y)
             {return  (x.keyval >  y.keyval);}

  };


// A version that uses a comparison function.  This is useful for
// sorting objects with multiple data members based on a particular
// member or combination of members.

template<class T>
void partition_cmp(T *data, size_t len, size_t &partition,
               int (*cmp)(CONST T&, CONST T&));

template<class T>
void quick_sort_cmp(T *data, size_t len,
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
    
    partition_cmp(data, len, part_index, cmp);

    quick_sort_cmp(data, part_index + 1, cmp);
    quick_sort_cmp(data + part_index + 1, len - part_index - 1, cmp);
}

template<class T>
void partition_cmp(T *data, size_t len, size_t &part_index,
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
void insertion_sort_cmp(T *data, size_t len,
                        int (*cmp)(CONST T&, CONST T&));

template<class T>
void quicker_sort_cmp(T *data, size_t len,
                      int (*cmp)(CONST T&, CONST T&),
                      size_t min_file_len = 10)
{
    quick_sort_cmp(data, len, cmp, min_file_len);
    insertion_sort_cmp(data, len, cmp);
}

template<class T>
void insertion_sort_cmp(T *data, size_t len,
                        int (*cmp)(CONST T&, CONST T&))
{
    T *p, *q, test;

    for (p = data + 1; p < data + len; p++) {
        for (q = p - 1, test = *p; cmp(*q, test) > 0; q--) {
            *(q+1) = *q;
            if (q == data) {
	      q--; // to make assignment below correct
	      break;
            }
        }
        *(q+1) = test;
    }
}


// A version that uses the < operator.  This should be faster for
// intrinsic types such as int, where the compiler can generate very
// good code and avoid a function call inside the innermost loop of
// partition().

template<class T>
void partition_op(T *data, size_t len, size_t &partition);

template<class T>
void quick_sort_op(T *data, size_t len,
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
    
    partition_op(data, len, part_index);

    quick_sort_op(data, part_index + 1);
    quick_sort_op(data + part_index + 1, len - part_index - 1);
}

template<class T>
void partition_op(T *data, size_t len, size_t &part_index)
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
        } while (*q > tpart);
        do {
            p++;
        } while (*p < tpart);

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
void insertion_sort_op(T *data, size_t len);

template<class T>
void quicker_sort_op(T *data, size_t len,
                     size_t min_file_len = 10)
{
    quick_sort_op(data, len, min_file_len);
    insertion_sort_op(data, len);
}

template<class T>
void insertion_sort_op(T *data, size_t len)
{
    T *p, *q, test;

    for (p = data + 1; p < data + len; p++) {
        for (q = p - 1, test = *p; *q > test; q--) {
            *(q+1) = *q;
            if (q == data) {
	      q--; // to make assignment below correct
	      break;
            }
        }
        *(q+1) = test;
    }
}


// A version that uses a comparison object.

template<class T>
void partition_obj(T *data, size_t len, size_t &partition,
                   comparator<T> *cmp);

template<class T>
void quick_sort_obj(T *data, size_t len, comparator<T> *cmp,
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
    
    partition_obj(data, len, part_index, cmp);

    quick_sort_obj(data, part_index + 1, cmp);
    quick_sort_obj(data + part_index + 1, len - part_index - 1, cmp);
}

template<class T>
void partition_obj(T *data, size_t len, size_t &part_index,
                   comparator<T> *cmp)
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
        } while (cmp->compare(*q, tpart) > 0);
        do {
            p++;
        } while (cmp->compare(*p, tpart) < 0);

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
void insertion_sort_obj(T *data, size_t len,
                        comparator<T> *cmp);

template<class T>
void quicker_sort_obj(T *data, size_t len,
                      comparator<T> *cmp,
                      size_t min_file_len = 10)
{
    quick_sort_obj(data, len, cmp, min_file_len);
    insertion_sort_obj(data, len, cmp);
}

template<class T>
void insertion_sort_obj(T *data, size_t len,
                        comparator<T> *cmp)
{
    T *p, *q, test;

    for (p = data + 1; p < data + len; p++) {
        for (q = p - 1, test = *p; (cmp->compare(*q, test) > 0); q--) {
            *(q+1) = *q;
	    if (q==data) {
	      q--; // to make assignment below correct
	      break;
	    }
        }
        *(q+1) = test;
    }
}

#endif // _QUICKSORT_H 














































