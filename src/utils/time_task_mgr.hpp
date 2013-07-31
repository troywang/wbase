/*
 * timer_manager.h
 *
 *  Created on: Apr 17, 2013
 *      Author: wanghuan
 */

#ifndef TIMER_MANAGER_H_
#define TIMER_MANAGER_H_

#include "task_manager.hpp"

namespace wbase { namespace common { namespace utils {

class timer_task : public task
{
public:
	enum ACTION {INVALID = -1, CONTINUE, DONE};

	//call run() after timeout_ms every interval_ms unless timer_run() returns STOP
	//in subclass, cancel() should make timer_run() return STOP to prevent task from being scheduled repeatedly
	timer_task (uint32_t timeout_ms = 1000, uint32_t interval_ms = 1000)
	{
		m_timeout = timeout_ms;
		m_interval = interval_ms;
		m_action = INVALID;
	}

	virtual std::string string()
	{
		std::ostringstream oss;
		oss << task::string();
		oss << "[";
		oss << "timeout: " << m_timeout << ", ";
		oss << "interval: " << m_interval << ", ";
		oss << "action: " << m_action;
		oss << "]";
		return oss.str();
	}

					void	run() { m_action = timer_run(); }
		virtual  ACTION		timer_run() = 0;

					void	set_timeout(uint32_t timeout_ms) { m_timeout = timeout_ms; }
					void	set_interval(uint32_t interval_ms) { m_interval = interval_ms; }

				uint32_t	timeout() { return m_timeout; }
				uint32_t 	interval() { return m_interval; }
			      ACTION	action() { return m_action; }

private:
       volatile uint32_t	m_timeout;
	   volatile uint32_t	m_interval;
	              ACTION	m_action;
};

template<typename T>
class timer_scheduler : public scheduler<T>
{
public:
	typedef typename scheduler<T>::taskp taskp;
	typedef std::multimap<boost::system_time, taskp> task_map;
	typedef typename task_map::iterator task_iterator;

	bool add_task(const taskp &task);

	void get_task(/* out */taskp &task);

	bool put_task(const taskp &task);

	bool timed_get(/* out */taskp &task, uint32_t timeout_ms);

private:
	void do_add(const taskp &task, const boost::system_time &until);

private:
	boost::mutex				m_mutex;
	boost::condition_variable	m_cond;
					 task_map 	m_tasks;
};

template<typename T>
bool timer_scheduler<T>::add_task(const taskp &task)
{
	if (!task.get())
		return false;

	uint32_t timeout = task->timeout();
	boost::system_time until = boost::get_system_time() + boost::posix_time::milliseconds(timeout);
	do_add(task, until);
	return true;
}

template<typename T>
void timer_scheduler<T>::get_task(/* out */ taskp &task)
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

template<typename T>
bool timer_scheduler<T>::put_task(const taskp &task)
{
	if (task->stat() == task::CANCELED)
		return false;

	if (task->action() != timer_task::CONTINUE)
		return false;

	//schedule for next run
	uint32_t interval = task->interval();
	boost::system_time until = boost::get_system_time() + boost::posix_time::milliseconds(interval);
	do_add(task, until);
	return true;
}

template<typename T>
bool timer_scheduler<T>::timed_get(/* out */taskp &task, uint32_t timeout_ms)
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

template<typename T>
void timer_scheduler<T>::do_add(const taskp &task, const boost::system_time &until)
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

typedef task_manager<timer_task, timer_scheduler> time_task_mgr;

} } }

#endif /* TIMER_MANAGER_H_ */
