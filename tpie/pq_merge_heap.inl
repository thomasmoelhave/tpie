
template <typename T, typename Comparator>
pq_merge_heap<T, Comparator>::pq_merge_heap(TPIE_OS_OFFSET elements) {
	maxsize = elements;
	heap = new T[elements];
	if(heap == NULL) throw std::bad_alloc();
	runs = new TPIE_OS_OFFSET[elements];
	if(runs == NULL) throw std::bad_alloc();

	m_size = 0;
}

template <typename T, typename Comparator>
pq_merge_heap<T, Comparator>::~pq_merge_heap() {
	delete[] heap;
	delete[] runs;
}

template <typename T, typename Comparator>
void pq_merge_heap<T, Comparator>::push(const T& x, TPIE_OS_OFFSET run) {
	heap[m_size] = x;
	runs[m_size] = run;
	TPIE_OS_OFFSET parent;
	TPIE_OS_OFFSET child;
	child = m_size;
	parent = ( child - 1 ) / 2;
	m_size++;
	//cout << "pq_merge_heap: push" << endl;
	while(parent >= 0 && comp_(heap[child],heap[parent])) {
		//cout << "pq_merge_heap: fixup, child: " << heap[child] << " parent:" << heap[parent] << endl;
		std::swap(heap[child],heap[parent]);
		std::swap(runs[child],runs[parent]);
		child = parent;
		parent = ( child - 1 ) / 2;
	}
#ifndef NDEBUG
	validate();
#endif
}

template <typename T, typename Comparator>
void pq_merge_heap<T, Comparator>::pop() {
	m_size--;
	if(m_size != 0) {
		heap[0] = heap[m_size];
		runs[0] = runs[m_size];
		fixDown();
	}
#ifndef NDEBUG
	validate();
#endif
}

template <typename T, typename Comparator>
void pq_merge_heap<T, Comparator>::pop_and_push(const T& x, TPIE_OS_OFFSET run) {
	heap[0] = x;
	runs[0] = run;
	fixDown();
#ifndef NDEBUG
	validate();
#endif
}

template <typename T, typename Comparator>
const T& pq_merge_heap<T, Comparator>::top() {
	min = heap[0];
	return min;
}

template <typename T, typename Comparator>
const TPIE_OS_OFFSET pq_merge_heap<T, Comparator>::top_run() {
	return runs[0];
}

template <typename T, typename Comparator>
const TPIE_OS_OFFSET pq_merge_heap<T, Comparator>::size() {
	return m_size;
}

template <typename T, typename Comparator>
const bool pq_merge_heap<T, Comparator>::empty() {
	return m_size == 0;
}

///////////////////////////////////////
// Private
///////////////////////////////////////

template <typename T, typename Comparator>
void pq_merge_heap<T, Comparator>::fixDown() {
	TPIE_OS_OFFSET parent, child1, child2;
	assert(m_size > 0);
	bool done = false;
	if(m_size == 1) done = true;
	parent = 0;
	child1 = parent * 2 + 1;
	child2 = parent * 2 + 2;

	//cout << "Loop Start, heap size:" << m_size<< endl;
	while(!done) {
		//cout << "pq_merge_heap: loop top" << child1 << " " << child2 << " " << m_size << endl;
		if(child1 == m_size-1) {
			//cout << "pq_merge_heap: fixdown 0" << endl;
			if(comp_(heap[child1],heap[parent])) {
				//cout << "pq_merge_heap: swap, fixdown 0 " << heap[child1] << " " << heap[parent] << endl;
				assert(child1 < maxsize);
				assert(parent < maxsize);
				std::swap(heap[child1],heap[parent]);
				std::swap(runs[child1],runs[parent]);
			}
			done = true;
			continue;
		} 
		if(child1 > m_size-1) {
			//cout << "pq_merge_heap done 1" << endl;
			done = true;
			continue;
		}
		if(child2 > m_size-1) {
			//cout << "pq_merge_heap done 2" << endl;
			done = true;
			continue;
		}
		//cout << "pq_merge_heap: loop step " << child1 << " " << child2 << " " << parent << " " << m_size << endl; 
		if(comp_(heap[child1],heap[child2]) && comp_(heap[child1],heap[parent])) {
			//cout << "pq_merge_heap: fixdown 1: " << heap[child1] << "(" << child1 << ") " << heap[parent] << "("<<parent << ")" << endl;
			assert(child1 < maxsize);
			assert(parent < maxsize);
			std::swap(heap[child1],heap[parent]);
			std::swap(runs[child1],runs[parent]);
			parent = child1;
		} else if(comp_(heap[child2],heap[parent])) {
			//cout << "pq_merge_heap: fixdown 2: " << heap[child2] << "(" << child2 << ") " << heap[parent] << "("<<parent << ")" << endl;
			assert(child2 < maxsize);
			assert(parent < maxsize);
			std::swap(heap[child2],heap[parent]);
			std::swap(runs[child2],runs[parent]);
			parent = child2;
		} else {
			done = true; 
			continue;
		}
		child1 = parent * 2 + 1;
		child2 = parent * 2 + 2;
	}
}

template <typename T, typename Comparator>
void pq_merge_heap<T, Comparator>::validate() {
#ifndef NDEBUG
#ifdef PQ_VALIDATE
	TPIE_OS_OFFSET child1, child2;
	for(TPIE_OS_OFFSET i = 0; i<m_size; i++) {
		child1 = i * 2 + 1;
		child2 = i * 2 + 2;
		if(child1<m_size) {
			if(comp_(heap[child1],heap[i])) dump();
			assert(!comp_(heap[child1],heap[i]));
		}
		if(child2<m_size) {
			if(comp_(heap[child2],heap[i])) dump();
			assert(!comp_(heap[child2],heap[i]));
		}
	}
#endif
#endif
}

template <typename T, typename Comparator>
void pq_merge_heap<T, Comparator>::dump() {
	TP_LOG_DEBUG("pq_merge_heap: "); 
	for(TPIE_OS_OFFSET i = 0; i<m_size; i++) {
		TP_LOG_DEBUG(heap[i] << ", ");
	}
	TP_LOG_DEBUG("\n");
}
