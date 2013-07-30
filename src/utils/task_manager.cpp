/*
 * thread_pool.cpp
 *
 *  Created on: Apr 11, 2013
 *      Author: wanghuan@readsns.com
 */

#include <sstream>
#include <iostream>
#include "task_manager.hpp"
using namespace std;

namespace wbase { namespace common { namespace utils {

task::task()
{
	m_status = WAITING;
	m_create = time(NULL);
	m_begin = m_end = 0;
	m_prog = 0;
}

void task::start()
{
	boost::mutex::scoped_lock lock(m_mutex);
	m_begin = time(NULL);
	m_status = RUNNING;
	lock.unlock();
	run();
	lock.lock();
	if (m_status == RUNNING) {
		m_status = COMPLETED;
		m_end = time(NULL);
		m_cond.notify_all(); //可能有多个线程同时在wait task
		lock.unlock();
		on_complete();
	}
}

void task::stop()
{
	boost::mutex::scoped_lock lock(m_mutex);
	lock.unlock();
	cancel();
	lock.lock();
	if (!is_done()) {
		m_status = CANCELED;
		m_end = time(NULL);
		m_cond.notify_all();
	} else {
		m_status = CANCELED; //m_status should always be CANCELED once stop() has been called
	}
	lock.unlock();
	on_cancel();
}

std::string task::string()
{
	std::ostringstream oss;
	oss << "[";
	oss << "create_time: " << m_create << ", ";
	oss << "begin_time: " << m_begin << ", ";
	oss << "end_time: " << m_end << ", ";
	oss << "progress: " << m_prog << ", ";
	oss << "status: " << m_status;
	oss << "]";
	return oss.str();
}

void task::wait()
{
	boost::mutex::scoped_lock lock(m_mutex);
	//condition variables can get triggered by the os for no reason
	//an indicator and a loop should be used
	while (!is_done())
		m_cond.wait(lock);
}

bool task::timed_wait(uint32_t timeout)
{
	boost::system_time until = boost::get_system_time() + boost::posix_time::milliseconds(timeout);
	return timed_wait(until);
}

bool task::timed_wait(const boost::system_time &until)
{
	boost::mutex::scoped_lock lock(m_mutex);
	if (is_done())
		return true;
	if (!m_cond.timed_wait(lock, until)) {
		return false;
	}
	return true;
}

} } }
