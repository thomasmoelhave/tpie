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

#ifndef __TPIE_PARALLEL_SORT_H__
#define __TPIE_PARALLEL_SORT_H__

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/cstdint.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <cmath>
#include <functional>
#include <tpie/progress_indicator_base.h>
namespace tpie {
	
template <typename iterator_type, typename comp_type=
		  std::less<typename boost::iterator_value<iterator_type>::type>,
		  size_t min_size=1024*1024*8/sizeof(typename boost::iterator_value<iterator_type>::type)>
class parallel_sort_impl {
private:
	// The type of the values we sort
	typedef typename boost::iterator_value<iterator_type>::type value_type;
	
	// Guistimate how much work a sort uses
	static inline boost::uint64_t sortWork(boost::uint64_t n) {return log(n)*n*1.9/log(2);}
	
	// Partition acording to pivot
	template <typename comp_t>
	static inline iterator_type unguarded_partition(iterator_type first, 
													iterator_type last, 
													value_type pivot,
													comp_t & comp) {
		while (true) {
			while (comp(*first, pivot)) ++first;
			--last;
			while (comp(pivot, *last)) --last;
			if (!(first < last)) return first;
			std::iter_swap(first, last);
			++first;
		}
	}

	// Pick a good element for partitioning
	static inline value_type pick_pivot(iterator_type a, iterator_type b) {
		const size_t z=5;
		iterator_type sample[z];
		sample[0] = a + (b-a)/2;
		for(size_t i=1; i < z; ++i) {sample[i] = a+random() % (b-a);}
		for(size_t i=0; i < z/2; ++i) {
			size_t z=i;
			for(size_t j=i+1; j < z; ++j) 
				if (*sample[j] < *sample[z]) z=j;
			std::swap(sample[i], sample[z]);
		}
		return *sample[z/2];
	}

	// Partition the array
	inline std::pair<iterator_type, iterator_type> partition(iterator_type a, iterator_type b, comp_type & comp) {
		value_type pivot = pick_pivot(a, b);
		iterator_type l = unguarded_partition(a, b, pivot, comp);
		return std::make_pair(l, l);
	}
	
	// Wrap up a thread, to join it when it terminates
	struct thread_holder {
		boost::thread thread;
		~thread_holder() {thread.join();}
	};

	// A struct that sets kill to true and notifies the condition when destructed
	struct notify_on_exit {
		boost::condition_variable & cond;
		bool & kill;
		notify_on_exit(boost::condition_variable & _, bool & __): cond(_), kill(__) {};
		~notify_on_exit() {kill=true; cond.notify_all();}
	};

	void worker(comp_type comp) {
		boost::mutex::scoped_lock lock(mutex);
		while (true) {
			while (!kill && job_count == 0) {
				if (working == 0) return;
				cond.wait(lock);
			}
			if (kill) return;
			++working;
			std::pair<iterator_type, iterator_type> job = jobs[--job_count];
			while ((job.second - job.first) >= min_size) {
				lock.unlock();
				std::pair<iterator_type, iterator_type> r = partition(job.first, job.second, comp);
				lock.lock();
				work_estimate += job.second - job.first;
				jobs[job_count++] = std::make_pair(r.second, job.second);
				cond.notify_all();
				job.second = r.first;
			}
			lock.unlock();
			std::sort(job.first, job.second, comp);
			lock.lock();
			work_estimate += sortWork(job.second-job.first);
			--working;
			cond.notify_all();
		}
	}
	static void worker_(parallel_sort_impl * self, comp_type & comp) {self->worker(comp);}
public:
	parallel_sort_impl(progress_indicator_base * p): pi(p) {}

	void operator()(iterator_type a, iterator_type b, comp_type comp=std::less<value_type>() ) {
		const size_t tc=boost::thread::hardware_concurrency();
		thread_holder threads[tc];
		boost::mutex::scoped_lock lock(mutex);
		boost::uint64_t n = b-a;
		working = 0;
		kill = false;
		if (pi) {
			pi->set_min_range(0);
			pi->set_max_range(sortWork(b-a));
		}
		job_count = 0;
		jobs[job_count++] = std::make_pair(a,b);
		for(size_t i=0; i < tc; ++i) {
			boost::thread t(&worker_, this, comp);
			threads[i].thread.swap(t);
		}
		notify_on_exit nox(cond, kill);
		while(working != 0 || job_count != 0) {
			cond.wait(lock);
			if (work_estimate > 0) {
				if (pi) pi->step(work_estimate);
				work_estimate = 0;
			}
		}
		if (pi)	pi->done();
	}
private:
	progress_indicator_base * pi;
	boost::uint64_t work_estimate;
	boost::uint64_t total_work_estimate;
	bool kill;
	boost::condition_variable cond;
	boost::mutex mutex;
	size_t working;

	//Magic constant 128 must be larger then log(n) - log(min_size) + #cores 
	std::pair<iterator_type, iterator_type> jobs[128];
	size_t job_count;
};
	
template <typename iterator_type, typename comp_type>
void parallel_sort(iterator_type a, 
				   iterator_type b, 
				   comp_type comp=std::less<typename boost::iterator_value<iterator_type>::type>(),
				   progress_indicator_base * pi=0) {
	parallel_sort_impl<iterator_type, comp_type> s(pi);
	s(a,b,comp);
}

}
#endif //__TPIE_PARALLEL_SORT_H__
