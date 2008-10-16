#define CPPMERGEHEAP

#include <ami.h>

template <typename T, typename Comparator>
MergeHeap<T, Comparator>::MergeHeap(TPIE_OS_OFFSET elements) {
  maxsize = elements;
  heap = new T[elements];
  if(heap == NULL) throw std::bad_alloc();
  runs = new TPIE_OS_OFFSET[elements];
  if(runs == NULL) throw std::bad_alloc();

  m_size = 0;
}

template <typename T, typename Comparator>
MergeHeap<T, Comparator>::~MergeHeap() {
  delete heap;
  delete runs;
}

template <typename T, typename Comparator>
void MergeHeap<T, Comparator>::push(const T& x, TPIE_OS_OFFSET run) {
  heap[m_size] = x;
  runs[m_size] = run;
  TPIE_OS_OFFSET parent;
  TPIE_OS_OFFSET child;
  child = m_size;
  parent = ( child - 1 ) / 2;
  m_size++;
//cout << "MergeHeap: push" << endl;
  while(parent >= 0 && comp_(heap[child],heap[parent])) {
//cout << "MergeHeap: fixup, child: " << heap[child] << " parent:" << heap[parent] << endl;
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
void MergeHeap<T, Comparator>::pop() {
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
void MergeHeap<T, Comparator>::pop_and_push(const T& x, TPIE_OS_OFFSET run) {
  heap[0] = x;
  runs[0] = run;
  fixDown();
  #ifndef NDEBUG
    validate();
  #endif
}

template <typename T, typename Comparator>
const T& MergeHeap<T, Comparator>::top() {
  min = heap[0];
  return min;
}

template <typename T, typename Comparator>
const TPIE_OS_OFFSET MergeHeap<T, Comparator>::top_run() {
  return runs[0];
}

template <typename T, typename Comparator>
const TPIE_OS_OFFSET MergeHeap<T, Comparator>::size() {
  return m_size;
}

template <typename T, typename Comparator>
const bool MergeHeap<T, Comparator>::empty() {
  return m_size == 0;
}

///////////////////////////////////////
// Private
///////////////////////////////////////

template <typename T, typename Comparator>
void MergeHeap<T, Comparator>::fixDown() {
  TPIE_OS_OFFSET parent, child1, child2;
  assert(m_size > 0);
  bool done = false;
  if(m_size == 1) done = true;
  parent = 0;
  child1 = parent * 2 + 1;
  child2 = parent * 2 + 2;

//cout << "Loop Start, heap size:" << m_size<< endl;
  while(!done) {
//cout << "MergeHeap: loop top" << child1 << " " << child2 << " " << m_size << endl;
    if(child1 == m_size-1) {
//cout << "MergeHeap: fixdown 0" << endl;
      if(comp_(heap[child1],heap[parent])) {
//cout << "MergeHeap: swap, fixdown 0 " << heap[child1] << " " << heap[parent] << endl;
        assert(child1 < maxsize);
        assert(parent < maxsize);
        std::swap(heap[child1],heap[parent]);
        std::swap(runs[child1],runs[parent]);
      }
      done = true;
      continue;
    } 
    if(child1 > m_size-1) {
//cout << "MergeHeap done 1" << endl;
      done = true;
      continue;
    }
    if(child2 > m_size-1) {
//cout << "MergeHeap done 2" << endl;
      done = true;
      continue;
    }
//cout << "MergeHeap: loop step " << child1 << " " << child2 << " " << parent << " " << m_size << endl; 
    if(comp_(heap[child1],heap[child2]) && comp_(heap[child1],heap[parent])) {
//cout << "MergeHeap: fixdown 1: " << heap[child1] << "(" << child1 << ") " << heap[parent] << "("<<parent << ")" << endl;
      assert(child1 < maxsize);
      assert(parent < maxsize);
      std::swap(heap[child1],heap[parent]);
      std::swap(runs[child1],runs[parent]);
      parent = child1;
    } else if(comp_(heap[child2],heap[parent])) {
//cout << "MergeHeap: fixdown 2: " << heap[child2] << "(" << child2 << ") " << heap[parent] << "("<<parent << ")" << endl;
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
void MergeHeap<T, Comparator>::validate() {
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
}

template <typename T, typename Comparator>
void MergeHeap<T, Comparator>::dump() {
  cout << "MergeHeap: "; 
  for(TPIE_OS_OFFSET i = 0; i<m_size; i++) {
    cout << heap[i] << ", ";
  }
  cout << endl;
}
