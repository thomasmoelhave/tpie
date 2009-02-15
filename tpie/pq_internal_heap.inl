// Copyright 2008, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

#ifndef __HEAP_H__
#define __HEAP_H__

/////////////////////////////////////////////////////////
///
/// \class Heap
/// \author Lars Hvam Petersen
///
/// Adapted from: http://www.cs.princeton.edu/~rs/Algs3.c1-4/
///
/////////////////////////////////////////////////////////
template <typename T, typename Comparator = std::less<T> >
class Heap {
 public:
  /////////////////////////////////////////////////////////
  ///
  /// Constructor
  ///
  /// \param maxN Maximal size of queue
  ///
  /////////////////////////////////////////////////////////
  Heap(unsigned int maxN) { 
    pq = new T[maxN+1]; 
    N = 0; 
  }

  Heap(T* arr, unsigned int size) {
    pq = arr;
    N = size;
  }
  
  /////////////////////////////////////////////////////////
  ///
  /// Destructor
  ///
  /////////////////////////////////////////////////////////
  ~Heap() { 
    delete pq; 
  } 
  
  /////////////////////////////////////////////////////////
  ///
  /// Return true if queue is empty otherwise false
  ///
  /// \return Boolean - empty or not
  ///
  /////////////////////////////////////////////////////////
  bool empty() { 
    return N == 0; 
  }

  /////////////////////////////////////////////////////////
  ///
  /// Returns the size of the queue
  ///
  /// \return Queue size
  ///
  /////////////////////////////////////////////////////////
  unsigned size() {
    return (unsigned)N;
  }

  /////////////////////////////////////////////////////////
  ///
  /// Insert an element into the heap 
  ///
  /// \param v The element that should be inserted
  ///
  /////////////////////////////////////////////////////////
  void insert(T v) { 
//  cout << "insert " << v << endl;
    pq[++N] = v; 
    fixUp(pq, N); 
  }

  /////////////////////////////////////////////////////////
  ///
  /// Remove the minimal element from heap
  ///
  /// \return Minimal element
  ///
  /////////////////////////////////////////////////////////
  T delmin() { 
    exch(pq[1], pq[N]); 
    fixDown2(pq, 1, N-1); 
    return pq[N--]; 
  }

  /////////////////////////////////////////////////////////
  ///
  /// Peek the minimal element
  ///
  /// \return Minimal element
  ///
  /////////////////////////////////////////////////////////
  T peekmin() {
    return pq[1];
  }

  /////////////////////////////////////////////////////////
  ///
  /// Set the size 
  ///
  /// \param ne Size
  ///
  /////////////////////////////////////////////////////////
  void set_size(unsigned int ne) {
    N = ne;
  }

  /////////////////////////////////////////////////////////
  ///
  /// Returns a pointer to the underlaying array 
  ///
  /// \return Array
  ///
  /////////////////////////////////////////////////////////
  T* get_arr() {
    return pq;
  }

 private:
  Comparator comp_;

  inline void exch(T& i, T& j) { 
    T t = i;
    i = j;
    j = t;
  }

  void fixDown2(T a[], int k, int N) { 
    int j;
    while(2*k <= N) {
  	  j = 2*k;
	    if(j < N && comp_(a[j+1], a[j])) j++; // compare, a[j] > a[j+1]
	    if(! comp_(a[j], a[k]) ) break; // compare, a[k] > a[j]
	    exch(a[k], a[j]); 
      k = j;
	  }
  }
  
  void fixUp(T a[], int k) {
    while(k > 1 && comp_(a[k], a[k/2])) { // compare, a[k/2] > a[k]
      exch(a[k], a[k/2]); k = k/2; 
    }
  }

  inline void fixDown(T a[], int k, const int N) { 
    int j;
    while(2*k <= N) { 
      j = 2*k;
      if(j < N && comp_(a[j], a[j+1])) j++; // compare
      if(!comp_(a[k], a[j])) break; // compare
      exch(a[k], a[j]); 
      k = j;
    }
  }
  
  T *pq; 
  int N;
};

#endif
