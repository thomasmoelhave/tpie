#define BTE_COLLECTION_IMP_MMAP
#define BTE_STREAM_IMP_UFS
#include <tpie/config.h>
#include <tpie/stream.h>
#include <tpie/mm.h>
#include <tpie/sort.h>
#include <tpie/priority_queue.h>
#include <tpie/progress_indicator_arrow.h>

#include <boost/cstdint.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/random.hpp>

#include <iostream>
#include <limits>

namespace pt = boost::posix_time;
typedef boost::int64_t itype;

struct cmp{
	int compare(itype a, itype b){
		if(a < b)
			return -1;
		else if (a == b)
			return 0;
		return 1;
	}
};

boost::mt19937 rng(1337);
boost::uniform_int<itype> uniform(std::numeric_limits<itype>::min(), std::numeric_limits<itype>::max());
boost::uniform_smallint<> smuniform(0, 4);
boost::variate_generator<boost::mt19937&, boost::uniform_int<itype> > gen(rng, uniform);
boost::variate_generator<boost::mt19937&, boost::uniform_smallint<> > smgen(rng, smuniform);

int main(){
	const itype large = itype(10000)*1024*1024/sizeof(itype);
	const TPIE_OS_SIZE_T MEM = 1024*1024*1024;

	tpie::MM_manager.set_memory_limit(MEM);

	{
		tpie::ami::stream<itype> file;

		tpie::progress_indicator_arrow writing("Writing", "Writing to temporary file", 0, large, 1);
		pt::ptime write_begin(pt::microsec_clock::local_time());
		for(itype i = 0; i < large; i++){
			file.write_item(gen());
			writing.step();
		}
		pt::ptime write_end(pt::microsec_clock::local_time());
		writing.done("Done");

		std::cout << "Time spent: " << pt::to_simple_string(write_end - write_begin) << std::endl;

// 		tpie::progress_indicator_arrow reading("Reading", "Reading from temporary file", 0, large, 1);
// 		file.seek(0);
// 		pt::ptime read_begin(pt::microsec_clock::local_time());
// 		for(itype i=0; i < large; i++){
// 			itype *ptr;
// 			file.read_item(&ptr);
// 			reading.step();
// 		}
// 		pt::ptime read_end(pt::microsec_clock::local_time());
// 		reading.done("Done");

//		std::cout << "Time spent: " << pt::to_simple_string(read_end - read_begin) << std::endl;
		
		tpie::progress_indicator_arrow sorting("Sorting", "Sorting the temporary file", 0, large, 1);
		
		file.seek(0);	
		cmp comp;
		
		pt::ptime sort_begin(pt::microsec_clock::local_time());
		tpie::ami::sort(&file, &comp, &sorting);
		pt::ptime sort_end(pt::microsec_clock::local_time());
		
		std::cout << "Time spent: " << pt::to_simple_string(sort_end - sort_begin) << std::endl;
	}
	
// 	{
// 		const itype pushs = large/2;
// 		tpie::progress_indicator_arrow queueing("Queueing", "Pushing to priority queue", 0, pushs, 1);
// 		tpie::ami::priority_queue<itype> pq;
// 		pt::ptime queue_begin(pt::microsec_clock::local_time());
// 		for(itype i=0; i < pushs; i++){
// 			if(!pq.empty()){
// 				pq.top();
// 				pq.pop();
// 			}
// 			int count = smgen();
// 			for(int j=0; j < count; j++)
// 				pq.push(gen());
// 			queueing.step();
// 		}
// 		pt::ptime queue_end(pt::microsec_clock::local_time());
// 		queueing.done("Done");
// 		std::cout << "Time spent: " << pt::to_simple_string(queue_end - queue_begin) << std::endl;
		
// 		tpie::progress_indicator_arrow popping("Popping", "Popping from priority queue", 0, pq.size(), 1);
// 		pt::ptime pop_begin(pt::microsec_clock::local_time());
// 		while(!pq.empty()){
// 			pq.top();
// 			pq.pop();
// 			popping.step();
// 		}
// 		pt::ptime pop_end(pt::microsec_clock::local_time());
// 		popping.done("Done");
// 		std::cout << "Time spent: " << pt::to_simple_string(pop_end - pop_begin) << std::endl;
// 	}
	
	return 0;
}
