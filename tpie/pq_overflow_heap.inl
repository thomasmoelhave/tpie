
template<typename T, typename Comparator>
pq_overflow_heap<T, Comparator>::pq_overflow_heap(TPIE_OS_SIZE_T maxsize) {
	h = new Heap<T, Comparator>(maxsize);
	//cout << "OPQ constructor" << endl;
	this->maxsize = maxsize;
}

template<typename T, typename Comparator>
pq_overflow_heap<T, Comparator>::~pq_overflow_heap() {
	//cout << "OPQ destructur" << endl;
	delete h;
}

template<typename T, typename Comparator>
inline void pq_overflow_heap<T, Comparator>::push(const T& x) {
#ifndef NDEBUG
	if(h->size() == maxsize) {
		TP_LOG_FATAL_ID("pq_overflow_heap: push error");
		exit(-1);
	}
#endif
	h->insert(x);
}

template<typename T, typename Comparator>
inline void pq_overflow_heap<T, Comparator>::pop() {
	assert(!empty());
	h->delmin();
}

template<typename T, typename Comparator>
inline const T& pq_overflow_heap<T, Comparator>::top() {
	assert(!empty());
	dummy = h->peekmin();
	return dummy;
}

template<typename T, typename Comparator>
inline const TPIE_OS_OFFSET pq_overflow_heap<T, Comparator>::size() {
	return h->size();
}

template<typename T, typename Comparator>
inline const bool pq_overflow_heap<T, Comparator>::full() {
	return maxsize == h->size();
}

template<typename T, typename Comparator>
inline T* pq_overflow_heap<T, Comparator>::sorted_array() {
	std::sort(h->get_arr()+1, h->get_arr()+1+h->size(), comp_); // compare
	return h->get_arr()+1;
}

template<typename T, typename Comparator>
inline const TPIE_OS_OFFSET pq_overflow_heap<T, Comparator>::sorted_size() {
	return maxsize;
}

template<typename T, typename Comparator>
inline void pq_overflow_heap<T, Comparator>::sorted_pop() {
	h->set_size(0);
}

template<typename T, typename Comparator>
inline const bool pq_overflow_heap<T, Comparator>::empty() {
	return h->empty();
} 
