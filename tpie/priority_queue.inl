template<typename T, typename Comparator, typename OPQType>
priority_queue<T, Comparator, OPQType>::priority_queue(double f) { // constructor 

    TPIE_OS_SIZE_T mm_avail = MM_manager.memory_limit();
    TP_LOG_DEBUG("priority_queue: Memory limit: " << mm_avail/1024/1024 << "mb("<< mm_avail << "bytes)" << "\n");
    mm_avail = (TPIE_OS_SIZE_T)((double)mm_avail*f);
    TP_LOG_DEBUG("m_for_queue: " << mm_avail << "\n");
    TP_LOG_DEBUG("memory before alloc: " << MM_manager.memory_available() << "b" << "\n");
    {
	TPIE_OS_OFFSET max_k = 500;
	stream<T> tmp;
	TPIE_OS_SIZE_T usage;
	tmp.main_memory_usage(&usage, MM_STREAM_USAGE_MAXIMUM);
	TPIE_OS_OFFSET need = max_k*max_k*3*sizeof(TPIE_OS_OFFSET) // slot states
	    + max_k*2*sizeof(TPIE_OS_OFFSET) // group states
	    + 4*usage // aux streams
	    + 1000;
	if(mm_avail < need) {
	    TP_LOG_FATAL_ID("Priority Queue: Not enough memory available. Increase allowed memory.");
	    exit(-1);
	}

	setting_m = mm_avail/sizeof(T);

	//  1/8m+ 4m = total
	// m = total/(1/8+4)
	setting_m = (setting_m*8)/33; // opq, gbuffer0, and mergebuffer(2*m)
	setting_mmark = (setting_m/8)/sizeof(T);
	// one block from each stream plus one element from each: use merge buffer ram
	setting_k = std::min((TPIE_OS_SIZE_T)max_k,(int)((setting_m*2/sizeof(T))-sizeof(T)*max_k)/((usage/sizeof(T))+sizeof(T)));
	//cout << "max calc k:" << (int)((setting_m*2/sizeof(T))-sizeof(T)*max_k)/((usage/sizeof(T))+sizeof(T)) << "\n";
	if(setting_k <= 2) {
	    TP_LOG_FATAL_ID( "Priority Queue: Fanout too small.");
	    exit(-1);
	}
	TP_LOG_DEBUG("stream usage(bytes): " << usage << "\n");
    }

    current_r = 0;
    m_size = 0; // total size of priority queue
    buffer_size = 0;
    buffer_start = 0;

    TP_LOG_DEBUG("priority_queue" << "\n" 
		 << "\tsetting_k: " << setting_k << "\n" 
		 << "\tsetting_mmark: " << setting_mmark << "\n" 
		 << "\tsetting_m: " << setting_m << "\n");

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
    if(mergebuffer == NULL) throw std::bad_alloc();

    // clear memory
    for(TPIE_OS_OFFSET i = 0; i<setting_k*setting_k; i++) {
	slot_state[i*3] = 0;
	slot_state[i*3+1] = 0;
	slot_state[i*3+2] = i;
    }
    slot_data_id = setting_k*setting_k+1;

    for(TPIE_OS_OFFSET i = 0; i< setting_k*2; i++) {
	group_state[i] = 0;
    }

    // calculate maximum size of queue for debugging purposes
    if(setting_k < 5) { // the maxsize variable easily overflows...
	TPIE_OS_OFFSET maxsize = 0;
	for(TPIE_OS_OFFSET i = 0; i< setting_k; i++) {
	    maxsize = maxsize + setting_k*slot_max_size(setting_k*i);
	}
	TP_LOG_DEBUG("\tQueue max size(only external): " << maxsize << "\n");
    }
    std::stringstream ss;
    ss << tpie_tempnam("AMI_PQ_DATA");
    datafiles = ss.str();
    TP_LOG_DEBUG("memory after alloc: " << MM_manager.memory_available() << "b" << "\n");
}

template <typename T, typename Comparator, typename OPQType>
priority_queue<T, Comparator, OPQType>::~priority_queue() { // destructor
    for(TPIE_OS_OFFSET i = 0; i < setting_k*setting_k; i++) { // unlink slots
	TPIE_OS_UNLINK(slot_data(i));
    }
    for(TPIE_OS_OFFSET i = 0; i < setting_k; i++) { // unlink groups 
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
	TPIE_OS_OFFSET slot = free_slot(0);
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
	    TPIE_OS_OFFSET j = 0;
	    for(TPIE_OS_OFFSET i = group_start(0); i < group_start(0)+group_size(0); i++) {
		mergebuffer[j] = gbuffer0[i%setting_m];
		++j;
	    }
	    memcpy(&mergebuffer[j], &arr[0], sizeof(T)*opq->sorted_size());
	    std::sort(mergebuffer, mergebuffer+(group_size(0)+opq->sorted_size()), comp_);
	    memcpy(&gbuffer0[0], &mergebuffer[0], sizeof(T)*group_size(0));
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
    if(m_size == 0) {
	TP_LOG_FATAL_ID("Error, queue is empty, pop");
	exit(-1);
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
    if(buffer_size == 0 && opq->size() != m_size) {
	//cout << "fill min buffer" << "\n";
	fill_buffer();
	//cout << "done fill min buffer" << "\n";
    }
    //cout << "buffer filled" << "\n";
    if(buffer_size == 0 && opq->size() == 0) {
	//    dump();
	TP_LOG_FATAL_ID("Error, queue is empty, top");
	exit(-1);
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
const TPIE_OS_OFFSET priority_queue<T, Comparator, OPQType>::size() {
    return m_size;
}

template <typename T, typename Comparator, typename OPQType>
const bool priority_queue<T, Comparator, OPQType>::empty() {
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
		  << "DUMP:\tTotal size: " << m_size << ", OPQ size: " << opq->size() 
		  << ", OPQ top: ");
    if(opq->size()>0) {
	TP_LOG_DEBUG("" << opq->top());
    } else {
	TP_LOG_DEBUG("empty");
    }
    TP_LOG_DEBUG(", current_r: " << current_r << "\n"
		 << "\tBuffer size: " << buffer_size << ", buffer start: " << buffer_start 
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
	TP_LOG_DEBUG("\tGroup Buffer, size: " << group_size(i) << ", start: " 
		     << group_start(i) << "\n" << "\t\tBuffer(no ('s): ");

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
	    TP_LOG_DEBUG("\t\tSlot " << j << "(size: " << slot_size(j) 
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
TPIE_OS_OFFSET priority_queue<T, Comparator, OPQType>::free_slot(TPIE_OS_OFFSET group) {
    //cout << "free slot group " << group << "?" << "\n";
    TPIE_OS_OFFSET i;
    if(group>=setting_k) {
	TP_LOG_FATAL_ID("Error, queue is full no free slots in invalid group " << group << ". Increase k.");
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
    for(TPIE_OS_OFFSET i=0;i<current_r;i++) {
	if(group_size(i)<setting_mmark) {
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

    pq_merge_heap<T, Comparator> heap(current_r);
    stream<T>* data[current_r];
    for(TPIE_OS_OFFSET i = 0; i<current_r; i++) {
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
	TPIE_OS_OFFSET current_group = heap.top_run();
	if(current_group!= 0 && data[current_group]->tell() == setting_m) {
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
    mergebuffer = new T[setting_m*2];
    if(mergebuffer == NULL) throw std::bad_alloc();

    for(TPIE_OS_OFFSET i = 1; i<current_r; i++) {
	delete data[i];
    }
    //cout << "end fill buffer" << "\n";
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::fill_group_buffer(TPIE_OS_OFFSET group) {
    assert(group_size(group) < setting_mmark);
    // max k + 1 open streams
    // 1 merge heap
    // opq still in action

    // merge
    {
	stream<T> out(group_data(group));
	//cout << "seek to " << (group_start(group)+group_size(group))%setting_m << "\n";
	if(group > 0) {
	    if((err = out.seek((group_start(group)+group_size(group))%setting_m))!= NO_ERROR) {
		TP_LOG_FATAL_ID("AMI error " << err << " while seeking node");
		exit(-1);
	    }
	}
	//  seek_offset(out, (group_start(group)+group_size(group))%setting_m);

	delete[] mergebuffer;

	pq_merge_heap<T, Comparator> heap(setting_k);
	stream<T>* data[setting_k];
	for(TPIE_OS_OFFSET i = 0; i<setting_k; i++) {
	    if(slot_size(group*setting_k+i)>0) {
		data[i] = new stream<T>(slot_data(group*setting_k+i));
		//      assert(slot_size(group*setting_k+i>0));
		seek_offset(data[i], slot_start(group*setting_k+i));
		heap.push(*read_item(data[i]), group*setting_k+i);
	    } else {
		// dummy, well :o/
		data[i] = new stream<T>();
		data[i]->persist(PERSIST_DELETE);
	    }
	}

	while(!heap.empty() && group_size(group)!=setting_m) {
	    TPIE_OS_OFFSET current_slot = heap.top_run();
	    if(group == 0) {
		gbuffer0[(group_start(0)+group_size(0))%setting_m] = heap.top();
	    } else {
		if(out.tell() == setting_m) {
		    //cout << "fill group seeking to 0" << "\n";
		    //      seek_offset(out, 0);
		    out.seek(0);
		}
		//    write_item(out, heap.top());
		if((err = out.write_item(heap.top())) != NO_ERROR) {
		    TP_LOG_FATAL_ID("AMI error while reading item, code: " << err);
		    exit(-1);
		}
	    }
	    group_size_set(group, group_size(group) + 1);
	    slot_start_set(current_slot, slot_start(current_slot)+1);
	    slot_size_set(current_slot, slot_size(current_slot)-1);
	    if(slot_size(current_slot) == 0) {
		heap.pop();
	    } else {
		heap.pop_and_push(*read_item(data[current_slot-group*setting_k]), current_slot);
	    }
	}

	for(TPIE_OS_OFFSET i = 0; i<setting_k; i++) {
	    delete data[i];
	}
	//  delete out;
    }

    mergebuffer = new T[setting_m*2];
    if(mergebuffer == NULL) throw std::bad_alloc();

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
void priority_queue<T, Comparator, OPQType>::compact(TPIE_OS_OFFSET slot1) {
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
void priority_queue<T, Comparator, OPQType>::empty_group(TPIE_OS_OFFSET group) {
    //cout << "Empty group " << group << "\n";
    if(group > setting_k) {
	TP_LOG_FATAL_ID("Error: Priority queue is full");
	exit(-1);
    }

    TPIE_OS_OFFSET newslot = free_slot(group+1);

    assert(slot_size(newslot) == 0);
    slot_start_set(newslot, 0);
    if(current_r < newslot/setting_k+1) {
	//cout << "increasing current_r, empty_group" << "\n"; 
	current_r = newslot/setting_k+1;
    }

    bool ret = false;

    delete[] mergebuffer;

    stream<T>* newstream = new stream<T>(slot_data(newslot));
    pq_merge_heap<T, Comparator> heap(setting_k);

    stream<T>* data[setting_k];
    for(TPIE_OS_OFFSET i = 0; i<setting_k; i++) {
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
	TPIE_OS_OFFSET current_slot = heap.top_run();
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
    for(TPIE_OS_OFFSET i = 0; i<setting_k; i++) {
	if(ret == false) {
	    delete data[i];
	} else if(ret == true && slot_size(group*setting_k+i) >0 ) {
	    delete data[i];
	}
    }
    //cout << "end delete" << "\n";

    delete newstream;

    mergebuffer = new T[setting_m*2];
    if(mergebuffer == NULL) throw std::bad_alloc();

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
void priority_queue<T, Comparator, OPQType>::remove_group_buffer(TPIE_OS_OFFSET group) {
    //cout << "remove group buffer " << group << "\n";
#ifndef NDEBUG
    if(group == 0) {
	TP_LOG_FATAL_ID("remove group buffer 0, fy!");
	exit(-1);
    }
#endif

    // this is the easiest thing to do
    TPIE_OS_OFFSET slot = free_slot(0);
    if(group_size(group) == 0) return;

    assert(group < setting_k);
    T* arr = new T[group_size(group)];
    stream<T>* data = new stream<T>(group_data(group));
    seek_offset(data, group_start(group));
    TPIE_OS_OFFSET size = group_size(group);
    if(group_start(group) + group_size(group) <= setting_m) {
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
	memcpy(&mergebuffer[j], &arr[0], sizeof(T)*group_size(group));
	std::sort(mergebuffer, mergebuffer+(group_size(0)+group_size(group)), comp_);
	memcpy(&gbuffer0[0], &mergebuffer[0], sizeof(T)*group_size(0));
	group_start_set(0,0);
	memcpy(&arr[0], &mergebuffer[group_size(0)], sizeof(T)*group_size(group));
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
const void priority_queue<T, Comparator, OPQType>::seek_offset(stream<T>* data, TPIE_OS_OFFSET offset) {
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
void priority_queue<T, Comparator, OPQType>::slot_start_set(TPIE_OS_OFFSET slot, TPIE_OS_OFFSET n) {
    slot_state[slot*3] = n;
}

template <typename T, typename Comparator, typename OPQType>
const  TPIE_OS_OFFSET priority_queue<T, Comparator, OPQType>::slot_start(TPIE_OS_OFFSET slot) {
    return slot_state[slot*3];
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::slot_size_set(TPIE_OS_OFFSET slot, TPIE_OS_OFFSET n) {
    //cout << "change slot " << slot << " size" << "\n";
    assert(slot<setting_k*setting_k);
    slot_state[slot*3+1] = n;
}

template <typename T, typename Comparator, typename OPQType>
const  TPIE_OS_OFFSET priority_queue<T, Comparator, OPQType>::slot_size(TPIE_OS_OFFSET slot) {
    return slot_state[slot*3+1];
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::group_start_set(TPIE_OS_OFFSET group, TPIE_OS_OFFSET n) {
    group_state[group*2] = n;
}

template <typename T, typename Comparator, typename OPQType>
const  TPIE_OS_OFFSET priority_queue<T, Comparator, OPQType>::group_start(TPIE_OS_OFFSET group) {
    return group_state[group*2];
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::group_size_set(TPIE_OS_OFFSET group, TPIE_OS_OFFSET n) {
    assert(group<setting_k);
    group_state[group*2+1] = n;
}

template <typename T, typename Comparator, typename OPQType>
const  TPIE_OS_OFFSET priority_queue<T, Comparator, OPQType>::group_size(TPIE_OS_OFFSET group) {
    return group_state[group*2+1];
}

template <typename T, typename Comparator, typename OPQType>
const  char* priority_queue<T, Comparator, OPQType>::datafile(TPIE_OS_OFFSET id) {
    std::stringstream ss;
    ss << datafiles << id;
    return (filename = ss.str()).c_str();
}

template <typename T, typename Comparator, typename OPQType>
const  char* priority_queue<T, Comparator, OPQType>::datafile_group(TPIE_OS_OFFSET id) {
    std::stringstream ss;
    ss << datafiles << "g" <<id;
    return (filename = ss.str()).c_str();
}

template <typename T, typename Comparator, typename OPQType>
const  char* priority_queue<T, Comparator, OPQType>::slot_data(TPIE_OS_OFFSET slotid) {
    return datafile(slot_state[slotid*3+2]);
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::slot_data_set(TPIE_OS_OFFSET slotid, TPIE_OS_OFFSET n) {
    slot_state[slotid*3+2] = n;
}

template <typename T, typename Comparator, typename OPQType>
const  char* priority_queue<T, Comparator, OPQType>::group_data(TPIE_OS_OFFSET groupid) {
    return datafile_group(groupid);
}

template <typename T, typename Comparator, typename OPQType>
TPIE_OS_OFFSET priority_queue<T, Comparator, OPQType>::slot_max_size(TPIE_OS_OFFSET slotid) {
    return setting_m*(TPIE_OS_OFFSET)(pow((long double)setting_k,(long double)(slotid/setting_k))); // todo, too many casts
}

template <typename T, typename Comparator, typename OPQType>
void priority_queue<T, Comparator, OPQType>::write_slot(TPIE_OS_OFFSET slotid, T* arr, TPIE_OS_OFFSET len) {
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
