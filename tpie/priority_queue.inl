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

template<typename T, typename Comparator, typename OPQType>
priority_queue<T, Comparator, OPQType>::priority_queue(double f) { // constructor mem fraction
	assert(f<= 1.0 && f > 0);
	TPIE_OS_SIZE_T mm_avail = MM_manager.consecutive_memory_available();
	TP_LOG_DEBUG("priority_queue: Memory limit: " 
		<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(mm_avail/1024/1024) << "mb("
		<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(mm_avail) << "bytes)" << "\n");
	mm_avail = (TPIE_OS_SIZE_T)((double)mm_avail*f);
	init(mm_avail);
}

template<typename T, typename Comparator, typename OPQType>
priority_queue<T, Comparator, OPQType>::priority_queue(TPIE_OS_SIZE_T mm_avail) { // constructor absolute mem
	assert(mm_avail <= MM_manager.memory_limit() && mm_avail > 0);
	TP_LOG_DEBUG("priority_queue: Memory limit: " 
		<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(mm_avail/1024/1024) << "mb("
		<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(mm_avail) << "bytes)" << "\n");
	init(mm_avail);
}

template<typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::init(TPIE_OS_SIZE_T mm_avail) { // init 
	TP_LOG_DEBUG("m_for_queue: " 
		<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(mm_avail) << "\n");
	TP_LOG_DEBUG("memory before alloc: " 
		<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(MM_manager.memory_available()) << "b" << "\n");
	{
		//Calculate M
		setting_m = mm_avail/sizeof(T);
		//Get stream memory usage
		stream<T> tmp;
		TPIE_OS_SIZE_T usage;
		tmp.main_memory_usage(&usage, mem::STREAM_USAGE_MAXIMUM);

		TPIE_OS_SIZE_T alloc_overhead = MM_manager.space_overhead();


		//Compute overhead of the parameters
		const TPIE_OS_SIZE_T fanout_overhead = 2*sizeof(TPIE_OS_OFFSET)// group state
			+ (usage+sizeof(stream<T>*)+alloc_overhead) //temporary streams
			+ (sizeof(T)+sizeof(TPIE_OS_OFFSET)); //mergeheap
		const TPIE_OS_SIZE_T sq_fanout_overhead = 3*sizeof(TPIE_OS_OFFSET); //slot_state
		const TPIE_OS_SIZE_T heap_m_overhead = sizeof(T) //opg
			+ sizeof(T) //gbuffer0
			+ sizeof(T) //extra buffer for remove_group_buffer
			+ 2*sizeof(T); //mergebuffer
		const TPIE_OS_SIZE_T buffer_m_overhead = sizeof(T) + 2*sizeof(T); //buffer
		const TPIE_OS_SIZE_T extra_overhead = 2*(usage+sizeof(stream<T>*)+alloc_overhead) //temporary streams
			+ 2*(sizeof(T)+sizeof(TPIE_OS_OFFSET)); //mergeheap
		const TPIE_OS_SIZE_T additional_overhead = 16*1024; //Just leave a bit unused

		//Check that there is enough space for the simple overhead
		if(mm_avail < extra_overhead+additional_overhead){
			throw priority_queue_error("Not enough memory available for priority queue");
		}

		//Setup the fanout, heap_m and buffer_m
		mm_avail-=additional_overhead+extra_overhead; //Subtract the extra space used
		setting_mmark = (mm_avail/16)/buffer_m_overhead; //Set the buffer size
		mm_avail-=setting_mmark*buffer_m_overhead;
		setting_k = (mm_avail/2); 

		{
			//compute setting_k
			//some of these numbers get big which is the reason for all this
			//careful casting.
			TPIE_OS_OFFSET squared_tmp = static_cast<TPIE_OS_OFFSET>(fanout_overhead)*static_cast<TPIE_OS_OFFSET>(fanout_overhead); 
			squared_tmp += static_cast<TPIE_OS_OFFSET>(4*sq_fanout_overhead)*static_cast<TPIE_OS_OFFSET>(setting_k);
			long double dsquared_tmp = static_cast<long double>(squared_tmp);
			const TPIE_OS_SIZE_T root_discriminant = static_cast<TPIE_OS_SIZE_T>(std::floor(std::sqrt(dsquared_tmp)));

			const TPIE_OS_OFFSET nominator = root_discriminant-fanout_overhead;
			const TPIE_OS_OFFSET denominator = 2*sq_fanout_overhead;
			setting_k = static_cast<TPIE_OS_SIZE_T>(nominator/denominator); //Set fanout
		}

		mm_avail-=setting_k*heap_m_overhead+setting_k*setting_k*sq_fanout_overhead;
		setting_m = (mm_avail)/heap_m_overhead;

		//Check that minimum requirements on fanout and buffersizes are met
		const TPIE_OS_SIZE_T min_fanout=3;
		const TPIE_OS_SIZE_T min_heap_m=4;
		const TPIE_OS_SIZE_T min_buffer_m=2;
		if(setting_k<min_fanout || setting_m<min_heap_m || setting_mmark<min_buffer_m){
			TP_LOG_FATAL_ID("Priority Queue: Not enough memory. Increase allowed memory.");
			exit(-1);
		}

	}

	current_r = 0;
	m_size = 0; // total size of priority queue
	buffer_size = 0;
	buffer_start = 0;

	TP_LOG_DEBUG("priority_queue" << "\n" 
			<< "\tsetting_k: " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(setting_k) << "\n" 
			<< "\tsetting_mmark: " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(setting_mmark) << "\n" 
			<< "\tsetting_m: " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(setting_m) << "\n");

	assert(setting_k > 0);
	assert(current_r == 0);
	assert(setting_m > 0);
	assert(setting_mmark > 0);
	assert(setting_m > setting_mmark);
	if(setting_m < setting_mmark) {
		TP_LOG_FATAL_ID("wrong settings");
		exit(-1);
	}

	opq = new OPQType(setting_m);
	assert(OPQType::sorted_factor == 1);
	if(opq == NULL) throw std::bad_alloc();
	// state arrays contain: start + size
	slot_state = new TPIE_OS_OFFSET[setting_k*setting_k*3];
	if(slot_state == NULL) throw std::bad_alloc();
	group_state = new TPIE_OS_OFFSET[setting_k*2]; // R merge can max be k splitout
	if(group_state == NULL) throw std::bad_alloc();
	buffer = new T[setting_mmark];
	if(buffer == NULL) throw std::bad_alloc();
	gbuffer0 = new T[setting_m];
	if(gbuffer0 == NULL) throw std::bad_alloc();
	mergebuffer = new T[setting_m*2];

	// clear memory
	for(TPIE_OS_OFFSET i = 0; i<TPIE_OS_OFFSET(setting_k*setting_k); i++) {
		slot_state[i*3] = 0;
		slot_state[i*3+1] = 0;
		slot_state[i*3+2] = i;
	}
	slot_data_id = setting_k*setting_k+1;

	for(TPIE_OS_OFFSET i = 0; i< TPIE_OS_OFFSET(setting_k*2); i++) {
		group_state[i] = 0;
	}

	std::stringstream ss;
	ss << tempname::tpie_name("pq_data");
	datafiles = ss.str();
	TP_LOG_DEBUG("memory after alloc: " 
		<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(MM_manager.memory_available()) << "b" << "\n");
}

template <typename T, typename Comparator, typename OPQType>
priority_queue<T, Comparator, OPQType>::~priority_queue() { // destructor
	for(TPIE_OS_SIZE_T i = 0; i < setting_k*setting_k; i++) { // unlink slots
		TPIE_OS_UNLINK(slot_data(i));
	}
	for(TPIE_OS_SIZE_T i = 0; i < setting_k; i++) { // unlink groups 
		TPIE_OS_UNLINK(group_data(i));
	}

	delete opq;
	delete[] slot_state;
	delete[] group_state;
	delete[] buffer;
	delete[] gbuffer0;
	delete[] mergebuffer;
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::push(const T& x) {
	//cout << "push start" << "\n";
	if(opq->full()) {
		//cout << "----calling free" << "\n";
		TPIE_OS_SIZE_T slot = free_slot(0);
		//cout << "done calling free, free found: " << slot << "\n";
		assert(opq->sorted_size() == setting_m);
		T* arr = opq->sorted_array();
		if(buffer_size > 0) { // maintain heap invariant for buffer
			memcpy(&mergebuffer[0], &arr[0], sizeof(T)*opq->sorted_size());
			memcpy(&mergebuffer[opq->sorted_size()], &buffer[buffer_start], sizeof(T)*buffer_size);
			std::sort(mergebuffer, mergebuffer+(buffer_size+opq->sorted_size()), comp_);
			memcpy(&buffer[buffer_start], &mergebuffer[0], sizeof(T)*buffer_size);
			memcpy(&arr[0], &mergebuffer[buffer_size], sizeof(T)*opq->sorted_size());
		}
		if(group_size(0)> 0) { // maintain heap invariant for gbuffer0
			assert(group_size(0)+opq->sorted_size() <= setting_m*2);
			TPIE_OS_SIZE_T j = 0;
			for(TPIE_OS_OFFSET i = group_start(0); i < group_start(0)+group_size(0); i++) {
				mergebuffer[j] = gbuffer0[i%setting_m];
				++j;
			}
			memcpy(&mergebuffer[j], &arr[0], sizeof(T)*opq->sorted_size());
			std::sort(mergebuffer, mergebuffer+(group_size(0)+opq->sorted_size()), comp_);
			memcpy(&gbuffer0[0], &mergebuffer[0], static_cast<size_t>(sizeof(T)*group_size(0)));
			group_start_set(0,0);
			memcpy(&arr[0], &mergebuffer[group_size(0)], sizeof(T)*opq->sorted_size());
		}
		write_slot(slot, arr, opq->sorted_size());
		opq->sorted_pop();

	}
	opq->push(x);
	m_size++;
#ifndef NDEBUG
	validate();
#endif
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::pop() {
	//cout << "pop" << "\n";
	if(empty()) {
		throw priority_queue_error("pop() invoked on empty priority queue");
	}
	top();
	//cout << "return to pop from top" << "\n";
	if(min_in_buffer) {
		buffer_size--;
		buffer_start++;
		if(buffer_size == 0) {
			buffer_start = 0;
		}
	} else {
		opq->pop();
	}
	m_size--;
#ifndef NDEBUG
	validate();
#endif
	//cout << "end pop" << "\n";
}

template <typename T, typename Comparator, typename OPQType>
const T& priority_queue<T, Comparator, OPQType>::top() {
	//cout << "top of top" << "\n";
	//dump();
	if(buffer_size == 0 && TPIE_OS_OFFSET(opq->size()) != m_size) {
		//cout << "fill min buffer" << "\n";
		fill_buffer();
		//cout << "done fill min buffer" << "\n";
	}
	//cout << "buffer filled" << "\n";
	if(buffer_size == 0 && opq->size() == 0) {
		throw priority_queue_error("top() invoked on empty priority queue");
	} else if(opq->size() == 0) {
		min=buffer[buffer_start];
		min_in_buffer = true;
	} else if(buffer_size == 0) {
		min=opq->top();
		min_in_buffer = false;
	} else if(comp_(buffer[buffer_start], opq->top())) { // compare
		min=buffer[buffer_start];
		min_in_buffer = true;
	} else {
		min=opq->top();
		min_in_buffer = false;
	}
	//cout << "min in buffer: " << min_in_buffer << "\n";
	//cout << "end of top" << "\n";
#ifndef NDEBUG
	validate();
#endif
	return min;
}

template <typename T, typename Comparator, typename OPQType>
TPIE_OS_OFFSET priority_queue<T, Comparator, OPQType>::size() const {
	return m_size;
}

template <typename T, typename Comparator, typename OPQType>
bool priority_queue<T, Comparator, OPQType>::empty() const {
	return m_size == 0;
}

template <typename T, typename Comparator, typename OPQType> template <typename F>
F priority_queue<T, Comparator, OPQType>::pop_equals(F f) {
	T a = top();
	f(a);
	pop();
	if(size() == 0) return f;
	T b = top();
	while(!(comp_(a, b))) { // compare
		f(b);
		pop();
		if(size() == 0) return f;
		b = top();
	}
	return f;
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::dump() {
	TP_LOG_DEBUG( "--------------------------------------------------------------" << "\n"
			<< "DUMP:\tTotal size: " 
			<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(m_size) << ", OPQ size: " 
			<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(opq->size()) 
			<< ", OPQ top: ");
	if(opq->size()>0) {
		TP_LOG_DEBUG("" << opq->top());
	} else {
		TP_LOG_DEBUG("empty");
	}
	TP_LOG_DEBUG(", current_r: " 
			<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(current_r) << "\n"
			<< "\tBuffer size: " 
			<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(buffer_size) 
			<< ", buffer start: " 
			<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(buffer_start) 
			<< "\n" << "\t");

	// output main buffer
	for(TPIE_OS_OFFSET i = 0; i<setting_mmark; i++) {
		TP_LOG_DEBUG((i<buffer_start || buffer_start+buffer_size <=i ?"(":"") 
				<< buffer[i] 
				<< (i<buffer_start || buffer_start+buffer_size <=i ?")":"") 
				<< " ");
	}
	TP_LOG_DEBUG("\n");

	// output groups
	for(TPIE_OS_OFFSET i =0; i<current_r; i++) {
		TP_LOG_DEBUG("GROUP " << i << " ------------------------------------------------------" << "\n");
		TP_LOG_DEBUG("\tGroup Buffer, size: " 
				<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(group_size(i)) << ", start: " 
				<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(group_start(i)) << "\n" << "\t\tBuffer(no ('s): ");

		if(i == 0) { // group buffer 0 is special
			TP_LOG_DEBUG("internal: ");
			TPIE_OS_OFFSET k = 0;
			for(k = 0; k < setting_m; k++) {
				TP_LOG_DEBUG(gbuffer0[k] << " ");
			} 
			TP_LOG_DEBUG("\n");
		} else {  
			// output group buffer contents
			stream<T>* instream = new stream<T>(group_data(i));
			TPIE_OS_OFFSET k = 0;
			if(group_size(i) > 0) {
				for(k = 0; k < setting_m; k++) {
					TP_LOG_DEBUG(*read_item(instream) << " ");
				} 
			}
			delete instream;
			for(TPIE_OS_OFFSET l = k; l < setting_m; l++) {
				TP_LOG_DEBUG("() ");
			}
			TP_LOG_DEBUG("\n");
		}

		// output slots
		for(TPIE_OS_OFFSET j = i*setting_k; j<i*setting_k+setting_k; j++) {
			TP_LOG_DEBUG("\t\tSlot " << j << "(size: " 
					<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(slot_size(j)) 
					<< " start: " << slot_start(j) << "):");

			stream<T>* instream = new stream<T>(slot_data(j));
			TPIE_OS_OFFSET k;
			for(k = 0; k < slot_start(j)+slot_size(j); k++) {
				TP_LOG_DEBUG((k>=slot_start(j)?"":"(") << 
						*read_item(instream)<< 
						(k>=slot_start(j)?"":")") << " ");
			}
			delete instream;
			for(TPIE_OS_OFFSET l = k; l < slot_max_size(j); l++) {
				TP_LOG_DEBUG("() ");
			}

			TP_LOG_DEBUG("\n");
		} 
	}
	TP_LOG_DEBUG("--------------------------------------------------------------\n");
}

/////////////////////////////
// Private
/////////////////////////////

template <typename T, typename Comparator, typename OPQType>
TPIE_OS_SIZE_T priority_queue<T, Comparator, OPQType>::free_slot(TPIE_OS_SIZE_T group) {
	//cout << "free slot group " << group << "?" << "\n";
	TPIE_OS_SIZE_T i;
	if(group>=setting_k) {
		TP_LOG_FATAL_ID("Error, queue is full no free slots in invalid group " 
			<< static_cast<TPIE_OS_OUTPUT_SIZE_T>(group) << ". Increase k.");
		exit(-1);
	}
	for(i = group*setting_k; i < group*setting_k+setting_k; i++) {
		if(slot_size(i) == 0) {
			//cout << "Slot " << i << " is good" << "\n";
			break;
		}
	}

	// it all goes to next level, wee
	if(i == group*setting_k+setting_k) {
		//cout << "empty group from free slot" << "\n";
		empty_group(group);
		if(slot_size(group*setting_k) != 0) {
			//cout << "again" << "\n"; 
			return free_slot(group); // some group buffers might have been moved
		}
		return group*setting_k;
	}
	return i;
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::fill_buffer() {
	//cout << "fill buffer" << "\n";
	//dump();
	if(buffer_size !=0) {
		return;
	}
	if(current_r == 0) { // todo: check that this is ok
		return;
	}

	// refill group buffers, if needed
	//cout << "refill group buffers" << "\n";
	for(TPIE_OS_SIZE_T i=0;i<current_r;i++) {
		if(group_size(i)<TPIE_OS_OFFSET(setting_mmark)) {
			//cout << "fill group buffer " << i << "\n";
			fill_group_buffer(i);
			//cout << "fill group buffer " << i << " done" << "\n";
			//dump();
		}
		if(group_size(i) == 0 && i==current_r-1) {
			current_r--;
		}
	}

	//cout << "done filling groups" << "\n";
	// merge to buffer
	//cout << "current_r: " << current_r << "\n";
	delete[] mergebuffer;
	mergebuffer=NULL;

	pq_merge_heap<T, Comparator> heap(current_r);
	stream<T>** data = new stream<T>*[current_r];
	for(TPIE_OS_SIZE_T i = 0; i<current_r; i++) {
		if(i == 0 && group_size(i)>0) {
			heap.push(gbuffer0[group_start(0)], 0);
		} else if(group_size(i)>0) {
			data[i] = new stream<T>(group_data(i));
			//      assert(slot_size(group*setting_k+i>0));
			seek_offset(data[i], group_start(i));
			heap.push(*read_item(data[i]), i);
		} else if(i > 0) {
			// dummy, well :o/
			//cout << "create dummy " << i << "\n";
			data[i] = new stream<T>();
			data[i]->persist(PERSIST_DELETE);
		}
	}
	//cout << "init done" << "\n";

	while(!heap.empty() && buffer_size!=setting_mmark) {
		TPIE_OS_SIZE_T current_group = heap.top_run();
		if(current_group!= 0 && data[current_group]->tell() == TPIE_OS_OFFSET(setting_m)) {
			//cout << "fill group seeking to 0" << "\n";
			seek_offset(data[current_group], 0);
		}
		buffer[(buffer_size+buffer_start)%setting_m] = heap.top();
		buffer_size++;

		assert(group_size(current_group)-1 >= 0);
		group_size_set(current_group, group_size(current_group)-1);
		group_start_set(current_group, (group_start(current_group)+1)%setting_m);
		if(group_size(current_group) == 0) {
			heap.pop();
		} else {
			if(current_group == 0) {
				//cout << "0 push: " << "\n";
				//cout << gbuffer0[group_start(0)] << "\n";
				heap.pop_and_push(gbuffer0[group_start(0)], 0);
			} else {
				heap.pop_and_push(*read_item(data[current_group]), current_group);
			}
		}
	}
	//cout << "while done" << "\n";
	assert(mergebuffer==NULL);
	mergebuffer = new T[setting_m*2];

	for(TPIE_OS_SIZE_T i = 1; i<current_r; i++) {
		delete data[i];
	}

	delete[] data;
	//cout << "end fill buffer" << "\n";
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::fill_group_buffer(TPIE_OS_SIZE_T group) {
	assert(group_size(group) < TPIE_OS_OFFSET(setting_mmark));
	// max k + 1 open streams
	// 1 merge heap
	// opq still in action

	// merge
	{

		//group output stream, not used if group==0 in this case 
		//the in-memory gbuffer0 is used
		stream<T> out(group_data(group));
		if(group > 0) {
			if((err = out.seek((group_start(group)+group_size(group))%setting_m))!= NO_ERROR) {
				TP_LOG_FATAL_ID("AMI error " << err << " while seeking node");
				exit(-1);
			}
		}

		//get rid of mergebuffer so that we enough memory
		//for the heap and misc structures below
		//this array is reallocated below
		delete[] mergebuffer;
		mergebuffer=NULL;

		//merge heap for the setting_k slots
		pq_merge_heap<T, Comparator> heap(setting_k);

		//Create streams for the non-empty slots and initialize
		//internal heap with one element per slot
		stream<T>** data = new stream<T>*[setting_k];
		for(TPIE_OS_SIZE_T i = 0; i<setting_k; i++) {

			if(slot_size(group*setting_k+i)>0) {
				//slot is non-empry, opening stream
				TPIE_OS_SIZE_T slotid = group*setting_k+i;
				data[i] = new stream<T>(slot_data(slotid));

				//seek to start of slot
				seek_offset(data[i], slot_start(slotid));

				//push first item of slot on the stream
				heap.push(*read_item(data[i]), slotid);
			} else {
				data[i] = NULL;
			}
		}

		//perform actual reading until group if full or all 
		//the slots are empty
		while(!heap.empty() && group_size(group)!=TPIE_OS_OFFSET(setting_m)) {
			TPIE_OS_SIZE_T current_slot = heap.top_run();

			if(group == 0) {
				//use in-memory array for group 0
				gbuffer0[(group_start(0)+group_size(0))%setting_m] = heap.top();
			} else {
				//write to disk for group >0
				if(out.tell() == TPIE_OS_OFFSET(setting_m)) {
					out.seek(0);
				}

				if((err = out.write_item(heap.top())) != NO_ERROR) {
					TP_LOG_FATAL_ID("AMI error while reading item, code: " << err);
					exit(-1);
				}
			}

			//increase group size
			group_size_set(group, group_size(group) + 1);

			//decrease slot size and increase starting index
			slot_start_set(current_slot, slot_start(current_slot)+1);
			slot_size_set(current_slot, slot_size(current_slot)-1);

			//pop from heap and insert next element (if any) from the slot
			if(slot_size(current_slot) == 0) {
				heap.pop();
			} else {
				heap.pop_and_push(*read_item(data[current_slot-group*setting_k]), current_slot);
			}
		}

		//cleanup
		for(TPIE_OS_SIZE_T i = 0; i<setting_k; i++) {
			// deleting a NULL pointer is safe
			delete data[i];
		}
		delete[] data;
	}

	//restore mergebuffer
	assert(mergebuffer==NULL);
	mergebuffer = new T[setting_m*2];

	// compact if needed
	/*  for(TPIE_OS_OFFSET i=group*setting_k;i<group*setting_k+setting_k; i++) {
		if(slot_size(i) <= slot_max_size(i)/2 && slot_size(i) >  0) {
	//cout << "compact from fill group buffer slot: " << i << "\n";
	compact(i);
	}
	}
	*/
}


template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::compact(TPIE_OS_SIZE_T slot1) {
	//  std::cout << "compact slot " << slot1 << "\n";
	assert(slot_size(slot1) > 0);

	for(TPIE_OS_OFFSET i = (slot1/setting_k)*setting_k; i < (slot1/setting_k)*setting_k + setting_k; i++) {
		if(i != slot1) {
			if(slot_size(i) > 0 && slot_size(i) + slot_size(slot1) <= slot_max_size(slot1)) {
				TPIE_OS_OFFSET slot2 = i;
				//cout << "compacting slot " << slot1 << " with " << slot2 << "\n";

				stream<T>* stream1 = new stream<T>(slot_data(slot1));
				seek_offset(stream1, slot_start(slot1));
				T e1 = *read_item(stream1);
				TPIE_OS_OFFSET used1 = 0;
				stream<T>* stream2 = new stream<T>(slot_data(slot2));
				seek_offset(stream2, slot_start(slot2));
				T e2 = *read_item(stream2);
				TPIE_OS_OFFSET used2 = 0;

				TPIE_OS_OFFSET new_data_id = slot_data_id++;
				stream<T>* out = new stream<T>(datafile(new_data_id));

				while(used1 + used2 < slot_size(slot1) + slot_size(slot2)) {
					if(used1 == slot_size(slot1)) { // rest from slot2
						write_item(out, e2);
						used2++;
						if(used2 < slot_size(slot2)) e2 = *read_item(stream2);
					} else if(used2 == slot_size(slot2)) { // rest from slot1
						write_item(out, e1);
						used1++;
						if(used1 < slot_size(slot1)) e1 = *read_item(stream1);
					} else if(comp_(e1, e2)) { // compare - 10/1-07
						write_item(out, e1);
						used1++;
						if(used1 < slot_size(slot1)) e1 = *read_item(stream1);
					} else {
						write_item(out, e2);
						used2++;
						if(used2 < slot_size(slot2)) e2 = *read_item(stream2);
					}
				}

				delete stream1;
				delete stream2;

				slot_start_set(slot1, 0);
				TPIE_OS_UNLINK(slot_data(slot1));
				slot_data_set(slot1, new_data_id);
				slot_size_set(slot1, slot_size(slot1) + slot_size(slot2));
				slot_size_set(slot2, 0);
				slot_start_set(slot2, 0);

				delete out;
				return;
			}
		}
	}
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::empty_group(TPIE_OS_SIZE_T group) {
	//cout << "Empty group " << group << "\n";
	if(group > setting_k) {
		TP_LOG_FATAL_ID("Error: Priority queue is full");
		exit(-1);
	}

	TPIE_OS_SIZE_T newslot = free_slot(group+1);

	assert(slot_size(newslot) == 0);
	slot_start_set(newslot, 0);
	if(current_r < newslot/setting_k+1) {
		//cout << "increasing current_r, empty_group" << "\n"; 
		current_r = newslot/setting_k+1;
	}

	bool ret = false;

	delete[] mergebuffer;
	mergebuffer=NULL;

	stream<T>* newstream = new stream<T>(slot_data(newslot));
	pq_merge_heap<T, Comparator> heap(setting_k);

	stream<T>** data = new stream<T>*[setting_k];
	for(TPIE_OS_SIZE_T i = 0; i<setting_k; i++) {
		data[i] = new stream<T>(slot_data(group*setting_k+i));
		if(slot_size(group*setting_k+i) == 0) {
			//      std::cout << "no need to emtpy group "<<group<<", slot: " << group*setting_k+i << " is empty" << "\n";
			ret = true;
			break;
		}
		assert(slot_size(group*setting_k+i)>0);
		seek_offset(data[i], slot_start(group*setting_k+i));
		heap.push(*read_item(data[i]), group*setting_k+i);
	}
	//cout << "init done" << "\n";

	while(!heap.empty() && !ret) {
		TPIE_OS_SIZE_T current_slot = heap.top_run();
		write_item(newstream, heap.top());
		slot_size_set(newslot,slot_size(newslot)+1);
		//cout << heap.top() << " from slot " << current_slot << "\n";
		slot_start_set(current_slot, slot_start(current_slot)+1);
		slot_size_set(current_slot, slot_size(current_slot)-1);
		if(slot_size(current_slot) == 0) {
			heap.pop();
		} else {
			heap.pop_and_push(*read_item(data[current_slot-group*setting_k]), current_slot);
		}
	}

	//cout << "start delete" << "\n";
	for(TPIE_OS_SIZE_T i = 0; i<setting_k; i++) {
		delete data[i];
	}

	delete[] data;
	//cout << "end delete" << "\n";

	delete newstream;

	assert(mergebuffer==NULL);
	mergebuffer = new T[setting_m*2];

	if(group_size(group+1) > 0 && !ret) {
		remove_group_buffer(group+1); // todo, this might recurse?
	}
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::validate() {
#ifndef NDEBUG
#ifdef PQ_VALIDATE
	cout << "validate start" << "\n";
	// validate size
	TPIE_OS_OFFSET size = 0;
	size = size + opq->size();
	size = size + buffer_size;
	for(TPIE_OS_OFFSET i = 0; i<setting_k;i++) {
		size = size + group_size(i);
	}
	for(TPIE_OS_OFFSET i = 0; i<setting_k*setting_k;i++) {
		size = size + slot_size(i);
	}
	if(m_size != size) {
		TP_LOG_FATAL_ID("Error: Validate: Size not ok");
		exit(-1);
	}

	// validate internal order in "nodes"
	if(buffer_size > 0) { // buffer
		T last = buffer[buffer_start];
		for(TPIE_OS_OFFSET i = buffer_start; i<buffer_start+buffer_size;i++) {
			if(comp_(buffer[i],last)) {
				//Kasper and Mark: Should this exit(-1)?
				TP_LOG_WARNING_ID("Error: Buffer ordered validation failed");
			}
			last = buffer[i];
		}
	}
	// todo: validate gbuffer0
	for(TPIE_OS_OFFSET i = 1; i < setting_k; i++) { // groups, nb: cyclic
		if(group_size(i) > 0) {
			stream<T>* stream = new stream<T>(group_data(i));
			seek_offset(stream, group_start(i));
			if(stream->tell() == setting_m) {
				seek_offset(stream, 0);
			}
			T last = *read_item(stream);
			for(TPIE_OS_OFFSET j = 1; j < group_size(i); j++) {
				if(stream->tell() == setting_m) {
					seek_offset(stream, 0);
				}
				T read = *read_item(stream);
				if(comp_(read, last)) { // compare
					dump();
					TP_LOG_FATAL_ID("Error: Group buffer " << i << " order invalid (last: " << last << ", read: " << read << ")");
					exit(-1);
				} 
			}
			delete stream;
		}
	}
	for(TPIE_OS_OFFSET i = 0; i < setting_k*setting_k; i++) { // slots
		if(slot_size(i) > 0){
			stream<T>* stream = new stream<T>(slot_data(i));
			seek_offset(stream, slot_start(i));
			T last = *read_item(stream);
			for(TPIE_OS_OFFSET j = 1; j < slot_size(i); j++) {
				T read = *read_item(stream);
				if(comp_(read, last)) { // compare
					TP_LOG_FATAL_ID("Error: Slot " << i << " order invalid (last: " << last << ", read: " << read << ")");
					exit(-1);
				}
			}
			delete stream;
		}
	}

	// validate heap properties
	if(buffer_size > 0) { // buffer --> group buffers
		T buf_max = buffer[buffer_start+buffer_size-1];
		for(TPIE_OS_OFFSET i = 1; i < setting_k; i++) { // todo: gbuffer0
			if(group_size(i) > 0) {
				stream<T>* stream = new stream<T>(group_data(i));
				seek_offset(stream, group_start(i));
				if(stream->tell() == setting_m) {
					seek_offset(stream, 0);
				}
				T first = *read_item(stream);
				if(comp_(first, buf_max)) { // compare
					dump();
					TP_LOG_FATAL_ID("Error: Heap property invalid, buffer -> group buffer " << i << "(buffer: " << buf_max << ", first: " << first << ")");
					exit(-1);
				}
				delete stream;
			}
		}
	}

	// todo: gbuffer0
	for(TPIE_OS_OFFSET i = 1; i < setting_k; i++) { // group buffers --> slots
		if(group_size(i) > 0) {
			stream<T>* stream = new stream<T>(group_data(i));
			seek_offset(stream, (group_start(i)+group_size(i)-1)%setting_m);
			T item_group = *read_item(stream);
			//cout << "item_group: " << item_group << "\n";
			delete stream;

			for(TPIE_OS_OFFSET j = i*setting_k; j<i*setting_k+setting_k;j++) {
				if(slot_size(j) > 0) {
					stream<T>* stream = new stream<T>(slot_data(j));
					seek_offset(stream, slot_start(j));
					T item_slot = *read_item(stream);
					delete stream;

					if(comp_(item_slot, item_group)) { // compare
						dump();
						TP_LOG_FATAL_ID("Error: Heap property invalid, group buffer " << i << " -> slot " << j << "(group: " << item_group << ", slot: " << item_slot << ")");
						exit(-1);
					}
				}
			}
		}
	}
	//cout << "validate end" << "\n";
#endif
#endif
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::remove_group_buffer(TPIE_OS_SIZE_T group) {
	//cout << "remove group buffer " << group << "\n";
#ifndef NDEBUG
	if(group == 0) {
		TP_LOG_FATAL_ID("remove group buffer 0, fy!");
		exit(-1);
	}
#endif

	// this is the easiest thing to do
	TPIE_OS_SIZE_T slot = free_slot(0);
	if(group_size(group) == 0) return;

	TP_LOG_DEBUG_ID("Remove group buffer " << group << " of size " << group_size(group) << " with available memory " << MM_manager.memory_available());

	assert(group < setting_k);
	T* arr = new T[static_cast<size_t>(group_size(group))];
	stream<T>* data = new stream<T>(group_data(group));
	seek_offset(data, group_start(group));
	TPIE_OS_OFFSET size = group_size(group);
	if(group_start(group) + group_size(group) <= TPIE_OS_OFFSET(setting_m)) {
		if((err = data->read_array(arr, &size)) != NO_ERROR) {
			TP_LOG_FATAL_ID("AMI_ERROR " << err << " during read_array()");
			exit(1);
		}
	} else {
		// two reads
		TPIE_OS_OFFSET first_read = setting_m - data->tell();
		TPIE_OS_OFFSET second_read = size - first_read;
		//cout << "read array " << first_read << " " << second_read << "\n";
		if((err = data->read_array(arr, &first_read)) != NO_ERROR) {
			TP_LOG_FATAL_ID("AMI_ERROR " << err << " during read_array()");
			exit(1);
		}
		seek_offset(data,0);
		if((err = data->read_array(arr+first_read, &second_read)) != NO_ERROR) {
			TP_LOG_FATAL_ID("AMI_ERROR " << err << " during read_array()");
			exit(1);
		}
	}
	delete data;
	assert(group_size(group) > 0);
	/* 
	   for(TPIE_OS_OFFSET i = 0; i < group_size(group); i++) {
	   std::cout << "arr[" << i << "]: " << arr[i] << "\n";
	   }
	   for(TPIE_OS_OFFSET i = group_start(0); i < group_start(0)+group_size(0); i++) {
	   std::cout << "gbuffer0[" << (i%setting_m) << "]: " << gbuffer0[i%setting_m] << "\n";
	   }
	   */
	// make sure that the new slot in group 0 is heap ordered with gbuffer0
	if(group > 0 && group_size(0) != 0) {
		// this code is also used in PQFishspear
		TPIE_OS_OFFSET j = 0;
		for(TPIE_OS_OFFSET i = group_start(0); i < group_start(0)+group_size(0); i++) {
			mergebuffer[j] = gbuffer0[i%setting_m];
			++j;
		}
		memcpy(&mergebuffer[j], &arr[0], static_cast<size_t>(sizeof(T)*group_size(group)));
		std::sort(mergebuffer, mergebuffer+(group_size(0)+group_size(group)), comp_);
		memcpy(&gbuffer0[0], &mergebuffer[0], static_cast<size_t>(sizeof(T)*group_size(0)));
		group_start_set(0,0);
		memcpy(&arr[0], &mergebuffer[group_size(0)], static_cast<size_t>(sizeof(T)*group_size(group)));
	}

	/*
	   for(TPIE_OS_OFFSET i = 0; i < group_size(group); i++) {
	   std::cout << "arr[" << i << "]: " << arr[i] << "\n";
	   }
	   for(TPIE_OS_OFFSET i = group_start(0); i < group_start(0)+group_size(0); i++) {
	   std::cout << "gbuffer0[" << (i%setting_m) << "]: " << gbuffer0[i%setting_m] << "\n";
	   }
	   */

	write_slot(slot, arr, group_size(group));
	group_start_set(group, 0);
	group_size_set(group, 0);
	//cout << "compact from remove_group_buffer" << "\n";
	//  compact(slot);
	delete[] arr;
	//cout << "this dump" << "\n";
	//dump();
	//  std::cout << "remove grp buffer done" << "\n";
}

//////////////////
// TPIE wrappers
template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::seek_offset(stream<T>* data, TPIE_OS_OFFSET offset) {
	if((err = data->seek(offset))!= NO_ERROR) {
		TP_LOG_FATAL_ID("AMI_ERROR " << err << " while seeking node");
		exit(-1);
	}
}
template <typename T, typename Comparator, typename OPQType>
T* priority_queue<T, Comparator, OPQType>::read_item(stream<T>* data) { 
	T* read_ptr;
	if((err = data->read_item(&read_ptr)) != NO_ERROR) {
		TP_LOG_FATAL_ID("AMI error while reading item, code: " << err); 
		exit(-1);
	}
	return read_ptr;
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::write_item(stream<T>* data, T write) { 
	if((err = data->write_item(write)) != NO_ERROR) {
		TP_LOG_FATAL_ID("AMI error while reading item, code: " << err);
		exit(-1);
	}
}
// end TPIE wrappers

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::slot_start_set(TPIE_OS_SIZE_T slot, TPIE_OS_OFFSET n) {
	slot_state[slot*3] = n;
}

template <typename T, typename Comparator, typename OPQType>
TPIE_OS_OFFSET priority_queue<T, Comparator, OPQType>::slot_start(TPIE_OS_SIZE_T slot) const {
	return slot_state[slot*3];
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::slot_size_set(TPIE_OS_SIZE_T slot, TPIE_OS_OFFSET n) {
	//cout << "change slot " << slot << " size" << "\n";
	assert(slot<setting_k*setting_k);
	slot_state[slot*3+1] = n;
}

template <typename T, typename Comparator, typename OPQType>
TPIE_OS_OFFSET priority_queue<T, Comparator, OPQType>::slot_size(TPIE_OS_SIZE_T slot) const {
	return slot_state[slot*3+1];
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::group_start_set(TPIE_OS_SIZE_T group, TPIE_OS_OFFSET n) {
	group_state[group*2] = n;
}

template <typename T, typename Comparator, typename OPQType>
TPIE_OS_OFFSET priority_queue<T, Comparator, OPQType>::group_start(TPIE_OS_SIZE_T group) const {
	return group_state[group*2];
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::group_size_set(TPIE_OS_SIZE_T group, TPIE_OS_OFFSET n) {
	assert(group<setting_k);
	group_state[group*2+1] = n;
}

template <typename T, typename Comparator, typename OPQType>
TPIE_OS_OFFSET priority_queue<T, Comparator, OPQType>::group_size(TPIE_OS_SIZE_T group) const {
	return group_state[group*2+1];
}

	template <typename T, typename Comparator, typename OPQType>
const std::string& priority_queue<T, Comparator, OPQType>::datafile(TPIE_OS_OFFSET id) 
{
	std::stringstream ss;
	ss << datafiles << id;
	filename = ss.str();
	return filename;
}

	template <typename T, typename Comparator, typename OPQType>
const std::string& priority_queue<T, Comparator, OPQType>::datafile_group(TPIE_OS_OFFSET id) 
{
	std::stringstream ss;
	ss << datafiles << "g" <<id;
	filename = ss.str();
	return filename;
}

template <typename T, typename Comparator, typename OPQType>
const std::string& priority_queue<T, Comparator, OPQType>::slot_data(TPIE_OS_SIZE_T slotid) {
	return datafile(slot_state[slotid*3+2]);
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::slot_data_set(TPIE_OS_SIZE_T slotid, TPIE_OS_OFFSET n) {
	slot_state[slotid*3+2] = n;
}

template <typename T, typename Comparator, typename OPQType>
const std::string& priority_queue<T, Comparator, OPQType>::group_data(TPIE_OS_SIZE_T groupid) {
	return datafile_group(groupid);
}

template <typename T, typename Comparator, typename OPQType>
TPIE_OS_OFFSET priority_queue<T, Comparator, OPQType>::slot_max_size(TPIE_OS_SIZE_T slotid) {
	return setting_m*(TPIE_OS_OFFSET)(pow((long double)setting_k,(long double)(slotid/setting_k))); // todo, too many casts
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::write_slot(TPIE_OS_SIZE_T slotid, T* arr, TPIE_OS_OFFSET len) {
	assert(len > 0);
	//cout << "write slot " << slotid << " " << len << "\n";
	//cout << "write slot " << slot_data(slotid) << "\n";
	stream<T>* data = new stream<T>(slot_data(slotid));
	//cout << "write slot new done" << "\n";
	if((err = data->write_array(arr, len)) != NO_ERROR) {
		TP_LOG_FATAL_ID("AMI_ERROR " << err << " during write_slot()");
		exit(1);
	}
	delete data;
	slot_start_set(slotid, 0);
	slot_size_set(slotid, len);
	if(current_r == 0 && slotid < setting_k) {
		current_r = 1;
	}
	//cout << "write slot done" << std::endl;
}

/////////////////////
