#ifndef _MINMAXHEAP_H
#define _MINMAXHEAP_H

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <strstream.h>

#include "app_config.h"
#include <ami.h>  



/***************************************************************** 
 ***************************************************************** 
 ***************************************************************** 

Priority queue templated on a single type;

T is assumed to be a class with get_priority() and get_elt()
implemented;

Supported operations: min, extract_min, insert, max, extract_max

***************************************************************** 
***************************************************************** 
*****************************************************************/

typedef unsigned int HeapIndex;


template <class T>
class MinMaxHeap {
private:  
  HeapIndex maxsize, lastindex;
  T *A;

public:
  MinMaxHeap(HeapIndex size) : maxsize(size) { 
    char str[100];
    sprintf(str, "MinMaxHeap: allocate %ld\n", (size+1)*sizeof(T));
    LOG_DEBUG_ID(str);
    
    lastindex = 0;
    LOG_DEBUG_ID("minmaxheap: allocation");
    A = new T[maxsize+1]; 
  };
  
  ~MinMaxHeap(void) { 
    LOG_DEBUG_ID("minmaxheap: deallocation");
    delete [] A; 
  };
  
  bool full(void) const { return size() >= maxsize; };
  bool empty(void) const { return size() == 0; };

  HeapIndex size(void) const { 
    assert(A);
    return lastindex; };
  
  T get(HeapIndex i) const { assert(i <= size()); return A[i]; }
   
  //build a heap from an array of elements; 
  //if size > maxsize, insert first maxsize elements from array;
  //return nb of elements that did not fit;
  inline HeapIndex fill(T* arr, HeapIndex n);
  
  inline bool insert(const T& elt);
  inline bool min(T& elt) const ;
  inline bool extract_min(T& elt);
  inline bool max(T& elt) const;
  inline bool extract_max(T& elt);

  inline void reset() {};

  inline void print() const;
  inline void print_range() const;
  inline friend ostream& operator<<(ostream& s, const MinMaxHeap<T> &pq) {
    HeapIndex i;
    s <<  "[";
    for(i = 1; i <= pq.size(); i++) {
      s << " " << pq.get(i);
    }
    s << "]";
    return s;
  }

private:
  inline long log2(long n) const;
  int isOnMaxLevel(HeapIndex i) const { return (log2(i) % 2); };
  int isOnMinLevel(HeapIndex i) const { return !isOnMaxLevel(i); };

  HeapIndex leftChild(HeapIndex i) const { return 2*i; };
  HeapIndex rightChild(HeapIndex i) const { return 2*i + 1; };
  int hasRightChild(HeapIndex i) const { return (rightChild(i) <= size()); };
  HeapIndex parent(HeapIndex i) const { return (i/2); };
  HeapIndex grandparent(HeapIndex i) const { return (i/4); };
  int hasChildren(HeapIndex i) const { return (2*i) <= size(); }; // 1 or more
  inline void swap(HeapIndex a, HeapIndex b);

  inline T leftChildValue(HeapIndex i) const;
  inline T rightChildValue(HeapIndex i) const;
  inline HeapIndex smallestChild(HeapIndex i) const;
  inline HeapIndex smallestChildGrandchild(HeapIndex i) const;
  inline HeapIndex largestChild(HeapIndex i) const;
  inline HeapIndex largestChildGrandchild(HeapIndex i) const;
  inline int isGrandchildOf(HeapIndex i, HeapIndex m) const;

  inline void trickleDownMin(HeapIndex i);
  inline void trickleDownMax(HeapIndex i);
  inline void trickleDown(HeapIndex i);

  inline void bubbleUp(HeapIndex i);
  inline void bubbleUpMin(HeapIndex i);
  inline void bubbleUpMax(HeapIndex i);
};


// index 0 is invalid
// index <= size

// ----------------------------------------------------------------------

template <class T> 
inline long MinMaxHeap<T>::log2(long n) const {
  long i=-1;
  // let log2(0)==-1
  while(n) {
	n = n >> 1;
	i++;
  }
  return i;
}


// ----------------------------------------------------------------------

template <class T> 
inline void MinMaxHeap<T>::swap(HeapIndex a, HeapIndex b) {
  T tmp;
  tmp = A[a];
  A[a] = A[b];
  A[b] = tmp;
}


// ----------------------------------------------------------------------

// child must exist
template <class T>
inline T MinMaxHeap<T>::leftChildValue(HeapIndex i) const {
  HeapIndex p = leftChild(i);
  assert(p <= size());
  return A[p];
}

// ----------------------------------------------------------------------

// child must exist
template <class T>
inline T MinMaxHeap<T>::rightChildValue(HeapIndex i) const {
  HeapIndex p = rightChild(i);
  assert(p <= size());
  return A[p];
}


// ----------------------------------------------------------------------

// returns index of the smallest of children of node
// it is an error to call this function if node has no children
template <class T>
inline HeapIndex MinMaxHeap<T>::smallestChild(HeapIndex i) const {
  assert(hasChildren(i));
  if(hasRightChild(i) && (leftChildValue(i) > rightChildValue(i))) {
	return rightChild(i);
  } else {
	return leftChild(i);
  }
}

// ----------------------------------------------------------------------

template <class T>
inline HeapIndex MinMaxHeap<T>::largestChild(HeapIndex i) const {
  assert(hasChildren(i));
  if(hasRightChild(i) && (leftChildValue(i) < rightChildValue(i))) {
	return rightChild(i);
  } else {
	return leftChild(i);
  }
}

// ----------------------------------------------------------------------

// error to call on node without children
template <class T>
inline HeapIndex MinMaxHeap<T>::smallestChildGrandchild(HeapIndex i) const {
  HeapIndex p,q;
  HeapIndex minpos = 0;

  assert(hasChildren(i));

  p = leftChild(i);
  if(hasChildren(p)) {
	q = smallestChild(p);
	if(A[p] > A[q]) p = q;
  }
  // p is smallest of left child, its grandchildren
  minpos = p;

  if(hasRightChild(i)) {
	p = rightChild(i);
	if(hasChildren(p)) {
	  q = smallestChild(p);
	  if(A[p] > A[q]) p = q;
	}
	// p is smallest of right child, its grandchildren
	if(A[p] < A[minpos]) minpos = p;
  }
  return minpos;
}

// ----------------------------------------------------------------------

template <class T>
inline HeapIndex MinMaxHeap<T>::largestChildGrandchild(HeapIndex i) const {
  HeapIndex p,q;
  HeapIndex maxpos = 0;

  assert(hasChildren(i));

  p = leftChild(i);
  if(hasChildren(p)) {
	q = largestChild(p);
	if(A[p] < A[q]) p = q;
  }
  // p is smallest of left child, its grandchildren
  maxpos = p;

  if(hasRightChild(i)) {
	p = rightChild(i);
	if(hasChildren(p)) {
	  q = largestChild(p);
	  if(A[p] < A[q]) p = q;
	}
	// p is smallest of right child, its grandchildren
	if(A[p] > A[maxpos]) maxpos = p;
  }
  return maxpos;
}

// ----------------------------------------------------------------------

// this is pretty loose - only to differentiate between child and grandchild
template <class T>
inline int MinMaxHeap<T>::isGrandchildOf(HeapIndex i, HeapIndex m) const {
  return (m >= i*4);
}

// ----------------------------------------------------------------------

template <class T>
inline void MinMaxHeap<T>::trickleDownMin(HeapIndex i) {
  HeapIndex m;
  bool done = false;
  
  while (!done) {
    
    if (!hasChildren(i)) {
      done = true;
      return;
    }
    m = smallestChildGrandchild(i);
    if(isGrandchildOf(i, m)) {
      if(A[m] < A[i]) {
	swap(i, m);
	if(A[m] > A[parent(m)]) {
	  swap(m, parent(m));
	}
	//trickleDownMin(m);
	i = m;
      } else {
	done = true;
      }
    } else {
      if(A[m] < A[i]) {
	swap(i, m);
      }
      done = true;
    }
  }//while
}

// ----------------------------------------------------------------------

// unverified
template <class T>
inline void MinMaxHeap<T>::trickleDownMax(HeapIndex i) {
  HeapIndex m;
  bool done = false;

  while (!done) {
    if(!hasChildren(i)) {
     done = true;
     return;
    }
    
    m = largestChildGrandchild(i);
    if(isGrandchildOf(i, m)) {
      if(A[m] > A[i]) {
	swap(i, m);
	if(A[m] < A[parent(m)]) {
	  swap(m, parent(m));
	}
	//trickleDownMax(m);
	i = m;
      } else {
	done = true;
      }
    } else {
      if(A[m] > A[i]) {
	swap(i, m);
      }
      done = true;
    }
  } //while
}


// ----------------------------------------------------------------------


template <class T>
inline void MinMaxHeap<T>::trickleDown(HeapIndex i) {
  if(isOnMinLevel(i)) {
	trickleDownMin(i);
  } else {
	trickleDownMax(i);
  }
}

// ----------------------------------------------------------------------
template <class T>
inline void MinMaxHeap<T>::bubbleUp(HeapIndex i) {
  HeapIndex m;
  m = parent(i);
  
  if(isOnMinLevel(i)) {
	if (m && (A[i] > A[m])) {
	  swap(i, m);
	  bubbleUpMax(m);
	} else {
	  bubbleUpMin(i);
	} 
  } else {
	if (m && (A[i] < A[m])) {
	  swap(i, m);
	  bubbleUpMin(m);
	} else {
	  bubbleUpMax(i);
	}
  }
}


// ----------------------------------------------------------------------
template <class T>
inline void MinMaxHeap<T>::bubbleUpMin(HeapIndex i) {
  HeapIndex m;
  m = grandparent(i);

  while (m && (A[i] < A[m])) {
	 swap(i,m);
	 //bubbleUpMin(m);
	 i = m;
	 m = grandparent(i);
	 
  }
}



// ----------------------------------------------------------------------
template <class T>
inline void MinMaxHeap<T>::bubbleUpMax(HeapIndex i) {
  HeapIndex m;
  m = grandparent(i);
  
  while(m && (A[i] > A[m])) {
	swap(i,m);
	//bubbleUpMax(m);
	i=m;
	m = grandparent(i);
  }
}




// ----------------------------------------------------------------------
template <class T>
inline void MinMaxHeap<T>::print() const {
  cout << "[";
  for (unsigned int i=1; i<=size(); i++) {
    cout << A[i].get_priority().field1() <<",";
  }
  cout << "]";
}

// ----------------------------------------------------------------------
template <class T>
inline void MinMaxHeap<T>::print_range() const {
  cout << "[";
  T a, b;
  min(a);
  max(b);
  if (size) {
    cout << a.get_priority().field1() << ".."
	 << b.get_priority().field1();
  }
  cout << " (" << size() << ")]";
}


// ----------------------------------------------------------------------
template <class T>
inline bool MinMaxHeap<T>::insert(const T& elt) {

  if(lastindex == maxsize) return false;

  lastindex++;
  A[lastindex] = elt;
  bubbleUp(lastindex);
  
  return true;
}

// ----------------------------------------------------------------------
template <class T>
inline bool MinMaxHeap<T>::extract_min(T& elt) {

  assert(A);

  if(lastindex == 0) return false;

  elt = A[1];
  A[1] = A[lastindex];
  lastindex--;
  trickleDown(1);
  
  return true;
}

// ----------------------------------------------------------------------
template <class T>
inline bool MinMaxHeap<T>::extract_max(T& elt) {

  assert(A);
  
  HeapIndex p;					// max
  if(lastindex == 0) return false;
  
  if(hasChildren(1)) {
	p = largestChild(1);
  } else {
	p = 1;
  }
  elt = A[p];
  A[p] = A[lastindex];
  lastindex--;
  trickleDown(p);
  
  return true;
}

// ----------------------------------------------------------------------
template <class T>
inline bool MinMaxHeap<T>::min(T& elt) const {
  
  assert(A);
  
  if(lastindex == 0) return false;

  elt = A[1];
  return true;
}

// ----------------------------------------------------------------------
template <class T>
inline bool MinMaxHeap<T>::max(T& elt) const {
  
  assert(A);
  
  HeapIndex p;					// max
  if(lastindex == 0) return false;
  
  if(hasChildren(1)) {
	p = largestChild(1);
  } else {
	p = 1;
  }
  elt = A[p];
  return true;
}


// ----------------------------------------------------------------------
//build a heap from an array of elements; 
//if size > maxsize, insert first maxsize elements from array;
//return nb of elements that did not fit;
template <class T>
inline HeapIndex MinMaxHeap<T>::fill(T* arr, HeapIndex n) {
  HeapIndex i;
  //heap must be empty
  assert(size()==0);
  for (i = 0; i<n; i++) {
    if (!insert(arr[i])) { 
      break;
    }
  }
  if (i < n) {
    assert(i == maxsize);
    return n - i;
  } else {
    return 0;
  }
};
 
#endif
