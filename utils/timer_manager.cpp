/*
 * timer_manager.cpp
 *
 *  Created on: Apr 17, 2013
 *      Author: wanghuan
 */

#include <sstream>
#include "timer_manager.h"
namespace wbase { namespace common { namespace utils {

timer_task::timer_task(uint32_t timeout_ms, uint32_t interval_ms, int until_code)
{
	m_timeout = timeout_ms;
	m_interval = interval_ms;
	m_untilcode = until_code;
}

std::string timer_task::string()
{
	std::ostringstream oss;
	oss << task::string();
	oss << "[";
	oss << "timeout: " << m_timeout << ", ";
	oss << "interval: " << m_interval << ", ";
	oss << "untilcode: " << m_untilcode;
	oss << "]";
	return oss.str();
}

void timer_scheduler::do_add(const boost::shared_ptr<timer_task> &task, const boost::system_time &until)
{
	boost::mutex::scoped_lock lock(m_mutex);

	bool flag = false; //should notify?
	if (m_tasks.empty() || until < m_tasks.begin()->first) {
		flag = true;
	}

	m_tasks.insert(std::make_pair(until, task));

	if (flag) //this one timeout the earliest
		m_cond.notify_one();
}

void timer_scheduler::add(const taskp &task)
{
	boost::shared_ptr<timer_task> ttask = boost::dynamic_pointer_cast<timer_task>(task);
	uint32_t timeout = ttask->get_timeout();
	boost::system_time until = boost::get_system_time() + boost::posix_time::milliseconds(timeout);
	do_add(ttask, until);
}

void timer_scheduler::get(/* out */taskp &task)
{
	boost::mutex::scoped_lock lock(m_mutex);
	while (true)
	{
		boost::system_time now = boost::get_system_time();
		task_iterator it = m_tasks.upper_bound(now);
		if (it != m_tasks.begin()) { // has expired task
			task = m_tasks.begin()->second;
			break;
		} else if (m_tasks.empty()){ // has no expired task
			m_cond.wait(lock); // no task came in until timeout
		} else { // wait for the first one to timeout
			m_cond.timed_wait(lock, m_tasks.begin()->first);
		}
	}

	//one task expired
	m_tasks.erase(m_tasks.begin());
}

bool timer_scheduler::timed_get(/* out */taskp &task, uint32_t timeout_ms)
{
	boost::mutex::scoped_lock lock(m_mutex);
	boost::system_time until = boost::get_system_time() + boost::posix_time::milliseconds(timeout_ms);

	while (true)
	{
		boost::system_time now = boost::get_system_time();
		task_iterator it = m_tasks.upper_bound(now);
		if (it != m_tasks.begin()) { // has expired task
			task = m_tasks.begin()->second;
			break;
		} else if (m_tasks.empty()){ // has no expired task
			if (!m_cond.timed_wait(lock, until)) // no task came in until timeout
				return false;
		} else {
			boost::system_time now = boost::get_system_time();
			if (now > until)
				return false;
			// wait for the first one to timeout
			// a new task expired earlier may come in at this point
			// wake up and loop again
			boost::system_time least = m_tasks.begin()->first;
			m_cond.timed_wait(lock, std::min(least, until));
		}
	}

	//one task expired
	m_tasks.erase(m_tasks.begin());
	return true;
}

void timer_scheduler::put(const taskp &task)
{
	if (task->stat() == task::CANCELED)
		return;

	boost::shared_ptr<timer_task> ttask = boost::dynamic_pointer_cast<timer_task>(task);
	if (task->code() == ttask->get_untilcode())
		return;

	//schedule for next run
	uint32_t interval = ttask->get_interval();
	boost::system_time until = boost::get_system_time() + boost::posix_time::milliseconds(interval);
	do_add(ttask, until);
}


timer_manager::timer_manager(uint32_t min, uint32_t max, uint32_t idle) : m_task_mgr(min, max, idle,
		boost::shared_ptr<scheduler>(new timer_scheduler()))
{
}

void timer_manager::add(const boost::shared_ptr<timer_task> &task)
{
	m_task_mgr.add(task);
}

void timer_manager::stop(const boost::shared_ptr<timer_task> &task)
{
	m_task_mgr.stop(task);
}

} } }



