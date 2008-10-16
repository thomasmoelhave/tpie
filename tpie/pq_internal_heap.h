#ifndef __TPIE_PQ_INTERNAL_HEAP_H__
#define __TPIE_PQ_INTERNAL_HEAP_H__

namespace tpie{

/////////////////////////////////////////////////////////
///
/// \class Heap
/// \author Lars Hvam Petersen
/// 
/// Standard binary internal heap.
///
/////////////////////////////////////////////////////////
	template <typename T, typename Comparator = std::less<T> >
	class pq_internal_heap {
	public:
		/////////////////////////////////////////////////////////
		///
		/// Constructor
		///
		/// \param max_size Maximum size of queue
		///
		/////////////////////////////////////////////////////////
		pq_internal_heap(unsigned int max_size) { 
			pq = new T[max_size]; 
			sz = 0; 
		}

		pq_internal_heap(T* arr, unsigned int length) {
			pq = arr;
			sz = length;
		}
  
		/////////////////////////////////////////////////////////
		///
		/// Destructor
		///
		/////////////////////////////////////////////////////////
		~pq_internal_heap() { delete[] pq; } 
  
		/////////////////////////////////////////////////////////
		///
		/// Return true if queue is empty otherwise false
		///
		/// \return Boolean - empty or not
		///
		/////////////////////////////////////////////////////////
		bool empty() { return sz == 0; }

		/////////////////////////////////////////////////////////
		///
		/// Returns the size of the queue
		///
		/// \return Queue size
		///
		/////////////////////////////////////////////////////////
		unsigned size() {
			return sz;
		}

		/////////////////////////////////////////////////////////
		///
		/// Insert an element into the heap 
		///
		/// \param v The element that should be inserted
		///
		/////////////////////////////////////////////////////////
		void insert(T v) { 
			pq[sz++] = v; 
			bubbleUp(sz-1);
		}

		/////////////////////////////////////////////////////////
		///
		/// Remove the minimal element from heap
		///
		/// \return Minimal element
		///
		/////////////////////////////////////////////////////////
		T delmin() { 
			swap(pq[0], pq[--sz]);
			bubbleDown(); 
			return pq[sz];
		}

		/////////////////////////////////////////////////////////
		///
		/// Peek the minimal element
		///
		/// \return Minimal element
		///
		/////////////////////////////////////////////////////////
		T peekmin() {return pq[0]; }
	
		/////////////////////////////////////////////////////////
		///
		/// Set the size 
		///
		/// \param ne Size
		///
		/////////////////////////////////////////////////////////
		void set_size(unsigned int ne) { sz = ne; }
	
		/////////////////////////////////////////////////////////
		///
		/// Returns a pointer to the underlaying array 
		///
		/// \return Array
		///
		/////////////////////////////////////////////////////////
		T* get_arr() { return pq; }
	
	private:
		Comparator comp_;

		inline int left_child(int k){return 2*k+1;}
		inline int right_child(int k){return 2*k+2;}
		inline int parent(int k){return (k-1)/2;}

		void bubbleDown() { 
			int k=0;
			int j;
			while((j=left_child(k)) < sz) {
				if(j < sz-1 && comp_(pq[j+1], pq[j])) j++; // compare, pq[j] > pq[j+1]
				if(! comp_(pq[j], pq[k]) ) break; // compare, pq[k] > pq[j]
				swap(pq[k], pq[j]); 
				k = j;
			}
		}
  
		void bubbleUp(int k) {
			int j;
			while(k > 0 && comp_(pq[k], pq[(j=parent(k))])) { // compare, pq[k/2] > pq[k]
				swap(pq[k], pq[j]);
				k = j; 
			}
		}
		
		T *pq; 
		unsigned int sz;
	};

}
#endif
