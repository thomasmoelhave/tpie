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

#include <tpie/job.h>
#include <tpie/array.h>
#include <tpie/internal_queue.h>
#include <tpie/exception.h>

namespace tpie {
 
class job_manager * the_job_manager = 0;

class job_manager {
public:
	job_manager() : m_jobs(128), m_done(false) {}
	~job_manager() {
		boost::mutex::scoped_lock lock(jobs_mutex);
		m_done = true;
		m_has_data.notify_all();
		lock.unlock();
		for (size_t i = 0; i < m_thread_pool.size(); ++i) {
			m_thread_pool[i].join();
		}
	}
	void init_pool(size_t threads) {
		m_thread_pool.resize(threads);
		for (size_t i = 0; i < threads; ++i) {
			boost::thread t(worker);
			// thread is move-constructable
			m_thread_pool[i].swap(t);
		}
	}
	boost::mutex jobs_mutex;
private:
	tpie::internal_queue<tpie::job *> m_jobs;
	boost::condition_variable m_has_data;
	tpie::array<boost::thread> m_thread_pool;
	bool m_done;
	static void worker() {
		for (;;) {
			boost::mutex::scoped_lock lock(the_job_manager->jobs_mutex);
			while (the_job_manager->m_jobs.empty() && !the_job_manager->m_done) the_job_manager->m_has_data.wait(lock);
			if (the_job_manager->m_done) break;
			tpie::job * j = the_job_manager->m_jobs.front();
			the_job_manager->m_jobs.pop();
			lock.unlock();
			j->run();
		}
	};
	friend class tpie::job;
};

void init_job() {
	the_job_manager = tpie_new<job_manager>();
	the_job_manager->init_pool(boost::thread::hardware_concurrency());
}

void finish_job() {
	tpie_delete(the_job_manager);
	the_job_manager = 0;
}

void job::join() {
	boost::mutex::scoped_lock lock(the_job_manager->jobs_mutex);
	while (m_dependencies) {
		m_done.wait(lock);
	}
}

void job::enqueue(job * parent) {
	boost::mutex::scoped_lock lock(the_job_manager->jobs_mutex);
	if (the_job_manager->m_done) throw job_manager_exception();
	m_parent = parent;
	++m_dependencies;
	if (m_parent) ++m_parent->m_dependencies;
	if (the_job_manager->m_jobs.full()) {
		lock.unlock();
		run();
		return;
	}
	the_job_manager->m_jobs.push(this);
	the_job_manager->m_has_data.notify_one();
}

void job::run() {
	(*this)();
	done();
}

void job::done() {
	boost::mutex::scoped_lock lock(the_job_manager->jobs_mutex);
	--m_dependencies;
	if (m_parent) m_parent->done();
	if (m_dependencies) return;
	m_done.notify_all();
	on_done();
}

} // namespace tpie

