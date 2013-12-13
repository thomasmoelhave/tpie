// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2013, The TPIE development team
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

#ifndef __TPIE_BLOCKING_QUEUE_H__
#define __TPIE_BLOCKING_QUEUE_H__

////////////////////////////////////////////////////////////////////////////////
/// \file blocking_queue.h
/// \brief Blocking queue based on std::queue and boost
////////////////////////////////////////////////////////////////////////////////
#include <queue>
#include <boost/thread/lockable_concepts.hpp> 

namespace tpie {

namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \brief a basic implementation of a blocked queue based on std::queue
///////////////////////////////////////////////////////////////////////////////
template<class T>
class blocking_queue {
public:
    bool empty() const {
        boost::mutex::scoped_lock lock(m_mutex);
        return m_queue.empty();
    }

    size_t size() const {
        boost::mutex::scoped_lock lock(m_mutex);
        return m_queue.size();
    }

    T & front() {
        boost::mutex::scoped_lock lock(m_mutex);
        while(m_queue.empty())
            m_condition.wait(lock);
        return m_queue.front();
    }

    const T & front() const {
        boost::mutex::scoped_lock lock(m_mutex);
        while(m_queue.empty())
            m_condition.wait(lock);
        return m_queue.front();
    }

    void push(const T & item) {
        boost::mutex::scoped_lock lock(m_mutex);
        m_queue.push(item);

        lock.unlock(); // avoid problems if a waiting thread immediately wakes up
        m_condition.notify_one();
    }

    T pop() {
        boost::mutex::scoped_lock lock(m_mutex);
        while(m_queue.empty())
            m_condition.wait(lock);
        T item = m_queue.front();
        m_queue.pop();
        return item;
    }
private:
    std::queue<T> m_queue;
    mutable boost::mutex m_mutex;
    mutable boost::condition_variable m_condition;
};

} // bits namespace

} //  tpie namespace

#endif // __TPIE_BLOCKING_QUEUE_H__
