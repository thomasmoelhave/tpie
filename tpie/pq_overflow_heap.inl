#define CPPOPQHEAP

#include "Heap.h"

using namespace std;

template<typename T, typename Comparator>
OPQHeap<T, Comparator>::OPQHeap(TPIE_OS_OFFSET maxsize) {
	h = new Heap<T, Comparator>(maxsize);
	//cout << "OPQ constructor" << endl;
	this->maxsize = maxsize;
}

template<typename T, typename Comparator>
OPQHeap<T, Comparator>::~OPQHeap() {
	//cout << "OPQ destructur" << endl;
	delete h;
}

template<typename T, typename Comparator>
inline void OPQHeap<T, Comparator>::push(const T& x) {
#ifndef NDEBUG
	if(h->size() == maxsize) {
		cout << "OPQHeap: push error" << endl;
		exit(-1);
	}
#endif
	h->insert(x);
}

template<typename T, typename Comparator>
inline void OPQHeap<T, Comparator>::pop() {
	assert(!empty());
	h->delmin();
}

template<typename T, typename Comparator>
inline const T& OPQHeap<T, Comparator>::top() {
	assert(!empty());
	dummy = h->peekmin();
	return dummy;
}

template<typename T, typename Comparator>
inline const TPIE_OS_OFFSET OPQHeap<T, Comparator>::size() {
	return h->size();
}

template<typename T, typename Comparator>
inline const bool OPQHeap<T, Comparator>::full() {
	return maxsize == h->size();
}

template<typename T, typename Comparator>
inline T* OPQHeap<T, Comparator>::sorted_array() {
	std::sort(h->get_arr()+1, h->get_arr()+1+h->size(), comp_); // compare
	return h->get_arr()+1;
}

template<typename T, typename Comparator>
inline const TPIE_OS_OFFSET OPQHeap<T, Comparator>::sorted_size() {
	return maxsize;
}

template<typename T, typename Comparator>
inline void OPQHeap<T, Comparator>::sorted_pop() {
	h->set_size(0);
}

template<typename T, typename Comparator>
inline const bool OPQHeap<T, Comparator>::empty() {
	return h->empty();
} 
