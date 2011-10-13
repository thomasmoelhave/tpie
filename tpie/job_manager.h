#ifndef __TPIE_JOB_MANAGER_H
#define __TPIE_JOB_MANAGER_H

#include <stddef.h>
#include <boost/thread.hpp>

namespace tpie {

class job {
public:
	inline job() : m_dependencies(0) {}
	virtual void operator()() = 0;
	virtual ~job() {}
	void join();
	void enqueue(job * parent = 0);
protected:
	virtual void on_done() {}
private:
	size_t m_dependencies;
	boost::condition_variable m_done;
	job * m_parent;
	void run();
	void done();
	friend class job_manager;
};

void init_job();
void finish_job();

} // namespace tpie

#endif
