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


template<typename T, typename Comparator>
pq_overflow_heap<T, Comparator>::pq_overflow_heap(memory_size_type maxsize) {
	h = new pq_internal_heap<T, Comparator>(maxsize);
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
inline memory_size_type pq_overflow_heap<T, Comparator>::size() const {
	return h->size();
}

template<typename T, typename Comparator>
inline bool pq_overflow_heap<T, Comparator>::full() const {
	return maxsize == h->size();
}

template<typename T, typename Comparator>
inline T* pq_overflow_heap<T, Comparator>::sorted_array() {
	std::sort(h->get_arr(), h->get_arr()+h->size(), comp_); // compare
	return h->get_arr();
}

template<typename T, typename Comparator>
inline memory_size_type pq_overflow_heap<T, Comparator>::sorted_size() const{
	return maxsize;
}

template<typename T, typename Comparator>
inline void pq_overflow_heap<T, Comparator>::sorted_pop() {
	h->set_size(0);
}

template<typename T, typename Comparator>
inline bool pq_overflow_heap<T, Comparator>::empty() const {
	return h->empty();
} 
