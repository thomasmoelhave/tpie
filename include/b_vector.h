//
// File:    b_vector.h
// Authors: Octavian Procopiuc <tavi@cs.duke.edu>
//
// Definition of the b_vector class.
//
// $Id: b_vector.h,v 1.4 2001-05-17 20:10:19 tavi Exp $
//

#ifndef _B_VECTOR_H
#define _B_VECTOR_H

#include <string.h>

template<class T>
class b_vector {
private:
  T* p_;
  size_t capacity_;

public:

  typedef T* iterator;

  b_vector(T* p, size_t cap): p_(p), capacity_(cap) {}

  // Get a reference to the i'th element.
  T& operator[](size_t i) { return *(p_ + i); }
  // Get a const reference to the i'th element.
  const T& operator[](size_t i) const {return *(p_ + i); }

  size_t capacity() const { return capacity_; }

  // Copy length elements from the source vector, starting with 
  // element s_start, to this block, starting with element start. 
  // Return the number of elements copied. Source can be *this.
  size_t copy(size_t start, size_t length,
	      const b_vector<T>& source, size_t s_start = 0);

  // Copy from an array of elements.
  size_t copy(size_t start, size_t length, const T* source);

  // Insert item t in position pos; all items from position pos onward
  // are shifted one position higher; the last item is lost.
  void insert(const T& t, size_t pos);

  // Erase the item in position pos and shift all items from position
  // pos+1 onward one position lower; the last item becomes identical
  // with the next to last item.
  void erase(size_t pos);

};


////////////////////////////////
///////// **b_vector** /////////
////////////////////////////////

//// *b_vector::copy* ////
template<class T>
size_t b_vector<T>::copy(size_t start, size_t length,
	      const b_vector<T>& source, size_t s_start = 0) {
  
  // copy_length will store the actual number of items that can be copied.
  size_t copy_length = length;

  if (start < capacity_ && s_start < source.capacity()) {
    // Check how much of length we can copy.
    copy_length = (copy_length > capacity_ - start) ? 
      capacity_ - start: copy_length;
    copy_length = (copy_length > source.capacity() - s_start) ? 
      source.capacity() - s_start: copy_length;
    
    memmove(&(*this)[start], &source[s_start], copy_length * sizeof(T));
  } else {
    // start is too big. No copying.
    copy_length = 0;
  }
  
  return copy_length;
}

//// *b_vector::copy* ////
template<class T>
size_t b_vector<T>::copy(size_t start, size_t length, const T* source) {

  size_t copy_length = length;

  if (start < capacity_) {
    // Check how much of length we can copy.
    copy_length = (copy_length > capacity_ - start) ? 
      capacity_ - start: copy_length;

    memmove(&(*this)[start], source, copy_length * sizeof(T));
  } else
    copy_length = 0;

  return copy_length;
}

//// *b_vector::insert* ////
template<class T>
void b_vector<T>::insert(const T& t, size_t pos) {
  copy(pos + 1, capacity_ - pos - 1, *this, pos);
  copy(pos, 1, &t);
}

//// *b_vector::erase* ////
template<class T>
void b_vector<T>::erase(size_t pos) {
  copy(pos, capacity_ - pos - 1, *this, pos + 1);
}

#endif // _B_VECTOR_H
