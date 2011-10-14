// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2011, The TPIE development team
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
#include <boost/random/mersenne_twister.hpp>
#include <cmath>
#include <functional>
#include <tpie/progress_indicator_base.h>
#include <tpie/dummy_progress.h>
#include <tpie/internal_queue.h>
#include <tpie/job.h>
namespace tpie {

template <typename iterator_type, typename comp_type,
		  size_t min_size=1024*1024*8/sizeof(typename boost::iterator_value<iterator_type>::type)>
class parallel_sort_impl {
private:
	boost::mt19937 rng;	

	// The type of the values we sort
	typedef typename boost::iterator_value<iterator_type>::type value_type;

	// Guistimate how much work a sort uses
	static inline boost::uint64_t sortWork(boost::uint64_t n) {return 
			static_cast<uint64_t>(
				log(static_cast<double>(n))*n*1.9/log(static_cast<double>(2)));}
	
	// Partition acording to pivot
	template <typename comp_t>
	static inline iterator_type unguarded_partition(iterator_type first, 
													iterator_type last, 
													comp_t & comp) {
		iterator_type pivot = first;
		while (true) {
			do --last;
			while (comp(*pivot, *last));

			do {
				if (first == last) break;
				++first;
			} while (comp(*first, *pivot));

			if (first == last) break;

			std::iter_swap(first, last);
		}
		std::iter_swap(last, pivot);
		return last;
	}

	static inline iterator_type median(iterator_type a, iterator_type b, iterator_type c, comp_type & comp) {
		if (comp(*a, *b)) {
			if (comp(*b, *c)) return b;
			else if (comp(*a, *c)) return c;
			else return a;
		} else {
			if (comp(*a, *c)) return a;
			else if (comp(*b, *c)) return c;
			else return b;
		}
	}

	// Pick a good element for partitioning
	static inline iterator_type pick_pivot(iterator_type a, iterator_type b, comp_type & comp) {
		if (a == b) return a;
		assert(a < b);
		size_t step = (b-a)/8;
		return median(median(a+0, a+step, a+step*2, comp),
					  median(a+step*3, a+step*4, a+step*5, comp),
					  median(a+step*6, a+step*7, b-1, comp), comp);
	}

	// Partition the array and return the pivot
	static inline iterator_type partition(iterator_type a, iterator_type b, comp_type & comp) {
		iterator_type pivot = pick_pivot(a, b, comp);

		std::iter_swap(pivot, a);
		iterator_type l = unguarded_partition(a, b, comp);

		return l;
	}

	static void progress(ptrdiff_t /*amount*/) {
		//assert(amount >= 0);
		// should this only be executed by the master thread?
		//#pragma omp critical
		//{
		//	if (pi) pi->step(amount);
		//}
	}

	static inline void qsort(iterator_type a, iterator_type b, comp_type comp) {
		assert(a <= b);
		assert(&*a != 0);
		while (static_cast<size_t>(b - a) >= min_size) {
			progress(70);
			iterator_type pivot = partition(a, b, comp);
			progress(b - a - 70);
			qsort_job * j = tpie_new<qsort_job>(a, pivot, comp);
			j->enqueue();
			a = pivot+1;
		}
		std::sort(a, b, comp);
		progress(sortWork(b - a));
	}

	class qsort_job : public job {
	public:
		qsort_job(iterator_type a, iterator_type b, comp_type comp) : a(a), b(b), comp(comp) {}
		virtual void operator()() {
			qsort(a, b, comp);
		}
	protected:
		virtual void on_done() {
			tpie_delete(this);
		}
	private:
		iterator_type a;
		iterator_type b;
		comp_type comp;
	};
public:
	parallel_sort_impl(progress_indicator_base * p): pi(p) {}

	void operator()(iterator_type a, iterator_type b, comp_type comp=std::less<value_type>() ) {
		work_estimate = 0;
		if (pi) pi->init(sortWork(b-a));
		qsort_job * master = tpie_new<qsort_job>(a, b, comp);
		master->enqueue();
		master->join();
		if (pi)	pi->done();
	}
private:
	static const size_t max_job_count=256;
	progress_indicator_base * pi;
	boost::uint64_t work_estimate;
	boost::uint64_t total_work_estimate;
	bool kill;
	boost::condition_variable cond;
	boost::mutex mutex;
	size_t working;

	std::pair<iterator_type, iterator_type> jobs[max_job_count];
	size_t job_count;
};

template <bool Progress, typename iterator_type, typename comp_type>
void parallel_sort(iterator_type a, 
				   iterator_type b, 
				   typename tpie::progress_types<Progress>::base & pi,
				   comp_type comp=std::less<typename boost::iterator_value<iterator_type>::type>()) {
	parallel_sort_impl<iterator_type, comp_type> s(&pi);
	s(a,b,comp);
}

template <typename iterator_type, typename comp_type>
void parallel_sort(iterator_type a, 
				   iterator_type b, 
				   comp_type comp=std::less<typename boost::iterator_value<iterator_type>::type>()) {
	parallel_sort_impl<iterator_type, comp_type> s(0);
	s(a,b,comp);
}


}
#endif //__TPIE_PARALLEL_SORT_H__
