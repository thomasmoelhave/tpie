// Copyright (c) 1994 Darren Erik Vengroff
//
// File: quicksort.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 9/28/94
//
// A basic implementation of quicksort for use in core by AMI_sort() on
// streams or substreams that are small enough.
//
// $Id: quicksort.h,v 1.22 2003-04-17 19:54:53 jan Exp $
//
#ifndef _QUICKSORT_H
#define _QUICKSORT_H
 
// Get definitions for working with Unix and Windows
#include <portability.h>

#include <comparator.h>

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


// Comment (jan): This version must no be used anymore!

// // A version that uses a comparison function.  This is useful for
// // sorting objects with multiple data members based on a particular
// // member or combination of members.

// End Comment.


// A version that uses the < operator.  This should be faster for
// intrinsic types such as int, where the compiler can generate very
// good code and avoid a function call inside the innermost loop of
// partition().

template<class T>
void partition_op(T *data, size_t len, size_t &partition);

template<class T>
void __quick_sort_op(T *data, size_t len,
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

    __quick_sort_op(data, part_index + 1, min_file_len);
    __quick_sort_op(data + part_index + 1, len - part_index - 1, min_file_len);
}

template<class T>
void partition_op(T *data, size_t len, size_t &part_index)
{
    T *ptpart, tpart;
    T *p, *q;
    T t0;

    // Try to get a good partition value and avoid being bitten by already
    // sorted input.

    ptpart = data + (TPIE_OS_RANDOM() % len);

    tpart = *ptpart;
    *ptpart = data[0];
    data[0] = tpart;

    // Walk through the array and partition them.

    for (p = data - 1, q = data + len; ; ) {

        do {
            q--;
            //} while (*q > tpart); dh. changed to limit operators required
        } while (tpart < *q );
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
void quick_sort_op(T *data, size_t len,
                     size_t min_file_len = 20)
{
    __quick_sort_op(data, len, min_file_len);
    insertion_sort_op(data, len);
}

template<class T>
void insertion_sort_op(T *data, size_t len)
{
    T *p, *q, test;

    for (p = data + 1; p < data + len; p++) {
        //for (q = p - 1, test = *p; *q > test; q--) { dh
        for (q = p - 1, test = *p; test < *q; q--) {
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
template<class T, class CMPR>
void partition_obj(T *data, size_t len, size_t &partition,
                   CMPR  *cmp);

template<class T, class CMPR>
void __quick_sort_obj(T *data, size_t len, CMPR *cmp,
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

    __quick_sort_obj(data, part_index + 1, cmp, min_file_len);
    __quick_sort_obj(data + part_index + 1, len - part_index - 1, cmp, min_file_len);
}

template<class T, class CMPR>
void partition_obj(T *data, size_t len, size_t &part_index,
                   CMPR *cmp)
{
    T *ptpart, tpart;
    T *p, *q;
    T t0;

    // Try to get a good partition value and avoid being bitten by already
    // sorted input.

    ptpart = data + (TPIE_OS_RANDOM() % len);

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


template<class T, class CMPR>
void insertion_sort_obj(T *data, size_t len,
                        CMPR *cmp);

template<class T, class CMPR>
void quick_sort_obj(T *data, size_t len,
                      CMPR *cmp,
                      size_t min_file_len = 20)
{
    __quick_sort_obj(data, len, cmp, min_file_len);
    insertion_sort_obj(data, len, cmp);
}

template<class T, class CMPR>
void insertion_sort_obj(T *data, size_t len,
                        CMPR *cmp)
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














































