// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
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

///////////////////////////////////////////////////////////////////////////
/// \file mergeheap.h
/// This file contains several merge heap templates. 
/// Originally written by Rakesh Barve.  
///
/// Modified by David Hutchinson 2000 03 02
///
/// Modified by Jakob Truelsen 2011, to contain simple wrappers for the internal heap

#ifndef _MERGE_HEAP_H
#define _MERGE_HEAP_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
#include <tpie/memory.h>
#include <tpie/internal_priority_queue.h>

namespace tpie {
	namespace ami {
	
		///////////////////////////////////////////////////////////////////////////
		/// This is a record pointer element. Encapsulates the record pointer,
		/// along with the label run_id indicating the run the record
		/// originates from.
		///////////////////////////////////////////////////////////////////////////
		template<class REC>
		class heap_ptr {
		public:
			heap_ptr() : recptr(NULL), run_id(0) {};
			heap_ptr(REC * a, size_t b) : recptr(a), run_id(b) {};
			REC            *recptr;
			size_t run_id;
		};

		///////////////////////////////////////////////////////////////////////////
		/// A record pointer heap base class - also serves as the full
		/// implementation for objects with a < comparison operator.
		///////////////////////////////////////////////////////////////////////////
		template<class REC, class comp_t=std::less<REC> >
		class merge_heap_ptr_op {
		private:
			struct comp: public std::binary_function<heap_ptr<REC>, heap_ptr<REC>, bool> {
				comp_t c;
				comp(comp_t & _): c(_) {}
				inline bool operator()(const heap_ptr<REC> & a, const heap_ptr<REC> & b) {
					return c(*a.recptr, *b.recptr);
				}
			};
			
			internal_priority_queue<heap_ptr<REC>, comp> pq;
			
		public:
			merge_heap_ptr_op(comp_t c=comp_t()): pq(0, comp(c)) {}
			
			///////////////////////////////////////////////////////////////////////////
			/// Reports the size of Heap (number of elements).
			///////////////////////////////////////////////////////////////////////////
			size_t sizeofheap() {return pq.size();}
			
			///////////////////////////////////////////////////////////////////////////
			/// Returns the run with the minimum key.
			///////////////////////////////////////////////////////////////////////////
			inline size_t get_min_run_id() {return pq.top().run_id;}
			
			///////////////////////////////////////////////////////////////////////////
			/// Allocates space for the heap.
			///////////////////////////////////////////////////////////////////////////
			void allocate(size_t size) {pq.resize(size);}
			
			///////////////////////////////////////////////////////////////////////////
			/// Copies an (initial) element into the heap array.
			///////////////////////////////////////////////////////////////////////////
			void insert(REC *ptr, size_t run_id) {pq.unsafe_push(heap_ptr<REC>(ptr, run_id) );}
			
			///////////////////////////////////////////////////////////////////////////
			/// Extracts minimum element from heap array.
			/// If you follow this with an immediate insert, consider using
			/// delete_min_and_insert().
			///////////////////////////////////////////////////////////////////////////
			void extract_min(REC& el, size_t& run_id) {
				el=*pq.top().ptr;
				run_id=*pq.top().run_id();
				pq.pop();
			}
			
			///////////////////////////////////////////////////////////////////////////
			/// Deallocates the space used by the heap.
			///////////////////////////////////////////////////////////////////////////
			void deallocate() {pq.resize(0);}
			
			///////////////////////////////////////////////////////////////////////////
			/// Heapifies an initial array of elements;
			///////////////////////////////////////////////////////////////////////////
			void initialize() {pq.make_safe(); }
			
			///////////////////////////////////////////////////////////////////////////
			// Deletes the current minimum and inserts the new item from the same
			// source / run.
			///////////////////////////////////////////////////////////////////////////
			inline void delete_min_and_insert(REC *nextelement_same_run) {
				if (nextelement_same_run)
					pq.pop_and_push(heap_ptr<REC>(nextelement_same_run, pq.top().run_id));
				else
					pq.pop();
			}
			
			///////////////////////////////////////////////////////////////////////////
			/// Returns the main memory space usage per item.
			///////////////////////////////////////////////////////////////////////////
			inline size_t space_per_item() { return pq.memory_coefficient(); }
			
			///////////////////////////////////////////////////////////////////////////
			/// Returns the fixed main memory space overhead, regardless of item count.
			///////////////////////////////////////////////////////////////////////////
			inline size_t space_overhead() { return pq.memory_overhead(); }
		};
		
		template <class REC, class CMPR>
		struct COMP_WRAPPER {
			CMPR * cmpr;
			COMP_WRAPPER(CMPR * c): cmpr(c) {}
			bool operator()(const REC & a, const REC & b) {
				return cmpr->compare(a,b) < 0;
			}
		};
		
		///////////////////////////////////////////////////////////////////////////
		/// A record pointer heap that uses a comparison object
		///////////////////////////////////////////////////////////////////////////
		template<class REC, class CMPR>
		class merge_heap_ptr_obj: public merge_heap_ptr_op<REC, COMP_WRAPPER<REC, CMPR> >{
		public:
			merge_heap_ptr_obj(CMPR *cmptr): 
				merge_heap_ptr_op<REC, COMP_WRAPPER<REC, CMPR> >(COMP_WRAPPER<REC, CMPR>(cmptr)) {};
		};
		
		///////////////////////////////////////////////////////////////////////////
		/// This is a heap element. Encapsulates the key, along with
		/// the label run_id indicating the run the key originates from.
		///////////////////////////////////////////////////////////////////////////
		template<class KEY>
		class heap_element {
		public:
			heap_element() : key(), run_id(0) {}
			heap_element(KEY k, size_t r): key(k), run_id(r) {}

			KEY key;
			size_t run_id;
		};
		
		///////////////////////////////////////////////////////////////////////////
		/// A merge heap object base class - also serves as the full
		/// implementation for objects with a < comparison operator
		///////////////////////////////////////////////////////////////////////////
		template<class REC, class comp_t=std::less<REC> >
		class merge_heap_op {
		private:
			struct comp: public std::binary_function<heap_element<REC>, heap_element<REC>, bool> {
				comp_t c;
				comp(comp_t & _): c(_) {}
				inline bool operator()(const heap_element<REC> & a, const heap_element<REC> & b) {
					return c(a.key, b.key);
				}
			};
			
			internal_priority_queue<heap_element<REC>, comp> pq;
			
		public:
			merge_heap_op(comp_t c=comp_t()): pq(0, comp(c)) {}
			
			///////////////////////////////////////////////////////////////////////////
			/// Reports the  size of Heap (number of elements).
			///////////////////////////////////////////////////////////////////////////
			size_t sizeofheap() {return pq.size();}
			
			///////////////////////////////////////////////////////////////////////////
			/// Returns the run with the minimum key.
			///////////////////////////////////////////////////////////////////////////
			inline size_t get_min_run_id() {return pq.top().run_id;};
			
			///////////////////////////////////////////////////////////////////////////
			/// Allocates space for the heap.
			///////////////////////////////////////////////////////////////////////////
			void allocate(size_t size) {pq.resize(size);}
			
			///////////////////////////////////////////////////////////////////////////
			/// Copies an (initial) element into the heap array/
			///////////////////////////////////////////////////////////////////////////
			void insert(REC *ptr, size_t run_id) {pq.unsafe_push(heap_element<REC>(*ptr, run_id));}
			
			///////////////////////////////////////////////////////////////////////////
			/// Extracts minimum element from heap array.
			/// If you follow this with an immediate insert, consider using
			/// delete_min_and_insert().
			///////////////////////////////////////////////////////////////////////////
			void extract_min(REC& el, size_t& run_id) {
				el=pq.top().key;
				run_id=pq.top().run_id;
				pq.pop();
			}
			
			///////////////////////////////////////////////////////////////////////////
			/// Deallocates the space used by the heap.
			///////////////////////////////////////////////////////////////////////////
			void deallocate() {pq.resize(0);}
			
			///////////////////////////////////////////////////////////////////////////
			/// Heapifies an initial array of elements.
			///////////////////////////////////////////////////////////////////////////
			void initialize(void) {pq.make_safe();}
			
			///////////////////////////////////////////////////////////////////////////
			// Deletes the current minimum and inserts the new item from the same
			// source / run.
			///////////////////////////////////////////////////////////////////////////
			inline void delete_min_and_insert(REC *nextelement_same_run) {
				if (nextelement_same_run)
					pq.pop_and_push(heap_element<REC>(*nextelement_same_run, pq.top().run_id));
				else
					pq.pop();
			}
			
			///////////////////////////////////////////////////////////////////////////
			/// Returns the  main memory space usage per item
			///////////////////////////////////////////////////////////////////////////
			inline size_t space_per_item(void) {return pq.memory_coefficient();}
			
			///////////////////////////////////////////////////////////////////////////
			/// Returns the  fixed main memory space overhead, regardless of item count.
			///////////////////////////////////////////////////////////////////////////
			inline size_t space_overhead(void) {return pq.memory_overhead();}
		};
		
	
		// ********************************************************************
		// * A merge heap that uses a comparison object                       *
		// ********************************************************************
		template<class REC, class CMPR>
		class merge_heap_obj: public merge_heap_op<REC, COMP_WRAPPER<REC, CMPR> > {
		public:
			merge_heap_obj(CMPR * cmp): 
				merge_heap_op<REC, COMP_WRAPPER<REC, CMPR> >(COMP_WRAPPER<REC, CMPR>(cmp)) {}
		};
		

		// ********************************************************************
		// * A merge heap key-object base class                               *
		// * Also serves as a full impelementation of a                       *
		// * key-merge heap that uses a comparison operator <                 *
		// ********************************************************************
	
		// The merge_heap_kop object maintains only the keys in its heap,
		// and uses the member function "copy" of the user-provided class CMPR
		// to copy these keys from each record.
	
		template <class REC, class KEY, class CMPR, class comp_t=std::less<KEY> >
		class merge_heap_kop{
		protected:
			struct comp: public std::binary_function<heap_element<KEY>, heap_element<KEY>, bool> {
				comp_t c;
				comp(comp_t & _): c(_) {}
				inline bool operator()(const heap_element<KEY> & x, const heap_element<KEY> & y) {
					return c(x.key, y.key);
				}
			};
			
			internal_priority_queue<heap_element<KEY>, comp> pq;
			CMPR * obj;
		public:
			// Constructor/Destructor 
			merge_heap_kop(CMPR* o, comp_t c=comp_t()): pq(0, comp(c)), obj(o) {}
			
			// Report size of Heap (number of elements)
			size_t sizeofheap(void) {return pq.size();}
			
			// Return the run with the minimum key.
			inline size_t get_min_run_id(void) {return pq.top().run_id;};
			
			void allocate(size_t size) {pq.resize(size);}
			void insert(REC *ptr, size_t run_id) {
					KEY k;
					obj->copy(&k, *ptr);
					pq.pop_and_push(heap_element<KEY>(k, run_id));
			}
			void deallocate() {pq.resize(0);}
			
			// Delete the current minimum and insert the new item from the same
			// source / run.
			inline void delete_min_and_insert(REC *nextelement_same_run) {
				if (nextelement_same_run) {
					KEY k;
					obj->copy(&k, *nextelement_same_run);
					pq.pop_and_push(heap_element<KEY>(k, pq.top().run_id));
				} else 
					pq.pop();
			}
			
			// Return main memory space usage per item
			inline size_t space_per_item(void) {return pq.memory_coefficient();}
			
			// Return fixed main memory space overhead, regardless of item count
			inline size_t space_overhead() {return pq.memory_overhead();}
			
			// heapify's an initial array of elements
			void initialize() {pq.make_safe();}
		};
		
	
		// ********************************************************************
		// * A key-merge heap that uses a comparison object                   *
		// ********************************************************************
	
		// The merge_heap_kobj object maintains only the keys in its heap,
		// and uses the member function "copy" of the user-provided class CMPR
		// to copy these keys from each record. It uses the member function
		// "compare" of the user-provided class CMPR to determine the relative
		// order of two such keys in the sort order.
		
		template<class REC, class KEY, class CMPR>
		class merge_heap_kobj: public merge_heap_kop<REC,KEY,CMPR, COMP_WRAPPER<KEY, CMPR> >{
		public:
			merge_heap_kobj(CMPR * obj): merge_heap_kop<REC,KEY,CMPR, COMP_WRAPPER<KEY, CMPR> >(
				obj, COMP_WRAPPER<KEY, CMPR>(obj) ) {};
		};
			
	}   //  ami namespace
}  //  tpie namespace 

#endif // _MERGE_HEAP_H
